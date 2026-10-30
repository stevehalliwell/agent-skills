// Migration CLI is authoring-only; native C supplies declarations/semantic evidence.
// Optional read-only live checks: GD_NATIVE_EXE and GD_PRIMARY_MIGRATION_ATTENDANT=public-runner.mjs.
import test from 'node:test';
import assert from 'node:assert/strict';
import { readFile, readdir, writeFile, mkdtemp, rm, access, cp, mkdir, symlink, unlink } from 'node:fs/promises';
import { join, resolve, dirname, relative } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';

const home = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const root = resolve(home, '../..');
const script = join(root, 'scripts/migrate-workbench-primary-targets.mjs');
function invoke(args, expected = 0, nodeArgs = []) {
  const result = spawnSync(process.execPath, [...nodeArgs, script, ...args], { encoding: 'utf8', timeout: 120000,
    maxBuffer: 16 * 1024 * 1024, windowsHide: true, env: { ...process.env, GD_AGENT: '1' } });
  assert.equal(result.status, expected, result.stderr || result.error?.message || result.stdout);
  return result;
}
async function tree(path, prefix = '') {
  const result = {};
  for (const item of await readdir(join(path, prefix), { withFileTypes: true })) {
    if (item.name === '.attendant') continue;
    const name = join(prefix, item.name);
    if (item.isDirectory()) Object.assign(result, await tree(path, name));
    else result[name] = createHash('sha256').update(await readFile(join(path, name))).digest('hex');
  }
  return result;
}

test('primary-target migration CLI exposes help and structured noninteractive failures', () => {
  assert.match(invoke(['--help']).stdout, /Plan only by default/);
  for (const args of [[], ['--unknown'], ['--root']]) {
    const result = invoke(args, 2), report = JSON.parse(result.stdout);
    assert.equal(report.ok, false);
    assert.equal(report.partial, false);
    assert.deepEqual(report.changed, []);
    assert.equal(report.errors[0].code, 'cli.arguments');
    assert.match(result.stderr, /primary-target migration:/);
  }
});

test('primary-target migration plans are read-only and reject conflicting anchors/source-contained staging', {
  skip: !(process.env.GD_NATIVE_EXE && process.env.GD_PRIMARY_MIGRATION_ATTENDANT) && 'Set native executable and public migration Attendant runner',
}, async t => {
  const temporary = await mkdtemp(join(tmpdir(), 'gd-primary-migration-'));
  t.after(() => rm(temporary, { recursive: true, force: true }));
  const targets = join(temporary, 'targets.json');
  const deferred = { 'little-plumber-guy-nes': { 'mechanics/death-and-retry#later-game-end': null } };
  await writeFile(targets, JSON.stringify(deferred));
  const paths = ['crate-puzzle', 'little-plumber-guy-nes'].map(name => join(home, 'examples', name));
  const before = await Promise.all(paths.map(path => tree(path)));
  const args = ['--root', root, '--native', resolve(process.env.GD_NATIVE_EXE),
    '--attendant', resolve(process.env.GD_PRIMARY_MIGRATION_ATTENDANT), '--targets', targets, '--json'];
  const result = invoke([...args, '--fields', 'summary']);
  assert.equal(result.stderr, '');
  const plan = JSON.parse(result.stdout);
  assert.deepEqual(Object.keys(plan), ['ok', 'summary']);
  assert.equal(plan.summary.mode, 'plan');
  assert.equal(plan.summary.changed, 0);
  assert.equal(plan.summary.proposed_files, 0);
  assert.equal(plan.summary.cases, 180);
  assert.equal(plan.summary.native_equivalence, false); // No false claim that plan ran staging checks.
  assert.deepEqual(plan.summary.deferred, ['crate-puzzle/mechanics/challenge-restart#restart',
    'crate-puzzle/mechanics/wall-click#proposal-only', 'little-plumber-guy-nes/mechanics/death-and-retry#later-game-end']);
  const unsafe = join(paths[0], 'unsafe-primary-migration-stage');
  await assert.rejects(access(unsafe));
  const containment = JSON.parse(invoke([...args, '--stage', unsafe], 2).stdout);
  assert.equal(containment.errors[0].code, 'migration.containment');
  assert.equal(containment.partial, false);
  await assert.rejects(access(unsafe));
  await writeFile(targets, JSON.stringify({ 'little-plumber-guy-nes': {
    ...deferred['little-plumber-guy-nes'], 'interactions/shell-meets-shell#two-sliding-shells': 'Second',
  } }));
  const conflict = JSON.parse(invoke(args, 2).stdout);
  assert.equal(conflict.errors[0].code, 'migration.stale_target');
  assert.equal(conflict.partial, false);
  assert.deepEqual(conflict.changed, []);
  assert.deepEqual(await Promise.all(paths.map(path => tree(path))), before);
});

