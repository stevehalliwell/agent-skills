// Focused native CLI contract checks. No JS oracle; only generated workspace copies.
// Usage: node tests/core/cli-check.mjs
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { cpSync, existsSync, mkdirSync, mkdtempSync, readFileSync, readdirSync, rmSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
mkdirSync(join(root, 'build/cli-check'), { recursive: true });
const report = { ok: false, run: mkdtempSync(join(root, 'build/cli-check/run-')), cases: [] };
function inventory(folder, includeState = false) {
  return Object.fromEntries(readdirSync(folder, { withFileTypes: true }).filter(entry => includeState || entry.name !== '.attendant').flatMap(entry => {
    const path = join(folder, entry.name);
    return entry.isDirectory() ? Object.entries(inventory(path, includeState)) : [[path, createHash('sha256').update(readFileSync(path)).digest('hex')]];
  }));
}
try {
  for (const compiler of ['cjit', 'zig']) {
    const project = join(report.run, `${compiler} café Ω`), executable = join(root, `build/attendant-${compiler}.exe`);
    cpSync(join(root, 'tests/fixtures/project'), project, { recursive: true });
    const check = name => report.cases.push({ compiler, name });
    function cli(args, status = 0, input) {
      const result = spawnSync(executable, [...args, '-p', project], { encoding: 'utf8', input, timeout: 30000, windowsHide: true });
      assert.equal(result.status, status, `${args[0]}: ${result.error || result.stderr}\n${result.stdout}`);
      return JSON.parse(result.stdout);
    }
    const original = inventory(project), first = join(project, 'records/tasks/first.md');
    assert.deepEqual(cli(['validate', '--strict']).diagnostics, []);
    assert.deepEqual(inventory(project), original); assert(!existsSync(join(project, '.attendant')));
    check('strict validation does not write source or prepare projection');
    assert.equal(cli(['sync']).state, 'rebuilt'); assert.equal(cli(['sync']).state, 'reused');
    assert.deepEqual(inventory(project), original);
    const healthBefore = inventory(project, true);
    assert.equal(cli(['doctor']).ok, true); assert.deepEqual(inventory(project, true), healthBefore);
    check('cold build, unchanged reuse and read-only doctor');
    const schema = cli(['schema']); assert.deepEqual(schema.collections.map(collection => collection.name), ['decisions', 'tasks']);
    const query = sql => cli(['query', '-s', sql]);
    assert.deepEqual(query('SELECT name,status FROM tasks ORDER BY name').rows,
      [{ name: 'first', status: 'active' }, { name: 'second', status: 'todo' }]);
    assert.equal(cli(['query', '-s', 'SELECT name FROM tasks WHERE status=:status', '-P', '{"status":"active"}']).rows[0].name, 'first');
    assert.equal(cli(['search', '-q', 'projection']).rows[0].name, 'first');
    assert.equal(query("SELECT name FROM tasks WHERE name='absent'").returned, 0);
    check('schema, parameterized SQL, hidden filtering, empty result and first search');
    const deniedBefore = inventory(project);
    assert.equal(cli(['query', '-s', 'DELETE FROM tasks'], 1).error.code, 'sql-denied');
    assert.deepEqual(inventory(project), deniedBefore); check('query authorizer rejects writes');
    const source = readFileSync(first, 'utf8'), body = source.slice(source.indexOf('\n---', 4));
    cli(['update', '-c', 'tasks', '-i', JSON.stringify([{ name: 'first', fields: { status: 'done' } }])]);
    const updated = readFileSync(first, 'utf8');
    assert.equal(updated, source.replace('status: active', 'status: "done"'));
    assert(updated.endsWith(body)); assert.equal(query("SELECT status FROM tasks WHERE name='first'").rows[0].status, 'done');
    check('metadata update preserves body and unrelated formatting; projection follows');
    const beforeBatch = inventory(project);
    cli(['update', '-c', 'tasks', '-i', JSON.stringify([
      { name: 'first', fields: { status: 'todo' } }, { name: 'second', fields: { status: 'not-an-enum-value' } }
    ])], 1);
    assert.deepEqual(inventory(project), beforeBatch); check('invalid batch makes no partial source changes');
    const create = [{ name: 'created', fields: { summary: 'native café Ω 😀', labels: ['native'], status: 'todo' } }];
    const input = join(project, 'items.json'); writeFileSync(input, JSON.stringify(create));
    cli(['create', '-c', 'tasks', '-i', '@' + input]); rmSync(input);
    assert.equal(query("SELECT enabled,priority,summary FROM tasks WHERE name='created'").rows[0].priority, 1);
    cli(['update', '-c', 'tasks', '-i', '-'], 0, JSON.stringify([{ name: 'created', fields: { summary: 'stdin value' } }]));
    assert.equal(query("SELECT summary FROM tasks WHERE name='created'").rows[0].summary, 'stdin value');
    check('create applies defaults/template; @file and stdin inputs work');
    cli(['create', '-c', 'tasks', '-i', '[{"name":"exact-max","fields":{"priority":9223372036854775807,"misc":[18446744073709551615,0]}}]']);
    cli(['create', '-c', 'tasks', '-i', '[{"name":"exact-min","fields":{"priority":-9223372036854775808}}]']);
    assert.equal(query("SELECT CAST(priority AS TEXT) AS value FROM tasks WHERE name='exact-max'").rows[0].value, '9223372036854775807');
    assert.equal(query("SELECT CAST(priority AS TEXT) AS value FROM tasks WHERE name='exact-min'").rows[0].value, '-9223372036854775808');
    assert.match(query("SELECT misc FROM tasks WHERE name='exact-max'").rows[0].misc, /18446744073709551615/);
    const beforeInvalid = inventory(project);
    for (const fields of ['{"priority":18446744073709551615}', '{"due":"2026-02-29"}', '{"priority":1e999}', '{"depends_on":["missing"]}'])
      cli(['create', '-c', 'tasks', '-i', `[{"name":"rejected","fields":${fields}}]`], 1);
    assert.deepEqual(inventory(project), beforeInvalid);
    check('exact integer bounds and u64 arrays; reject unsupported scalar, invalid date, nonfinite and missing reference');
    const separators = 'NEL\u0085LS\u2028PS\u2029';
    cli(['update', '-c', 'tasks', '-i', JSON.stringify([{ name: 'created', fields: { summary: separators, due: '2028-02-29', observed_at: '2026-10-31T12:00:00.000Z' } }])]);
    assert.equal(query("SELECT summary FROM tasks WHERE name='created'").rows[0].summary, separators);
    writeFileSync(first, readFileSync(first, 'utf8') + '\nuniquesearchtoken\n');
    assert.equal(cli(['sync']).state, 'updated');
    assert.equal(cli(['search', '-q', 'uniquesearchtoken']).rows[0].name, 'first');
    rmSync(join(project, 'records/tasks/created.md'));
    assert.equal(query("SELECT name FROM tasks WHERE name='created'").returned, 0);
    rmSync(join(project, '.attendant'), { recursive: true });
    assert.equal(query("SELECT name FROM tasks WHERE name='first'").returned, 1);
    check('Unicode separators, canonical dates, changed-body search, deletion reconciliation and disposable-cache rebuild');
    const added = cli(['add-table', '-d', 'records/notes', '-a', 'notes']);
    assert.equal(added.name, 'notes'); assert(existsSync(added.schemaPath));
    assert(cli(['schema']).collections.some(collection => collection.name === 'notes'));
    check('add-table publishes collection seeds and registration');
  }
  report.ok = true;
} finally { writeFileSync(join(root, 'build/cli-check-results.json'), JSON.stringify(report, null, 2) + '\n'); }
console.log(JSON.stringify({ ok: report.ok, cases: report.cases.length, run: report.run }));
