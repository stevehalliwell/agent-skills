import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtemp, mkdir, writeFile, readFile, readdir, rm } from 'node:fs/promises';
import { resolve, join } from 'node:path';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
const executable=process.env.GD_NATIVE_EXE && resolve(process.env.GD_NATIVE_EXE);
function invoke(arguments_,status=0,environment={}) {
  const result=spawnSync(executable,['query',...arguments_],{encoding:'utf8',timeout:30000,maxBuffer:32*1024*1024,windowsHide:true,env:{...process.env,GD_AGENT:'1',...environment}});
  assert.equal(result.status,status,result.stderr+'\n'+result.stdout.slice(0,2000));
  return result;
}
function run(arguments_,status=0) { const result=invoke(arguments_,status); const value=JSON.parse(result.stdout); assert.equal(value.ok,status===0); return value; }
async function fixture(t) {
  const root=await mkdtemp(join(tmpdir(),'gd-state-chart-')); t.after(()=>rm(root,{recursive:true,force:true}));
  for (const role of ['nouns','axes','mechanics']) await mkdir(join(root,role));
  const save=(role,name,body)=>writeFile(join(root,role,name+'.md'),body,'utf8');
  await save('axes','State','---\nvalue_type: enum\nallowed_values: [ready, spent, unused]\nclosed: true\n---\n');
  await save('axes','Life','---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: false\n---\n');
  await save('axes','Count','---\nvalue_type: integer\nunit: item\n---\n');
  await save('nouns','unit','---\nid: unit-fixture\naxes: [axes/State, axes/Life, axes/Count]\n---\n');
  await save('nouns','other','---\naxes: [axes/Life]\n---\n');
  await save('mechanics','use',`---
status: accepted
---
\`\`\`gd-rule
case guarded:
    target: A
    context: ANY
    unknown_conditions: External timing remains unspecified.
    A:
        noun: nouns/unit
        requires: State.ready AND Life.alive
        skip_if: Life.dead
        other_axes: ANY
        becomes: State.spent
    B:
        noun: nouns/unit
        requires: ANY
        becomes: Life.dead
case preserve:
    target: A
    context: ANY
    A:
        noun: nouns/unit
        requires: State.ready
        other_axes: ANY
        preserve: State
case unspecified:
    target: A
    context: ANY
    unknown_effects: Direction reversal magnitude unresolved.
    A:
        noun: nouns/unit
        requires: State.ready
        other_axes: ANY
case pair:
    target: A
    context: ANY
    A:
        noun: nouns/unit
        requires: State.ready
        other_axes: ANY
        becomes: State.spent
    B:
        noun: nouns/unit
        requires: State.spent
        other_axes: ANY
        becomes: State.ready
case caller:
    target: A
    context: ANY
    A:
        noun: nouns/unit
        requires: State.ready
        other_axes: ANY
        causes: mechanics/helper#write with Victim=A, Other=B
    B:
        noun: nouns/unit
        requires: ANY
case excluded:
    target: A
    context: ANY
    A:
        noun: nouns/unit
        requires: ANY
        skip_if: State.ready
        other_axes: ANY
        becomes: State.spent
\`\`\`
`);
  await save('mechanics','helper',`---\nstatus: proposed\n---\n\`\`\`gd-rule
case write:
    target: Victim
    context: ANY
    Victim:
        noun: nouns/unit
        requires: ANY
        becomes: State.spent
    Other:
        noun: nouns/unit
        requires: ANY
        becomes: Life.dead
\`\`\`\n`);
  const inputs=['nouns','axes','mechanics'].flatMap(role=>['--'+role,`${role}=${join(root,role)}`]);
  return {root,save,inputs,base:['--question','state-chart','--projection','actors',...inputs,'--object','nouns/unit','--axis','axes/State']};
}
async function snapshot(directory) {
  const result=[];
  for (const entry of (await readdir(directory,{withFileTypes:true})).sort((a,b)=>a.name.localeCompare(b.name))) {
    const path=join(directory,entry.name);
    if (entry.isDirectory()) result.push(...await snapshot(path));
    else result.push([path,createHash('sha256').update(await readFile(path)).digest('hex')]);
  }
  return result;
}
test('state chart retains one-axis transitions, same-noun identities, guards, side effects and mapped calls',{skip:!executable},async t=>{
  const {root,base}=await fixture(t); const before=await snapshot(root); const result=run(base);
  assert.equal(result.question,'state-chart'); assert.deepEqual(result.data.axis_definition.allowed_values,['ready','spent','unused']);
  const scenarios=result.data.scenarios;
  const get=(name,actor='A',from='ready')=>scenarios.find(row=>row.source.case===name && row.actor.participant===actor && row.from===from);
  const guarded=get('guarded'); assert.equal(guarded.applicability,'unknown');
  assert.deepEqual(guarded.transitions.map(row=>[row.kind,row.to]),[['state','spent']]);
  assert.ok(guarded.proposals.some(row=>row.affected.participant==='B' && row.proposal.axis==='axes/Life' && row.proposal.state==='dead'));
  const declaration=result.data.cases.find(row=>row.index===guarded.source.snapshot_case_index).declaration;
  assert.equal(declaration.context,'ANY'); assert.match(declaration.unknown_conditions,/timing/);
  assert.equal(declaration.participants[0].requires,'State.ready AND Life.alive'); assert.equal(declaration.participants[0].skip_if,'Life.dead');
  assert.equal(get('guarded','A','spent').applicability,'false'); assert.deepEqual(get('guarded','A','spent').transitions,[]);
  assert.deepEqual(get('preserve').transitions.map(row=>[row.kind,row.to]),[['preserve','ready']]);
  assert.equal(get('unspecified').axis_result,'unspecified'); assert.deepEqual(get('unspecified').transitions,[]);
  assert.equal(get('excluded').applicability,'false'); assert.deepEqual(get('excluded').transitions,[]);
  assert.deepEqual(get('pair').transitions.map(row=>row.to),['spent']);
  assert.deepEqual(get('pair','B','spent').transitions.map(row=>row.to),['ready']);
  assert.notEqual(get('pair').actor.snapshot_participant_index,get('pair','B','spent').actor.snapshot_participant_index);
  const caller=get('caller'); assert.deepEqual(caller.transitions.map(row=>row.to),['spent']);
  assert.equal(caller.proposals[caller.transitions[0].proposal_index].proposal.origin.record,'mechanics/helper');
  assert.ok(caller.activations.some(row=>row.via.length===2 && row.via[1].call_text.includes('Victim=A')));
  assert.ok(result.data.cases.some(row=>row.declaration.case==='write' && row.declaration.status==='proposed'));
  for (const scenario of scenarios) for (const row of scenario.proposals) {
    assert.ok(scenario.activations[row.activation]); assert.ok(row.proposal.origin.source_path); assert.ok(row.proposal.line>0);
  }
  const native=run(['--question','actor-states','--projection','actors',...base.slice(4,-4),'--case','mechanics/use#caller','--participant','A','--axis','axes/State']);
  for (const variant of native.data.variants) {
    const chart=get('caller','A',variant.value);
    assert.equal(chart.applicability,variant.applicability);
    assert.deepEqual(chart.proposals.map(row=>row.proposal),variant.proposals.map(row=>row.proposal));
  }
  const accepted=run([...base,'--status','accepted']);
  assert.ok(accepted.data.scenarios.every(row=>row.source.status==='accepted'));
  assert.ok(accepted.data.cases.some(row=>row.declaration.status==='proposed'));
  assert.equal(result.coverage.state_chart.gameplay_roles_inferred,false); assert.equal(result.coverage.state_chart.combined_state_reachability_evaluated,false);
  assert.equal(result.coverage.final_outcomes_proven,false); assert.deepEqual(await snapshot(root),before);
});
test('state chart discovery, errors, field selection, empty and exhausted scope stay distinct',{skip:!executable},async t=>{
  const {base,inputs}=await fixture(t);
  const question=run(['--list']).data.questions.find(row=>row.name==='state-chart');
  assert.deepEqual(question.required_parameters,['--object','--axis']); assert.equal(question.ui_available,true); assert.equal(question.ui_mode,'state-charts'); assert.equal(question.ipc_available,true); assert.equal(question.ipc_form,'chart');
  assert.match(invoke(['--list','--format','text']).stdout,/state-chart \[actors\]/);
  assert.deepEqual(Object.keys(run([...base,'--fields','scenarios']).data),['scenarios']);
  assert.match(invoke([...base,'--format','text']).stdout,/No selected-axis proposal retained; preservation is not implied/);
  assert.equal(JSON.parse(invoke([...base,'--json'],0,{GD_AGENT:'0'}).stdout).question,'state-chart');
  for (const extra of [['--to','x'],['--effect','state'],['--assume','A:axes/Life=alive','--case','mechanics/use#guarded'],['--participant','A']])
    assert.equal(run([...base,...extra],2).errors.at(-1).code,'query.parameters');
  assert.equal(run([...base.slice(0,3),'effects',...base.slice(4)],2).errors[0].code,'query.projection_unavailable');
  assert.equal(run([...base,'--fields','state_bindings'],2).errors[0].code,'cli.arguments');
  assert.equal(run(['--question','state-chart','--projection','actors',...inputs,'--axis','axes/State'],2).errors.at(-1).code,'query.parameters');
  assert.equal(run([...base.slice(0,-2)],2).errors.at(-1).code,'query.parameters');
  assert.equal(run([...base.slice(0,-2),'--axis','axes/Count'],2).errors.at(-1).code,'chart.axis_type');
  assert.equal(run([...base.slice(0,-4),'--object','nouns/other','--axis','axes/State'],2).errors.at(-1).code,'query.axis');
  assert.equal(run([...base,'--value','absent'],2).errors.at(-1).code,'coverage.value');
  assert.equal(run([...base,'--value','ready','--value','ready'],2).errors.at(-1).code,'coverage.duplicate_value');
  const empty=run([...base,'--status','absent']); assert.deepEqual(empty.data.scenarios,[]); assert.equal(empty.coverage.limited,false);
  const limited=run([...base,'--work-limit','0']); assert.deepEqual(limited.data.scenarios,[]); assert.equal(limited.coverage.limited,true); assert.equal(limited.coverage.complete,false);
  assert.ok(limited.data.state_values.every(row=>row.no_retained_candidate===null));
});
test('joint chart assumptions stay on exact local binding and agree with native actor variants',{skip:!executable},async t=>{
  const {root,base,inputs}=await fixture(t); const before=await snapshot(root);
  const result=run([...base,'--with-axis','axes/Life']);
  assert.equal(result.selectors.companion_axis,'axes/Life');
  assert.deepEqual(Object.keys(run([...base,'--with-axis','axes/Life','--fields','companion_axis_definition']).data),['companion_axis_definition']);
  assert.deepEqual(result.data.companion_axis_definition.allowed_values,['alive','dead']);
  assert.equal(result.coverage.state_chart.unretained_hypotheses,0);
  assert.equal(result.data.scenarios.length,run(base).data.scenarios.length*2);
  for (const life of ['alive','dead']) {
    const actor=run(['--question','actor-states','--projection','actors',...inputs,'--case','mechanics/use#guarded','--participant','A','--axis','axes/State','--assume',`A:axes/Life=${life}`]);
    for (const variant of actor.data.variants) {
      const scenario=result.data.scenarios.find(row=>row.source.case==='guarded' && row.actor.participant==='A' && row.from===variant.value && row.companion_value===life);
      assert.equal(scenario.applicability,variant.applicability);
      assert.deepEqual(scenario.proposals.map(row=>row.proposal),variant.proposals.map(row=>row.proposal));
    }
  }
  const dead=result.data.scenarios.find(row=>row.source.case==='guarded' && row.actor.participant==='A' && row.from==='ready' && row.companion_value==='dead');
  assert.equal(dead.applicability,'false');
  const peer=result.data.scenarios.find(row=>row.source.case==='guarded' && row.actor.participant==='B' && row.from==='ready' && row.companion_value==='alive');
  assert.equal(peer.applicability,'unknown'); // A's Life remains unknown; no same-noun alias.
  assert.ok(peer.transitions.some(row=>row.axis==='axes/Life' && row.from==='alive' && row.to==='dead'));
  assert.equal(result.coverage.state_chart.combined_state_reachability_evaluated,false);
  assert.deepEqual(await snapshot(root),before);
});
test('joint chart rejects wrong axes and keeps zero-work, filtered and partial scopes explicit',{skip:!executable},async t=>{
  const {base,inputs,save}=await fixture(t);
  for (const axis of ['axes/State','axes/Count','axes/missing']) assert.equal(run([...base,'--with-axis',axis],2).errors.at(-1).code,'chart.companion_axis');
  await save('axes','Unbound','---\nvalue_type: enum\nallowed_values: [one, two]\n---\n');
  assert.equal(run([...base,'--with-axis','axes/Unbound'],2).errors.at(-1).code,'chart.companion_axis');
  assert.equal(run(['--list','--with-axis','axes/Life'],2).ok,false);
  assert.equal(run(['--question','object-bindings','--projection','actors',...inputs,'--with-axis','axes/Life'],2).ok,false);
  const limited=run([...base,'--with-axis','axes/Life','--work-limit','0']);
  assert.equal(limited.coverage.limited,true);assert.deepEqual(limited.data.scenarios,[]);
  const empty=run([...base,'--with-axis','axes/Life','--status','absent']);assert.deepEqual(empty.data.scenarios,[]);assert.equal(empty.coverage.limited,false);
  const bounded=run([...base,'--with-axis','axes/Life','--work-limit','1500']);
  assert.equal(bounded.coverage.limited,true);assert.ok(bounded.coverage.state_chart.unretained_hypotheses>0);
  assert.ok(bounded.data.state_values.every(row=>row.no_retained_candidate!==true));
});
test('state chart bounds retained scenario prefix separately from matching and unexamined hypotheses',{skip:!executable},async t=>{
  const {base,save}=await fixture(t);
  await save('axes','State','---\nvalue_type: enum\nallowed_values: [ready, spent]\nclosed: true\n---\n');
  await save('nouns','unit','---\naxes: [axes/State]\n---\n');
  await save('mechanics','use','```gd-rule\n'+Array.from({length:2049},(_,i)=>`case c${i}:\n    target: A\n    context: ANY\n    A:\n        noun: nouns/unit\n        requires: ANY\n        other_axes: ANY\n`).join('')+'```\n');
  await save('mechanics','helper','');
  const result=run(base); assert.equal(result.coverage.state_chart.retention_limited,true); assert.equal(result.coverage.limited,true);
  assert.equal(result.data.scenarios.length,4096); assert.equal(result.coverage.state_chart.retained_activations,4096);
  assert.equal(result.coverage.state_chart.storage_limited,true); assert.equal(result.coverage.state_chart.retention_work_limited,false);
  assert.equal(result.coverage.state_chart.unretained_hypotheses,2);
  assert.ok(result.data.state_values.some(row=>row.unfinished_bindings>0));
  assert.equal(result.coverage.state_query_complete,false); assert.equal(result.coverage.state_chart.retained_proposals,0);
});
test('joint chart retains existing scenario cap without dropping the companion dimension from omission counts',{skip:!executable},async t=>{
  const {base,save}=await fixture(t);
  await save('axes','State','---\nvalue_type: enum\nallowed_values: [ready, spent]\nclosed: true\n---\n');
  await save('mechanics','use','```gd-rule\n'+Array.from({length:1025},(_,i)=>`case c${i}:\n    target: A\n    context: ANY\n    A:\n        noun: nouns/unit\n        requires: ANY\n        other_axes: ANY\n`).join('')+'```\n');
  await save('mechanics','helper','');
  const result=run([...base,'--with-axis','axes/Life']);
  assert.equal(result.data.scenarios.length,4096);assert.equal(result.coverage.state_chart.storage_limited,true);
  assert.equal(result.coverage.state_chart.retained_binding_hypotheses,4100);
  assert.equal(result.coverage.state_chart.unretained_hypotheses,4);assert.equal(result.coverage.state_query_complete,false);
  assert.ok(result.data.state_values.every(row=>row.unfinished_bindings>0));
});
test('state chart works on shipped Shellback and Crate axes without authored source changes',{skip:!executable},async()=>{
  for (const [example,noun,axis] of [['little-plumber-guy-nes','shellback','ShellState'],['crate-puzzle','switch','Occupancy']]) {
    const root=resolve('skills/game-design-workbench/examples',example,'game-design'); const before=await snapshot(root);
    const inputs=['nouns','axes','interactions','mechanics','verbs','resources','control-vars'].flatMap(role=>['--'+role,`${role}=${join(root,role)}`]);
    const result=run(['--question','state-chart','--projection','actors',...inputs,'--object','nouns/'+noun,'--axis','axes/'+axis]);
    assert.ok(result.data.scenarios.length>0); assert.equal(result.coverage.state_chart.retention_limited,false);
    if (noun==='shellback') {
      const kick=result.data.scenarios.find(row=>row.source.qualified==='mechanics/shell-travel#kick-resting' && row.from==='resting');
      assert.ok(kick.transitions.some(row=>row.to==='sliding'));
      const stop=result.data.scenarios.find(row=>row.source.qualified==='mechanics/shell-travel#descending-stop' && row.from==='sliding');
      assert.ok(stop.transitions.some(row=>row.to==='resting'));
      assert.ok(stop.proposals.some(row=>row.affected.noun==='nouns/hero' && row.proposal.state==='rising'));
      const unknown=result.data.cases.find(row=>row.declaration.case==='two-shell-unknown'); assert.match(unknown.declaration.unknown_effects,/unresolved/);
    }
    assert.deepEqual(await snapshot(root),before);
  }
});
