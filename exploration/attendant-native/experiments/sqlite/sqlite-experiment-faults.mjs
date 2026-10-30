// SQL rollback and deterministic process-kill checks for disposable-cache variants.
// Usage: node sqlite-experiment-faults.mjs <capture-manifest.json>
// Only owned build/ copies; hooks flush dirty pages then signal before commit.
// This tests process death at selected points, NOT OS crash or power loss.
import assert from 'node:assert/strict';
import { spawn, spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { cpSync, existsSync, mkdirSync, mkdtempSync, readFileSync, writeFileSync, utimesSync, rmSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { setTimeout as delay } from 'node:timers/promises';
import { DatabaseSync } from 'node:sqlite';
import { profiles } from './sqlite-experiment-source.mjs';
import { inventory } from '../../tools/dataset-files.mjs';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
if (process.argv[2] === '--help') { console.log('Usage: node sqlite-experiment-faults.mjs <manifest>; SQL rollback and selected process-kill tests in owned build/ copies, not power-loss tests'); process.exit(0); }
assert.ok(process.argv[2], 'Usage: sqlite-experiment-faults.mjs <manifest>');
const manifest = JSON.parse(readFileSync(resolve(process.argv[2]), 'utf8'));
const pristine = manifest.datasets[0].pristine, originalInventory = inventory(pristine);
mkdirSync(join(root, 'build/sqlite-faults'), { recursive: true });
const results = [];
for (const profile of profiles) {
  const executable = join(root, `build/projection-zig-experiment-${profile}.exe`);
  const directory = mkdtempSync(join(root, 'build/sqlite-faults/check-')), work = join(directory, 'work');
  cpSync(pristine, work, { recursive: true });
  assert.deepEqual(inventory(work), originalInventory, 'Copied config escapes owned workspace');
  const database = join(work, '.attendant/attendant.sqlite');
  const cases = [];
  function run(args, status = 0) {
    const result = spawnSync(executable, [...args, '-p', work], { encoding: 'utf8', timeout: 30_000, env: { ...process.env, ATTENDANT_EXPERIMENT_PHASE: '', ATTENDANT_EXPERIMENT_SIGNAL: '' } });
    assert.equal(result.status, status, result.stderr + result.stdout);
    return JSON.parse(result.stdout);
  }
  function inspect(action) { const db = new DatabaseSync(database); try { return action(db); } finally { db.close(); } }
  async function killAt(phase, args) {
    const signal = join(directory, 'pause.signal'); rmSync(signal, { force: true });
    const child = spawn(executable, args, { stdio: ['ignore', 'pipe', 'pipe'], env: { ...process.env, ATTENDANT_EXPERIMENT_PHASE: phase, ATTENDANT_EXPERIMENT_SIGNAL: signal } });
    let output = '', error;
    child.stdout.on('data', bytes => { output += bytes; }); child.stderr.on('data', bytes => { output += bytes; });
    const closed = new Promise(resolve => { child.once('error', value => { error = value; resolve(); }); child.once('close', resolve); });
    try {
      const deadline = Date.now() + 20_000;
      while (Date.now() < deadline && child.exitCode === null && !error) {
        if (existsSync(signal) && readFileSync(signal, 'utf8') === phase) break;
        await delay(20);
      }
      assert.ok(existsSync(signal), `Missing ${phase} pause: ${error?.message ?? output}`);
      assert.equal(readFileSync(signal, 'utf8'), phase);
      assert.ok(child.kill(), 'Could not terminate owned child');
      await closed;
      assert.notEqual(child.exitCode, 0, 'Killed command reported success');
    } finally { if (child.exitCode === null && child.signalCode === null) child.kill(); await closed; }
  }
  run(['sync']); run(['search', '-q', 'task']);
  const target = inspect(db => ({ ...db.prepare('SELECT id,source_path,source_hash FROM tasks ORDER BY name LIMIT 1').get() }));
  const original = readFileSync(target.source_path), edited = Buffer.concat([original, Buffer.from('\nexperimentfaultprobe\n')]);
  const editedHash = createHash('sha256').update(edited).digest('hex');
  const oldRow = inspect(db => ({ ...db.prepare('SELECT * FROM tasks WHERE id=?').get(target.id) }));
  inspect(db => db.exec("CREATE TRIGGER experiment_fault BEFORE INSERT ON tasks BEGIN SELECT RAISE(ABORT,'update-fault'); END"));
  writeFileSync(target.source_path, edited); utimesSync(target.source_path, new Date('2026-01-02'), new Date('2026-01-02'));
  assert.match(run(['sync'], 1).error.message, /update-fault/);
  inspect(db => {
    assert.deepEqual({ ...db.prepare('SELECT * FROM tasks WHERE id=?').get(target.id) }, oldRow);
    assert.equal(db.prepare('SELECT source_hash FROM __attendant_cache WHERE source_path=?').get(target.source_path).source_hash, target.source_hash);
    assert.equal(db.prepare('PRAGMA quick_check').get().quick_check, 'ok');
    assert.equal(db.prepare("SELECT count(*) n FROM __attendant_fts_tasks WHERE __attendant_fts_tasks MATCH 'experimentfaultprobe'").get().n, 0);
    db.exec('DROP TRIGGER experiment_fault');
  });
  assert.deepEqual(readFileSync(target.source_path), edited);
  cases.push('mid-delta SQL failure rolls back row/cache/search; authored edit preserved');
  if (profile === 'memory') {
    const reader = new DatabaseSync(database);
    try {
      reader.exec('BEGIN'); reader.prepare('SELECT count(*) FROM tasks').get();
      assert.match(run(['sync'], 1).error.message, /busy|locked/i);
      assert.equal(reader.prepare('SELECT source_hash FROM tasks WHERE id=?').get(target.id).source_hash, target.source_hash);
      reader.exec('ROLLBACK');
    } finally { reader.close(); }
    cases.push('held rollback-journal reader blocks writer with bounded structured error');
  }
  if (['control', 'wal-off', 'bulk-off', 'memory'].includes(profile)) {
    // Source differs from old DB. Kill after changed pages flush but before commit.
    await killAt('delta', ['sync', '-p', work]);
    run(['sync']);
    inspect(db => {
      assert.equal(db.prepare('PRAGMA quick_check').get().quick_check, 'ok');
      assert.equal(db.prepare('SELECT source_hash FROM tasks WHERE id=?').get(target.id).source_hash, editedHash);
    });
    assert.deepEqual(run(['search', '-q', 'experimentfaultprobe']).rows.map(row => row.id), [target.id]);
    cases.push('killed flushed incremental write repairs/reuses complete current projection');
    const oldMain = createHash('sha256').update(readFileSync(database)).digest('hex');
    await killAt('build', [work]); // Standalone project-path entry forces private full rebuild.
    assert.equal(createHash('sha256').update(readFileSync(database)).digest('hex'), oldMain, 'Killed builder replaced published DB');
    run(['sync']);
    cases.push('killed private builder leaves published DB untouched');
  }
  if (profile === 'lazy' || profile === 'lazy-wal-off') {
    run(['sync']); // Invalidate old indexes for edited source.
    await killAt('search', ['search', '-q', 'experimentfaultprobe', '-p', work]);
    run(['sync']);
    inspect(db => {
      assert.equal(db.prepare("SELECT value FROM __attendant_metadata WHERE key='search_ready'").get().value, '0');
      assert.equal(db.prepare("SELECT count(*) n FROM sqlite_master WHERE name GLOB '__attendant_fts_*' OR name GLOB '__attendant_trigram_*'").get().n, 0);
    });
    assert.deepEqual(run(['search', '-q', 'experimentfaultprobe']).rows.map(row => row.id), [target.id]);
    cases.push('killed lazy population leaves absent indexes and rebuilds on next search');
  }
  writeFileSync(target.source_path, original);
  assert.deepEqual(inventory(work), originalInventory);
  assert.deepEqual(inventory(pristine), originalInventory);
  results.push({ profile, directory, cases });
  console.log(JSON.stringify(results.at(-1)));
}
writeFileSync(join(root, 'build/sqlite-experiment-faults.json'), JSON.stringify({ complete: true, results, limitation: 'Selected process-kill points, not power-loss/crash-consistency qualification' }, null, 2) + '\n');
