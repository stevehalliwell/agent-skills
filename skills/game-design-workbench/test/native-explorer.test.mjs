// Unified entry contract; no window/input automation. State/render checks live
// in native/test-explorer.ps1. Existing native commands remain compatible.
import test from 'node:test';
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { resolve } from 'node:path';
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
    ['--nouns','one=.','--nouns','two=.'],['--related','context=.','--fields','nodes']]) {
    assert.equal(failure(args).errors[0].code,'cli.arguments',JSON.stringify(args));
  }
});
test('unified entry reports absent graph build or unavailable projection structurally', {skip:!executable},()=>{
  const output=failure(['--related','context=.']);
  assert.ok(['view.graph_unavailable','explorer.unavailable'].includes(output.errors[0].code));
  const help=spawnSync(executable,['--help'],{encoding:'utf8',timeout:30000});
  assert.equal(help.status,0);
  assert.match(help.stdout,/view matrix\|skill-atoms\|mda\|resources\|explorer/);
  assert.match(help.stdout,/No automatic folder discovery/);
});
