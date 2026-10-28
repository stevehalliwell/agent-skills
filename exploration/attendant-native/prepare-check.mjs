// Native lifecycle acceptance. Node is test infrastructure, never native runtime.
// Usage: node prepare-check.mjs <native-executable> [...]
// Copies normalized fixtures into build/review; never operates on caller datasets.
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { DatabaseSync } from 'node:sqlite';
import { cpSync, mkdirSync, readFileSync, readdirSync, renameSync, rmSync, statSync, utimesSync, writeFileSync } from 'node:fs';
import { basename, dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const root = dirname(fileURLToPath(import.meta.url));
const runner = resolve(root, '../../skills/attendant/scripts/attendant.mjs');
const executables = process.argv.slice(2).map(path => resolve(path));
if (!executables.length || process.argv.includes('--help')) {
  console.log('Usage: node prepare-check.mjs <native-executable> [...]\nRequires Node >=24.10.0. All generated data stays under build/review/.');
  process.exit(executables.length ? 0 : 2);
}
function run(command, args, expected = 0, options = {}) {
  const result = spawnSync(command, args, { encoding: 'utf8', maxBuffer: 24 * 1024 * 1024, timeout: 30_000, ...options });
  assert.equal(result.status, expected, `${command} ${args.join(' ')}\n${result.error ?? ''}\n${result.stderr}\n${result.stdout}`);
  return JSON.parse(result.stdout);
}
function hash(path) { return createHash('sha256').update(readFileSync(path)).digest('hex'); }
function sourceHashes(path) {
  return Object.fromEntries(readdirSync(path, { withFileTypes: true }).filter(x => x.name !== '.attendant').flatMap(x => {
    const file = join(path, x.name);
    return x.isDirectory() ? Object.entries(sourceHashes(file)) : [[file, hash(file)]];
  }));
}
const report = [];
for (const executable of executables) for (const variant of ['relative', 'absolute']) {
  const project = join(root, 'build/review', `${basename(executable, '.exe')}-${variant} preparation café Ω`);
  rmSync(project, { recursive: true, force: true });
  cpSync(join(root, 'fixtures/project'), project, { recursive: true });
  const config = join(project, '.pi/attendant.tables');
  if (variant === 'absolute') writeFileSync(config, `\uFEFF${join(project, 'records/decisions').replaceAll('\\', '/')}/\tas\tdecisions\nrecords/tasks/\n`);
  const initialConfig = readFileSync(config, 'utf8');
  const native = (...args) => run(executable, [...args, '-p', project]);
  const oracle = (...args) => run(process.execPath, [runner, ...args, '-p', project]);
  const database = join(project, '.attendant/attendant.sqlite');
  const first = join(project, 'records/tasks/first.md');
  const hidden = join(project, 'records/tasks/hidden.md');
  const schema = join(project, 'records/tasks/.schema.md');
  const cases = [];
  const nativeEnvironment = { ...process.env };
  for (const key of Object.keys(nativeEnvironment)) if (key.toLowerCase() === 'path') delete nativeEnvironment[key];
  nativeEnvironment.PATH = join(process.env.SystemRoot, 'System32');
  function check(label, operation) {
    const frozen = sourceHashes(project);
    operation();
    assert.deepEqual(sourceHashes(project), frozen, `${label}: native operation changed source`);
    const published = hash(database);
    assert.deepEqual(native('sync'), { collections: 2, records: 3, diagnostics: [], state: 'reused' });
    assert.equal(hash(database), published, `${label}: unchanged native sync wrote DB`);
    // Cold layout intentionally rejects legacy consumption; enable FTS before
    // asserting indexed-layout interoperability and unchanged runner reuse.
    const mode = new DatabaseSync(database, { readOnly: true });
    const unindexed = mode.prepare("SELECT value FROM __attendant_metadata WHERE key='fts_enabled'").get()?.value === '0'; mode.close();
    if (unindexed) native('search', '-q', 'lantern');
    const indexed = hash(database);
    assert.equal(oracle('sync').state, 'reused', `${label}: runner rejected native DB`);
    assert.deepEqual(sourceHashes(project), frozen, `${label}: runner repaired source`);
    assert.equal(hash(database), indexed, `${label}: runner wrote DB`);
    cases.push(label);
  }
  for (const args of [['sync'], ['schema'], ['query', '-s', 'SELECT count(*) n FROM tasks'], ['search', '-q', 'lantern']]) {
    rmSync(join(project, '.attendant'), { recursive: true, force: true });
    check(`missing DB via ${args[0]}`, () => {
      const result = run(executable, [...args, '-p', project], 0, { env: nativeEnvironment });
      if (args[0] === 'sync') assert.equal(result.state, 'rebuilt');
      if (args[0] === 'query') assert.deepEqual(result.rows, [{ n: 2 }]);
      if (args[0] === 'search') assert.equal(result.returned, 2);
    });
  }
  const unchanged = hash(database);
  check('unchanged reads', () => {
    native('schema'); native('query', '-s', 'SELECT * FROM tasks'); native('search', '-q', 'lantern');
    assert.equal(hash(database), unchanged);
  });
  // Hashing must detect an equal-size edit even with the exact cached mtime.
  const seconds = 1_790_000_000;
  utimesSync(first, seconds, seconds);
  check('mtime-only cache refresh', () => assert.equal(native('sync').state, 'reused'));
  const prior = statSync(first);
  writeFileSync(first, readFileSync(first, 'utf8').replace('projection lantern.', 'projection beacons.'));
  utimesSync(first, seconds, seconds);
  assert.equal(statSync(first).mtimeMs, prior.mtimeMs); assert.equal(statSync(first).size, prior.size);
  check('equal stat content edit via search', () => {
    assert.deepEqual(native('search', '-q', 'beacons').rows.map(row => row.name), ['first']);
  });
  writeFileSync(first, readFileSync(first, 'utf8').replace('status: active', 'status: done'));
  check('field edit via query', () => assert.deepEqual(native('query', '-s', "SELECT status FROM tasks WHERE name='first'").rows, [{ status: 'done' }]));

  const added = join(project, 'records/tasks/third.md');
  writeFileSync(added, readFileSync(first, 'utf8').replace('deab9d71', 'deab9d74').replace('name: first', 'name: third'));
  let frozen = sourceHashes(project);
  assert.deepEqual(native('query', '-s', 'SELECT name FROM tasks ORDER BY name').rows.map(row => row.name), ['first', 'second', 'third']);
  assert.equal(native('sync').records, 4); assert.equal(oracle('sync').state, 'reused');
  assert.deepEqual(sourceHashes(project), frozen); cases.push('added record via query');
  const renamed = join(project, 'records/tasks/renamed.md');
  renameSync(added, renamed);
  writeFileSync(renamed, readFileSync(renamed, 'utf8').replace('name: third', 'name: renamed'));
  frozen = sourceHashes(project);
  assert.equal(native('sync').state, 'updated');
  assert.deepEqual(native('query', '-s', "SELECT name FROM tasks WHERE name='renamed'").rows, [{ name: 'renamed' }]);
  assert.equal(oracle('sync').state, 'reused'); assert.deepEqual(sourceHashes(project), frozen); cases.push('renamed record');
  rmSync(renamed);
  check('deleted record via schema', () => { native('schema'); assert.equal(native('sync').records, 3); });

  writeFileSync(hidden, readFileSync(hidden, 'utf8') + '\nHidden source changed.\n');
  check('hidden record edit', () => {
    assert.equal(native('sync').state, 'updated');
    assert.equal(native('search', '-q', '"hidden-only-token"').returned, 0);
  });
  const hiddenText = readFileSync(hidden, 'utf8');
  writeFileSync(hidden, hiddenText.replace('hidden: true', 'hidden: false'));
  frozen = sourceHashes(project);
  assert.equal(native('search', '-q', '"hidden-only-token"').returned, 1);
  assert.equal(native('sync').records, 4); assert.equal(oracle('sync').state, 'reused');
  assert.deepEqual(sourceHashes(project), frozen); cases.push('hidden to visible');
  writeFileSync(hidden, hiddenText);
  check('visible to hidden', () => assert.equal(native('sync').state, 'updated'));

  writeFileSync(config, initialConfig + '\n# changed config\n');
  check('changed config via query', () => native('query', '-s', 'SELECT name FROM tasks'));
  const schemaText = readFileSync(schema, 'utf8');
  writeFileSync(schema, schemaText.replace('misc: null', 'misc: null\nextra: ?date'));
  check('changed schema via query', () => assert.deepEqual(native('query', '-s', 'SELECT extra FROM tasks').rows, [{ extra: null }, { extra: null }]));
  writeFileSync(schema, schemaText);
  check('restored schema via sync', () => assert.equal(native('sync').state, 'rebuilt'));
  const extraDirectory = join(project, 'records/extra');
  cpSync(join(project, 'records/decisions'), extraDirectory, { recursive: true });
  writeFileSync(config, initialConfig + '\nrecords/extra as extra\n');
  frozen = sourceHashes(project);
  assert.equal(native('sync').state, 'rebuilt');
  assert.deepEqual(native('sync'), { collections: 3, records: 4, diagnostics: [], state: 'reused' });
  assert.equal(oracle('sync').state, 'reused'); assert.deepEqual(sourceHashes(project), frozen);
  cases.push('added collection');
  writeFileSync(config, initialConfig + '\nrecords/extra as renamed_extra\n');
  frozen = sourceHashes(project);
  assert.equal(native('query', '-s', 'SELECT name FROM renamed_extra').returned, 1);
  assert.equal(native('sync').collections, 3); assert.equal(oracle('sync').state, 'reused');
  assert.deepEqual(sourceHashes(project), frozen); cases.push('renamed collection alias');
  writeFileSync(config, initialConfig + '\n# changed config\n');
  check('removed collection', () => assert.equal(native('sync').state, 'rebuilt'));
  rmSync(extraDirectory, { recursive: true });
  // Old or broken disposable state is rebuilt, not returned to the caller.
  let connection = new DatabaseSync(database);
  connection.exec("UPDATE __attendant_metadata SET value='old' WHERE key='layout_version'"); connection.close();
  check('old layout', () => assert.equal(native('sync').state, 'rebuilt'));
  connection = new DatabaseSync(database); connection.exec('DROP TABLE tasks'); connection.close();
  check('missing projected table', () => assert.equal(native('sync').state, 'rebuilt'));
  connection = new DatabaseSync(database); connection.exec('DROP TABLE __attendant_edges'); connection.close();
  check('missing edges table', () => {
    assert.equal(native('sync').state, 'rebuilt');
    assert.ok(native('query', '-s', 'SELECT * FROM __attendant_edges').returned > 0);
  });
  writeFileSync(database, 'not a SQLite database');
  check('corrupt DB', () => { native('schema'); });

  let published = hash(database);
  const bad = join(project, 'records/tasks/bad.md');
  writeFileSync(bad, '---\nname: [unfinished\n---\nbody\n');
  frozen = sourceHashes(project);
  for (const args of [['sync'], ['schema'], ['query', '-s', 'SELECT 1'], ['search', '-q', 'lantern']]) {
    const result = run(executable, [...args, '-p', project]);
    assert.ok(result.diagnostics.some(item => item.path === bad));
    assert.deepEqual(sourceHashes(project), frozen);
  }
  rmSync(bad);
  assert.equal(native('sync').state, 'updated'); published = hash(database);
  cases.push('malformed source reported skipped unchanged across all commands');
  // Force a failure during SQL construction, after opening the temporary DB.
  writeFileSync(config, initialConfig + '\nrecords/decisions as TASKS\n');
  frozen = sourceHashes(project);
  assert.match(run(executable, ['sync', '-p', project], 1).error.message, /SQLite/);
  assert.equal(hash(database), published); assert.deepEqual(sourceHashes(project), frozen);
  assert.ok(!readdirSync(join(project, '.attendant')).some(name => name.startsWith('native-')));
  cases.push('SQL build failure preserves DB and removes temporary file');
  writeFileSync(config, initialConfig + '\nrecords/decisions as same_sources\n');
  frozen = sourceHashes(project);
  assert.match(run(executable, ['sync', '-p', project], 1).error.message, /UNIQUE constraint failed: __attendant_cache.source_path/);
  assert.equal(hash(database), published); assert.deepEqual(sourceHashes(project), frozen);
  assert.ok(!readdirSync(join(project, '.attendant')).some(name => name.startsWith('native-')));
  writeFileSync(config, initialConfig + '\n# changed config\n');
  cases.push('SQL step failure preserves DB and removes temporary file');
  check('recovery after invalid source', () => assert.equal(native('sync').state, 'reused'));

  // WAL writers wait only for bounded time; failed updates preserve old data.
  writeFileSync(first, readFileSync(first, 'utf8') + '\nPublication failure test.\n');
  frozen = sourceHashes(project);
  connection = new DatabaseSync(database); connection.exec('BEGIN EXCLUSIVE');
  try {
    const failure = run(executable, ['sync', '-p', project], 1);
    assert.match(failure.error.message, /database is locked/);
    assert.equal(hash(database), published); assert.deepEqual(sourceHashes(project), frozen);
    assert.ok(!readdirSync(join(project, '.attendant')).some(name => name.startsWith('native-')));
  } finally { connection.exec('ROLLBACK'); connection.close(); }
  cases.push('busy writer preserves DB and creates no temporary file');
  check('recovery after busy writer', () => assert.equal(native('sync').state, 'updated'));
  writeFileSync(first, readFileSync(first, 'utf8') + '\nWAL refusal test.\n');
  frozen = sourceHashes(project);
  connection = new DatabaseSync(database); connection.exec('PRAGMA journal_mode=WAL; BEGIN IMMEDIATE');
  const walPublished = hash(database);
  try {
    assert.match(run(executable, ['sync', '-p', project], 1).error.message, /database is locked/);
    assert.equal(hash(database), walPublished); assert.deepEqual(sourceHashes(project), frozen);
    assert.ok(!readdirSync(join(project, '.attendant')).some(name => name.startsWith('native-')));
  } finally { connection.exec('ROLLBACK; PRAGMA wal_checkpoint(TRUNCATE); PRAGMA journal_mode=DELETE'); connection.close(); }
  cases.push('active WAL writer bounded refusal');
  check('recovery after WAL writer release', () => assert.equal(native('sync').state, 'updated'));

  // A killed writer leaves a genuine hot rollback journal. Native must not
  // publish over it: recovery of old pages could corrupt the replacement DB.
  const killed = spawnSync(process.execPath, ['-e', `
    const { DatabaseSync } = require('node:sqlite');
    const database = new DatabaseSync(process.argv[1]);
    database.exec("PRAGMA journal_mode=DELETE; PRAGMA synchronous=FULL; PRAGMA cache_size=5; BEGIN IMMEDIATE; UPDATE __attendant_cache SET content=content||hex(zeroblob(65536))");
    process.kill(process.pid, 'SIGKILL');
  `, database], { timeout: 30_000 });
  assert.notEqual(killed.status, 0);
  const journal = `${database}-journal`;
  assert.equal(readFileSync(journal).subarray(0, 8).toString('hex'), 'd9d505f920a163d7');
  const crashedDatabase = hash(database), hotJournal = hash(journal);
  frozen = sourceHashes(project);
  for (const command of ['sync', 'schema']) {
    assert.match(run(executable, [command, '-p', project], 1).error.message, /Existing rollback journal/);
    assert.equal(hash(database), crashedDatabase); assert.equal(hash(journal), hotJournal);
    assert.deepEqual(sourceHashes(project), frozen);
  }
  cases.push('hot rollback journal refusal preserves DB and journal');
  connection = new DatabaseSync(database);
  connection.prepare('SELECT count(*) FROM __attendant_cache').get(); connection.close();
  check('recovery after external SQLite rollback', () => assert.equal(native('sync').state, 'reused'));
  for (const args of [['sync', '-l', '1'], ['sync', '-s', 'SELECT 1']]) {
    assert.equal(run(executable, [...args, '-p', project], 1).error.code, 'cli-option');
  }
  const result = { executable, variant, project, cases, sourceUnchanged: true, runnerReused: true };
  report.push(result); console.log(JSON.stringify(result));
}
mkdirSync(join(root, 'build/review'), { recursive: true });
writeFileSync(join(root, 'build/review/prepare-results.json'), JSON.stringify(report, null, 2) + '\n');
