// Native skill integration over init-project's current templates; no JS runtime oracle.
// Usage: node tests/core/workflow-check.mjs (build both CLI compilers first).
// Creates owned workspaces under build/workflow-check; JSON stdout/report, throws on failure.
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { copyFileSync, existsSync, mkdirSync, mkdtempSync, readFileSync, readdirSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
const templates = resolve(root, '../../init-project/templates/records');
mkdirSync(join(root, 'build/workflow-check'), { recursive: true });
const report = { ok: false, platform: process.platform, run: mkdtempSync(join(root, 'build/workflow-check/run-')), cases: [] };
function inventory(folder) {
  return Object.fromEntries(readdirSync(folder, { withFileTypes: true }).flatMap(entry => {
    const path = join(folder, entry.name);
    return entry.isDirectory() ? Object.entries(inventory(path)) : [[path, createHash('sha256').update(readFileSync(path)).digest('hex')]];
  }));
}
try {
  assert(existsSync(templates), 'Run workflow-check from repository checkout with sibling skills/init-project/templates.');
  for (const compiler of ['cjit', 'zig']) {
    const project = join(report.run, `${compiler} native skills café Ω`);
    mkdirSync(join(project, '.pi'), { recursive: true });
    writeFileSync(join(project, '.pi/attendant.tables'), '');
    writeFileSync(join(project, '.gitignore'), '.attendant/\n');
    const executable = join(root, `build/attendant-${compiler}${process.platform === 'win32' ? '.exe' : ''}`);
    function cli(args, status = 0) {
      const result = spawnSync(executable, [...args, '-p', project], { encoding: 'utf8', timeout: 30000, windowsHide: true });
      assert.equal(result.status, status, `${args[0]}: ${result.error || result.stderr}\n${result.stdout}`);
      return JSON.parse(result.stdout);
    }
    for (const collection of ['tasks', 'decisions']) {
      const added = cli(['add-table', '-d', `records/${collection}`, '-a', collection]);
      assert.equal(added.name, collection);
      const folder = join(project, `records/${collection}`);
      for (const file of ['.schema.md', '.usage.md', '.template.md']) {
        assert.equal(readFileSync(join(folder, file), 'utf8'), file === '.schema.md' ? '---\n---\n' : '');
        copyFileSync(join(templates, collection, file), join(folder, file));
      }
    }
    const schema = cli(['schema']);
    for (const collection of schema.collections) {
      assert.equal(collection.usage, readFileSync(join(templates, collection.name, '.usage.md'), 'utf8'));
      assert.equal(resolve(collection.usagePath), join(project, `records/${collection.name}/.usage.md`));
    }
    report.cases.push({ compiler, name: 'empty setup and approved template authoring; native schema discovers current usage' });
    const items = join(project, 'items.json');
    writeFileSync(items, JSON.stringify([{ name: 'native-capture', fields: { status: 'needs-refinement', desc: 'Native workflow capture' } }]));
    const created = cli(['create', '-c', 'tasks', '-i', '@' + items]);
    assert.equal(created.records.length, 1);
    const source = join(project, 'records/tasks/native-capture.md');
    const original = readFileSync(source, 'utf8');
    assert(original.endsWith(readFileSync(join(templates, 'tasks/.template.md'), 'utf8')));
    const prose = original + '\nHuman-authored nativeworkflowuniquetoken detail.\n';
    writeFileSync(source, prose);
    const body = prose.slice(prose.indexOf('\n---', 4) + 4);
    for (const status of ['todo', 'doing', 'review']) {
      cli(['update', '-c', 'tasks', '-i', JSON.stringify([{ name: 'native-capture', fields: { status } }])]);
      const saved = readFileSync(source, 'utf8');
      assert.equal(saved.slice(saved.indexOf('\n---', 4) + 4), body);
    }
    const rows = cli(['query', '-s', 'SELECT name,status,priority,created_at,source_path FROM tasks WHERE status=:status ORDER BY priority DESC,created_at,name', '-P', '{"status":"review"}']).rows;
    assert.equal(rows.length, 1); assert.equal(rows[0].priority, 0); assert.equal(rows[0].name, 'native-capture');
    assert.equal(typeof rows[0].created_at, 'string');
    assert.equal(cli(['search', '-q', 'nativeworkflowuniquetoken', '-c', 'tasks']).rows[0].name, 'native-capture');
    report.cases.push({ compiler, name: 'capture, refinement/implementation status flow, ordering/search and body preservation' });
    cli(['create', '-c', 'decisions', '-i', JSON.stringify([{ name: 'native-decision', fields: { status: 'accepted', revisit_triggers: ['native-change'] } }])]);
    const decision = cli(['query', '-s', "SELECT 'decisions' AS collection,name,status FROM decisions WHERE revisit_triggers LIKE :trigger_pattern", '-P', '{"trigger_pattern":"%\\"native-change\\"%"}']).rows;
    assert.deepEqual(decision, [{ collection: 'decisions', name: 'native-decision', status: 'accepted' }]);
    report.cases.push({ compiler, name: 'tradeoff-review decision template and documented revisit-trigger query' });
    const before = inventory(project);
    assert.equal(cli(['doctor']).ok, true);
    cli(['migrate'], 1);
    assert.deepEqual(inventory(project), before);
    assert.deepEqual(cli(['validate', '--strict']).diagnostics, []);
    report.cases.push({ compiler, name: 'read-only health check, unsupported migration and strict validation' });
  }
  report.ok = true;
} finally { writeFileSync(join(root, 'build/workflow-check-results.json'), JSON.stringify(report, null, 2) + '\n'); }
console.log(JSON.stringify({ ok: report.ok, cases: report.cases.length, run: report.run }));