test('primary-target migration guards canonical destinations and owns failed temporary writes', {
  skip: !(process.env.GD_NATIVE_EXE && process.env.GD_PRIMARY_MIGRATION_ATTENDANT) && 'Set native executable and public migration Attendant runner',
}, async t => {
  const temporary = await mkdtemp(join(tmpdir(), 'gd-primary-failures-'));
  t.after(() => rm(temporary, { recursive: true, force: true }));
  const copyRoot = join(temporary, 'repo'), base = join(copyRoot, 'skills/game-design-workbench/examples');
  await mkdir(base, { recursive: true });
  const paths = ['crate-puzzle', 'little-plumber-guy-nes'].map(name => join(base, name));
  for (const path of paths) await cp(join(home, 'examples', path.endsWith('crate-puzzle') ? 'crate-puzzle' : 'little-plumber-guy-nes'), path,
    { recursive: true, filter: source => !source.endsWith('.attendant') });
  // Historical source fixture deliberately lacks A; compatibility/migration evidence, not live authoring guidance.
  const legacy = await readFile(join(home, 'test/fixtures/legacy-primary-target.md'));
  const record = join(paths[0], 'game-design/interactions/occupied-switch.md');
  await writeFile(record, legacy);
  const targets = join(temporary, 'targets.json');
  await writeFile(targets, JSON.stringify({ 'little-plumber-guy-nes': { 'mechanics/death-and-retry#later-game-end': null } }));
  const args = ['--root', copyRoot, '--native', resolve(process.env.GD_NATIVE_EXE),
    '--attendant', resolve(process.env.GD_PRIMARY_MIGRATION_ATTENDANT), '--targets', targets, '--json'];
  const before = await Promise.all(paths.map(path => tree(path)));
  await t.test('junctions into source fail before any destination is created', async context => {
    const alias = join(temporary, 'external-stage-parent');
    try { await symlink(paths[0], alias, 'junction'); }
    catch (error) {
      if (['EPERM', 'EACCES', 'ENOSYS'].includes(error.code)) { context.skip(`Directory links unavailable: ${error.code}`); return; }
      throw error;
    }
    const report = JSON.parse(invoke([...args, '--stage', join(alias, 'unexpected-stage')], 2).stdout);
    assert.equal(report.errors[0].code, 'migration.containment');
    assert.equal(report.partial, false);
    await assert.rejects(access(join(paths[0], 'unexpected-stage')));
    const broken = join(temporary, 'unresolved-stage-parent');
    const absentTarget = join(paths[0], 'absent-stage-parent');
    await symlink(absentTarget, broken, 'junction');
    const unresolved = JSON.parse(invoke([...args, '--stage', join(broken, 'unexpected-stage')], 2).stdout);
    assert.equal(unresolved.errors[0].code, 'migration.containment');
    await assert.rejects(access(absentTarget));
    const defaultParent = join(copyRoot, 'skills/game-design-workbench/native/build');
    await mkdir(dirname(defaultParent), { recursive: true });
    await symlink(paths[0], defaultParent, 'junction');
    const automatic = JSON.parse(invoke([...args, '--apply'], 2).stdout);
    assert.equal(automatic.errors[0].code, 'migration.containment');
    assert.equal(automatic.partial, false);
    await unlink(defaultParent);
    assert.deepEqual(await Promise.all(paths.map(path => tree(path))), before);
  });
  await t.test('partial write removes only owned temporary file and preserves source bytes', async () => {
    const preload = join(temporary, 'partial-write.mjs');
    await writeFile(preload, `import fs from 'node:fs/promises';
import { syncBuiltinESMExports } from 'node:module';
const acquire=fs.open;
fs.open=async function(path,...args) {
  const handle=await acquire(path,...args);
  if (String(path).includes('.primary-') && String(path).endsWith('.tmp')) {
    const write=handle.writeFile.bind(handle);
    handle.writeFile=async function() { await write('partial'); throw Object.assign(new Error('Injected partial write'),{code:'EIO'}); };
  }
  return handle;
};
syncBuiltinESMExports();\n`);
    const report = JSON.parse(invoke([...args, '--apply'], 2, ['--import', pathToFileURL(preload).href]).stdout);
    assert.equal(report.errors[0].code, 'EIO');
    assert.equal(report.partial, false);
    assert.deepEqual(report.changed, []);
    assert.deepEqual(report.temporary_files, []);
    assert.ok(report.stage);
    assert.deepEqual(await Promise.all(paths.map(path => tree(path))), before);
  });
  await t.test('exclusive-open collision never deletes another writers file', async () => {
    const preload = join(temporary, 'collision.mjs');
    await writeFile(preload, `import fs from 'node:fs/promises';
import { syncBuiltinESMExports } from 'node:module';
const acquire=fs.open;
fs.open=async function(path,...args) {
  if (String(path).includes('.primary-') && String(path).endsWith('.tmp')) await fs.writeFile(path,'foreign sentinel',{flag:'wx'});
  return acquire(path,...args);
};
syncBuiltinESMExports();\n`);
    const report = JSON.parse(invoke([...args, '--apply'], 2, ['--import', pathToFileURL(preload).href]).stdout);
    assert.equal(report.errors[0].code, 'EEXIST');
    assert.equal(report.partial, false);
    assert.deepEqual(report.changed, []);
    assert.deepEqual(report.temporary_files, []);
    const names = await readdir(dirname(record));
    const foreign = names.filter(name => name.startsWith('occupied-switch.md.primary-') && name.endsWith('.tmp'));
    assert.equal(foreign.length, 1);
    const path = join(dirname(record), foreign[0]);
    assert.equal(await readFile(path, 'utf8'), 'foreign sentinel');
    await unlink(path); // Test owns injected sentinel; migration does not.
    assert.deepEqual(await Promise.all(paths.map(path => tree(path))), before);
  });
  await t.test('semantic changes still fail equivalence before apply', async () => {
    const preload=join(temporary,'semantic-change.mjs');
    await writeFile(preload,`import childProcess from 'node:child_process';
import { syncBuiltinESMExports } from 'node:module';
const execute=childProcess.spawnSync;
childProcess.spawnSync=function(command,args,...rest) {
  const result=execute(command,args,...rest);
  if (result.status===0 && args[0]==='resources' && args.some(value=>String(value).includes('primary-target-stage-'))) {
    const report=JSON.parse(result.stdout);
    report.data.relations.find(row=>row.kind==='quantity_write').expression='injected changed expression';
    result.stdout=JSON.stringify(report);
  }
  return result;
};
syncBuiltinESMExports();\n`);
    const report=JSON.parse(invoke([...args,'--apply'],2,['--import',pathToFileURL(preload).href]).stdout);
    assert.equal(report.errors[0].code,'migration.equivalence'); assert.equal(report.partial,false);
    assert.deepEqual(report.changed,[]); assert.deepEqual(await Promise.all(paths.map(path=>tree(path))),before);
  });
  await t.test('successful staged replacement preserves native facts and reruns without changes', async () => {
    // Synthetic mapped caller exercises target-derived instance labels; historical fixture stays unchanged.
    const caller='case mapped-caller:\n    target: Switch\n    context: ANY\n    Crate:\n        noun: nouns/crate\n        requires: ANY\n    Switch:\n        noun: nouns/switch\n        requires: ANY\n        causes: interactions/occupied-switch#occupied with Crate=Crate, Switch=Switch, Exit=Exit\n    Exit:\n        noun: nouns/exit\n        requires: ANY\n';
    await writeFile(record,legacy.toString('utf8').replace('```\n',caller+'```\n'));
    const report = JSON.parse(invoke([...args, '--apply']).stdout);
    assert.equal(report.summary.changed, 1);
    assert.equal(report.summary.proposed_files, 1);
    assert.equal(report.summary.native_equivalence, true);
    const text = await readFile(record, 'utf8');
    assert.match(text, /case occupied:\n    target: Switch\n    becomes: occupied/);
    const expected = structuredClone(before);
    expected[0][relative(paths[0], record)] = createHash('sha256').update(text).digest('hex');
    assert.deepEqual(await Promise.all(paths.map(path => tree(path))), expected);
    const repeated = JSON.parse(invoke([...args, '--apply']).stdout);
    assert.equal(repeated.summary.changed, 0);
    assert.equal(repeated.summary.proposed_files, 0);
    assert.equal(repeated.summary.native_equivalence, true);
    assert.deepEqual(await Promise.all(paths.map(path => tree(path))), expected);
  });
});
