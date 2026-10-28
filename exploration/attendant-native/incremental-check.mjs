// Native incremental/WAL regressions. Generated fixtures only; no live projects.
// Usage: node incremental-check.mjs <native-executable> [...]
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { cpSync, mkdirSync, readFileSync, readdirSync, rmSync, utimesSync, writeFileSync } from 'node:fs';
import { basename, dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { DatabaseSync } from 'node:sqlite';
const root = dirname(fileURLToPath(import.meta.url)), executables = process.argv.slice(2).map(path => resolve(path));
if (!executables.length || process.argv.includes('--help')) {
  console.log('Usage: node incremental-check.mjs <native-executable> [...]'); process.exit(executables.length ? 0 : 2);
}
const digest = bytes => createHash('sha256').update(bytes).digest('hex');
function hashes(directory) {
  return Object.fromEntries(readdirSync(directory, { withFileTypes: true }).flatMap(entry => {
    const path = join(directory, entry.name);
    return entry.isDirectory() ? Object.entries(hashes(path)) : [[path, digest(readFileSync(path))]];
  }));
}
const report = [];
for (const executable of executables) for (const variant of ['relative', 'absolute']) {
  const project = join(root, 'build/review', `${basename(executable, '.exe')}-${variant} incremental café Ω`);
  rmSync(project, { recursive: true, force: true }); cpSync(join(root, 'fixtures/project'), project, { recursive: true });
  if (variant === 'absolute') writeFileSync(join(project, '.pi/attendant.tables'), `${join(project, 'records/decisions').replaceAll('\\', '/')}/ as decisions\n${join(project, 'records/tasks').replaceAll('\\', '/')}/ as tasks\n`);
  const database = join(project, '.attendant/attendant.sqlite'), first = join(project, 'records/tasks/first.md'), second = join(project, 'records/tasks/second.md');
  const cases = [];
  function native(args, expected = 0) {
    const result = spawnSync(executable, [...args, '-p', project], { encoding: 'utf8', maxBuffer: 24 * 1024 * 1024, timeout: 15_000 });
    assert.equal(result.status, expected, `${result.error ?? ''}\n${result.stderr}\n${result.stdout}`);
    return JSON.parse(result.stdout);
  }
  function db(action) { const connection = new DatabaseSync(database); try { return action(connection); } finally { connection.close(); } }
  function rows(connection, table) { return connection.prepare(`SELECT * FROM ${table} ORDER BY rowid`).all().map(row => ({ ...row })); }
  function snapshot() { return db(connection => Object.fromEntries(['tasks', '__attendant_cache', '__attendant_diagnostics', '__attendant_edges', '__attendant_fts_tasks', '__attendant_trigram_tasks', 'mutation_audit'].map(table => [table, rows(connection, table)]))); }
  assert.equal(native(['sync']).state, 'rebuilt');
  assert.equal(db(connection => connection.prepare('PRAGMA journal_mode').get().journal_mode), 'wal');
  cases.push('fresh projection persists WAL mode');
  native(['search', '-q', 'lantern']); // Enable once before changed-only FTS assertions.
  db(connection => connection.exec(`CREATE TABLE mutation_audit(kind TEXT,source_path TEXT);
    CREATE TRIGGER audit_insert AFTER INSERT ON tasks BEGIN INSERT INTO mutation_audit VALUES('insert',new.source_path); END;
    CREATE TRIGGER audit_delete AFTER DELETE ON tasks BEGIN INSERT INTO mutation_audit VALUES('delete',old.source_path); END;`));
  const secondFts = db(connection => ({ ...connection.prepare("SELECT rowid,* FROM __attendant_fts_tasks WHERE record_id=(SELECT id FROM tasks WHERE name='second')").get() }));
  writeFileSync(first, readFileSync(first, 'utf8') + '\nIncremental body token quasarbench.\n');
  assert.equal(native(['sync']).state, 'updated');
  db(connection => {
    assert.deepEqual(rows(connection, 'mutation_audit').map(row => [row.kind, row.source_path]), [['delete', first], ['insert', first]]);
    assert.deepEqual({ ...connection.prepare("SELECT rowid,* FROM __attendant_fts_tasks WHERE record_id=(SELECT id FROM tasks WHERE name='second')").get() }, secondFts);
    assert.equal(connection.prepare("SELECT count(*) n FROM __attendant_fts_tasks WHERE __attendant_fts_tasks MATCH 'quasarbench'").get().n, 1);
    connection.exec('DELETE FROM mutation_audit');
  });
  cases.push('body edit touches only changed row and indexes');
  const time = new Date('2026-01-01T00:00:00.000Z'); utimesSync(first, time, time);
  assert.equal(native(['sync']).state, 'reused');
  db(connection => {
    assert.equal(rows(connection, 'mutation_audit').length, 0);
    assert.equal(connection.prepare('SELECT mtime_ms FROM __attendant_cache WHERE source_path=?').get(first).mtime_ms, time.getTime());
  });
  cases.push('mtime-only refresh does not rewrite rows or indexes');

  // Failure after deletes and one insert must roll back the entire delta.
  db(connection => connection.exec("CREATE TRIGGER inject_failure BEFORE INSERT ON tasks WHEN new.name='second' BEGIN SELECT raise(ABORT,'delta-fault'); END;"));
  const before = snapshot();
  writeFileSync(first, readFileSync(first, 'utf8') + '\nBatch edit one.\n');
  writeFileSync(second, readFileSync(second, 'utf8') + '\nBatch edit two.\n');
  const sourceBeforeFailure = hashes(join(project, 'records'));
  assert.match(native(['sync'], 1).error.message, /delta-fault/);
  assert.deepEqual(snapshot(), before); assert.deepEqual(hashes(join(project, 'records')), sourceBeforeFailure);
  assert.ok(!readdirSync(join(project, '.attendant')).some(name => name.startsWith('native-')));
  db(connection => connection.exec('DROP TRIGGER inject_failure'));
  assert.equal(native(['sync']).state, 'updated');
  cases.push('mid-delta SQL failure rolls back rows cache diagnostics edges and both indexes');

  const identities = db(connection => connection.prepare('SELECT name,id FROM tasks ORDER BY name').all());
  const a = readFileSync(first, 'utf8'), b = readFileSync(second, 'utf8');
  writeFileSync(first, a.replace(identities[0].id, identities[1].id)); writeFileSync(second, b.replace(identities[1].id, identities[0].id));
  assert.equal(native(['sync']).state, 'updated');
  assert.deepEqual(db(connection => connection.prepare('SELECT id FROM tasks ORDER BY name').all().map(row => row.id)), [identities[1].id, identities[0].id]);
  cases.push('ID swap deletes affected set before reinsertion');

  // Earlier duplicate changes visibility of an otherwise byte-identical source.
  const duplicate = join(project, 'records/tasks/000-duplicate.md');
  writeFileSync(duplicate, readFileSync(first, 'utf8').replace('name: first', 'name: duplicate'));
  const duplicateResult = native(['sync']); assert.equal(duplicateResult.state, 'updated');
  assert.ok(duplicateResult.diagnostics.some(item => item.code === 'record-id-duplicate'));
  assert.equal(db(connection => connection.prepare("SELECT count(*) n FROM tasks WHERE name='first'").get().n), 0);
  rmSync(duplicate); assert.equal(native(['sync']).state, 'updated');
  assert.equal(db(connection => connection.prepare("SELECT count(*) n FROM tasks WHERE name='first'").get().n), 1);
  cases.push('global validation blocks and restores untouched source row');

  const held = new DatabaseSync(database);
  try {
    held.exec('BEGIN');
    const oldHash = held.prepare("SELECT source_hash FROM tasks WHERE name='first'").get().source_hash;
    writeFileSync(first, readFileSync(first, 'utf8') + '\nSnapshot reader token nebularbench.\n');
    assert.equal(native(['sync']).state, 'updated');
    assert.equal(held.prepare("SELECT source_hash FROM tasks WHERE name='first'").get().source_hash, oldHash);
    assert.equal(native(['query', '-s', "SELECT source_hash FROM tasks WHERE name='first'"]).rows[0].source_hash, digest(readFileSync(first)));
    const frozen = hashes(join(project, '.attendant'));
    assert.equal(native(['doctor'], 1).projection, 'stale'); assert.deepEqual(hashes(join(project, '.attendant')), frozen);
    held.exec('ROLLBACK');
    cases.push('WAL snapshot reader coexists with delta and doctor stays non-writing');

    held.exec('BEGIN'); held.prepare('SELECT count(*) FROM tasks').get();
    native(['update', '-c', 'tasks', '-i', JSON.stringify([{ name: 'first', fields: { desc: 'Native incremental write.' } }])]);
    native(['create', '-c', 'tasks', '-i', JSON.stringify([{ name: 'created-incremental', fields: { desc: 'Heldwalcreationtoken with held reader.' } }])]);
    assert.equal(held.prepare("SELECT count(*) n FROM tasks WHERE name='created-incremental'").get().n, 0);
    held.exec('ROLLBACK');
    assert.deepEqual(native(['query', '-s', "SELECT desc FROM tasks WHERE name='first'"]).rows, [{ desc: 'Native incremental write.' }]);
    assert.equal(native(['search', '-q', 'Heldwalcreationtoken']).returned, 1);
    cases.push('create and update share incremental refresh with held WAL reader');

    held.exec('BEGIN'); held.prepare('SELECT count(*) FROM tasks').get();
    const config = join(project, '.pi/attendant.tables'), original = readFileSync(config, 'utf8');
    writeFileSync(config, original + '\n# force contract rebuild\n');
    assert.match(native(['sync'], 1).error.message, /Existing WAL sidecars/);
    held.exec('ROLLBACK'); writeFileSync(config, original);
    assert.equal(native(['sync']).state, 'reused');
    cases.push('contract rebuild refuses active readers without deleting sidecars');
  } finally { try { held.exec('ROLLBACK'); } catch {} held.close(); }
  assert.equal(native(['sync']).state, 'reused');
  assert.equal(db(connection => connection.prepare('PRAGMA quick_check').get().quick_check), 'ok');
  writeFileSync(database + '-wal', ''); writeFileSync(database + '-shm', '');
  const emptySidecars = hashes(join(project, '.attendant'));
  assert.equal(native(['doctor']).projection, 'fresh'); assert.deepEqual(hashes(join(project, '.attendant')), emptySidecars);
  rmSync(database + '-wal'); rmSync(database + '-shm');
  cases.push('doctor accepts empty WAL and SHM without creating or changing files');
  mkdirSync(database + '-wal');
  assert.equal(native(['doctor'], 1).projection, 'stale'); rmSync(database + '-wal', { recursive: true });
  cases.push('doctor refuses invalid WAL directory instead of assuming absent log');
  report.push({ executable, variant, cases }); console.log(JSON.stringify(report.at(-1)));
}
mkdirSync(join(root, 'build/review'), { recursive: true });
writeFileSync(join(root, 'build/review/incremental-results.json'), JSON.stringify(report, null, 2) + '\n');
