import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtemp, mkdir, writeFile, rm } from 'node:fs/promises';
import { resolve, join } from 'node:path';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';
const executable=process.env.GD_NATIVE_EXE && resolve(process.env.GD_NATIVE_EXE);
function run(inputs,extra=[],status=0) {
  const result=spawnSync(executable,['resources',...inputs,...extra],{encoding:'utf8',timeout:30000,maxBuffer:16*1024*1024,windowsHide:true,env:{...process.env,GD_AGENT:'1'}});
  assert.equal(result.status,status,result.stderr+'\n'+result.stdout);
  return JSON.parse(result.stdout);
}
test('conditional resource witnesses preserve AND side inputs, alternatives, status and identity',{skip:!executable},async()=>{
  const root=await mkdtemp(join(tmpdir(),'gd-resource-witness-'));
  try {
    for(const role of ['nouns','axes','mechanics','resources','interactions']) await mkdir(join(root,role));
    const save=(role,name,body)=>writeFile(join(root,role,name+'.md'),body);
    const names=['Silver','Gold','Wood','Key'];
    for(const axis of names) await save('axes',axis,`---\nvalue_type: integer\nunit: ${axis.toLowerCase()}\n---\n`);
    await save('nouns','wallet','---\naxes: [axes/Silver, axes/Gold, axes/Wood, axes/Key]\n---\n');
    for(const pool of ['silver','gold','wood','key','merchant-gold','merchant-key']) await save('resources',pool,'---\nnoun: nouns/wallet\n---\n');
    function rule(name,requires,writes,status='accepted',pools={Silver:'silver',Gold:'gold',Wood:'wood',Key:'key'},preserve='') {
      return `---\nstatus: ${status}\n---\n\`\`\`gd-rule\ncase ${name}:\n    context: ANY\n    Wallet:\n        noun: nouns/wallet\n        pools:\n${Object.entries(pools).map(([axis,pool])=>`            ${axis}: resources/${pool}`).join('\n')}\n        requires: ${requires}\n${preserve ? `        preserve: ${preserve}\n`:''}        set:\n${Object.entries(writes).map(([axis,expression])=>`            ${axis}: ${expression}`).join('\n')}\n\`\`\`\n`;
    }
    await save('mechanics','exchange',rule('exchange','Silver >= 100[silver]',{Silver:'this - 100[silver]',Gold:'this + 1[gold]'}));
    await save('mechanics','key',rule('key','Gold >= 1[gold] AND Wood >= 3[wood]',{Gold:'this - 1[gold]',Wood:'this - 3[wood]',Key:'this + 1[key]'}));
    await save('mechanics','alternative',rule('alternative','Silver >= 200[silver]',{Silver:'this - 200[silver]',Key:'this + 1[key]'},'proposed'));
    await save('mechanics','chest',rule('chest','Key >= 1[key]',{Silver:'this + 10[silver]'},'accepted',undefined,'Key'));
    await save('mechanics','merchant',rule('merchant','Gold >= 1[gold]',{Gold:'this - 1[gold]',Key:'this + 1[key]'},'accepted',{Gold:'merchant-gold',Key:'merchant-key'}));
    await save('mechanics','capped',rule('capped','ANY',{Key:'min(this + 1[key], 3[key])'}));
    const inputs=['mechanics','nouns','axes','resources','interactions'].flatMap(role=>['--'+role,`${role}=${join(root,role)}`]);
    const overview=run(inputs); const facet=pool=>overview.data.nodes.find(n=>n.kind==='facet' && n.pool==='resources/'+pool)?.id;
    const silver=facet('silver'), key=facet('key'), gold=facet('gold'), merchant=facet('merchant-key');
    assert.ok(silver && key && gold && merchant); assert.equal(overview.data.routes.queried,false);
    const result=run(inputs,['--from',silver,'--to',key,'--fields','routes']);
    assert.deepEqual(Object.keys(result.data),['routes']); assert.equal(result.coverage.complete,false);
    const routes=result.data.routes; assert.equal(routes.status,'conditional_witnesses'); assert.equal(routes.limited,false);
    const ordered=routes.witnesses.find(w=>w.nodes.filter(n=>n.startsWith('case:')).length===2);
    assert.ok(ordered); assert.ok(routes.witnesses.some(w=>w.proposal_reliance));
    assert.deepEqual(ordered.steps.map(step=>step.meaning),['checked_debit','checked_credit_not_final_outcome','checked_debit','checked_credit_not_final_outcome']);
    const keyActivation=routes.activations.find(a=>a.whole_case_guards.name==='key');
    assert.match(keyActivation.whole_case_guards.participants[0].requires,/ AND /);
    assert.ok(keyActivation.all_direct_relations_including_side_inputs.some(r=>r.expression==='this - 3[wood]' && r.delta.number===-3));
    assert.ok(routes.activations.find(a=>a.whole_case_guards.name==='exchange').all_direct_relations_including_side_inputs.some(r=>r.delta?.number===-100));
    assert.ok(routes.frontiers_all_eligible_sources.some(r=>r.expression==='min(this + 1[key], 3[key])' && r.delta.number===null));
    assert.ok(routes.witnesses.every(w=>w.conditional && w.feasibility==='not_checked'));
    const accepted=run(inputs,['--from',silver,'--to',key,'--accepted-only']).data.routes;
    assert.ok(accepted.witnesses.length>0); assert.ok(accepted.witnesses.every(w=>!w.proposal_reliance));
    assert.equal(run(inputs,['--from',silver,'--to',merchant]).data.routes.status,'no_known_direct_witness');
    const enabled=run(inputs,['--from',key,'--to',silver]).data.routes;
    assert.ok(enabled.witnesses.some(w=>w.steps.some(s=>s.meaning==='enabling_guard_not_consumed')));
    assert.ok(enabled.activations.find(a=>a.whole_case_guards.name==='chest').all_direct_relations_including_side_inputs.every(r=>r.delta?.number!==-1));
    const obtain=run(inputs,['--to',key]).data.routes; assert.ok(obtain.witnesses.length>0); assert.ok(obtain.cycle_edges>0);
    const ambiguous=run(inputs,['--to','resources/key'],2); assert.equal(ambiguous.errors[0].code,'resources.selector_ambiguous'); assert.ok(ambiguous.data.routes.selector_candidates.length>1);
    await rm(join(root,'resources/wood.md'));
    const missing=run(inputs,['--from',silver,'--to',key,'--accepted-only']).data.routes;
    assert.ok(missing.witnesses.length>0 && missing.witnesses.every(w=>w.unresolved));
    // A source remains visible but cannot supply an accepted-only/default route when coverage is not defined.
    await save('interactions','excluded',rule('excluded','ANY',{Key:'this + 20[key]'}).replace('status: accepted','status: accepted\ncoverage: unknown'));
    const coverage=run(inputs,['--to',key]).data.routes;
    assert.ok(!coverage.activations.some(a=>a.whole_case_guards.name==='excluded'));
  } finally { await rm(root,{recursive:true,force:true}); }
});
