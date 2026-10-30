import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtemp, mkdir, writeFile, readFile, rm } from 'node:fs/promises';
import { join, resolve } from 'node:path';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
const executable=process.env.GD_NATIVE_EXE && resolve(process.env.GD_NATIVE_EXE);
function invoke(arguments_,status=0,environment={}) {
  const result=spawnSync(executable,['query',...arguments_],{encoding:'utf8',timeout:30000,maxBuffer:32*1024*1024,windowsHide:true,env:{...process.env,GD_AGENT:'1',...environment}});
  assert.equal(result.status,status,result.stderr+'\n'+result.stdout); return result;
}
function run(arguments_,status=0) { return JSON.parse(invoke(arguments_,status).stdout); }
async function fixture(t) {
  const root=await mkdtemp(join(tmpdir(),'gd-state-coverage-')); t.after(()=>rm(root,{recursive:true,force:true}));
  for (const role of ['nouns','axes','mechanics']) await mkdir(join(root,role));
  const files=[];
  const save=async(role,name,text)=>{ const path=join(root,role,name+'.md'); await writeFile(path,text); files.push(path); };
  await save('axes','Form','---\nid: form-axis\nvalue_type: enum\nallowed_values: [small, big, unused]\nclosed: true\n---\n');
  await save('axes','Points','---\nvalue_type: integer\nunit: point\nminimum: 0\nmaximum: 10\n---\n');
  await save('nouns','unit','---\nid: unit-noun\naxes: [axes/Form, axes/Points]\n---\n');
  await save('mechanics','small',`---\nid: small-root\nstatus: accepted\n---\n\`\`\`gd-rule
case small:
    target: A
    context: ANY
    A:
        noun: nouns/unit
        requires: Form.small
        other_axes: ANY
        becomes: Form.unused
\`\`\`\n`);
  await save('mechanics','pair',`---\nid: pair-root\nstatus: proposed\n---\n\`\`\`gd-rule
case pair:
    target: B
    context: ANY
    A:
        noun: nouns/unit
        requires: Form.big
        other_axes: ANY
    B:
        noun: nouns/unit
        requires: Form.big
        other_axes: ANY
\`\`\`\n`);
  const inputs=['nouns','axes','mechanics'].flatMap(role=>['--'+role,`${role}=${join(root,role)}`]);
  const base=['--question','state-coverage','--projection','actors',...inputs,'--axis','axes/Form'];
  const hashes=()=>Promise.all(files.map(async path=>createHash('sha256').update(await readFile(path)).digest('hex')));
  return {root,files,save,inputs,base,hashes};
}
test('state coverage retains independent local hypotheses, whole cases, declared unused values and typed provenance',{skip:!executable},async t=>{
  const {base,hashes}=await fixture(t); const before=await hashes(); const value=run(base);
  assert.equal(value.ok,true); assert.equal(value.coverage.complete,true); assert.equal(value.coverage.state_source_incomplete,false);
  assert.deepEqual(value.data.state_values.map(row=>row.value),['small','big','unused']);
  assert.equal(value.data.axis_definition.record_id,'form-axis'); assert.equal(value.data.axis_definition.value_type,'enum');
  const [small,big,unused]=value.data.state_values;
  assert.equal(small.definite_bindings,1); assert.equal(small.definite_cases,1); assert.equal(small.excluded_cases,1);
  assert.equal(big.undetermined_bindings,2); assert.equal(big.undetermined_cases,1); assert.equal(big.definite_cases,0);
  assert.equal(unused.excluded_bindings,3); assert.equal(unused.excluded_cases,2); assert.equal(unused.no_retained_candidate,true);
  assert.ok(value.data.state_values.every(row=>row.declared && !row.unfinished_bindings));
  assert.equal(small.no_retained_candidate,false); assert.equal(big.no_retained_candidate,false);
  assert.equal(value.data.state_bindings.length,3); assert.ok(value.data.state_bindings.every(binding=>binding.source.record_id && binding.actor.noun_record_id==='unit-noun'));
  for (const binding of value.data.state_bindings) {
    const expected=run(['--question','actor-states',...base.slice(2),'--case',binding.source.qualified,'--participant',binding.actor.participant]);
    assert.deepEqual(binding.values.map(item=>item.applicability),expected.data.variants.map(item=>item.applicability));
  }
  assert.deepEqual(Object.keys(run([...base,'--fields','state_values']).data),['state_values']);
  assert.match(invoke([...base,'--format','text']).stdout,/Value unused: 0 definite cases; 0 undetermined cases; 2 excluded cases/);
  assert.deepEqual(run([...base,'--object','nouns/unit']).data.state_values,value.data.state_values);
  const accepted=run([...base,'--status','accepted']); assert.equal(accepted.data.state_bindings.length,1); assert.equal(accepted.data.state_values[1].no_retained_candidate,true);
  const local=run([...base,'--case','mechanics/pair#pair','--participant','B']); assert.equal(local.data.state_bindings.length,1); assert.equal(local.data.state_values[1].undetermined_cases,1);
  assert.deepEqual(await hashes(),before);
});
test('state coverage distinguishes empty scope, exhausted work, invalid selectors and finite numeric samples',{skip:!executable},async t=>{
  const {base,inputs,save}=await fixture(t);
  const catalogue=run(['--list']).data.questions.find(question=>question.name==='state-coverage');
  assert.equal(catalogue.ui_available,false); assert.equal(catalogue.ipc_available,false); assert.deepEqual(catalogue.required_parameters,['--axis']);
  for (const limit of ['0','1024']) {
    const value=run([...base,'--work-limit',limit]); assert.equal(value.coverage.complete,false); assert.equal(value.coverage.limited,true);
    assert.ok(value.data.state_values.every(row=>row.no_retained_candidate===null && row.definite_cases===0 && row.undetermined_cases===0));
    assert.ok(value.coverage.inventory_work+value.coverage.state_coverage_work<=Number(limit));
  }
  const empty=run([...base,'--status','absent']); assert.equal(empty.coverage.complete,true); assert.deepEqual(empty.data.state_bindings,[]); assert.ok(empty.data.state_values.every(row=>row.no_retained_candidate===true));
  assert.equal(run([...base,'--case','mechanics/small#small','--participant','TYPO'],2).errors.at(-1).code,'query.binding');
  assert.equal(run([...base,'--case','mechanics/small#small','--participant','TYPO','--status','absent'],2).errors.at(-1).code,'query.binding');
  const noNouns=inputs.filter((item,index)=>index!==0 && index!==1);
  assert.equal(run(['--question','state-coverage','--projection','actors',...noNouns,'--axis','axes/Form'],2).errors[0].code,'cli.arguments');
  await save('axes','Other','---\nvalue_type: enum\nallowed_values: [x]\nclosed: true\n---\n');
  assert.equal(run(['--question','state-coverage','--projection','actors',...inputs,'--axis','axes/Other','--case','mechanics/small#small','--participant','A'],2).errors.at(-1).code,'query.axis');
  await save('axes','Unspecified','---\nvalue_type: unspecified\n---\n');
  assert.equal(run(['--question','state-coverage','--projection','actors',...inputs,'--axis','axes/Unspecified','--value','1'],2).errors.at(-1).code,'coverage.axis_type');
  for (const arguments_ of [['--participant','A'],['--assume','A:axes/Form=small'],['--to','opaque'],['--effect','state']]) assert.equal(run([...base,...arguments_],2).ok,false);
  for (const arguments_ of [['--value','missing'],['--value','small','--value','small']]) assert.equal(run([...base,...arguments_],2).ok,false);
  assert.equal(run([...base,'--fields','variants'],2).ok,false);
  const numeric=['--question','state-coverage','--projection','actors',...inputs,'--axis','axes/Points'];
  assert.equal(run(numeric,2).errors.at(-1).code,'coverage.values');
  const samples=run([...numeric,'--value','9','--value','10']); assert.deepEqual(samples.data.state_values.map(row=>row.value),['9','10']); assert.ok(samples.data.state_values.every(row=>!row.declared));
  assert.equal(samples.data.axis_definition.unit,'point'); assert.equal(samples.data.axis_definition.minimum,'0'); assert.equal(samples.data.axis_definition.maximum,'10');
  assert.equal(run([...numeric,'--value','1.0']).data.state_values[0].value,'1.0'); // Native integer parser accepts exact integral decimals.
  for (const arguments_ of [['--value','1.5'],['--value','9[point]'],['--value','9','--value','9.0']]) assert.equal(run([...numeric,...arguments_],2).ok,false);
  assert.equal(JSON.parse(invoke(base,0,{GD_AGENT:'1'}).stdout).question,'state-coverage');
});
test('state coverage preserves draft/source incompleteness and the 64-value mask boundary',{skip:!executable},async t=>{
  const {base,save,inputs}=await fixture(t);
  await save('mechanics','draft','---\nstatus: draft\n---\n```gd-rule\n```\n');
  const incomplete=run(base); assert.equal(incomplete.coverage.state_source_incomplete,true); assert.equal(incomplete.data.state_values[2].no_retained_candidate,null);
  await save('mechanics','empty-case','---\nstatus: accepted\n---\n```gd-rule\ncase empty:\n    context: ANY\n```\n');
  const emptyCase=run([...base,'--case','mechanics/empty-case#empty']); assert.equal(emptyCase.coverage.state_source_incomplete,true); assert.ok(emptyCase.data.state_values.every(row=>row.no_retained_candidate===null));
  const values=Array.from({length:64},(_,i)=>'v'+i);
  await save('axes','Many',`---\nvalue_type: enum\nallowed_values: [${values.join(', ')}]\nclosed: true\n---\n`);
  await save('nouns','many','---\naxes: [axes/Many]\n---\n');
  await save('mechanics','many',`---\nstatus: accepted\n---\n\`\`\`gd-rule\ncase all:\n    target: A\n    context: ANY\n    A:\n        noun: nouns/many\n        requires: ANY\n\`\`\`\n`);
  const many=run(['--question','state-coverage','--projection','actors',...inputs,'--axis','axes/Many','--case','mechanics/many#all']);
  assert.equal(many.data.state_values.length,64); assert.equal(many.data.state_bindings[0].values.length,64);
  assert.ok(many.data.state_values.every(row=>row.definite_cases===1 && row.no_retained_candidate===false && !row.unfinished_bindings));
  assert.equal(many.data.state_bindings[0].values[63].applicability,'true');
  await save('axes','Many',`---\nvalue_type: enum\nallowed_values: [${[...values,'v64'].join(', ')}]\nclosed: true\n---\n`);
  const over=['--question','state-coverage','--projection','actors',...inputs,'--axis','axes/Many','--case','mechanics/many#all'];
  assert.equal(run(over,2).errors.at(-1).code,'rules.axis_value_limit'); // Native reader's bound takes precedence; explicit sampling cannot repair invalid source.
});
