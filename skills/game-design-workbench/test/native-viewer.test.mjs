// Headless Windows IPC only. Build explicitly with CJIT; never starts a GUI.
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
function raw(session,request,delay=0) {
  return new Promise((done,fail)=>{
    const socket=createConnection('\\\\.\\pipe\\gd-workbench-'+session); let text='';
    socket.setTimeout(4000,()=>socket.destroy(new Error('Raw request timed out')));
    socket.on('error',fail);
    socket.on('connect',()=>{socket.write(request);if(delay){socket.pause();setTimeout(()=>socket.resume(),delay);}});
    socket.on('data',chunk=>{
      text+=chunk;
      let result;try {result=JSON.parse(text);}catch{return;}
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
test('production explorer IPC adapter: CPU-only, both examples unchanged',{skip:process.platform!=='win32'||!adapter},async t=>{
  for(const name of ['crate-puzzle','little-plumber-guy-nes'])await t.test(name,async()=>{
    const root=join(examples,name,'game-design'),before=await sourceHashes(root);
    const result=spawnSync(adapter,['--ipc-helpers',root],{encoding:'utf8',windowsHide:true,timeout:30000,maxBuffer:2*1024*1024});
    assert.equal(result.status,0,result.stderr+'\n'+result.stdout);
    const value=JSON.parse(result.stdout);assert.equal(value.ok,true);assert.ok(value.checks>=180);assert.ok(value.control_checks>=500);assert.ok(value.feedback_checks>=10);
    const snapshot=value.snapshot;assert.equal(snapshot.ok,true);assert.equal(snapshot.projection,'matrix');
    assert.equal(snapshot.panes.length,8);assert.equal(snapshot.draft.modified_records,0);assert.equal(snapshot.draft.undo_steps,0);
    assert.equal(snapshot.panes.find(pane=>pane.projection==='actors').query.pending.values,'unapplied Ω');
    assert.notEqual(snapshot.panes.find(pane=>pane.projection==='actors').query.applied?.values,'unapplied Ω');
    const rules=snapshot.panes.find(pane=>pane.projection==='resources').rules;
    assert.equal(rules.pending.bindings,true);assert.equal(rules.applied.bindings,true);
    assert.deepEqual(rules.pending,rules.applied);
    assert.ok(snapshot.snapshots.every(family=>family.collections.every(collection=>collection.alias&&collection.directory)));
    assert.equal(snapshot.limits.source_writes,false);assert.equal(snapshot.limits.derived_navigation,true);assert.equal(snapshot.limits.application_controls,false);assert.equal(snapshot.limits.application_actions,true);assert.equal(snapshot.limits.source_refresh,true);
    assert.equal('source' in snapshot,false);
    assert.equal(snapshot.agent_hint.stale,true);assert.equal(snapshot.reload_generation,'2');assert.equal(typeof snapshot.agent_hint.id,'string');
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
test('viewer protocol and native local request/reply without any GUI',{skip:process.platform!=='win32'||!cli||!fixture},async t=>{
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
        const socket=createConnection('\\\\.\\pipe\\gd-workbench-'+first.session);
        socket.on('error',fail);socket.on('connect',()=>{socket.destroy();setTimeout(done,30);});
      });
      assert.equal(invoke(['inspect','--target',first.session]).mutations,2);
    });
    await t.test('invalid CLI combinations fail as JSON without dispatch',()=>{
      for(const arguments_ of [[],['unknown'],['hint','--projection','skills'],['inspect','--projection','skills'],['inspect','--target','../bad'],['list','--reference','x'],['inspect','--target',first.session,'--target',first.session],['nodes','--limit','0'],['nodes','--limit','65'],['nodes','--limit','1.5'],['nodes','--offset','-1'],['nodes','--offset','4097'],['nodes','--offset','01'],['inspect','--limit','1'],['nodes','--id','x'],['refresh','--projection','skills'],['actions','--limit','1'],['action','--projection','actors'],['action','--action','query.run'],['inspect','--action','query.run'],['hint','--projection','effects','--reference','x','--id','y'],['navigate','--projection','actors','--id','x'.repeat(4096)]]) {
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
