import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtemp, mkdir, writeFile, rm } from 'node:fs/promises';
import { resolve, join } from 'node:path';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';
const executable=process.env.GD_NATIVE_EXE && resolve(process.env.GD_NATIVE_EXE);
function run(root,extra=[]) {
  const args=['nouns','axes','mechanics'].flatMap(role=>['--'+role,`${role}=${join(root,role)}`]);
  const result=spawnSync(executable,['resources',...args,...extra],{encoding:'utf8',timeout:30000,maxBuffer:32*1024*1024,windowsHide:true,env:{...process.env,GD_AGENT:'1'}});
  assert.equal(result.status,0,result.stderr+'\n'+result.stdout); return JSON.parse(result.stdout);
}
async function fixture() {
  const root=await mkdtemp(join(tmpdir(),'gd-placement-identity-'));
  for(const role of ['nouns','axes','mechanics']) await mkdir(join(root,role));
  await writeFile(join(root,'axes/Points.md'),'---\nvalue_type: integer\nunit: point\n---\n');
  await writeFile(join(root,'axes/Mood.md'),'---\nvalue_type: enum\nallowed_values: [calm]\nclosed: false\n---\n');
  await writeFile(join(root,'nouns/bank.md'),'---\naxes: [axes/Mood, axes/Points]\n---\n');
  return root;
}
test('unresolved child/ancestor declarations with equal labels and offsets retain distinct IDs/selectors',{skip:!executable},async()=>{
  const root=await fixture();
  try {
    await writeFile(join(root,'mechanics/root.md'),'---\nstatus: accepted\n---\n```gd-rule\ncase run:\n    target: A\n    context: ANY\n    A:\n        noun: nouns/bank\n        requires: mystery_root\n        causes: mechanics/helper#run with A=A\n```\n');
    await writeFile(join(root,'mechanics/helper.md'),'---\nstatus: accepted\n---\n```gd-rule\ncase run:\n    target: A\n    context: ANY\n    A:\n        noun: nouns/bank\n        requires: mystery_helper\n        set:\n            Points: this + 1[point]\n```\n');
    const value=run(root), instance=value.data.nodes.find(n=>n.instantiated);
    assert.equal(new Set(value.data.nodes.map(n=>n.id)).size,value.data.nodes.length);
    const rows=value.data.relations.filter(r=>r.activation===instance.id && r.kind==='requirement');
    assert.equal(rows.length,2); assert.equal(new Set(rows.map(r=>r.facet)).size,2);
    assert.deepEqual(new Set(rows.map(r=>r.source.record)),new Set(['mechanics/root','mechanics/helper']));
    const output=value.data.relations.find(r=>r.activation===instance.id && r.kind==='quantity_write').facet;
    for(const row of rows) {
      const selected=run(root,['--from',row.facet,'--to',output]).data.routes;
      assert.equal(selected.selected_from,row.facet); assert.equal(selected.selector_matches,0);
      assert.ok(selected.witnesses.length>0 && selected.witnesses.every(w=>w.unresolved));
    }
  } finally { await rm(root,{recursive:true,force:true}); }
});
test('whole-template calls with more than 64 selected-case bindings resolve every concrete actor',{skip:!executable},async()=>{
  const root=await fixture();
  try {
    const body=Array.from({length:65},(_,i)=>`case variant${i}:\n    target: Balance\n    context: ANY\n    Balance:\n        noun: nouns/bank\n        requires: ANY\n        set:\n            Points: this + 1[point]\n`).join('');
    await writeFile(join(root,'mechanics/template.md'),'---\nstatus: accepted\n---\n```gd-rule\n'+body+'```\n');
    await writeFile(join(root,'mechanics/caller.md'),'---\nstatus: accepted\n---\n```gd-rule\ncase run:\n    target: Bank\n    context: ANY\n    Bank:\n        noun: nouns/bank\n        requires: ANY\n        causes: mechanics/template with Balance=Bank\n```\n');
    const value=run(root), instances=value.data.nodes.filter(n=>n.instantiated);
    assert.equal(instances.length,65); assert.equal(value.coverage.graph_limited,false);
    assert.ok(instances.every(n=>n.instantiation.concrete_primary_target==='Bank' && n.instantiation.participants[0].concrete_participant==='Bank'));
    const ids=new Set(instances.map(n=>n.id));
    const writes=value.data.relations.filter(r=>ids.has(r.activation) && r.kind==='quantity_write');
    assert.equal(writes.length,65); assert.ok(writes.every(r=>r.concrete_participant==='Bank'));
    assert.equal(new Set(writes.map(r=>r.facet)).size,1); // Same bound actor/axis, not 65 mutable credits.
  } finally { await rm(root,{recursive:true,force:true}); }
});
