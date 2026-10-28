// Native read-command differential and containment checks.
// Usage: node read-check.mjs <native-executable> [...]
// Runs projection acceptance first; all generated work stays under build/review.
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { readFileSync, writeFileSync, readdirSync, mkdirSync, rmSync, cpSync } from 'node:fs';
import { join, resolve, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';

const root = dirname(fileURLToPath(import.meta.url));
const runner = resolve(root, '../../skills/attendant/scripts/attendant.mjs');
const executables = process.argv.slice(2).map(path => resolve(path));
if (!executables.length || process.argv.includes('--help')) {
  console.log('Usage: node read-check.mjs <native-executable> [...]');
  process.exit(executables.length ? 0 : 2);
}
const setup = spawnSync(process.execPath, [join(root, 'check.mjs'), ...executables], { encoding: 'utf8', timeout: 120_000 });
assert.equal(setup.status, 0, setup.stderr + setup.stdout);
const scenarios = JSON.parse(readFileSync(join(root, 'build/review/results.json'), 'utf8'));
function run(command, args, options = {}) {
  const result = spawnSync(command, args, { encoding: 'utf8', maxBuffer: 24 * 1024 * 1024, timeout: 30_000, ...options });
  assert.equal(result.status, options.expected ?? 0, `${command} ${args.join(' ')}\n${result.error ?? ''}\n${result.stderr}\n${result.stdout}`);
  return JSON.parse(result.stdout);
}
function sourceHashes(path) {
  return Object.fromEntries(readdirSync(path, { withFileTypes: true }).filter(x => x.name !== '.attendant').flatMap(x => {
    const file = join(path, x.name);
    return x.isDirectory() ? Object.entries(sourceHashes(file)) : [[file, createHash('sha256').update(readFileSync(file)).digest('hex')]];
  }));
}
const report = [];
for (const { executable, project, variant } of scenarios) {
  const native = (...args) => run(executable, [...args, '-p', project]);
  const oracle = (...args) => run(process.execPath, [runner, ...args, '-p', project]);
  const frozen = sourceHashes(project);
  // Usage is live source guidance, unlike projection content; verify empty,
  // changed and missing cases without replacing the snapshot.
  const usagePath = join(project, 'records/tasks/.usage.md');
  const usageOriginal = readFileSync(usagePath);
  for (const usage of ['', '# Changed guidance\n']) {
    writeFileSync(usagePath, usage);
    assert.deepEqual(native('schema'), oracle('schema'));
  }
  rmSync(usagePath);
  assert.deepEqual(native('schema'), oracle('schema'));
  writeFileSync(usagePath, usageOriginal);
  const db = join(project, '.attendant/attendant.sqlite');
  const databaseHash = () => createHash('sha256').update(readFileSync(db)).digest('hex');
  const before = databaseHash();
  const comparisons = [
    ['schema'],
    ['query', '-s', 'SELECT name, status, priority FROM tasks ORDER BY name'],
    ['query', '-s', 'SELECT name FROM tasks WHERE status=:status', '-P', '{"status":"active"}'],
    ['query', '-s', 'SELECT :boolean b, @none n, $number x, :text t', '-P', '{"boolean":true,"none":null,"number":0.1,"text":"café Ω\\u0000tail"}'],
    ['query', '-s', 'SELECT :x x', '-P', '{"x":1,"x":2}'],
    ['query', '-s', 'SELECT ?1 x', '-P', '{"1":3}'],
    ['query', '-s', 'SELECT :x x', '-P', '{"x":1,":x":2,"x":3}'],
    ['query', '-s', 'SELECT :x x', '-P', '{"x":[1],"x":3}'],
    ['query', '-s', 'SELECT :unbound x'],
    ['query', '-s', 'SELECT 1 x, 2 x, 3 AS "123", 4 AS "2"'],
    ['query', '-s', "SELECT x'00ff42' bytes, NULL none, 1e999 infinity"],
    ['query', '-s', "SELECT t.name, d.status FROM tasks t JOIN decisions d ON t.decision = 'decisions/' || d.name"],
    ['query', '-s', 'SELECT * FROM __attendant_edges ORDER BY source_path,field'],
    ['query', '-s', 'PRAGMA table_info(tasks)'],
    ['query', '-s', 'SELECT name FROM tasks ORDER BY name', '-l', '1'],
    ['query', '-s', "SELECT 'semi;colon' x"],
    ['search', '-q', 'lantern'],
    ['search', '-q', 'anter'],
    ['search', '-q', 'café'],
    ['search', '-q', 'lantern OR native'],
    ['search', '-q', '"lantern id"'],
    ['search', '-q', 'lantern', '-c', 'tasks', '-l', '1'],
    ['search', '-q', 'dependencies', '-c', 'decisions'],
  ];
  for (const args of comparisons) {
    const actual = native(...args), expected = oracle(...args);
    if (args[0] === 'search') {
      // SQLite builds may differ by a few FP ULPs in bm25, not match order.
      assert.equal(actual.rows.length, expected.rows.length);
      for (let i = 0; i < actual.rows.length; i++) {
        const score = expected.rows[i].score;
        assert.ok(Math.abs(actual.rows[i].score - score) <= Number.EPSILON * 16 * Math.max(Math.abs(score), 1e-30));
        actual.rows[i].score = score;
      }
    }
    assert.deepEqual(actual, expected, `Native differs: ${args.join(' ')}`);
  }
  // Intentional corrections to reporting bugs in mjs: metadata survives empty
  // results, and limit zero really returns no rows.
  assert.deepEqual(native('query', '-s', "SELECT 'semi;colon' x;").rows, [{ x: 'semi;colon' }]);
  const empty = native('query', '-s', 'SELECT name FROM tasks WHERE 0');
  assert.deepEqual(empty, { columns: ['name'], rows: [], returned: 0, truncated: false });
  const zero = native('query', '-s', 'SELECT name FROM tasks', '-l', '0');
  assert.deepEqual(zero, { columns: ['name'], rows: [], returned: 0, truncated: true });
  const searchEmpty = native('search', '-q', 'notafoundtoken');
  assert.equal(searchEmpty.returned, 0);
  assert.deepEqual(searchEmpty.columns, ['collection', 'id', 'source_path', 'name', 'score', 'snippet']);
  assert.equal(native('search', '-q', 'lantern', '-l', '0').returned, 0);

  // Check raw bytes too: spawnSync's UTF-8 string decoding would hide invalid
  // native stdout by replacing it before JSON.parse sees it.
  for (const hex of ['ff', 'c080', 'eda080', 'e282', 'f4908080', '00ff0061']) {
    const sql = `SELECT CAST(x'${hex}' AS TEXT) x`;
    const raw = spawnSync(executable, ['query', '-s', sql, '-p', project], { timeout: 30_000 });
    assert.equal(raw.status, 0);
    const text = new TextDecoder('utf-8', { fatal: true }).decode(raw.stdout);
    assert.deepEqual(JSON.parse(text), oracle('query', '-s', sql));
  }
  for (const sql of ["SELECT replace(hex(zeroblob(1572864)), '0', char(1)) AS x", 'SELECT zeroblob(1048576) x']) {
    const actual = native('query', '-s', sql), expected = oracle('query', '-s', sql);
    assert.deepEqual(actual, expected);
    assert.equal(actual.returned, 0); assert.equal(actual.truncated, true);
  }
  const byteCap = 'SELECT hex(zeroblob(2200000)) x UNION ALL SELECT hex(zeroblob(2200000)) UNION ALL SELECT hex(zeroblob(2200000))';
  const large = native('query', '-s', byteCap);
  assert.deepEqual(large, oracle('query', '-s', byteCap));
  assert.equal(large.returned, 2); assert.equal(large.truncated, true);
  const inputs = join(root, 'build/read-inputs'); mkdirSync(inputs, { recursive: true });
  writeFileSync(join(inputs, 'query.sql'), 'SELECT :status status');
  writeFileSync(join(inputs, 'params.json'), '{"status":"active"}');
  assert.deepEqual(native('query', '-s', `@${join(inputs, 'query.sql')}`, '-P', `@${join(inputs, 'params.json')}`).rows, [{ status: 'active' }]);
  assert.deepEqual(run(executable, ['query', '-s', '-', '-p', project], { input: 'SELECT 7 x' }).rows, [{ x: 7 }]);
  assert.deepEqual(run(executable, ['query', '-s', 'SELECT :x x', '-P', '-', '-p', project], { input: '{"x":true}' }).rows, [{ x: 1 }]);
  assert.deepEqual(run(executable, ['query', '-s', 'SELECT 9 x'], { cwd: project }).rows, [{ x: 9 }]);
  assert.ok(run(executable, ['--help']).usage);
  assert.ok(native('query', '--help').query);

  const failures = [
    ['query', '-s', 'DELETE FROM tasks'],
    ['query', '-s', "UPDATE tasks SET status='done'"],
    ['query', '-s', 'DROP TABLE tasks'],
    ['query', '-s', "ATTACH DATABASE ':memory:' AS other"],
    ['query', '-s', 'CREATE TEMP TABLE nope(x)'],
    ['query', '-s', "SELECT load_extension('nope')"],
    ['query', '-s', 'SELECT * FROM __attendant_cache'],
    ['query', '-s', 'SELECT * FROM __attendant_metadata'],
    ['query', '-s', 'SELECT 1; SELECT 2'],
    ['query', '-s', 'SELECT 1 -- comment'],
    ['query', '-s', 'SELECT 1 /* comment */'],
    ['query', '-s', 'SELECT :x', '-P', '{x:1}'],
    ['query', '-s', 'SELECT :x', '-P', '{"x":1,}'],
    ['query', '-s', 'SELECT :x', '-P', '{"x":1} {}'],
    ['query', '-s', 'SELECT :x', '-P', '[]'],
    ['query', '-s', 'SELECT :x', '-P', '{"x":[1]}'],
    ['query', '-s', 'SELECT :x', '-P', '{"unknown":1}'],
    ['query', '-s', 'SELECT 9223372036854775807'],
    ['query', '-s', 'SELECT 1', '--sql', 'SELECT 2'],
    ['query', '-s', 'SELECT 1', '-l', '-1'],
    ['query', '-s', '-', '-P', '-'],
    ['query', '--unknown', 'x'],
    ['search', '-q', ''],
    ['search', '-q', 'lantern', '-c', 'missing'],
    ['search', '-q', 'lantern', '-c', 'tasks,tasks'],
    ['search', '-q', '"unfinished'],
    ['schema', '-s', 'SELECT 1'],
  ];
  for (const args of failures) {
    const failure = run(executable, [...args, '-p', project], { expected: 1, input: '{}' });
    assert.equal(typeof failure.error.code, 'string');
    assert.equal(typeof failure.error.message, 'string');
  }
  const budget = run(executable, ['query', '-p', project, '-s', 'WITH RECURSIVE r(x) AS (VALUES(1) UNION ALL SELECT x+1 FROM r) SELECT sum(x) FROM r'], { expected: 1 });
  assert.equal(budget.error.code, 'sql-budget');
  const capped = native('query', '-s', 'WITH RECURSIVE r(x) AS (VALUES(1) UNION ALL SELECT x+1 FROM r WHERE x<10001) SELECT x FROM r', '-l', '10001');
  assert.equal(capped.returned, 10000); assert.equal(capped.truncated, true);
  assert.deepEqual(sourceHashes(project), frozen, 'Read commands changed source');
  assert.equal(databaseHash(), before, 'Read commands changed DB');
  // Changed configuration/schema now triggers automatic preparation.
  for (const path of [join(project, '.pi/attendant.tables'), join(project, 'records/tasks/.schema.md')]) {
    const original = readFileSync(path);
    writeFileSync(path, Buffer.concat([original, Buffer.from('\n# changed\n')]));
    assert.deepEqual(native('schema'), oracle('schema'));
    assert.equal(native('sync').state, 'reused');
    writeFileSync(path, original);
    assert.equal(native('sync').state, 'rebuilt');
  }
  assert.deepEqual(sourceHashes(project), frozen);
  assert.equal(native('sync').state, 'reused');
  // Read interoperability goes both ways: native commands also consume a DB
  // generated entirely by the unchanged mjs runner.
  rmSync(join(project, '.attendant'), { recursive: true, force: true });
  assert.equal(oracle('sync').state, 'rebuilt');
  const runnerSnapshot = databaseHash();
  assert.equal(native('sync').state, 'reused', 'Native preparation must reuse a runner-produced DB');
  for (const args of [['schema'], ['query', '-s', 'SELECT name FROM tasks ORDER BY name'], ['search', '-q', 'lantern']]) {
    const actual = native(...args), expected = oracle(...args);
    if (args[0] === 'search') {
      for (let i = 0; i < actual.rows.length; i++) {
        assert.ok(Math.abs(actual.rows[i].score - expected.rows[i].score) <= Number.EPSILON * 16 * Math.abs(expected.rows[i].score));
        actual.rows[i].score = expected.rows[i].score;
      }
    }
    assert.deepEqual(actual, expected);
  }
  assert.equal(databaseHash(), runnerSnapshot, 'Native consumption replaced or wrote runner DB');
  rmSync(join(project, '.attendant'), { recursive: true, force: true });
  run(executable, [project]);
  assert.equal(oracle('sync').state, 'reused');
  assert.deepEqual(sourceHashes(project), frozen);
  const tieProject = `${project} ties`;
  rmSync(tieProject, { recursive: true, force: true }); mkdirSync(join(tieProject, '.pi'), { recursive: true });
  const aliases = ['alpha', 'Beta', 'a-b', 'a_b', 'a.b', 'a b', 'café', 'cafe'];
  const config = aliases.map((alias, i) => {
    cpSync(join(root, 'fixtures/project/records/decisions'), join(tieProject, `records${i}`), { recursive: true });
    return `records${i} as ${alias}`;
  }).join('\n');
  writeFileSync(join(tieProject, '.pi/attendant.tables'), config);
  run(executable, [tieProject]);
  for (const limit of ['1', '100']) {
    const args = ['search', '-q', 'dependencies', '-l', limit, '-p', tieProject];
    const actual = run(executable, args), expected = run(process.execPath, [runner, ...args]);
    const again = run(executable, args);
    assert.deepEqual(actual, again, 'Native tie ordering must be repeatable');
    assert.deepEqual(actual.rows.map(row => row.collection), [...aliases].sort().slice(0, +limit));
    if (+limit >= aliases.length) assert.deepEqual(actual.rows.map(row => row.collection).sort(), expected.rows.map(row => row.collection).sort());
  }
  const missing = join(root, 'build/no-projection');
  rmSync(missing, { recursive: true, force: true }); mkdirSync(missing, { recursive: true });
  const error = run(executable, ['schema', '-p', missing], { expected: 1 });
  assert.ok(error.error);
  report.push({ executable, variant, comparisons: comparisons.length, rejectedCases: failures.length, snapshotUnchanged: true, sourceUnchanged: true, consumedRunnerProjection: true });
  console.log(JSON.stringify(report.at(-1)));
}
writeFileSync(join(root, 'build/review/read-results.json'), JSON.stringify(report, null, 2) + '\n');
