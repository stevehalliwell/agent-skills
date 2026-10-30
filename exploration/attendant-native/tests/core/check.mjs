// Reproducible end-to-end oracle. Node is test infrastructure, not native runtime.
// Usage: node exploration/attendant-native/tests/core/check.mjs <native-executable> [...]
// Generated datasets and evidence stay under ignored build/review/.
import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import { spawnSync } from 'node:child_process';
import { DatabaseSync } from 'node:sqlite';
import { cpSync, existsSync, mkdirSync, readFileSync, readdirSync, rmSync, writeFileSync } from 'node:fs';
import { basename, dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const root = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
const runner = resolve(root, '../../skills/attendant/scripts/attendant.mjs');
const executables = process.argv.slice(2).map(x => resolve(x));
if (!executables.length || executables.includes('--help')) {
  console.log('Usage: node tests/core/check.mjs <native-executable> [...]\nRequires Node >=24.10.0. Builds reference and native DBs in isolated copied fixtures.');
  process.exit(executables.length ? 0 : 2);
}
function run(command, args, expected = 0) {
  const result = spawnSync(command, args, { encoding: 'utf8', maxBuffer: 16 * 1024 * 1024 });
  assert.equal(result.status, expected, `${command} ${args.join(' ')}\n${result.error ?? ''}\n${result.stderr}\n${result.stdout}`);
  return JSON.parse(result.stdout);
}
function cli(project, ...args) { return run(process.execPath, [runner, ...args, '--project', project]); }
function files(path) {
  return readdirSync(path, { withFileTypes: true }).filter(e => e.name !== '.attendant').flatMap(e =>
    e.isDirectory() ? files(join(path, e.name)) : [join(path, e.name)]).sort();
}
function hashes(project) {
  return Object.fromEntries(files(project).map(path => [path, createHash('sha256').update(readFileSync(path)).digest('hex')]));
}
function plain(rows) { return rows.map(row => ({ ...row })); }
function snapshot(project) {
  const db = new DatabaseSync(join(project, '.attendant/attendant.sqlite'));
  try {
    const tables = ['decisions', 'tasks', '__attendant_metadata', '__attendant_cache', '__attendant_diagnostics', '__attendant_edges',
      '__attendant_fts_decisions', '__attendant_trigram_decisions', '__attendant_fts_tasks', '__attendant_trigram_tasks'];
    return Object.fromEntries(tables.map(table => {
      const rows = plain(db.prepare(`SELECT * FROM "${table}"`).all()).filter(row => table !== '__attendant_metadata' || row.key !== 'fts_enabled').map(row => {
        // JSON syntax can differ while representing the same cached metadata.
        if (row.front_matter) row.front_matter = JSON.parse(row.front_matter);
        // Typed native JSON may retain .0 where legacy float JSON omits it.
        // This shared fixture uses JS-representable values; tests/core/source-check.mjs
        // separately verifies exact integers and ambiguous f64 with lossless JSON.
        if (typeof row.misc === 'string') row.misc = JSON.parse(row.misc);
        return row;
      });
      rows.sort((a, b) => JSON.stringify(a).localeCompare(JSON.stringify(b)));
      return [table, { columns: plain(db.prepare(`PRAGMA table_info("${table}")`).all()), rows }];
    }));
  } finally { db.close(); }
}
function publicResults(project) {
  const schema = cli(project, 'schema');
  const query = cli(project, 'query', '--sql', 'SELECT name, status, priority, misc FROM tasks WHERE status = :status ORDER BY name', '--params', '{"status":"active"}');
  assert.equal(query.rows.length, 1);
  assert.equal(query.rows[0].name, 'first');
  const joined = cli(project, 'query', '--sql', 'SELECT t.name, d.status FROM tasks t JOIN decisions d ON t.decision = \'decisions/\' || d.name');
  assert.equal(joined.rows.length, 1);
  const search = cli(project, 'search', '--query', 'lantern');
  assert.deepEqual(search.rows.map(row => row.name).sort(), ['first', 'second']);
  const partial = cli(project, 'search', '--query', 'anter');
  assert.deepEqual(partial.rows.map(row => row.name).sort(), ['first', 'second']);
  assert.equal(cli(project, 'search', '--query', '"hidden-only-token"').returned, 0);
  return { schema, query, joined, search, partial };
}
const review = join(root, 'build/review');
mkdirSync(review, { recursive: true });
const summaries = [];
const jobs = executables.flatMap(exe => ['relative', 'absolute'].map(variant => ({ exe, variant })));
for (const { exe, variant } of jobs) {
  assert.ok(existsSync(exe), `Missing executable: ${exe}`);
  const label = `${basename(exe, '.exe')}-${variant}`;
  // Spaces and Unicode exercise Windows path conversion, hashes and source paths.
  const project = join(review, `${label} café Ω`);
  rmSync(project, { recursive: true, force: true });
  cpSync(join(root, 'tests/fixtures/project'), project, { recursive: true });
  if (variant === 'absolute') {
    // Runner preserves absolute config spelling in schema hashes; include a
    // trailing separator, slash spelling, tab-delimited alias, and implicit alias.
    writeFileSync(join(project, '.pi/attendant.tables'), `\uFEFF${join(project, 'records/decisions').replaceAll('\\', '/')}/\tas\tdecisions\nrecords/tasks/\n`);
  }
  // Exercise CRLF and no-final-newline independently of checkout's line endings.
  const crlf = join(project, 'records/tasks/first.md');
  writeFileSync(crlf, readFileSync(crlf, 'utf8').replace(/\r?\n/g, '\r\n'));
  const noFinal = join(project, 'records/tasks/second.md');
  writeFileSync(noFinal, readFileSync(noFinal, 'utf8').replace(/\r?\n/g, '\n').trimEnd());
  const before = hashes(project);
  const referenceBuild = cli(project, 'sync');
  assert.equal(referenceBuild.state, 'rebuilt');
  assert.deepEqual(referenceBuild.diagnostics, []);
  assert.deepEqual(hashes(project), before, 'Reference fixture must already be normalized');
  const reference = snapshot(project);
  const expectedPublic = publicResults(project);
  rmSync(join(project, '.attendant'), { recursive: true, force: true });
  const nativeBuild = run(exe, [project]);
  assert.equal(nativeBuild.records, 3);
  assert.equal(nativeBuild.collections, 2);
  assert.deepEqual(hashes(project), before, 'Native builder must not change source');
  // First search publishes a full indexed rebuild, before any runner consumes it.
  const cold = new DatabaseSync(join(project, '.attendant/attendant.sqlite'));
  const deferred = cold.prepare("SELECT value FROM __attendant_metadata WHERE key='fts_enabled'").get();
  if (deferred) {
    assert.equal(deferred.value, '0');
    assert.equal(cold.prepare("SELECT count(*) n FROM sqlite_schema WHERE name='__attendant_fts_tasks'").get().n, 0);
  }
  cold.close();
  run(exe, ['search', '-q', 'lantern', '-p', project]);
  // Compare before the runner is allowed to refresh or mutate the native DB.
  const native = snapshot(project);
  assert.deepEqual(native, reference, 'Native logical DB must match reference before CLI consumption');
  const reused = cli(project, 'sync');
  assert.equal(reused.state, 'reused', 'Runner must not replace native DB');
  assert.deepEqual(publicResults(project), expectedPublic);
  assert.deepEqual(hashes(project), before, 'CLI consumption must not repair source');
  assert.equal(cli(project, 'sync').state, 'reused');
  // Invalid/unsupported records are now reported and skipped, never repaired.
  const bad = join(project, 'records/tasks/bad.md');
  const failures = [
    '---\nname: [unfinished\n---\nbody',
    '---\nname: first\nname: duplicate\n---\nbody',
    '---\nname: &anchor first\nother: *anchor\n---\nbody',
    '---\nname: {nested: unsupported}\n---\nbody',
  ];
  for (const text of failures) {
    writeFileSync(bad, text);
    const frozen = hashes(project);
    const result = run(exe, [project]);
    assert.equal(result.records, 3);
    assert.ok(result.diagnostics.some(item => item.path === bad));
    assert.deepEqual(hashes(project), frozen);
    assert.equal(cli(project, 'sync').state, 'reused');
  }
  rmSync(bad);
  run(exe, [project]);
  assert.deepEqual(hashes(project), before);
  const result = { executable: exe, variant, project, referenceBuild, nativeBuild, reuse: reused.state, failureCases: failures.length, checks: 'logical tables/columns/FTS, public schema/query/search, source hashes, reported/skipped invalid inputs' };
  summaries.push(result);
  console.log(JSON.stringify(result));
}
writeFileSync(join(review, 'results.json'), JSON.stringify(summaries, null, 2) + '\n');
