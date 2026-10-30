import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtemp, mkdir, writeFile, readFile, rm } from 'node:fs/promises';
import { resolve, join } from 'node:path';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
const executable=process.env.GD_NATIVE_EXE && resolve(process.env.GD_NATIVE_EXE);
function invoke(arguments_,status=0,environment={}) {
  const result=spawnSync(executable,['query',...arguments_],{encoding:'utf8',timeout:30000,maxBuffer:32*1024*1024,windowsHide:true,env:{...process.env,GD_AGENT:'1',...environment}});
  assert.equal(result.status,status,result.stderr+'\n'+result.stdout);
  return result;
}
function run(arguments_,status=0) { const result=invoke(arguments_,status); const value=JSON.parse(result.stdout); assert.equal(value.ok,status===0); return value; }
test('built-in catalogue is explicit, machine-readable and rejects unsupported projection/parameters',{skip:!executable},()=>{
  assert.match(invoke(['--help']).stdout,/gd query --list/);
  const catalogue=run(['--list']);
  assert.deepEqual(catalogue.data.questions.map(question=>question.name),['object-bindings','actor-states','effect-contributors']);
  assert.ok(catalogue.data.questions.every(question=>question.ui_available===false && question.required_bindings.any_of.includes('mechanics')));
  assert.deepEqual(catalogue.data.questions[1].required_parameters,['--case','--axis']);
  assert.equal(catalogue.coverage.source_data_loaded,false);
  assert.deepEqual(run(['--list','--json','--fields','questions']),catalogue);
  assert.match(invoke(['--list','--format','text']).stdout,/actor-states \[actors\]/);
  for (const [question,projection] of [['object-bindings','effects'],['actor-states','effects'],['effect-contributors','actors']]) {
    const value=run(['--question',question,'--projection',projection],2); assert.equal(value.errors[0].code,'query.projection_unavailable');
  }
  assert.equal(run(['--list','--object','nouns/unit'],2).errors[0].code,'cli.arguments');
  assert.equal(run(['--question','not-built-in','--projection','actors'],2).errors[0].code,'cli.arguments');
});
test('headless actor queries retain local identities, finite variants, whole guards and typed proposals',{skip:!executable},async()=>{
  const root=await mkdtemp(join(tmpdir(),'gd-query-states-'));
  try {
    for(const role of ['nouns','axes','mechanics']) await mkdir(join(root,role));
    const files=[];
    const save=async(role,name,body)=>{ const path=join(root,role,name+'.md'); files.push(path); await writeFile(path,body); };
    await save('axes','Form','---\nvalue_type: enum\nallowed_values: [small, big]\nclosed: true\n---\n');
    await save('axes','Life','---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: true\n---\n');
    await save('axes','Points','---\nvalue_type: integer\nunit: point\nminimum: 0\nmaximum: 10\n---\n');
    await save('nouns','unit','---\naxes: [axes/Form, axes/Life, axes/Points]\n---\n');
    await save('mechanics','caller',`---\nid: caller-fixture\nstatus: accepted\n---\n\`\`\`gd-rule
case root:
    target: A
    context: ANY
    A:
        noun: nouns/unit
        requires: ANY
        causes: mechanics/helper#small with Victim=A, Other=B; mechanics/helper#big with Victim=A, Other=B
    B:
        noun: nouns/unit
        requires: ANY
\`\`\`\n`);
    await save('mechanics','helper',`---\nid: helper-fixture\nstatus: proposed\n---\n\`\`\`gd-rule
case small:
    target: Victim
    context: ANY
    Victim:
        noun: nouns/unit
        requires: Form.small
        other_axes: ANY
        becomes: Life.dead
    Other:
        noun: nouns/unit
        requires: ANY
case big:
    target: Victim
    context: ANY
    Victim:
        noun: nouns/unit
        requires: Form.big
        other_axes: ANY
        set:
            Points: min(this + 1[point], 10[point])
    Other:
        noun: nouns/unit
        requires: ANY
        becomes: Life.dead
\`\`\`\n`);
    const hashes=await Promise.all(files.map(async path=>createHash('sha256').update(await readFile(path)).digest('hex')));
    const inputs=['nouns','axes','mechanics'].flatMap(role=>['--'+role,`${role}=${join(root,role)}`]);
    const inventory=['--question','object-bindings','--projection','actors',...inputs];
    const bindings=run(inventory); assert.equal(bindings.data.bindings.length,6);
    assert.equal(bindings.data.bindings.filter(binding=>binding.primary).length,3);
    assert.ok(bindings.data.bindings.every(binding=>binding.source.record_id && binding.axes.length===3));
    const points=bindings.data.bindings[0].axes.find(axis=>axis.reference==='axes/Points');
    assert.equal(points.unit,'point'); assert.equal(points.minimum,'0'); assert.equal(points.maximum,'10');
    assert.equal(run([...inventory,'--status','accepted']).data.bindings.length,2);
    assert.deepEqual(Object.keys(run([...inventory,'--fields','bindings']).data),['bindings']);
    const states=['--question','actor-states','--projection','actors',...inputs,'--case','mechanics/caller#root','--axis','axes/Form','--assume','A:axes/Points=9'];
    const value=run(states); assert.equal(value.selectors.participant,'A'); assert.equal(value.selectors.fixed_facts[0].value,'9');
    assert.deepEqual(value.data.variants.map(variant=>variant.value),['small','big']);
    assert.ok(value.data.variants.every(variant=>variant.activations.length===3 && variant.activations.filter(activation=>activation.applicability==='false').length===1));
    assert.deepEqual(value.data.variants.map(variant=>variant.proposals.length),[1,2]);
    assert.equal(value.data.variants[1].proposals.find(proposal=>proposal.family==='gain').proposal.number,10);
    assert.equal(value.data.variants[1].proposals.find(proposal=>proposal.family==='gain').declared_unit,'point');
    assert.ok(value.data.variants[1].proposals.some(proposal=>proposal.proposal.actor.participant==='B'));
    assert.ok(value.data.variants[0].activations.some(activation=>activation.via.length===2 && activation.via[1].call_text.includes('Victim=A')));
    const comparison=value.data.comparisons[0];
    assert.equal(comparison.effects.shared,1); assert.equal(value.coverage.final_outcomes_proven,false);
    const sharedEffect=comparison.effects.shared_members[0];
    assert.ok(value.data.variants.every(variant=>variant.effect_concepts.some(effect=>effect.index===sharedEffect && effect.kind==='state' && effect.value==='dead')));
    assert.ok(value.data.variants.every(variant=>variant.partners.some(partner=>partner.participant==='B' && comparison.partners.shared_members.includes(partner.snapshot_participant_index))));
    assert.equal(value.data.cases.find(item=>item.index===comparison.cases.first_only_members[0]).declaration.case,'small');
    assert.equal(value.data.cases.find(item=>item.index===comparison.cases.second_only_members[0]).declaration.case,'big');
    const accepted=run([...states,'--status','accepted']); assert.equal(accepted.data.variants.length,2);
    assert.ok(accepted.data.variants.flatMap(variant=>variant.proposals).every(proposal=>proposal.proposal.origin.status==='proposed'));
    const other=run([...states,'--participant','B']); assert.equal(other.selectors.participant,'B');
    assert.ok(other.data.variants.every(variant=>variant.proposals.length===3 && variant.activations.filter(activation=>activation.applicability==='unknown').length===2));
    const numeric=run(['--question','actor-states','--projection','actors',...inputs,'--case','mechanics/caller#root','--axis','axes/Points','--assume','A:axes/Form=big','--value','9','--value','10']);
    assert.deepEqual(numeric.data.variants.map(variant=>variant.proposals.find(proposal=>proposal.proposal.kind==='quantity').family),['gain','no_change']);
    assert.equal(run([...states,'--work-limit','0']).coverage.variant_limited,true);
    assert.equal(run([...states,'--assume','A:axes/Form=small'],2).errors.at(-1).code,'actors.fixed_axis');
    assert.equal(run([...states,'--value','absent'],2).errors.at(-1).code,'rules.scenario');
    assert.equal(run([...states,'--participant','missing'],2).errors.at(-1).code,'query.binding');
    assert.equal(run([...states,'--fields','concepts'],2).errors.at(-1).code,'cli.arguments');
    assert.match(invoke([...states,'--format','text']).stdout,/Value small: applicability true/);
    const contributors=['--question','effect-contributors','--projection','effects',...inputs,'--axis','axes/Life','--value','dead'];
    const all=run(contributors); assert.equal(all.data.concepts.length,1); assert.equal(all.data.concepts[0].incoming,4); assert.equal(all.data.contributors.length,4);
    const scoped=run([...contributors,'--case','mechanics/caller#root']); assert.equal(scoped.data.contributors.length,2);
    assert.ok(scoped.data.contributors.every(contributor=>contributor.via.length===2 && contributor.affected.noun==='nouns/unit'));
    assert.ok(scoped.data.cases.some(item=>item.declaration.participants.some(participant=>participant.requires==='Form.small')));
    assert.deepEqual(new Set(scoped.data.contributors.map(contributor=>contributor.affected.participant)),new Set(['A','B']));
    assert.equal(run([...contributors,'--status','accepted']).data.concepts[0].incoming,0);
    const exact=run(['--question','effect-contributors','--projection','effects',...inputs,'--to',all.data.concepts[0].id]); assert.equal(exact.data.contributors.length,4);
    assert.equal(run([...contributors,'--work-limit','0']).coverage.contributor_limited,true);
    assert.equal(run([...contributors,'--effect','gain'],2).errors.at(-1).code,'query.effect_type');
    assert.equal(run(['--question','effect-contributors','--projection','effects',...inputs,'--to','missing'],2).errors.at(-1).code,'query.selector_missing');
    const after=await Promise.all(files.map(async path=>createHash('sha256').update(await readFile(path)).digest('hex'))); assert.deepEqual(after,hashes);
  } finally { await rm(root,{recursive:true,force:true}); }
});
test('empty query inventory succeeds and source-status exclusions differ from exhausted work',{skip:!executable},async()=>{
  const root=await mkdtemp(join(tmpdir(),'gd-query-empty-'));
  try {
    const inputs=['--mechanics',`mechanics=${root}`];
    const base=['--question','object-bindings','--projection','actors',...inputs];
    assert.deepEqual(run(base).data.bindings,[]); assert.equal(run(base).coverage.complete,true);
    assert.equal(run([...base,'--work-limit','0']).coverage.limited,false);
    assert.equal(run([...base,'--status','all','--status','accepted'],2).errors[0].code,'query.status');
    const effects=run(['--question','effect-contributors','--projection','effects',...inputs]); assert.deepEqual(effects.data.concepts,[]); assert.deepEqual(effects.data.contributors,[]);
  } finally { await rm(root,{recursive:true,force:true}); }
});
