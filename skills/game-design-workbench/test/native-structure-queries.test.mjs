import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtemp, mkdir, writeFile, readFile, rm } from 'node:fs/promises';
import { resolve, join } from 'node:path';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
const executable=process.env.GD_NATIVE_EXE && resolve(process.env.GD_NATIVE_EXE);
function run(arguments_,status=0) {
  const result=spawnSync(executable,['query',...arguments_,'--json'],{encoding:'utf8',timeout:30000,maxBuffer:32*1024*1024,windowsHide:true,env:{...process.env,GD_AGENT:'1'}});
  assert.equal(result.status,status,result.stderr+'\n'+result.stdout);
  const value=JSON.parse(result.stdout); assert.equal(value.ok,status===0); return value;
}
function question(name,projection,bindings,extra=[]) { return run(['--question',name,'--projection',projection,...bindings,...extra]); }
async function fixture(t,records) {
  const root=await mkdtemp(join(tmpdir(),'gd-structure-')); t.after(()=>rm(root,{recursive:true,force:true}));
  const paths=[];
  for (const [folder,items] of Object.entries(records)) { await mkdir(join(root,folder)); for (const [name,content] of Object.entries(items)) { const path=join(root,folder,name+'.md'); await writeFile(path,content,'utf8'); paths.push(path); } }
  return {root,paths};
}
function record(name,fields='',status='accepted') { return `---\nid: structure-${name}\nname: ${name}\nstatus: ${status}\n${fields}---\nAuthored fixture ${name}.\n`; }
function index(value,id) { const found=value.data.nodes.find(node=>node.id===id); assert.ok(found,id); return found.index; }
async function digest(paths) { const hash=createHash('sha256'); for (const path of paths) hash.update(await readFile(path)); return hash.digest('hex'); }
const structural=['direct-connections','related-structure','reachability','structural-dominators','support-bypass','role-shapes','adjacency','declaration-gaps'];
test('structural catalogue advertises only delivered parameters and narrower UI availability',{skip:!executable},()=>{
  const catalogue=run(['--list']); const questions=catalogue.data.questions.filter(item=>item.projections);
  assert.deepEqual(questions.map(item=>item.name),structural);
  for (const item of questions) { assert.deepEqual(item.projections,['skills','challenges','relationships','matrix','mda','resources','actor-inventory']); assert.equal(item.ui_available,true); assert.match(item.ui_scope,/canonical projection/); assert.ok(item.ui_unavailable_parameters.includes('repeatable input beyond 8192 combined bytes per list')); assert.equal(item.ipc_available,false); assert.equal(item.work_limit_max,16777216); for (const bindings of Object.values(item.required_bindings_by_projection)) { assert.ok(Array.isArray(bindings.all_of)); assert.ok(Array.isArray(bindings.any_of)); } assert.deepEqual(item.required_bindings_by_projection.resources.any_of,['interactions','mechanics']); assert.deepEqual(item.required_bindings_by_projection['actor-inventory'].all_of,['nouns','axes']); }
  assert.ok(!questions.find(item=>item.name==='adjacency').parameters.includes('--from'));
  assert.ok(!questions.find(item=>item.name==='direct-connections').repeatable_parameters.includes('--start'));
  assert.equal(run(['--list','--work-limit','0'],2).errors[0].code,'cli.arguments');
});
test('mixed-kind projected paths, parallel counts, original backwards arrows and status boundaries',{skip:!executable},async t=>{
  const {root,paths}=await fixture(t,{skills:{},r:{a:record('a','nouns: [r/b]\nverbs: [r/c]\n'),b:record('b','context_nouns: [r/c, r/c]\n'),c:record('c','mechanics: [r/d]\n'),d:record('d','nouns: [r/d]\n','draft'),e:record('e')}});
  const bindings=['--skill-atoms',`skills=${root}/skills`,'--related',`r=${root}/r`], before=await digest(paths);
  const direct=question('direct-connections','relationships',bindings); const a=index(direct,'r/a'),b=index(direct,'r/b'),c=index(direct,'r/c'),d=index(direct,'r/d');
  assert.equal(direct.data.scope.links,6); assert.equal(direct.data.scope.isolates,1); assert.equal(direct.data.nodes[b].counts.outgoing,2); assert.equal(direct.data.nodes[b].counts.distinct_partners,2);
  assert.equal(direct.data.nodes[d].counts.incident_rows,2); assert.equal(direct.data.nodes[d].counts.distinct_partners,1);
  const selected=['--from','r/a','--to','r/d','--relation','nouns','--relation','context_nouns','--relation','mechanics'];
  const reached=question('reachability','relationships',bindings,selected); assert.ok(reached.data.evidence.rows[0].targets.includes(d)); assert.equal(reached.data.evidence.support_length,3); assert.ok(reached.data.evidence.support_suffix.every(arc=>!arc.reverse_traversal));
  const backwards=question('reachability','relationships',bindings,['--from','r/d','--to','r/a','--direction','backward',...selected.slice(4)]); assert.equal(backwards.data.evidence.support_length,3); assert.ok(backwards.data.evidence.support_suffix.every(arc=>arc.reverse_traversal)); assert.deepEqual(backwards.data.links.map(link=>[link.from,link.to]),direct.data.links.map(link=>[link.from,link.to]));
  const dominated=question('structural-dominators','relationships',bindings,['--from','r/a','--to','r/d']); assert.equal(dominated.data.evidence.path_nodes[c].dominates_target,true); assert.equal(dominated.data.evidence.path_nodes[b].dominates_target,false);
  const bypass=question('support-bypass','relationships',bindings,['--from','r/a','--to','r/d','--avoid','r/b']); assert.equal(bypass.data.evidence.bypass.found,true); const mandatory=question('support-bypass','relationships',bindings,[...selected,'--avoid','r/b']); assert.equal(mandatory.coverage.available,false); assert.equal(mandatory.coverage.query_complete,false); assert.equal(mandatory.data.evidence.bypass,null);
  const outside=question('support-bypass','relationships',bindings,['--from','r/a','--to','r/d','--avoid','r/e']); assert.equal(outside.coverage.available,false); assert.equal(outside.data.evidence.bypass,null); assert.equal(outside.data.selectors.avoid,'r/e');
  const accepted=question('reachability','relationships',bindings,['--from','r/a','--status','accepted']); assert.equal(accepted.data.nodes[d].shown,false); assert.ok(!accepted.data.evidence.rows[0].targets.includes(d)); assert.ok(accepted.data.evidence.rows[0].targets.includes(c));
  const related=question('related-structure','relationships',bindings,['--from','r/a','--to','r/b']); assert.equal(related.data.evidence.comparison.shared,1);
  const partial=question('reachability','relationships',bindings,[...selected,'--work-limit','0']); assert.equal(partial.coverage.query_complete,false); assert.equal(partial.coverage.native_search_work,0);
  assert.equal(question('role-shapes','relationships',bindings,['--work-limit','0']).coverage.query_complete,false);
  assert.equal(question('adjacency','relationships',bindings).data.evidence.input_link_rows,6);
  const scoped=question('direct-connections','relationships',bindings,['--scope-root','r/a']); assert.ok(scoped.data.scope.nodes<direct.data.scope.nodes); assert.ok(scoped.data.nodes.some(node=>node.id==='r/a'));
  assert.equal(a,index(reached,'r/a')); assert.equal(await digest(paths),before);
});
test('challenge prerequisite mechanic joins exclude later challenge content by default',{skip:!executable},async t=>{
  const {root}=await fixture(t,{s:{base:record('base','mechanics: [m/rule]\n'),advanced:record('advanced','prerequisites: [s/base]\n')},c:{room:record('room','requires: [s/advanced]\nnext: [c/later]\n'),later:record('later','mechanics: [m/future]\n')},m:{rule:record('rule'),future:record('future')}});
  const bindings=['--skill-atoms',`s=${root}/s`,'--challenges',`c=${root}/c`,'--mechanics',`m=${root}/m`];
  const skills=question('direct-connections','skills',bindings); assert.equal(skills.data.scope.links,1); assert.equal(skills.data.links[0].source.reference,'s/advanced');
  const challenges=question('reachability','challenges',bindings,['--from','c/room','--to','c/later']); assert.equal(challenges.data.scope.links,1); assert.equal(challenges.data.evidence.support_length,1);
  const value=question('reachability','relationships',bindings,['--from','c/room','--to','m/rule']);
  const targets=value.data.evidence.rows[0].targets; assert.ok(targets.includes(index(value,'m/rule'))); assert.ok(!targets.includes(index(value,'c/later'))); assert.ok(!targets.includes(index(value,'m/future'))); assert.equal(value.data.evidence.support_length,3); assert.ok(!value.data.scope.relation_kinds.includes('next'));
  const explicit=question('reachability','relationships',bindings,['--from','c/room','--to','m/future','--relation','all']); assert.ok(explicit.data.evidence.rows[0].targets.includes(index(explicit,'m/future')));
});
test('empty typed projection retains complete zero counts, not fabricated paths',{skip:!executable},async t=>{
  const {root}=await fixture(t,{skills:{}}); const bindings=['--skill-atoms',`skills=${root}/skills`];
  const arguments_=['query','--question','direct-connections','--projection','skills',...bindings];
  const automatic=spawnSync(executable,arguments_,{encoding:'utf8',timeout:30000,windowsHide:true,env:{...process.env,GD_AGENT:'1'}}); assert.equal(automatic.status,0,automatic.stderr); assert.equal(JSON.parse(automatic.stdout).ok,true);
  const text=spawnSync(executable,[...arguments_,'--format','text'],{encoding:'utf8',timeout:30000,windowsHide:true,env:{...process.env,GD_AGENT:'1'}}); assert.equal(text.status,0,text.stderr); assert.match(text.stdout,/direct-connections \[skills\]/); assert.match(text.stdout,/Native evidence\/source metadata/); assert.equal(JSON.parse(text.stdout.split('\n').find(line=>line.startsWith('{'))).ok,true);
  const value=question('direct-connections','skills',bindings); assert.equal(value.data.scope.nodes,0); assert.equal(value.data.scope.links,0); assert.equal(value.coverage.query_complete,true);
  assert.equal(question('adjacency','skills',bindings).data.evidence.input_link_rows,0);
  for (const extra of [['--from','skills/absent'],['--start','skills/absent']]) assert.equal(run(['--question','reachability','--projection','skills',...bindings,...extra],2).errors[0].code,'query.selector_missing');
});
test('sparse matrix is incidence, retains all native case sources and exposes no inventory flow',{skip:!executable},async t=>{
  const {root}=await fixture(t,{n:{actor:record('actor')},v:{touch:record('touch')},i:{one:record('one','coverage: defined\nnouns: [n/actor]\nverb: v/touch\n'),two:record('two','coverage: defined\nnouns: [n/actor]\nverb: v/touch\n','draft')}});
  const bindings=['--nouns',`n=${root}/n`,'--verbs',`v=${root}/v`,'--interactions',`i=${root}/i`];
  const value=question('direct-connections','matrix',bindings,['--status','accepted']); assert.equal(value.data.scope.incidence,true); assert.equal(value.data.scope.links,1); assert.ok(value.data.nodes.every(node=>node.counts.incoming===null && node.counts.outgoing===null)); assert.equal(value.data.links[0].loaded_case_count,2); assert.deepEqual(value.data.links[0].source_cases.map(item=>item.status).sort(),['accepted','draft']); assert.equal(value.coverage.final_outcomes_proven,false);
  const reached=question('reachability','matrix',bindings,['--from','n/actor','--to','v/touch']); assert.equal(reached.data.evidence.support_length,1); assert.equal(reached.data.scope.walk_direction,'undirected incidence');
  // Draft source rows cannot borrow activity from an excluded accepted case.
  await writeFile(join(root,'n','actor.md'),record('actor','','draft'));
  await writeFile(join(root,'v','touch.md'),record('touch','','draft'));
  assert.equal(question('direct-connections','matrix',bindings,['--status','draft']).data.scope.links,0);
  assert.equal(question('direct-connections','matrix',bindings,['--status','draft','--include-inactive']).data.scope.links,1);
});
test('Resource incidence retains native body and placement provenance without second evaluator',{skip:!executable},()=>{
  const root=resolve('skills/game-design-workbench/examples/crate-puzzle/game-design'); const bindings=['--mechanics',`mechanics=${root}/mechanics`,'--interactions',`interactions=${root}/interactions`,'--nouns',`nouns=${root}/nouns`,'--axes',`axes=${root}/axes`,'--resources',`resources=${root}/resources`];
  const value=question('direct-connections','resources',bindings); assert.equal(value.data.scope.incidence,true); assert.ok(value.data.nodes.length>0); assert.ok(value.data.nodes.every(node=>node.id)); assert.ok(value.data.links.every(link=>Number.isInteger(link.source_case) && Number.isInteger(link.target_participant))); assert.ok(value.data.case_context.length>0); assert.ok(value.data.case_context.some(item=>item.participants?.length)); assert.equal(value.coverage.final_outcomes_proven,false);
  const native=spawnSync(executable,['resources',...bindings,'--json'],{encoding:'utf8',timeout:30000,maxBuffer:32*1024*1024,windowsHide:true}); assert.equal(native.status,0,native.stderr); assert.deepEqual(value.data.native_resource_coverage,JSON.parse(native.stdout).coverage);
  const source=value.data.nodes.find(node=>node.case>=0); const selected=question('direct-connections','resources',bindings,['--from',source.id]); assert.ok(selected.data.evidence.marked_nodes.includes(source.index));
});
test('unloaded and missing reference identities retain placeholders; MDA type findings are not dropped',{skip:!executable},async t=>{
  const {root}=await fixture(t,{skills:{wrong:record('wrong','prerequisites: [r/x]\n')},r:{x:record('x','mechanics: [bound/absent, unloaded/unknown]\n')},bound:{},m:{},d:{bad:record('bad','mechanics: [d/bad]\n')},a:{}});
  const value=question('direct-connections','relationships',['--skill-atoms',`skills=${root}/skills`,'--related',`r=${root}/r`,'--related',`bound=${root}/bound`]);
  assert.deepEqual(value.data.declarations.filter(item=>item.target_record<0).map(item=>item.identity_resolution),['missing','unloaded']); assert.ok(value.data.declarations.every(item=>item.line>0));
  const wrong=question('reachability','skills',['--skill-atoms',`skills=${root}/skills`,'--related',`r=${root}/r`],['--from','skills/wrong']); assert.equal(wrong.coverage.query_complete,true); assert.equal(wrong.coverage.source_incomplete,true); assert.equal(wrong.coverage.complete,false); assert.ok(wrong.data.findings.some(item=>item.code==='skill.prerequisite_target')); assert.equal(value.coverage.source_incomplete,true); assert.equal(value.coverage.complete,false);
  const mda=question('direct-connections','mda',['--mechanics',`m=${root}/m`,'--dynamics',`d=${root}/d`,'--aesthetics',`a=${root}/a`]); assert.ok(mda.data.mda_findings.some(item=>item.code==='mda.reference_target')); assert.equal(mda.coverage.source_incomplete,true);
});
test('structural operations reject ignored options, hidden starts, unsupported projections and invalid budgets',{skip:!executable},async t=>{
  const {root}=await fixture(t,{skills:{a:record('a')}}); const bindings=['--skill-atoms',`skills=${root}/skills`];
  for (const [name,extra] of [['direct-connections',['--start','skills/a']],['adjacency',['--from','skills/a']],['role-shapes',['--to','skills/a']],['direct-connections',['--direction','backward']],['direct-connections',['--lens','same-role']],['reachability',[]],['direct-connections',['--avoid','skills/a']],['direct-connections',['--axis','axes/x']],['direct-connections',['--work-limit','16777217']],['direct-connections',['--status','all','--status','accepted']],['direct-connections',['--fields','concepts']]]) assert.equal(run(['--question',name,'--projection','skills',...bindings,...extra],2).ok,false);
  assert.equal(run(['--question','direct-connections','--projection','actors',...bindings],2).errors[0].code,'query.projection_unavailable');
  assert.equal(run(['--question','object-bindings','--projection','actors','--lens','any'],2).errors[0].code,'cli.arguments');
  assert.equal(run(['--question','direct-connections','--projection','mda',...bindings],2).errors[0].code,'query.projection_unavailable');
});
