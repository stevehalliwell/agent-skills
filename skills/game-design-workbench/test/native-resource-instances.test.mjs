import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtemp, mkdir, writeFile, rm } from 'node:fs/promises';
import { resolve, join } from 'node:path';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';
const executable=process.env.GD_NATIVE_EXE && resolve(process.env.GD_NATIVE_EXE);
function run(inputs,extra=[]) {
  const result=spawnSync(executable,['resources',...inputs,...extra],{encoding:'utf8',timeout:30000,maxBuffer:32*1024*1024,windowsHide:true,env:{...process.env,GD_AGENT:'1'}});
  assert.equal(result.status,0,result.stderr+'\n'+result.stdout);
  const value=JSON.parse(result.stdout); assert.equal(value.ok,true); return value;
}
test('mapped placements expose concrete nested results, scoped identity and whole ancestor inputs',{skip:!executable},async()=>{
  const root=await mkdtemp(join(tmpdir(),'gd-concrete-resource-'));
  try {
    for(const role of ['nouns','axes','mechanics','resources']) await mkdir(join(root,role));
    const save=(role,name,text)=>writeFile(join(root,role,name+'.md'),text);
    await save('axes','CoinAmount','---\nvalue_type: integer\nunit: coin\n---\n');
    await save('axes','Points','---\nvalue_type: integer\nunit: point\n---\n');
    await save('nouns','bank','---\naxes: [axes/CoinAmount, axes/Points]\n---\n');
    for(const name of ['coins','points','other-coins']) await save('resources',name,'---\nnoun: nouns/bank\n---\n');
    const inputs=['nouns','axes','mechanics','resources'].flatMap(role=>['--'+role,`${role}=${join(root,role)}`]);
    const caller=(pool='')=>`---\nstatus: accepted\n---\n\`\`\`gd-rule
case spend:
    target: Bank
    context: ANY
    Bank:
        noun: nouns/bank
${pool ? `        pools:\n            CoinAmount: resources/${pool}\n`:''}        requires: CoinAmount >= 2[coin]
        set:
            CoinAmount: this - 2[coin]
        causes: mechanics/wrapper#invoke with Account=Bank
\`\`\`\n`;
    await save('mechanics','caller',caller());
    await save('mechanics','wrapper',`---\nstatus: accepted\n---\n\`\`\`gd-rule
case invoke:
    target: Account
    context: ANY
    Account:
        noun: nouns/bank
        requires: ANY
        causes: mechanics/grant#award with Balance=Account
\`\`\`\n`);
    const grant=status=>`---\nstatus: ${status}\n---\n\`\`\`gd-rule
case award:
    target: Balance
    context: ANY
    set:
        Points: this + 1[point]
    Balance:
        noun: nouns/bank
        requires: ANY
        pools:
            CoinAmount: resources/coins
            Points: resources/points
\`\`\`\n`;
    await save('mechanics','grant',grant('accepted'));
    const overview=run(inputs), nodes=overview.data.nodes;
    assert.equal(new Set(nodes.map(n=>n.id)).size,nodes.length);
    const deepest=nodes.find(n=>n.instantiation?.depth===2);
    assert.ok(deepest);
    assert.equal(deepest.declaration.name,'award');
    assert.deepEqual(deepest.instantiation.participants,[{template_participant:'Balance',concrete_participant:'Bank'}]);
    assert.equal(deepest.instantiation.concrete_primary_target,'Bank');
    const concrete=overview.data.relations.find(r=>r.activation===deepest.id && r.expression==='this + 1[point]');
    assert.equal(concrete.concrete_participant,'Bank'); assert.equal(concrete.source.record,'mechanics/grant');
    assert.ok(nodes.some(n=>n.kind==='facet' && n.pool===null && n.representative_declaration?.source.record==='mechanics/caller' && n.axis==='axes/CoinAmount'));
    const coins=nodes.find(n=>n.kind==='facet' && n.pool==='resources/coins').id;
    const points=nodes.find(n=>n.kind==='facet' && n.pool==='resources/points').id;
    const result=run(inputs,['--from',coins,'--to',points]).data.routes;
    const witness=result.witnesses.find(w=>w.steps.some(s=>s.declaration.activation===deepest.id));
    assert.ok(witness);
    assert.ok(witness.steps.some(s=>s.declaration.contextual_ancestor_input && s.declaration.delta.number===-2 && s.declaration.source.record==='mechanics/caller'));
    const activations=result.activations;
    assert.ok(activations.some(a=>a.id===deepest.instantiation.parent_activation));
    assert.ok(activations.some(a=>a.whole_case_guards.name==='spend' && a.whole_case_guards.participants[0].requires==='CoinAmount >= 2[coin]'));
    // Follow explicit local identities through both call edges, not just pooled inputs.
    const local=nodes.find(n=>n.kind==='facet' && n.pool===null && n.axis==='axes/CoinAmount' && n.representative_declaration.source.record==='mechanics/caller').id;
    const through=run(inputs,['--from',local,'--to',points]).data.routes;
    assert.ok(through.witnesses.some(w=>w.steps.filter(s=>s.meaning==='mapped_instantiation_not_temporal_sequence').length===2));
    await save('mechanics','grant',grant('proposed'));
    const proposed=run(inputs,['--from',local,'--to',points]).data.routes;
    assert.ok(proposed.witnesses.some(w=>w.proposal_reliance));
    assert.equal(run(inputs,['--from',local,'--to',points,'--accepted-only']).data.routes.witnesses.length,0);
    await save('mechanics','grant',grant('accepted'));
    await save('mechanics','caller',caller('other-coins'));
    const conflict=run(inputs);
    assert.ok(conflict.coverage.mapped_binding_conflicts>0);
    assert.ok(conflict.data.findings.some(f=>f.code==='resources.mapped_pool_conflict' && f.source.record==='mechanics/grant'));
    const other=conflict.data.nodes.find(n=>n.kind==='facet' && n.pool==='resources/other-coins').id;
    assert.equal(run(inputs,['--from',other,'--to',points]).data.routes.witnesses.length,0);
    // Unrelated source-index changes do not change placement IDs.
    await save('mechanics','aaa','---\nstatus: accepted\n---\n```gd-rule\ncase earlier:\n    context: ANY\n```\n');
    const shifted=run(inputs);
    assert.ok(shifted.data.nodes.some(n=>n.id===deepest.id));
  } finally { await rm(root,{recursive:true,force:true}); }
});
