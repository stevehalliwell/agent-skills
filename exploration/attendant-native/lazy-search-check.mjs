// Focused experimental lazy-search lifecycle/rollback/reader checks.
// Usage: node lazy-search-check.mjs <capture-manifest.json> [lazy-executable ...]
// Only new owned copies below build/; no live or pristine source mutation.
import assert from 'node:assert/strict';
import { cpSync, mkdirSync, mkdtempSync, readFileSync, writeFileSync, utimesSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';
import { DatabaseSync } from 'node:sqlite';
import { inventory } from './dataset-files.mjs';
const root = dirname(fileURLToPath(import.meta.url));
if (process.argv[2] === '--help') { console.log('Usage: node lazy-search-check.mjs <manifest> [lazy-executable ...]; owns new build/ copies only'); process.exit(0); }
assert.ok(process.argv[2], 'Usage: lazy-search-check.mjs <manifest> [lazy-executable ...]');
const manifest = JSON.parse(readFileSync(resolve(process.argv[2]), 'utf8'));
const executables = process.argv.slice(3);
if (!executables.length) executables.push(...['lazy', 'lazy-wal-off'].map(name => join(root, `build/projection-zig-experiment-${name}.exe`)));
mkdirSync(join(root, 'build/lazy-search'), { recursive: true });
for (const executable of executables) {
  const directory = mkdtempSync(join(root, 'build/lazy-search/check-')), work = join(directory, 'work');
  const pristine = manifest.datasets[0].pristine, original = inventory(pristine);
  cpSync(pristine, work, { recursive: true });
  assert.deepEqual(inventory(work), original, 'Copied config must stay inside owned workspace');
  const database = join(work, '.attendant/attendant.sqlite');
  function run(args, status = 0) {
    const result = spawnSync(resolve(executable), [...args, '-p', work], { encoding: 'utf8', timeout: 30_000 });
    assert.equal(result.status, status, result.stderr + result.stdout);
    return JSON.parse(result.stdout);
  }
  function inspect(action) { const db = new DatabaseSync(database); try { return action(db); } finally { db.close(); } }
  function absent() { inspect(db => {
    assert.equal(db.prepare("SELECT value FROM __attendant_metadata WHERE key='search_ready'").get().value, '0');
    assert.equal(db.prepare("SELECT count(*) n FROM sqlite_master WHERE name GLOB '__attendant_fts_*' OR name GLOB '__attendant_trigram_*'").get().n, 0);
    assert.equal(db.prepare('PRAGMA quick_check').get().quick_check, 'ok');
  }); }
  assert.equal(run(['sync']).state, 'rebuilt'); absent();
  assert.ok(run(['query', '-s', 'SELECT name FROM tasks LIMIT 1']).returned > 0); absent();
  assert.equal(run(['doctor']).projection, 'fresh'); absent();
  inspect(db => db.exec("CREATE TRIGGER lazy_fault BEFORE UPDATE OF value ON __attendant_metadata WHEN NEW.key='search_ready' AND NEW.value='1' BEGIN SELECT RAISE(ABORT,'lazy-fault'); END"));
  assert.match(run(['search', '-q', 'task'], 1).error.message, /lazy-fault/); absent();
  inspect(db => db.exec('DROP TRIGGER lazy_fault'));
  assert.ok(run(['search', '-q', 'task']).returned > 0);
  inspect(db => assert.equal(db.prepare("SELECT value FROM __attendant_metadata WHERE key='search_ready'").get().value, '1'));
  const target = inspect(db => ({ ...db.prepare('SELECT id,source_path FROM tasks ORDER BY name LIMIT 1').get() }));
  const before = readFileSync(target.source_path), marker = 'lazylifecycleprobe';
  const reader = new DatabaseSync(database);
  try {
    reader.exec('BEGIN');
    assert.equal(reader.prepare('SELECT count(*) n FROM __attendant_fts_tasks WHERE __attendant_fts_tasks MATCH ?').get(marker).n, 0);
    writeFileSync(target.source_path, Buffer.concat([before, Buffer.from('\n' + marker + '\n')]));
    utimesSync(target.source_path, new Date('2026-01-02'), new Date('2026-01-02'));
    assert.equal(run(['sync']).state, 'updated'); absent();
    assert.deepEqual(run(['search', '-q', marker]).rows.map(row => row.id), [target.id]);
    assert.equal(reader.prepare('SELECT count(*) n FROM __attendant_fts_tasks WHERE __attendant_fts_tasks MATCH ?').get(marker).n, 0, 'Old reader snapshot changed');
    reader.exec('ROLLBACK');
  } finally { reader.close(); }
  assert.deepEqual(run(['search', '-q', marker]).rows.map(row => row.id), [target.id]);
  assert.equal(run(['sync']).state, 'reused');
  inspect(db => assert.equal(db.prepare("SELECT value FROM __attendant_metadata WHERE key='search_ready'").get().value, '1'));
  utimesSync(target.source_path, new Date('2026-01-03'), new Date('2026-01-03'));
  assert.equal(run(['sync']).state, 'reused');
  inspect(db => assert.equal(db.prepare("SELECT value FROM __attendant_metadata WHERE key='search_ready'").get().value, '1', 'Mtime-only change invalidated search'));
  writeFileSync(target.source_path, before);
  assert.deepEqual(inventory(work), original, 'Source bytes changed beyond deliberate test edit');
  assert.deepEqual(inventory(pristine), original, 'Pristine source changed');
  console.log(JSON.stringify({ executable, directory, cases: ['SQL-only DB has no search tables', 'doctor accepts unbuilt indexes', 'failed population rolls back all FTS and readiness', 'first search populates', 'source edit invalidates both indexes', 'held reader retains old search snapshot during invalidation/repopulation', 'later search reuses', 'mtime-only change preserves indexes'], sourceUnchanged: true }));
}
