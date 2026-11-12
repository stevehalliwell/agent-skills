// Headless Windows/macOS IPC. Build explicitly with CJIT; never starts a GUI.
// GD_NATIVE_EXE=.../gd.exe GD_VIEWER_TEST_EXE=.../test-viewer.exe node --test this-file
import test from 'node:test';
import assert from 'node:assert/strict';
import { spawn, spawnSync } from 'node:child_process';
import { createConnection } from 'node:net';
import { resolve, join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
import { readFile, readdir } from 'node:fs/promises';
import { createHash } from 'node:crypto';
const cli=process.env.GD_NATIVE_EXE && resolve(process.env.GD_NATIVE_EXE);
const fixture=process.env.GD_VIEWER_TEST_EXE && resolve(process.env.GD_VIEWER_TEST_EXE);
const adapter=process.env.GD_VIEWER_ADAPTER_EXE && resolve(process.env.GD_VIEWER_ADAPTER_EXE);
const examples=resolve(dirname(fileURLToPath(import.meta.url)),'../examples');
const nativeTransport=['win32','darwin'].includes(process.platform);
function endpoint(session) {
  return process.platform==='darwin' ? `/tmp/proto-pipes-${process.getuid()}/gd-workbench-${session}` : '\\\\.\\pipe\\gd-workbench-'+session;
}
function invoke(arguments_,status=0) {
  const result=spawnSync(cli,['viewer',...arguments_],{encoding:'utf8',timeout:7000,maxBuffer:2*1024*1024,windowsHide:true});
  assert.equal(result.status,status,result.stderr+'\n'+result.stdout);
  return JSON.parse(result.stdout);
}
async function owner() {
  const child=spawn(fixture,['--serve'],{windowsHide:true,stdio:['ignore','pipe','pipe']});
  let errors=''; child.stderr.on('data',chunk=>{errors+=chunk;});
  try {
    const session=await new Promise((done,fail)=>{
      let text=''; const timeout=setTimeout(()=>fail(new Error('Owner startup timed out: '+errors)),5000);
      child.once('error',error=>{clearTimeout(timeout);fail(error);});
      child.once('exit',code=>{clearTimeout(timeout);fail(new Error(`Owner exited ${code}: ${errors}`));});
      child.stdout.on('data',chunk=>{
        text+=chunk;
        if(text.includes('\n')) {clearTimeout(timeout);try {done(JSON.parse(text.split('\n')[0]).session);}catch(error){fail(error);}}
      });
    });
    return {child,session};
  } catch(error) {child.kill();throw error;}
}
function raw(session,request,delay=0,fragment=false) {
  return new Promise((done,fail)=>{
    const socket=createConnection(endpoint(session)); let bytes=Buffer.alloc(0);
    const payload=Buffer.from(request),header=Buffer.alloc(4);header.writeUInt32LE(payload.length);
    const framed=process.platform==='darwin'?Buffer.concat([header,payload]):payload;
    socket.setTimeout(4000,()=>socket.destroy(new Error('Raw request timed out')));
    socket.on('error',fail);
    socket.on('connect',()=>{
      if(fragment) {
        socket.write(framed.subarray(0,2));
        setTimeout(()=>socket.write(framed.subarray(2,7)),10);
        setTimeout(()=>socket.write(framed.subarray(7)),20);
      } else socket.write(framed);
      if(delay){socket.pause();setTimeout(()=>socket.resume(),delay);}
    });
    socket.on('data',chunk=>{
      bytes=Buffer.concat([bytes,chunk]);let payload=bytes;
      if(process.platform==='darwin') {
        if(bytes.length<4 || bytes.length<4+bytes.readUInt32LE(0))return;
        payload=bytes.subarray(4,4+bytes.readUInt32LE(0));
      }
      let result;try {result=JSON.parse(payload.toString('utf8'));}catch{return;}
      socket.destroy();done(result);
    });
  });
}
async function sourceHashes(root) {
  const hashes={};
  async function visit(directory) {
    for(const entry of await readdir(directory,{withFileTypes:true})) {
      const path=join(directory,entry.name);
      if(entry.isDirectory())await visit(path);
      else if(entry.isFile())hashes[path]=createHash('sha256').update(await readFile(path)).digest('hex');
    }
  }
  await visit(root);return hashes;
}
test('current-view Find adapter: both examples unchanged',{skip:!adapter},async t=>{
  for(const name of ['crate-puzzle','little-plumber-guy-nes'])await t.test(name,async()=>{
    const root=join(examples,name,'game-design'),before=await sourceHashes(root);
    const result=spawnSync(adapter,['--find-helpers',root],{encoding:'utf8',windowsHide:true,timeout:30000,maxBuffer:1024*1024});
    assert.equal(result.status,0,result.stderr+'\n'+result.stdout);
    const value=JSON.parse(result.stdout);assert.equal(value.ok,true);assert.ok(value.find_checks>=100);
    assert.deepEqual(await sourceHashes(root),before);
  });
});
test('shared UX adapter: both examples unchanged',{skip:!adapter},async t=>{
  for(const name of ['crate-puzzle','little-plumber-guy-nes'])await t.test(name,async()=>{
    const root=join(examples,name,'game-design'),before=await sourceHashes(root);
    const result=spawnSync(adapter,['--ux-helpers',root],{encoding:'utf8',windowsHide:true,timeout:30000,maxBuffer:1024*1024});
    assert.equal(result.status,0,result.stderr+'\n'+result.stdout);
    const value=JSON.parse(result.stdout);assert.equal(value.ok,true);assert.ok(value.shared_ux_checks>=200);
    assert.deepEqual(await sourceHashes(root),before);
  });
});
test('diagnostic flag parsing rejects implicit or malformed modes',{skip:!adapter},()=>{
  const root=join(examples,'crate-puzzle','game-design');
  for(const args of [['--layout-research',root],['--deep-checks',root],['--ipc-helpers',root,'--layout-reseach'],['--ipc-helpers',root,'--deep-check'],['--find-helpers',root,'--layout-research'],['--find-helpers',root,'--deep-checks'],['--ux-helpers',root,'--deep-checks'],['--ipc-helpers',root,'--layout-research','--layout-research'],['--ipc-helpers',root,'--deep-checks','--deep-checks']]){
    const result=spawnSync(adapter,args,{encoding:'utf8',windowsHide:true,timeout:30000});
    assert.equal(result.status,2,result.stderr);assert.equal(result.stdout,'');assert.match(result.stderr,/--ipc-helpers GAME-DESIGN-DIRECTORY \[--deep-checks\] \[--layout-research\]/);
  }
});
for(const [runDeepChecks,runLayoutResearch] of [[false,false],[true,false],[true,true]])test(runLayoutResearch ? 'layout research: explicit opt-in, both examples unchanged':runDeepChecks ? 'deep explorer checks: explicit opt-in, both examples unchanged':'production explorer IPC adapter: CPU-only, both examples unchanged',{
  skip:!nativeTransport||!adapter||(runLayoutResearch ? process.env.GD_LAYOUT_RESEARCH!=='1':runDeepChecks && process.env.GD_DEEP_CHECKS!=='1')
},async t=>{
  for(const name of ['crate-puzzle','little-plumber-guy-nes'])await t.test(name,async()=>{
    const root=join(examples,name,'game-design'),before=await sourceHashes(root);
    const args=['--ipc-helpers',root];if(runLayoutResearch)args.push('--layout-research');else if(runDeepChecks)args.push('--deep-checks');
    const result=spawnSync(adapter,args,{encoding:'utf8',windowsHide:true,timeout:90000,maxBuffer:2*1024*1024});
    assert.equal(result.status,0,result.stderr+'\n'+result.stdout);
    const value=JSON.parse(result.stdout);assert.equal(value.ok,true);assert.ok(value.checks>=180);assert.ok(value.control_checks>=500);assert.ok(value.feedback_checks>=10);assert.ok(value.sidebar_checks>=34);assert.ok(value.configuration_checks>=50);
    assert.equal(value.deep_checks,runDeepChecks);assert.equal(value.layout_research,runLayoutResearch);
    if(runDeepChecks)assert.ok(value.presentation_checks>=150);else assert.equal(value.presentation_checks,0);
    if(runLayoutResearch){
      const references=result.stderr.split(/\r?\n/).filter(line=>line.startsWith('layout-reference ')).map(line=>JSON.parse(line.slice('layout-reference '.length)));
      const topologies=references.filter(row=>row.kind==='topology');assert.equal(topologies.length,19);
      for(const topology of topologies){
        assert.equal(topology.aspect,1.4);assert.equal(topology.footprints,'adapter-only');assert.equal(topology.world_scale,1);
        assert.equal(topology.workspace_enabled,topology.profile==='bounded-workspace');
        if(topology.workspace_enabled){assert.ok(topology.workspace_right>topology.workspace_left);assert.ok(topology.workspace_bottom>topology.workspace_top);}
        assert.ok(topology.components>=1 && topology.components<=topology.nodes);
        assert.ok(topology.unique_neighbor_pairs>=topology.nodes-topology.components);
        assert.ok(topology.active_nonself_springs>=topology.unique_neighbor_pairs);
        assert.ok(topology.degree_median<=topology.degree_p90 && topology.degree_p90<=topology.degree_max);
        assert.ok(Math.abs(topology.degree_mean-2*topology.unique_neighbor_pairs/topology.nodes)<=0.000001);
        const samples=references.filter(row=>row.kind==='motion' && row.view===topology.view && row.seed===topology.seed && row.profile===topology.profile && row.startup_steps===topology.startup_steps);
        assert.deepEqual(samples.map(row=>row.step),[0,120,600,4096]);
        assert.deepEqual(samples.map(row=>row.window),[0,120,256,256]);
        for(const sample of samples){
          for(const key of ['rms','max_step','visual_rms','visual_max_step','centroid_drift','extent_width','extent_height'])assert.ok(Number.isFinite(sample[key]) && sample[key]>=0,key);
          assert.ok(sample.rms<=sample.max_step+0.000001);
          assert.ok(sample.visual_rms<=sample.visual_max_step+0.000001);
          assert.ok(sample.limited_steps<=sample.window && sample.max_collision_checks<=65536);
          assert.ok(sample.overlaps>=0 && sample.overlaps<=topology.nodes*(topology.nodes-1)/2);
          assert.ok(sample.outside_nodes>=0 && sample.outside_nodes<=topology.nodes && Number.isFinite(sample.max_overflow) && sample.max_overflow>=0);
          assert.ok(sample.boundary_nodes>=0 && sample.boundary_nodes<=topology.nodes);
          if(topology.workspace_enabled)assert.ok(sample.outside_nodes===0 && sample.max_overflow<=0.01);
        }
      }
      // Independent existing noun probe checks generic ring-window aggregation.
      for(const seed of ['native','neighborhood']){
        const legacy=result.stderr.match(new RegExp(`Noun force probe \\(${seed}\\).*tail RMS ([0-9.]+) max ([0-9.]+) / centroid drift ([0-9.]+)`));assert.ok(legacy);
        const measured=references.find(row=>row.kind==='motion' && row.view==='noun-interactions' && row.seed===seed && row.step===4096);
        assert.deepEqual([measured.rms,measured.max_step,measured.centroid_drift],legacy.slice(1).map(Number));
      }
      const raw120=references.find(row=>row.kind==='motion' && row.view==='resources' && row.seed==='native' && row.startup_steps===0 && row.step===120);
      const warm0=references.find(row=>row.kind==='motion' && row.view==='resources' && row.seed==='native' && row.startup_steps===120 && row.step===0);
      assert.deepEqual([raw120.extent_width,raw120.extent_height,raw120.overlaps],[warm0.extent_width,warm0.extent_height,warm0.overlaps]);
    }else assert.doesNotMatch(result.stderr,/^layout-reference /m);
    if(runDeepChecks){
      assert.match(result.stderr,/Resource footprint probe:/);
      for(const seed of ['native','neighborhood'])assert.ok(result.stderr.includes(`Noun force probe (${seed})`));
      const layers=value.layer_result;assert.equal(layers.ok,true);assert.match(layers.scope,/shown-canvas/);
      assert.equal(layers.nodes.length,layers.shown_nodes);assert.ok(layers.nodes.every(node=>typeof node.id==='string'));
      assert.ok(layers.links.every(link=>link.from>=0 && link.from<layers.nodes.length && link.to>=0 && link.to<layers.nodes.length));
      assert.equal(layers.foundations,null);assert.ok(layers.shapes?.valid);assert.ok(layers.overview?.valid);
    }else{assert.equal(value.layer_result,null);assert.doesNotMatch(result.stderr,/Resource footprint probe:|Noun force probe/);}
    const catalogue=value.control_catalogue;assert.equal(catalogue.ok,true);assert.equal(catalogue.projection,'actors');
    const form=catalogue.forms.find(form=>form.name==='query');assert.match(form.control_token,/^[a-f0-9]{16}$/);
    assert.equal(form.fields.length,16);assert.ok(form.applied);assert.equal(form.input_status,'pending-changes');assert.equal(form.fields.find(field=>field.name==='coverage').value,'false');
    assert.match(form.fields.find(field=>field.name==='root_case').value,/#/);
    assert.equal(form.fields.find(field=>field.name==='values').choices_closed,false);
    assert.ok(form.fields.find(field=>field.name==='values').choice_slots>=2);
    assert.ok(form.fields.every(field=>typeof field.editable==='boolean' && ['boolean','text','choice','integer'].includes(field.type)));
    const resourceForm=value.resource_controls.forms.find(form=>form.name==='resource');
    assert.match(resourceForm.control_token,/^[a-f0-9]{16}$/);assert.equal(resourceForm.fields.length,4);
    assert.equal(resourceForm.fields.find(field=>field.name==='to').choices_closed,true);
    assert.ok(resourceForm.fields.find(field=>field.name==='to').choice_slots>1);
    assert.equal(resourceForm.fields.find(field=>field.name==='from').value,'');
    assert.equal(resourceForm.fields.find(field=>field.name==='work_limit').maximum,16777216);
    assert.deepEqual(value.resource_actions.actions.filter(action=>action.form==='resource').map(action=>action.name),['resource.run','resource.clear']);
    assert.ok(value.resource_actions.actions.every(action=>action.available ? action.disabled_reason===null:typeof action.disabled_reason==='string' && action.disabled_reason.length>0));
    assert.equal(value.resource_actions.source_draft_edits,true);
    const snapshot=value.snapshot;assert.equal(snapshot.draft.redo_steps,0);assert.equal(snapshot.ok,true);assert.equal(snapshot.projection,'matrix');
    assert.equal(snapshot.panes.length,9);
    assert.equal(snapshot.panes.find(pane=>pane.projection==='noun-interactions').available,true);assert.equal(snapshot.draft.modified_records,0);assert.equal(snapshot.draft.undo_steps,0);
    assert.equal(snapshot.panes.find(pane=>pane.projection==='actors').query.pending.values,'unapplied Ω');
    assert.notEqual(snapshot.panes.find(pane=>pane.projection==='actors').query.applied?.values,'unapplied Ω');
    const rules=snapshot.panes.find(pane=>pane.projection==='resources').rules;
    assert.equal(rules.pending.bindings,true);assert.equal(rules.applied.bindings,true);
    assert.deepEqual(rules.pending,rules.applied);
    assert.ok(snapshot.snapshots.every(family=>family.collections.every(collection=>collection.alias&&collection.directory)));
    assert.equal(snapshot.limits.source_writes,false);assert.equal(snapshot.limits.derived_navigation,true);assert.equal(snapshot.limits.application_controls,false);assert.equal(snapshot.limits.application_actions,true);assert.equal(snapshot.limits.source_refresh,true);assert.equal(snapshot.limits.form_controls,true);
    assert.equal('source' in snapshot,false);
    assert.equal(snapshot.agent_hint.stale,runDeepChecks);assert.equal(snapshot.reload_generation,runDeepChecks ? '2':'1');assert.equal(typeof snapshot.agent_hint.id,'string');
    const page=value.node_page;assert.equal(page.projection,'matrix');assert.equal(page.prepared,true);
    assert.equal(page.canvas,'ordinary');assert.equal(page.nodes.length,2);assert.equal(page.offset,0);assert.equal(page.next_offset,2);assert.ok(page.total>2);
    for(const node of page.nodes) {
      assert.equal(typeof node.id,'string');assert.equal(typeof node.label,'string');assert.equal(typeof node.source,'string');
      assert.equal(typeof node.visible,'boolean');assert.equal(node.identity_available,true);
    }
    const source=value.source_context;assert.equal(source.ok,true);assert.equal(source.source.reference,source.selection.source.reference);
    assert.equal(typeof source.source.body,'string');assert.ok(source.source.body.length>0);
    assert.equal(source.source.path,source.selection.source.path);assert.equal(source.source.body_line,source.selection.source.body_line);
    assert.match(source.source.body_scope,/not full raw file/);assert.match(source.source.evidence_scope,/not all contributing/);
    assert.deepEqual(await sourceHashes(root),before);
  });
});
async function close(owner) {
  if(owner.child.exitCode!==null)return;
  await new Promise(done=>{owner.child.once('exit',done);owner.child.kill();});
}
test('viewer protocol and native local request/reply without any GUI',{skip:!nativeTransport||!cli||!fixture},async t=>{
  const checked=spawnSync(fixture,[],{encoding:'utf8',windowsHide:true,timeout:5000});
  assert.equal(checked.status,0,checked.stderr);assert.equal(JSON.parse(checked.stdout).ok,true);
  const first=await owner();let second;
  try {
    await t.test('discovery exposes exact per-start session; inspect does not mutate',()=>{
      assert.match(first.session,/^[1-9][0-9]*-[a-f0-9]{16}$/);
      assert.ok(invoke(['list']).viewers.some(viewer=>viewer.session===first.session));
      assert.equal(invoke(['inspect','--target',first.session]).mutations,0);
      assert.equal(invoke(['inspect','--target',first.session]).mutations,0);
    });
    await t.test('bounded node pages and opt-in source fields remain read-only',()=>{
      assert.deepEqual(invoke(['actions','--target',first.session]).actions,[]);
      assert.deepEqual(invoke(['controls','--target',first.session]).forms,[]);
      const options=invoke(['controls','--target',first.session,'--form','query','--control','participant','--offset','8192','--limit','1']);
      assert.equal(options.offset,8192);assert.equal(options.limit,1);
      assert.equal(invoke(['nodes','--target',first.session]).limit,64);
      const page=invoke(['nodes','--target',first.session,'--projection','effects','--offset','4096','--limit','1']);
      assert.equal(page.offset,4096);assert.equal(page.limit,1);assert.deepEqual(page.nodes,[]);
      assert.equal(invoke(['inspect','--target',first.session,'--fields','source']).mutations,0);
    });
    await t.test('hint/navigation preserve exact Unicode and escaping',()=>{
      const reference='skills/café Ω 😀 "quote" \\ slash';
      assert.equal(invoke(['hint','--target',first.session,'--projection','skills','--reference',reference]).reference,reference);
      assert.equal(invoke(['navigate','--target',first.session,'--projection','matrix']).reference,'');
      assert.equal(invoke(['inspect','--target',first.session]).mutations,2);
    });
    await t.test('fragmented framing and payload preserve a read-only request',{skip:process.platform!=='darwin'},async()=>{
      assert.equal((await raw(first.session,JSON.stringify({protocol:1,command:'inspect'}),0,true)).mutations,2);
    });
    await t.test('malformed/version/duplicate/inapplicable fields reject before owner dispatch',async()=>{
      for(const request of [
        '{"protocol":2,"command":"inspect"}',
        '{"protocol":1,"command":"inspect","command":"inspect"}',
        '{"protocol":1,"command":"inspect","unknown":true}',
        '{"protocol":1,"command":"navigate"}',
        '{"protocol":1,"command":"inspect","reference":"x"}',
        '{"protocol":1,"command":"inspect"}\u0000'
      ]) assert.equal((await raw(first.session,request)).code,'viewer.protocol');
      assert.equal((await raw(first.session,JSON.stringify({protocol:1,command:'inspect',target:'another-session'}))).code,'viewer.target');
      assert.equal(invoke(['inspect','--target',first.session]).mutations,2);
    });
    await t.test('oversized and disconnected peers do not poison next request',async()=>{
      assert.equal((await raw(first.session,'x'.repeat(9000))).code,'viewer.capacity');
      await new Promise((done,fail)=>{
        const socket=createConnection(endpoint(first.session));
        socket.on('error',fail);socket.on('connect',()=>{socket.destroy();setTimeout(done,30);});
      });
      assert.equal(invoke(['inspect','--target',first.session]).mutations,2);
    });
    await t.test('invalid CLI combinations fail as JSON without dispatch',()=>{
      for(const arguments_ of [[],['unknown'],['hint','--projection','skills'],['inspect','--projection','skills'],['inspect','--target','../bad'],['list','--reference','x'],['inspect','--target',first.session,'--target',first.session],['nodes','--limit','0'],['nodes','--limit','65'],['nodes','--limit','1.5'],['nodes','--offset','-1'],['nodes','--offset','4097'],['nodes','--offset','01'],['inspect','--limit','1'],['nodes','--id','x'],['refresh','--projection','skills'],['actions','--limit','1'],['controls','--control','axis'],['controls','--offset','0'],['controls','--form','query','--control','axis','--offset','8193'],['controls','--value','x'],['configure','--projection','actors','--form','query','--control','axis','--value','x'],['configure','--projection','actors','--form','query','--control','axis','--expect','token'],['configure','--form','query','--control','axis','--value','x','--expect','token'],['inspect','--form','query'],['refresh','--expect','token'],['action','--projection','actors'],['action','--action','query.run'],['inspect','--action','query.run'],['hint','--projection','effects','--reference','x','--id','y'],['navigate','--projection','actors','--id','x'.repeat(4096)]]) {
        assert.equal(invoke(arguments_,2).ok,false);
      }
      assert.equal(invoke(['navigate','--target',first.session,'--projection','actors','--id','"'.repeat(4095)],2).code,'viewer.capacity');
      assert.equal(invoke(['inspect','--target',first.session]).mutations,2);
    });
    await t.test('large delayed-reader reply retains owned backing; overflow returns no partial context',async()=>{
      const request=reference=>JSON.stringify({protocol:1,command:'navigate',projection:'skills',reference});
      assert.equal((await raw(first.session,request('large'),150)).text.length,256*1024);
      assert.equal((await raw(first.session,request('overflow'))).code,'viewer.capacity');
      assert.equal(invoke(['inspect','--target',first.session]).mutations,4);
    });
    await t.test('navigation timeout reports unknown outcome without retrying mutation',async()=>{
      const started=Date.now();
      const result=invoke(['navigate','--target',first.session,'--projection','skills','--reference','slow'],2);
      assert.equal(result.code,'viewer.exchange');assert.match(result.message,/unknown/);
      assert.ok(Date.now()-started<6000);
      await new Promise(done=>setTimeout(done,400));
      assert.equal(invoke(['inspect','--target',first.session]).mutations,5);
    });
    await t.test('exact native IDs remain distinct from source references, without truncation',()=>{
      for(const id of ['effect:Ω 😀 "quoted" \\ native','x'.repeat(4095)]) {
        const result=invoke(['navigate','--target',first.session,'--projection','effects','--id',id]);
        assert.equal(result.id,id);assert.equal(result.reference,'');
      }
      assert.equal(invoke(['inspect','--target',first.session]).mutations,7);
    });
    await t.test('explicit source refresh has no discard/projection options',()=>{
      assert.equal(invoke(['refresh','--target',first.session]).reloaded,true);
      assert.equal(invoke(['inspect','--target',first.session]).mutations,8);
    });
    await t.test('named application action travels through strict command envelope',()=>{
      const result=invoke(['action','--target',first.session,'--projection','matrix','--action','structure.run']);
      assert.equal(result.executed,true);assert.equal(result.action,'structure.run');
      assert.equal(invoke(['inspect','--target',first.session]).mutations,9);
    });
    await t.test('configuration preserves empty/multiline UTF-8 values and opaque expected tokens',()=>{
      for(const text of ['','café Ω\nquoted "value"\r\n\ttab']) {
        const result=invoke(['configure','--target',first.session,'--projection','actors','--form','query','--control','values','--value',text,'--expect','fixture-token']);
        assert.equal(result.configured,true);assert.equal(result.value,text);assert.equal(result.expect,'fixture-token');
      }
      assert.equal(invoke(['inspect','--target',first.session]).mutations,11);
    });
    second=await owner();
    await t.test('multiple sessions require exact target',()=>{
      assert.notEqual(first.session,second.session);
      assert.equal(invoke(['inspect'],2).code,'viewer.ambiguous');
      assert.equal(invoke(['inspect','--target',second.session]).mutations,0);
    });
    await close(second);second=null;
    await t.test('closed endpoint fails, never auto-starts a viewer',()=>{
      assert.equal(invoke(['inspect','--target','4294967295-0000000000000000'],2).code,'viewer.exchange');
    });
  } finally {if(second)await close(second);await close(first);}
});
