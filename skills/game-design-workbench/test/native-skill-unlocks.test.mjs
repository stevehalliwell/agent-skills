// Shared native skill exploration checks; no JS skill evaluator.
// GD_NATIVE_EXE=... node --test skills/game-design-workbench/test/native-skill-unlocks.test.mjs
import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtemp, mkdir, writeFile, readFile, rm } from 'node:fs/promises';
import { join, resolve } from 'node:path';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';
const executable=process.env.GD_NATIVE_EXE && resolve(process.env.GD_NATIVE_EXE);
function invoke(args, expected=0) {
  const result=spawnSync(executable,args,{encoding:'utf8',timeout:30000,maxBuffer:8*1024*1024,windowsHide:true});
  assert.equal(result.status,expected,`${result.stderr}\n${result.stdout}`);
  return JSON.parse(result.stdout);
}
test('explicit knowledge and rooted chains preserve AND semantics without inferring learning', {skip:!executable}, async()=>{
  const folder=await mkdtemp(join(tmpdir(),'skill-unlocks-'));
  try {
    const records={
      understanding:'prerequisites: []\nmechanics: [rules/impact]',
      jump:'prerequisites: []', fall:'prerequisites: []',
      jumping:'status: proposed\nprerequisites: [atoms/understanding, atoms/jump, atoms/jump]\nmechanics: [rules/impact]',
      falling:'prerequisites: [atoms/understanding, atoms/fall]\nmechanics: [rules/impact]',
      broken:'prerequisites: [atoms/absent]', cycle:'prerequisites: [atoms/cycle]',
      downstream:'prerequisites: [atoms/broken]'
    };
    for(const [name,text] of Object.entries(records)) await writeFile(join(folder,`${name}.md`),`---\n${text}\n---\nOriginal ${name}.\n`);
    const before=await readFile(join(folder,'jumping.md'));
    const args=['skill-atoms','--skill-atoms',`atoms=${folder}`,'--json'];
    let output=invoke([...args,'--known','atoms/understanding','--known','atoms/jump','--from','atoms/understanding','--fields','exploration']);
    assert.deepEqual(Object.keys(output.data),['exploration']);
    let rows=Object.fromEntries(output.data.exploration.skills.map(x=>[x.reference,x]));
    assert.equal(rows['atoms/jumping'].eligibility,'all_declared_prerequisites_supplied');
    assert.equal(rows['atoms/jumping'].supplied_count,2);
    assert.equal(rows['atoms/jumping'].known_assumption,false);
    assert.equal(rows['atoms/jumping'].unlocked_from_root,true);
    assert.deepEqual(rows['atoms/falling'].missing,['atoms/fall']);
    assert.equal(rows['atoms/understanding'].eligibility,'no_declared_prerequisites');
    for(const name of ['broken','cycle','downstream']) assert.equal(rows[`atoms/${name}`].eligibility,'unresolved');
    assert.ok(output.data.exploration.comparison_candidates.every(x=>x.shared_reference==='rules/impact' && x.root_line>0 && x.candidate_line>0));
    output=invoke([...args,'--known','atoms/jumping','--from','atoms/jumping']);
    rows=Object.fromEntries(output.data.exploration.skills.map(x=>[x.reference,x]));
    assert.equal(rows['atoms/jumping'].known_assumption,true);
    assert.equal(rows['atoms/jumping'].missing_count,2);
    assert.equal(rows['atoms/jump'].known_assumption,false);
    assert.equal(rows['atoms/understanding'].prerequisite_of_root,true);
    assert.equal(invoke([...args,'--known','atoms/absent'],2).errors[0].code,'skills.selector');
    assert.equal(invoke([...args,'--from','atoms/absent'],2).ok,false);
    assert.equal(invoke([...args,'--fields','bad'],2).errors[0].code,'cli.arguments');
    const repeated=Array.from({length:64},()=>['--known','atoms/jump']).flat();
    assert.equal(invoke([...args,...repeated]).data.exploration.skills.find(x=>x.reference==='atoms/jump').known_assumption,true);
    assert.equal(invoke([...args,...repeated,'--known','atoms/jump'],2).errors[0].code,'cli.arguments');
    const text=spawnSync(executable,[...args,'--format','text'],{encoding:'utf8',timeout:30000});
    // args uses --json: conflicting explicit formats must not be accepted.
    assert.equal(text.status,2);
    const human=spawnSync(executable,['skill-atoms','--skill-atoms',`atoms=${folder}`,'--from','atoms/understanding','--known','atoms/jump','--format','text'],{encoding:'utf8',timeout:30000});
    assert.equal(human.status,0,human.stderr);
    assert.match(human.stdout,/Hypothetical knowledge only/);
    assert.match(human.stdout,/Compare atoms\/jumping: shared mechanics rules\/impact/);
    assert.deepEqual(await readFile(join(folder,'jumping.md')),before);
    const empty=join(folder,'empty'); await mkdir(empty);
    assert.deepEqual(invoke(['skill-atoms','--skill-atoms',`atoms=${empty}`,'--json']).data.exploration.skills,[]);
  } finally { await rm(folder,{recursive:true,force:true}); }
});
test('live above-impact understanding unlocks separate methods without changing game rules', {skip:!executable},()=>{
  const game=resolve('skills/game-design-workbench/examples/little-plumber-guy-nes/game-design');
  const output=invoke(['skill-atoms','--skill-atoms',`skill-atoms=${game}/skill-atoms`,'--from','skill-atoms/stomp-walker','--known','skill-atoms/stomp-walker','--known','skill-atoms/land-on-target','--json']);
  const rows=Object.fromEntries(output.data.exploration.skills.map(x=>[x.reference,x]));
  assert.equal(rows['skill-atoms/jump-stomp'].eligibility,'all_declared_prerequisites_supplied');
  assert.equal(rows['skill-atoms/fall-stomp'].eligibility,'needs_more');
  assert.deepEqual(rows['skill-atoms/fall-stomp'].missing,['skill-atoms/control-fall']);
  assert.equal(rows['skill-atoms/jump'].known_assumption,false);
  assert.equal(rows['skill-atoms/stomp-walker'].eligibility,'no_declared_prerequisites');
});
