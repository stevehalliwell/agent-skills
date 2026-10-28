// Copied real-data workflow acceptance; never invoke a runner in captured/live projects.
// Usage: node real-data-check.mjs --manifest <capture/manifest.json> [--cjit <exe>] [--zig <exe>]
// Reuses captured inventory/containment checks. Fresh uniquely owned workspaces per run.
// Correctness only: no performance claims. Node is test infrastructure, not native runtime.
import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import { mkdirSync, mkdtempSync, readFileSync, writeFileSync, rmSync, renameSync, unlinkSync, utimesSync } from 'node:fs';
import { basename, dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';
import { inventory, localPath } from './dataset-files.mjs';
const root = dirname(fileURLToPath(import.meta.url));
const runnerScript = resolve(root, '../../skills/attendant/scripts/attendant.mjs');
const usage = 'node real-data-check.mjs --manifest <capture/manifest.json> [--cjit <exe>] [--zig <exe>]';
const fixedTime = new Date('2026-01-01T00:00:00.000Z');
const quote = text => '"' + text.replaceAll('"', '""') + '"';
const body = bytes => {
  const text = bytes.toString('utf8'), header = /^---\r?\n[\s\S]*?\r?\n---(?:\r?\n|$)/.exec(text);
  return header ? Buffer.from(text.slice(header[0].length)) : bytes;
};
const hash = path => createHash('sha256').update(readFileSync(path)).digest('hex');
export function parseOptions(args) {
  const options = { cjit: join(root, 'build/projection-cjit.exe'), zig: join(root, 'build/projection-zig.exe') }, seen = new Set();
  for (let i = 0; i < args.length; i++) {
    assert.ok(['--manifest', '--cjit', '--zig'].includes(args[i]), `Unknown option: ${args[i]}`);
    const key = args[i].slice(2); assert.ok(!seen.has(key), `Repeated option: ${args[i]}`); seen.add(key);
    assert.ok(args[i + 1] && !args[i + 1].startsWith('--'), `Missing value: ${args[i]}`); options[key] = resolve(args[++i]);
  }
  assert.ok(options.manifest, '--manifest is required'); return options;
}
function unicodeAudit(datasets) {
  // Compare actual captured characters with precisely the native NLS predicate.
  const points = [...new Set(datasets.flatMap(d => d.files.flatMap(f => [...readFileSync(localPath(d.pristine, f.path), 'utf8')].map(c => c.codePointAt(0)))))].filter(p => p > 127).sort((a, b) => a - b);
  const script = `$ErrorActionPreference='Stop'; $ProgressPreference='SilentlyContinue'; Add-Type -TypeDefinition @'
using System; using System.Runtime.InteropServices;
public class AttendantWordAudit {
 [DllImport("kernel32.dll", CharSet=CharSet.Unicode)] public static extern bool GetStringTypeW(uint type, string text, int count, [Out] ushort[] categories);
 public static bool Word(int point) { string text=char.ConvertFromUtf32(point); ushort[] categories=new ushort[text.Length]; if (!GetStringTypeW(1,text,text.Length,categories)) throw new InvalidOperationException("NLS classification failed"); return (categories[0] & 0x104)!=0; }
}
'@
@(${points.join(',')}) | ForEach-Object { Write-Output ("{0}:{1}" -f $_,([int][AttendantWordAudit]::Word($_))) }`;
  const result = spawnSync('powershell.exe', ['-NoProfile', '-NonInteractive', '-EncodedCommand', Buffer.from(script, 'utf16le').toString('base64')], { encoding: 'utf8', timeout: 30000 });
  assert.equal(result.status, 0, result.stderr);
  const categories = new Map(result.stdout.trim().split(/\r?\n/).filter(Boolean).map(line => line.split(':').map(Number)));
  assert.equal(categories.size, points.length, 'Incomplete NLS audit');
  return datasets.map(d => {
    const mismatches = [], affectedWords = new Set(), unicodeWords = new Set(); let supplementaryLettersOrNumbers = 0;
    for (const f of d.files) {
      const text = readFileSync(localPath(d.pristine, f.path), 'utf8');
      for (const match of text.matchAll(/[\p{L}\p{N}]+/gu)) if (/[^\x00-\x7f]/.test(match[0])) unicodeWords.add(match[0]);
      for (const character of new Set([...text])) {
        const point = character.codePointAt(0), jsWord = /[\p{L}\p{N}]/u.test(character);
        if (point > 65535 && jsWord) supplementaryLettersOrNumbers++;
        if (point > 127 && jsWord !== Boolean(categories.get(point))) {
          mismatches.push({ path: f.path, character, codepoint: 'U+' + point.toString(16).toUpperCase(), jsWord, nativeWord: Boolean(categories.get(point)) });
          for (const match of text.matchAll(/[\p{L}\p{N}]+/gu)) if (match[0].includes(character)) affectedWords.add(match[0]);
        }
      }
    }
    return { dataset: d.label, files: d.files.length, supplementaryLettersOrNumbers, mismatches, affectedWords: [...affectedWords].sort(), unicodeWords: [...unicodeWords].sort() };
  });
}
export function runChecks(options) {
  const manifest = JSON.parse(readFileSync(options.manifest, 'utf8'));
  assert.ok(Array.isArray(manifest.datasets) && manifest.datasets.length, 'Manifest has no datasets');
  const datasets = manifest.datasets.map(d => {
    const captured = inventory(d.pristine);
    assert.deepEqual(captured, { collections: d.collections, files: d.files }, 'Capture differs from manifest');
    return { ...d, label: basename(dirname(d.pristine)), captured };
  });
  assert.equal(new Set(datasets.map(d => d.label.toLowerCase())).size, datasets.length, 'Repeated dataset label');
  const parent = join(root, 'build/real-data-check'); mkdirSync(parent, { recursive: true });
  const directory = mkdtempSync(join(parent, 'run-'));
  const report = { complete: false, directory, manifest: options.manifest, unicode: [], results: [], artifacts: Object.fromEntries([['mjs', runnerScript], ['cjit', options.cjit], ['zig', options.zig], ['harness', fileURLToPath(import.meta.url)], ['inventory', join(root, 'dataset-files.mjs')]].map(([name, path]) => [name, { path, sha256: hash(path) }])) };
  const save = () => writeFileSync(join(directory, 'report.json'), JSON.stringify(report, null, 2) + '\n');
  try {
    report.unicode = unicodeAudit(datasets); save();
    for (const dataset of datasets) {
      let baseline;
      for (const [name, command, prefix] of [['mjs', process.execPath, [runnerScript]], ['cjit', options.cjit, []], ['zig', options.zig, []]]) {
        const project = join(directory, dataset.label, name), cases = [];
        for (const f of dataset.files) {
          const target = join(project, f.path); mkdirSync(dirname(target), { recursive: true });
          writeFileSync(target, readFileSync(localPath(dataset.pristine, f.path)), { flag: 'wx' }); utimesSync(target, fixedTime, fixedTime);
        }
        const result = { dataset: dataset.label, runner: name, project, cases }; report.results.push(result);
        const invoke = (args, expected = 0, executable = command, leading = prefix) => {
          const child = spawnSync(executable, [...leading, ...args, '-p', project], { encoding: 'utf8', timeout: 30000, maxBuffer: 24 * 1024 * 1024 });
          assert.equal(child.status, expected, `${name} ${args.join(' ')}: ${child.stderr}\n${child.stdout}`);
          if (expected) return; // Legacy failures intentionally have no stdout JSON.
          const data = JSON.parse(child.stdout); assert.ok(!data.error, child.stdout);
          assert.equal(data.diagnostics?.length ?? 0, 0, 'Unexpected diagnostics');
          assert.equal(data.corrections?.length ?? 0, 0, 'Unexpected correction'); return data;
        };
        const query = (sql, params = {}) => invoke(['query', '-s', sql, '-P', JSON.stringify(params), '-l', '10000']);
        const paths = new Map(dataset.collections.map(c => [c.name, join(project, c.directory)]));
        const tasks = paths.get('tasks'); assert.ok(tasks && paths.get('decisions'), 'Expected captured task/decision collections');
        const strict = () => { const r = invoke(['validate', '--strict']); assert.equal(r.diagnostics.length, 0); };
        strict(); assert.equal(invoke(['sync']).state, 'rebuilt');
        const beforeReads = inventory(project), schema = invoke(['schema']);
        for (const c of schema.collections) assert.equal(c.usage, readFileSync(join(paths.get(c.name), '.usage.md'), 'utf8'));
        const baselineSql = 'SELECT name,status,priority,desc,tags,depends_on FROM tasks ORDER BY name';
        const rows = query(baselineSql);
        if (name === 'mjs') baseline = rows; else assert.deepEqual(rows, baseline);
        for (const c of dataset.collections) assert.equal(query(`SELECT count(*) n FROM ${quote(c.name)}`).rows[0].n, c.records);
        assert.equal(invoke(['sync']).state, 'reused'); assert.deepEqual(inventory(project), beforeReads);
        cases.push('strict validation, schema/live usage, cold sync, baseline query parity and reuse without source writes'); save();
        for (const text of ['task', ...report.unicode.find(a => a.dataset === dataset.label).unicodeWords]) {
          const hits = invoke(['search', '-q', text, '-l', '10000']);
          assert.ok(hits.returned > 0, `Expected captured search term to find records: ${text}`);
          const key = `search:${text}`, normalized = hits.rows.map(r => `${r.collection}/${r.name}`).sort();
          if (name === 'mjs') result[key] = normalized;
          else assert.deepEqual(normalized, report.results.find(r => r.dataset === dataset.label && r.runner === 'mjs')[key], `Real-data search differs for ${text}`);
        }
        assert.deepEqual(inventory(project), beforeReads); cases.push('first and Unicode-context searches agree with mjs without source changes');
        const target = rows.rows[0].name, targetPath = join(tasks, target + '.md');
        assert.ok(dataset.files.some(f => join(project, f.path) === targetPath), 'Selected target must be captured filename');
        const oldBody = body(readFileSync(targetPath));
        const update = (collection, items, expected = 0) => invoke(['update', '-c', collection, '-i', JSON.stringify(items)], expected);
        update('tasks', [{ name: target, fields: { desc: 'Attendantworkflowexisting token.', priority: 17, tags: ['workflow-check'] } }]);
        const updated = query('SELECT desc,priority,tags FROM tasks WHERE name=:name', { name: target }).rows[0];
        assert.deepEqual(updated, { desc: 'Attendantworkflowexisting token.', priority: 17, tags: '["workflow-check"]' });
        assert.deepEqual(body(readFileSync(targetPath)), oldBody);
        assert.equal(invoke(['search', '-q', 'Attendantworkflowexisting']).returned, 1);
        cases.push('edit existing real record, parameterized query and indexed search; body preserved');
        const beforeReject = inventory(project);
        update('tasks', [{ name: target, fields: { desc: 'Must not publish.' } }, { name: target + '-missing', fields: { status: 'invalid-status' } }], 1);
        assert.deepEqual(inventory(project), beforeReject); cases.push('rejected update batch preserves all source bytes');
        const alpha = 'attendant-workflow-alpha', beta = 'attendant-workflow-beta', defaults = 'attendant-workflow-defaults';
        invoke(['create', '-c', 'tasks', '-i', JSON.stringify([{ name: alpha, fields: { status: 'todo', priority: 3, desc: 'Attendantworkflowalpha.', tags: ['workflow-check'] } }, { name: beta, fields: { status: 'todo', desc: 'Attendantworkflowbeta.' } }])]);
        invoke(['create', '-c', 'tasks', '-i', JSON.stringify([{ name: defaults }])]);
        const defaultRow = query('SELECT status,priority FROM tasks WHERE name=:name', { name: defaults }).rows[0];
        assert.deepEqual(defaultRow, { status: 'needs-refinement', priority: 0 });
        assert.deepEqual(body(readFileSync(join(tasks, defaults + '.md'))), body(readFileSync(join(tasks, '.template.md'))));
        update('tasks', [{ name: alpha, fields: { priority: 4 } }, { name: beta, fields: { priority: 5, depends_on: ['tasks/' + alpha] } }]);
        assert.equal(query('SELECT depends_on FROM tasks WHERE name=:name', { name: beta }).rows[0].depends_on, JSON.stringify(['tasks/' + alpha]));
        assert.deepEqual(query('SELECT name,priority FROM tasks WHERE name IN (:alpha,:beta) ORDER BY name', { alpha, beta }).rows, [{ name: alpha, priority: 4 }, { name: beta, priority: 5 }]);
        assert.equal(query("SELECT target_name FROM __attendant_edges WHERE source_collection='tasks' AND source_path=:path AND field='depends_on'", { path: join(tasks, beta + '.md') }).rows[0].target_name, alpha);
        strict(); cases.push('batch create/update, defaults, template body and reference edges validated on real schema');
        const decision = 'attendant-workflow-decision';
        invoke(['create', '-c', 'decisions', '-i', JSON.stringify([{ name: decision, fields: { status: 'proposed', desc: 'Attendantworkflowdecision.', revisit_triggers: ['workflow-check'] } }])]);
        update('decisions', [{ name: decision, fields: { revisit_triggers: ['workflow-check', 'changed-input'] } }]);
        assert.equal(query('SELECT revisit_triggers FROM decisions WHERE name=:name', { name: decision }).rows[0].revisit_triggers, '["workflow-check","changed-input"]');
        cases.push('create and update second collection with string-array fields');
        writeFileSync(targetPath, Buffer.concat([readFileSync(targetPath), Buffer.from('\nAttendantworkflowbodyedit.\n')]));
        assert.equal(invoke(['search', '-q', 'Attendantworkflowbodyedit']).returned, 1);
        cases.push('direct body edit picked up by automatic preparation and search');
        const defaultPath = join(tasks, defaults + '.md'), visible = readFileSync(defaultPath, 'utf8');
        writeFileSync(defaultPath, visible.replace(/\r?\n---(?:\r?\n|$)/, match => '\nhidden: true' + match));
        invoke(['sync']); assert.equal(query('SELECT name FROM tasks WHERE name=:name', { name: defaults }).returned, 0);
        writeFileSync(defaultPath, visible); invoke(['sync']); assert.equal(query('SELECT name FROM tasks WHERE name=:name', { name: defaults }).returned, 1);
        cases.push('hidden/unhidden transitions reconcile collection visibility');
        const renamed = join(tasks, defaults + '-renamed.md'); renameSync(defaultPath, renamed); invoke(['sync']);
        assert.equal(query('SELECT source_path FROM tasks WHERE name=:name', { name: defaults }).rows[0].source_path, renamed);
        unlinkSync(join(tasks, beta + '.md')); invoke(['sync']); assert.equal(query('SELECT name FROM tasks WHERE name=:name', { name: beta }).returned, 0);
        assert.equal(invoke(['search', '-q', 'Attendantworkflowbeta']).returned, 0);
        assert.equal(query('SELECT count(*) n FROM __attendant_edges WHERE source_path=:path', { path: join(tasks, beta + '.md') }).rows[0].n, 0);
        strict(); cases.push('external rename and deletion reconcile paths/cache/rows');
        const beforeRebuild = inventory(project), expectedFinal = query(baselineSql);
        rmSync(join(project, '.attendant'), { recursive: true, force: true });
        assert.equal(invoke(['search', '-q', 'Attendantworkflowbodyedit']).returned, 1);
        assert.deepEqual(query(baselineSql), expectedFinal); assert.deepEqual(inventory(project), beforeRebuild);
        cases.push('disposable cache deletion and search-first rebuild preserve current logical data/source');
        if (name !== 'mjs') assert.equal(invoke(['sync'], 0, process.execPath, [runnerScript]).state, 'reused');
        else assert.equal(invoke(['sync'], 0, options.zig, []).state, 'reused');
        assert.equal(invoke(['sync']).state, 'reused'); assert.deepEqual(query(baselineSql), expectedFinal);
        assert.equal(invoke(['doctor']).projection, 'fresh'); strict(); assert.deepEqual(inventory(project), beforeRebuild);
        cases.push('indexed producer handoff, doctor and strict final validation remain source-preserving');
        for (const f of dataset.files) if (join(project, f.path) !== targetPath) assert.equal(hash(join(project, f.path)), f.sha256, `Unintended source edit: ${f.path}`);
        assert.deepEqual(body(readFileSync(targetPath)), Buffer.concat([oldBody, Buffer.from('\nAttendantworkflowbodyedit.\n')]));
        assert.equal(query('SELECT count(*) n FROM tasks').rows[0].n, dataset.collections.find(c => c.name === 'tasks').records + 2);
        assert.equal(query('SELECT count(*) n FROM decisions').rows[0].n, dataset.collections.find(c => c.name === 'decisions').records + 1);
        result.finalRows = expectedFinal;
        if (name !== 'mjs') assert.deepEqual(expectedFinal, report.results.find(r => r.dataset === dataset.label && r.runner === 'mjs').finalRows);
        save(); console.error(`${dataset.label}/${name}: ${cases.length} cases passed`);
      }
      assert.deepEqual(inventory(dataset.pristine), dataset.captured, 'Pristine input changed');
    }
    report.complete = true; save(); return report;
  } catch (error) { report.error = error.message; save(); error.reportDirectory = directory; throw error; }
}
if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  if (process.argv.includes('--help')) { console.log(JSON.stringify({ usage, safety: 'fresh owned copies only; no live/pristine CLI invocation', output: 'unique build/real-data-check/run-*/report.json' })); }
  else {
    let options;
    try { options = parseOptions(process.argv.slice(2)); }
    catch (error) { console.log(JSON.stringify({ error: { code: 'invalid-arguments', message: error.message } })); process.exitCode = 2; }
    if (options) try { const report = runChecks(options); console.log(JSON.stringify({ complete: true, directory: report.directory, unicode: report.unicode, scenarios: report.results.map(({ dataset, runner, cases }) => ({ dataset, runner, cases: cases.length })) })); }
    catch (error) { console.log(JSON.stringify({ error: { code: 'real-data-check-failed', message: error.message, directory: error.reportDirectory } })); console.error(error.message); process.exitCode = 1; }
  }
}
