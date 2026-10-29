// Native Resource incidence report. No JS evaluator, UI automation or source migration.
import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtemp, mkdir, writeFile, readFile, rm, cp } from 'node:fs/promises';
import { resolve, join, dirname } from 'node:path';
import { tmpdir } from 'node:os';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';
const executable=process.env.GD_NATIVE_EXE && resolve(process.env.GD_NATIVE_EXE);
const home=resolve(dirname(fileURLToPath(import.meta.url)),'..');
function invoke(args, status=0) {
  const result=spawnSync(executable,['resources',...args],{encoding:'utf8',timeout:30000,maxBuffer:16*1024*1024,windowsHide:true,env:{...process.env,GD_AGENT:'1'}});
  assert.equal(result.status,status,result.stderr+'\n'+result.stdout);
  return result;
}
function report(args) {
  const result=invoke(args); assert.equal(result.stderr,'');
  const value=JSON.parse(result.stdout); assert.equal(value.ok,true); assert.equal(value.coverage.complete,false);
  assert.equal(value.coverage.operation,'resources'); return value;
}
function bindings(folder) {
  return ['mechanics','nouns','axes','resources'].flatMap(role=>['--'+role,`${role}=${join(folder,role)}`]);
}
function endpoints(value) {
  const ids=new Set(value.data.nodes.map(node=>node.id)); assert.equal(ids.size,value.data.nodes.length);
  for (const relation of value.data.relations) {
    assert.ok(ids.has(relation.activation),relation.activation); assert.ok(ids.has(relation.facet),relation.facet);
    assert.equal('from' in relation,false); assert.equal('to' in relation,false); // Incidence, not production direction.
  }
  return ids;
}
test('resource report exposes live retained declarations without implying routes or mutating source',{skip:!executable},async()=>{
  for (const example of ['crate-puzzle','little-plumber-guy-nes']) {
    const folder=join(home,'examples',example,'game-design');
    const roles=['interactions','mechanics','nouns','verbs','axes','resources','control-vars','systems'];
    const args=roles.flatMap(role=>['--'+role,`${role}=${join(folder,role)}`]);
    const sentinel=join(folder,'mechanics',example==='crate-puzzle' ? 'crate-pushing.md':'enemy-contact.md');
    const before=await readFile(sentinel);
    const value=report(args); endpoints(value);
    assert.ok(value.data.nodes.length>0 && value.data.relations.length>0);
    assert.equal(value.data.routes.available,false); assert.match(value.data.routes.reason,/not a no-path result/);
    assert.equal(value.coverage.composed_outcomes_available,false);
    assert.ok(value.data.relations.some(row=>row.kind==='quantity_write'));
    const selected=report([...args,'--fields','routes']); assert.deepEqual(Object.keys(selected.data),['routes']);
    const text=invoke([...args,'--format','text']).stdout; assert.match(text,/Direct declarations only/); assert.match(text,/composed outcomes unavailable/);
    assert.deepEqual(await readFile(sentinel),before);
  }
});
test('resource IDs, guards, quantities and unresolved occurrences retain distinct source identity',{skip:!executable},async()=>{
  const root=await mkdtemp(join(tmpdir(),'gd-resource-report-'));
  try {
    for (const role of ['mechanics','nouns','axes','resources']) await mkdir(join(root,role));
    const save=(role,name,body)=>writeFile(join(root,role,name+'.md'),body);
    await save('axes','Amount','---\nvalue_type: integer\nunit: coin\n---\n');
    await save('axes','Gold','---\nvalue_type: integer\nunit: gold\n---\n');
    await save('nouns','wallet','---\naxes: [axes/Amount, axes/Gold]\n---\n');
    await save('nouns','uncertain','---\naxes: [axes/Missing]\n---\n');
    await save('resources','money','---\nnoun: nouns/wallet\n---\n');
    await save('mechanics','trade',`---\nstatus: proposed\n---\n\`\`\`gd-rule
case exchange:
    context: ANY
    Wallet:
        noun: nouns/wallet
        pools:
            Amount: resources/money
        requires: Amount >= 100[coin] OR Amount >= 200[coin]
        set:
            Amount: this - 100[coin]
        causes: mechanics/grant
case unresolved:
    context: ANY
    Wallet:
        noun: nouns/uncertain
        requires: Missing >= 1[coin] OR Missing >= 2[coin]
        set:
            Missing: this + 1[coin]
\`\`\`\n`);
    await save('mechanics','grant',`---\nstatus: accepted\n---\n\`\`\`gd-rule
case award:
    context: ANY
    Wallet:
        noun: nouns/wallet
        requires: ANY
        set:
            Gold: this + 1[gold]
case zero:
    context: ANY
    Wallet:
        noun: nouns/wallet
        requires: ANY
        set:
            Gold: this + 0[gold]
case capped:
    context: ANY
    Wallet:
        noun: nouns/wallet
        requires: ANY
        set:
            Gold: min(this + 1[gold], 3[gold])
\`\`\`\n`);
    const value=report(bindings(root)); const ids=endpoints(value);
    const exchange=value.data.nodes.find(row=>row.kind==='case' && row.declaration.name==='exchange');
    assert.match(exchange.declaration.participants[0].requires,/ OR /); assert.equal(exchange.source.status,'proposed');
    const debit=value.data.relations.find(row=>row.kind==='quantity_write' && row.expression==='this - 100[coin]');
    assert.equal(debit.delta.recognized,true); assert.equal(debit.delta.known,true); assert.equal(debit.delta.number,-100);
    assert.equal(debit.target.axis,'axes/Amount');
    const call=value.data.relations.find(row=>row.kind==='invocation'); assert.ok(call);
    assert.equal(call.meaning,'guarded_case_reference_not_flattened'); assert.deepEqual(call.bindings,[{callee:'Wallet',caller:'Wallet'}]);
    assert.equal(value.data.relations.filter(row=>row.kind==='invocation').length,3);
    assert.ok(value.data.relations.filter(row=>row.kind==='invocation').every(row=>row.bindings.length===1));
    assert.equal(value.data.relations.find(row=>row.expression==='this + 0[gold]').delta.number,0);
    const capped=value.data.relations.find(row=>row.expression==='min(this + 1[gold], 3[gold])');
    assert.equal(capped.delta.recognized,false); assert.equal(capped.delta.number,null);
    assert.equal(value.coverage.routes_available,false); assert.ok(value.coverage.unresolved_facets>=3);
    const unresolved=value.data.nodes.filter(row=>row.identity_scope==='unresolved_declaration_occurrence');
    assert.equal(new Set(unresolved.map(row=>row.id)).size,unresolved.length);
    const unknown=value.data.relations.find(row=>row.kind==='quantity_write' && row.declared_axis==='Missing');
    assert.equal(unknown.delta.known,false); assert.equal(unknown.delta.number,null); assert.equal(unknown.target.axis,null);
    const pool=value.data.nodes.find(row=>row.kind==='resource_role' && row.reference==='resources/money');
    await rm(join(root,'resources/money.md'));
    const missing=report(bindings(root)); endpoints(missing);
    const missingPool=missing.data.nodes.find(row=>row.kind==='resource_role' && row.reference==='resources/money');
    assert.equal(missingPool.resolution,'missing'); assert.equal(missingPool.id,pool.id);
    const unloaded=report(['mechanics','nouns','axes'].flatMap(role=>['--'+role,`${role}=${join(root,role)}`]));
    const unloadedPool=unloaded.data.nodes.find(row=>row.kind==='resource_role' && row.reference==='resources/money');
    assert.equal(unloadedPool.resolution,'unloaded'); assert.equal(unloadedPool.id,pool.id);
    await save('resources','money','---\nnoun: nouns/wallet\n---\n');
    // Earlier source changes compacted record/case/token indices, not existing identities.
    await save('mechanics','aaa','```gd-rule\ncase first:\n    context: ANY\n```\n');
    const shifted=report(bindings(root)); const shiftedIds=endpoints(shifted);
    for (const id of ids) assert.ok(shiftedIds.has(id),id);
    // Copy root: IDs use qualified identities, not machine paths.
    const moved=join(root,'moved'); await mkdir(moved);
    for (const role of ['mechanics','nouns','axes','resources']) await cp(join(root,role),join(moved,role),{recursive:true});
    assert.deepEqual(new Set(report(bindings(moved)).data.nodes.map(row=>row.id)),shiftedIds);
    // Same authored name in distinct source files cannot silently collapse IDs.
    await save('mechanics','duplicate-a','---\nname: duplicate\n---\n```gd-rule\ncase marker:\n    context: ANY\n```\n');
    await save('mechanics','duplicate-b','---\nname: duplicate\n---\n```gd-rule\ncase marker:\n    context: ANY\n```\n');
    const duplicates=JSON.parse(invoke(bindings(root),2).stdout);
    assert.equal(duplicates.ok,false); assert.equal(duplicates.errors[0].code,'input.identity');
  } finally { await rm(root,{recursive:true,force:true}); }
});
test('resource CLI validates unsupported scope and empty designs without pretending no path',{skip:!executable},async()=>{
  const root=await mkdtemp(join(tmpdir(),'gd-resource-empty-'));
  try {
    const args=['--mechanics',`mechanics=${root}`]; const empty=report(args);
    assert.deepEqual(empty.data.nodes,[]); assert.deepEqual(empty.data.relations,[]); assert.equal(empty.data.routes.available,false);
    for (const extra of [['--fields','cases'],['--analyze'],['--case','mechanics/a#b'],['--from','some-node']]) {
      const value=JSON.parse(invoke([...args,...extra],2).stdout); assert.equal(value.ok,false); assert.equal(value.errors[0].code,'cli.arguments');
    }
    await writeFile(join(root,'broken.md'),'---\nname: [unterminated\n---\n');
    const bad=JSON.parse(invoke(args,2).stdout); assert.equal(bad.ok,false); assert.ok(bad.errors.length>0);
  } finally { await rm(root,{recursive:true,force:true}); }
});
