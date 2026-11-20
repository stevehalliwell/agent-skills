// Native relationship coverage; no JS graph evaluator.
// GD_NATIVE_EXE=... node --test skills/game-design-workbench/test/native-graph-inputs.test.mjs
import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtemp, mkdir, writeFile, readFile, rm } from 'node:fs/promises';
import { join, resolve } from 'node:path';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';
const executable=process.env.GD_NATIVE_EXE && resolve(process.env.GD_NATIVE_EXE);
function invoke(args,expected=0) {
  const result=spawnSync(executable,args,{encoding:'utf8',timeout:30000,maxBuffer:8*1024*1024,windowsHide:true});
  assert.equal(result.status,expected,`${result.stderr}\n${result.stdout}`);
  return JSON.parse(result.stdout);
}
test('challenge context links retain typed declarations and source lines without becoming route demands', {skip:!executable}, async()=>{
  const root=await mkdtemp(join(tmpdir(),'graph-inputs-'));
  try {
    const names=['skill-atoms','challenges','mechanics','interactions','nouns','verbs','resources','control-vars'];
    for(const name of names) await mkdir(join(root,name));
    await writeFile(join(root,'skill-atoms/jump.md'),'---\nstatus: proposed\n---\nJump.\n');
    const source='---\nstatus: rejected\nentry: true\nrequires: [skill-atoms/jump]\nmechanics:\n  - mechanics/rule\n  - mechanics/rule\ninteractions: [interactions/contact]\nnouns: [nouns/hero]\nverbs: [verbs/jump]\nresources: [resources/energy]\n---\nUnchanged body.\n';
    const path=join(root,'challenges/encounter.md'); await writeFile(path,source);
    for(const [name,body] of Object.entries({
      'mechanics/rule':'mechanics: [mechanics/rule]',
      'interactions/contact':'', 'nouns/hero':'', 'verbs/jump':'',
      'resources/energy':'noun: nouns/hero\ninitial_control: control-vars/start\ncapacity_control: control-vars/limit',
      'control-vars/start':'', 'control-vars/limit':''
    })) await writeFile(join(root,`${name}.md`),`---\n${body || 'status: accepted'}\n---\nContext.\n`);
    const args=['scaffolding','--skill-atoms',`skill-atoms=${root}/skill-atoms`,'--challenges',`challenges=${root}/challenges`,'--json'];
    const bare=invoke(args), challenge=bare.data.challenges[0];
    assert.equal(challenge.status,'rejected');
    assert.deepEqual(challenge.relationships.map(x=>x.field),['requires','mechanics','mechanics','interactions','nouns','verbs','resources']);
    assert.deepEqual(challenge.relationships.map(x=>x.line),[4,6,7,8,9,10,11]);
    assert.equal(challenge.relationships[0].loaded,true);
    assert.ok(challenge.relationships.slice(1).every(x=>!x.loaded && !x.ambiguous));
    assert.equal(bare.coverage.routes_complete,true);
    assert.equal(bare.coverage.requirements_complete,true);
    assert.equal(bare.data.checks.length,1);
    const related=names.slice(2).flatMap(name=>['--related',`${name}=${root}/${name}`]);
    const loaded=invoke([...args,...related]);
    assert.ok(loaded.data.challenges[0].relationships.every(x=>x.loaded));
    assert.deepEqual(loaded.data.checks,bare.data.checks);
    assert.deepEqual(loaded.data.witnesses,bare.data.witnesses);
    assert.equal(await readFile(path,'utf8'),source);
    for(const field of ['mechanics','interactions','nouns','verbs','resources']) {
      await writeFile(path,`---\nentry: true\n${field}: null\n---\nNull.\n`);
      assert.deepEqual(invoke(args).data.challenges[0].relationships,[]);
      for(const value of ['a/reference','42','[false]','[""]','[null]','{}']) {
        await writeFile(path,`---\nentry: true\n${field}: ${value}\n---\nInvalid.\n`);
        const invalid=invoke(args,2);
        assert.equal(invalid.ok,false);
        assert.equal(invalid.data.challenges.length,0,'failed record must roll back');
      }
      await writeFile(path,`---\n${field}: []\n${field}: []\n---\nDuplicate.\n`);
      assert.ok(invoke(args,2).errors.some(x=>x.code==='input.duplicate_field'));
    }
    // Resource links are scalar references even when loaded through --related.
    const resource=join(root,'resources/energy.md');
    for(const field of ['noun','initial_control','capacity_control']) {
      for(const value of ['[]','{}','true','42','""','[control-vars/start]']) {
        await writeFile(resource,`---\n${field}: ${value}\n---\nInvalid resource.\n`);
        const invalid=invoke(['skill-atoms','--skill-atoms',`skill-atoms=${root}/skill-atoms`,'--related',`resources=${root}/resources`,'--json'],2);
        assert.ok(invalid.errors.length);
      }
      await writeFile(resource,`---\n${field}: null\n---\nUnspecified.\n`);
      invoke(['skill-atoms','--skill-atoms',`skill-atoms=${root}/skill-atoms`,'--related',`resources=${root}/resources`,'--json']);
      await writeFile(resource,`---\n${field}: control-vars/start\n${field}: control-vars/start\n---\nDuplicate.\n`);
      assert.ok(invoke(['skill-atoms','--skill-atoms',`skill-atoms=${root}/skill-atoms`,'--related',`resources=${root}/resources`,'--json'],2).errors.some(x=>x.code==='input.duplicate_field'));
    }
  } finally { await rm(root,{recursive:true,force:true}); }
});
