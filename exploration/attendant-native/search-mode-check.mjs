// Native persistent deferred-FTS acceptance. Generated normalized fixtures only.
// Usage: node search-mode-check.mjs <native-executable> [...]
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { cpSync, mkdirSync, readFileSync, readdirSync, rmSync, writeFileSync, utimesSync } from 'node:fs';
import { basename, dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { DatabaseSync } from 'node:sqlite';
const root = dirname(fileURLToPath(import.meta.url)), executables = process.argv.slice(2).map(path => resolve(path));
if (!executables.length || process.argv.includes('--help')) {
  console.log('Usage: node search-mode-check.mjs <native-executable> [...]'); process.exit(executables.length ? 0 : 2);
}
const runner = resolve(root, '../../skills/attendant/scripts/attendant.mjs');
const digest = bytes => createHash('sha256').update(bytes).digest('hex');
function sourceHashes(directory) {
  return Object.fromEntries(readdirSync(directory, { withFileTypes: true }).filter(entry => entry.name !== '.attendant').flatMap(entry => {
    const path = join(directory, entry.name);
    return entry.isDirectory() ? Object.entries(sourceHashes(path)) : [[path, digest(readFileSync(path))]];
  }));
}
const report = [];
for (const executable of executables) for (const variant of ['relative', 'absolute']) {
  const project = join(root, 'build/review', `${basename(executable, '.exe')}-${variant} search mode café Ω`);
  rmSync(project, { recursive: true, force: true }); cpSync(join(root, 'fixtures/project'), project, { recursive: true });
  const config = join(project, '.pi/attendant.tables');
  if (variant === 'absolute') writeFileSync(config, `${join(project, 'records/decisions').replaceAll('\\', '/')}/ as decisions\n${join(project, 'records/tasks').replaceAll('\\', '/')}/ as tasks\n`);
  const database = join(project, '.attendant/attendant.sqlite'), first = join(project, 'records/tasks/first.md');
  function run(command, args, status = 0) {
    const result = spawnSync(command, [...args, '-p', project], { encoding: 'utf8', timeout: 20_000 });
    assert.equal(result.status, status, result.stderr + result.stdout); return JSON.parse(result.stdout);
  }
  const native = (args, status = 0) => run(executable, args, status);
  function inspect(action) { const db = new DatabaseSync(database); try { return action(db); } finally { db.close(); } }
  function mode(expected) { inspect(db => {
    assert.equal(db.prepare("SELECT value FROM __attendant_metadata WHERE key='fts_enabled'").get().value, String(expected));
    assert.equal(db.prepare("SELECT value FROM __attendant_metadata WHERE key='layout_version'").get().value, expected ? '2' : '3');
    assert.equal(db.prepare("SELECT count(*) n FROM sqlite_schema WHERE name IN ('__attendant_fts_tasks','__attendant_trigram_tasks')").get().n, expected ? 2 : 0);
    assert.equal(db.prepare('PRAGMA quick_check').get().quick_check, 'ok');
  }); }
  const cases = [], initialSource = sourceHashes(project);
  assert.equal(native(['sync']).state, 'rebuilt'); mode(0);
  const coldSchema = native(['schema']); native(['query', '-s', 'SELECT name FROM tasks']); mode(0);
  assert.equal(native(['doctor']).projection, 'fresh');
  assert.deepEqual(sourceHashes(project), initialSource);
  cases.push('sync schema query doctor keep cold layout unindexed; schema describes planned FTS');
  writeFileSync(first, readFileSync(first, 'utf8') + '\nColdmodesearchtoken.\n');
  assert.equal(native(['sync']).state, 'updated'); mode(0);
  native(['update', '-c', 'tasks', '-i', JSON.stringify([{ name: 'first', fields: { desc: 'Cold mode changed.' } }])]); mode(0);
  native(['create', '-c', 'tasks', '-i', JSON.stringify([{ name: 'cold-created', fields: { desc: 'Coldwrittensearchtoken.' } }])]); mode(0);
  cases.push('cold sync create update do not create FTS');
  const validConfig = readFileSync(config, 'utf8'), priorCache = digest(readFileSync(database));
  writeFileSync(config, validConfig + '\nrecords/decisions as TASKS\n');
  const failedSource = sourceHashes(project);
  assert.match(native(['search', '-q', 'Coldmodesearchtoken'], 1).error.message, /SQLite/);
  assert.equal(digest(readFileSync(database)), priorCache); assert.deepEqual(sourceHashes(project), failedSource);
  assert.ok(!readdirSync(join(project, '.attendant')).some(name => name.startsWith('native-')));
  writeFileSync(config, validConfig); mode(0);
  cases.push('failed private first-search construction preserves cold cache and removes owned temporary state');
  inspect(db => db.exec("CREATE TRIGGER fail_search_enable BEFORE UPDATE ON __attendant_metadata WHEN NEW.key='layout_version' AND NEW.value='2' BEGIN SELECT RAISE(ABORT,'injected search failure'); END"));
  const beforeFailure = sourceHashes(project);
  assert.match(native(['search', '-q', 'Coldmodesearchtoken'], 1).error.message, /injected search failure/);
  mode(0); assert.deepEqual(sourceHashes(project), beforeFailure);
  inspect(db => {
    assert.equal(db.prepare("SELECT count(*) n FROM sqlite_schema WHERE name GLOB '__attendant_fts_*' OR name GLOB '__attendant_trigram_*'").get().n, 0);
    db.exec('DROP TRIGGER fail_search_enable');
  });
  cases.push('failed population rolls back both indexes and enabled flag after all rows inserted');
  inspect(db => db.exec("CREATE TRIGGER fail_search_validation AFTER UPDATE ON __attendant_metadata WHEN NEW.key='layout_version' AND NEW.value='2' BEGIN DELETE FROM tasks WHERE name='first'; END"));
  assert.match(native(['search', '-q', 'Coldmodesearchtoken'], 1).error.message, /Populated search failed validation/);
  mode(0); assert.deepEqual(sourceHashes(project), beforeFailure);
  inspect(db => {
    assert.equal(db.prepare("SELECT count(*) n FROM tasks WHERE name='first'").get().n, 1);
    db.exec('DROP TRIGGER fail_search_validation');
  });
  cases.push('failed pre-commit snapshot validation restores collection rows and cold index state');
  const writer = new DatabaseSync(database);
  try {
    writer.exec('BEGIN IMMEDIATE');
    assert.match(native(['search', '-q', 'Coldmodesearchtoken'], 1).error.message, /locked|busy/i);
    mode(0); writer.exec('ROLLBACK');
  } finally { writer.close(); }
  cases.push('first-search writer contention fails bounded without enabling indexes');
  inspect(db => db.exec('CREATE TABLE cold_generation_sentinel(value TEXT); INSERT INTO cold_generation_sentinel VALUES(\'old\')'));
  const reader = new DatabaseSync(database), beforePopulation = sourceHashes(project);
  try {
    reader.exec('BEGIN'); reader.prepare('SELECT count(*) FROM tasks').get();
    assert.equal(native(['search', '-q', 'Coldmodesearchtoken']).returned, 1); mode(1);
    assert.equal(reader.prepare("SELECT value FROM __attendant_metadata WHERE key='fts_enabled'").get().value, '0');
    assert.equal(reader.prepare("SELECT count(*) n FROM sqlite_schema WHERE name='__attendant_fts_tasks'").get().n, 0);
    assert.equal(reader.prepare('SELECT value FROM cold_generation_sentinel').get().value, 'old');
    assert.deepEqual(sourceHashes(project), beforePopulation);
    reader.exec('ROLLBACK');
  } finally { reader.close(); }
  cases.push('first-search population succeeds with old WAL reader retaining complete cold snapshot');
  inspect(db => assert.equal(db.prepare("SELECT count(*) n FROM sqlite_schema WHERE name='cold_generation_sentinel'").get().n, 1, 'First search unnecessarily rebuilt collection/cache generation'));
  assert.deepEqual(native(['schema']), coldSchema);
  cases.push('first search preserves collection generation and schema JSON while enabling both indexes');
  assert.equal(native(['search', '-q', 'Coldwrittensearchtoken']).returned, 1);
  inspect(db => db.exec('CREATE TABLE indexed_generation_sentinel(value TEXT)'));
  writeFileSync(first, readFileSync(first, 'utf8') + '\nPersistedsearchtoken.\n');
  assert.equal(native(['sync']).state, 'updated'); mode(1);
  assert.equal(native(['search', '-q', 'Persistedsearchtoken']).returned, 1);
  inspect(db => assert.equal(db.prepare("SELECT count(*) n FROM sqlite_schema WHERE name='indexed_generation_sentinel'").get().n, 1, 'Later search unnecessarily rebuilt cache'));
  cases.push('enabled flag persists and source edits maintain indexes incrementally');
  utimesSync(first, new Date('2026-01-02'), new Date('2026-01-02'));
  assert.equal(native(['sync']).state, 'reused'); mode(1);
  const oldConfig = readFileSync(config, 'utf8'); writeFileSync(config, oldConfig + '\n# retain indexed mode\n');
  assert.equal(native(['sync']).state, 'rebuilt'); mode(1);
  assert.equal(native(['search', '-q', 'Persistedsearchtoken']).returned, 1);
  cases.push('mtime-only preserves indexes and contract rebuild retains enabled mode');
  rmSync(join(project, '.attendant'), { recursive: true, force: true }); native(['sync']); mode(0);
  inspect(db => db.exec('CREATE TABLE pending_generation_sentinel(value TEXT)'));
  writeFileSync(first, readFileSync(first, 'utf8') + '\nPendingsearchtoken.\n');
  assert.equal(native(['search', '-q', 'Pendingsearchtoken']).returned, 1); mode(1);
  inspect(db => assert.equal(db.prepare("SELECT count(*) n FROM sqlite_schema WHERE name='pending_generation_sentinel'").get().n, 1));
  cases.push('first search reconciles pending source delta then populates indexes without replacing cache');
  // Cold layout has explicit incompatible version so legacy CLI rebuilds safely.
  rmSync(join(project, '.attendant'), { recursive: true, force: true }); native(['sync']); mode(0);
  const beforeLegacy = sourceHashes(project);
  assert.equal(run(process.execPath, [runner, 'sync']).state, 'rebuilt');
  const legacy = digest(readFileSync(database));
  assert.equal(native(['sync']).state, 'reused'); assert.equal(digest(readFileSync(database)), legacy);
  assert.equal(native(['search', '-q', 'Persistedsearchtoken']).returned, 1);
  assert.equal(digest(readFileSync(database)), legacy, 'Legacy indexed snapshot was rewritten');
  assert.deepEqual(sourceHashes(project), beforeLegacy);
  cases.push('legacy rejects cold format then native reuses legacy indexed format without writing');
  rmSync(join(project, '.attendant'), { recursive: true, force: true });
  assert.equal(native(['search', '-q', 'Persistedsearchtoken']).returned, 1); mode(1);
  cases.push('search as first command directly builds indexed cache');
  report.push({ executable, variant, cases }); console.log(JSON.stringify(report.at(-1)));
}
mkdirSync(join(root, 'build/review'), { recursive: true });
writeFileSync(join(root, 'build/review/search-mode-results.json'), JSON.stringify(report, null, 2) + '\n');
