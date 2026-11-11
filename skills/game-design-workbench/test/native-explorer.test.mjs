// Unified entry contract; no window/input automation. State/render checks live
// in native/test-explorer.ps1. Working-branch command compatibility not required.
import test from 'node:test';
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { resolve, join } from 'node:path';
import { fileURLToPath } from 'node:url';
const executable=process.env.GD_NATIVE_EXE && resolve(process.env.GD_NATIVE_EXE);
function failure(args) {
  const result=spawnSync(executable,['view','explorer',...args,'--json'],{encoding:'utf8',timeout:30000,windowsHide:true});
  assert.equal(result.status,2,result.stderr);
  const output=JSON.parse(result.stdout);
  assert.equal(output.ok,false);
  assert.equal(output.coverage.complete,false);
  assert.ok(output.errors.length);
  return output;
}
test('unified explorer validates options without prompting or opening a window', {skip:!executable},()=>{
  for(const args of [[],['--frames','0'],['--mode','unknown'],['--mode','skills','--mode','unknown'],
    ['--related','context=.','--from','context/item'],['--related','context=.','--known','context/item'],
    ['--related','context=.','--screenshot','capture.png'],['--related','context=.','--frames','1','--screenshot','capture.jpg'],
    ['--nouns','one=.','--nouns','two=.'],['--related','context=.','--fields','nodes'],
    ['--related','context=.','--layout','grid'],['--related','context=.','--mode','mda','--layout','grid'],
    ['--related','context=.','--mode','matrix','--layout','table'],
    ['--related','context=.','--mode','noun-interactions','--layout','columns']]) {
    assert.equal(failure(args).errors[0].code,'cli.arguments',JSON.stringify(args));
  }
});
test('standalone graph layouts accept Free mode without changing native defaults', {skip:!executable},()=>{
  const root=fileURLToPath(new URL('../examples/crate-puzzle/game-design/',import.meta.url));
  const inputs=['--skill-atoms',`skill-atoms=${join(root,'skill-atoms')}`,'--challenges',`challenges=${join(root,'challenges')}`];
  for(const mode of ['skills','challenges','relationships'])for(const layout of [null,'prerequisites','rank','graph','table','columns']){
    const args=['view','skill-atoms',...inputs,'--mode',mode,'--json'];if(layout)args.push('--layout',layout);
    const result=spawnSync(executable,args,{encoding:'utf8',timeout:30000,windowsHide:true});
    assert.equal(result.status,2,result.stderr);const output=JSON.parse(result.stdout);
    assert.equal(output.errors[0].code,['table','columns'].includes(layout)?'cli.arguments':'view.unavailable',`${mode}/${layout}: ${result.stdout}`);
  }
});
test('unified entry reports absent graph build or unavailable projection structurally', {skip:!executable},()=>{
  const output=failure(['--related','context=.']);
  assert.ok(['view.graph_unavailable','explorer.unavailable'].includes(output.errors[0].code));
  for (const layout of ['grid','graph']) assert.ok(['view.graph_unavailable','explorer.unavailable'].includes(failure(['--related','context=.','--mode','noun-interactions','--layout',layout]).errors[0].code));
  for (const mode of ['actors','effects']) assert.ok(['view.graph_unavailable','explorer.unavailable'].includes(failure(['--related','context=.','--mode',mode]).errors[0].code));
  const help=spawnSync(executable,['--help'],{encoding:'utf8',timeout:30000});
  assert.equal(help.status,0);
  assert.match(help.stdout,/view matrix\|skill-atoms\|mda\|resources\|explorer/);
  assert.match(help.stdout,/No automatic folder discovery/);
  assert.match(help.stdout,/--layout grid\|graph\|columns/);
  assert.match(help.stdout,/resources\|actors\|effects/);
});
