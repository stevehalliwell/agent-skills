// Focused native model and real desktop GUI checks. All mutations use owned copies.
// Usage: node tests/gui/check.mjs (build both compilers, production and --check first).
import assert from 'node:assert/strict';
import { spawn, spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { cpSync, mkdirSync, mkdtempSync, readFileSync, readdirSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { guiDirectory, verifyBuild } from '../../tools/gui-build-manifest.mjs';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
const assets = join(root, 'build/deps/richmd/assets');
const suffix = process.platform === 'win32' ? '.exe' : '';
const gui = (compiler, testing = false) => join(root, guiDirectory, `attendant-gui-${compiler}${testing ? '-check' : ''}${suffix}`);
const cli = compiler => join(root, `build/attendant-${compiler}${suffix}`);
const guiEnvironment = { ...process.env };
if (process.platform === 'darwin') delete guiEnvironment.DYLD_LIBRARY_PATH;
const hash = value => createHash('sha256').update(value).digest('hex');
mkdirSync(join(root, 'build/gui-check'), { recursive: true });
const report = { ok: false, platform: process.platform, architecture: process.arch, run: mkdtempSync(join(root, 'build/gui-check/run-')), cases: [] };
function inventory(folder) {
  return Object.fromEntries(readdirSync(folder, { withFileTypes: true }).flatMap(entry => {
    const path = join(folder, entry.name);
    return entry.isDirectory() ? Object.entries(inventory(path)) : [[path, hash(readFileSync(path))]];
  }));
}
function child(command, args) {
  const result = spawnSync(command, args, { encoding: 'utf8', timeout: 60000, env: guiEnvironment, windowsHide: args.includes('--inspect') || args.includes('--reveal') });
  assert.equal(result.status, 0, `${result.error || ''}\n${result.stderr}\n${result.stdout}`);
  return JSON.parse(result.stdout);
}
function fixture(name, text = '# Workspace\r\n\r\nPlain Markdown café Ω 😀.\r\n') {
  const project = join(report.run, name);
  cpSync(join(root, 'tests/fixtures/project'), project, { recursive: true });
  writeFileSync(join(project, '.gui-test-owned'), 'Generated GUI fixture. Safe for test-driver edits.\n');
  writeFileSync(join(project, 'README.md'), text);
  return project;
}
function synchronous(compiler) {
  const built = spawnSync('bash', ['tools/build-gui-tests.sh', compiler], { cwd: root, env: guiEnvironment, encoding: 'utf8', timeout: 180000 });
  assert.equal(built.status, 0, `${built.error || ''}\n${built.stderr}\n${built.stdout}`);
  const runner = built.stdout.trim(), directory = join(report.run, `native-${compiler}`);
  mkdirSync(directory);
  const result = spawnSync(runner, ['-tsv'], { cwd: root, encoding: 'utf8', timeout: 900000,
    env: { ...guiEnvironment, ATTENDANT_TEST_DIRECTORY: directory, ATTENDANT_TEST_COMPILER: compiler } });
  writeFileSync(join(directory, 'results.tsv'), result.stdout || '');
  writeFileSync(join(directory, 'diagnostics.log'), result.stderr || '');
  report.native ??= [];
  report.native.push({ compiler, runner, directory, status: result.status });
  assert.equal(result.status, 0, `${result.error || ''}\n${result.stderr}\n${result.stdout}`);
  const rows = result.stdout.trimEnd().split('\n').map(line => line.replace(/\r$/, '').split('\t'));
  assert.deepEqual(rows[0], ['kind', 'status', 'source', 'name']);
  assert.deepEqual(rows[1], ['suite', '8', '', '']);
  assert.deepEqual(rows.at(-1), ['complete', '0', '8', '8']);
  assert.equal(rows.length, 11);
  const cases = rows.slice(2, -1);
  assert(cases.every(row => row.length === 4 && row[0] === 'test' && row[1] === 'ok'));
  const expected = ['production_render_abi_screenshot_without_workspace_writes', 'existing_c_document_selection_ipc_model',
    'terminal_render_failure_preserves_source', 'partial_discovery_diagnostics_preserve_source',
    'reference_navigation_and_tab_reuse_without_writes', 'literal_path_undo_isolation_and_redo',
    'real_edit_feedback_and_explicit_resolution', 'failed_queue_keeps_save_and_other_files'];
  assert.deepEqual(cases.map(row => row.slice(2)).sort(), expected.map(name => ['tests/gui/synchronous/checks.c', name]).sort());
  const models = [...result.stderr.matchAll(/^model-checks:(\d+)\r?$/gm)];
  assert.equal(models.length, 1);
  const checks = Number(models[0][1]); assert(Number.isInteger(checks) && checks > 0 && checks <= 2147483647);
  for (const row of cases) report.cases.push({ compiler, name: row[3], source: row[2],
    ...(row[3] === 'existing_c_document_selection_ipc_model' ? { result: { ok: true, checks } } : {}) });
}
const sql = 'SELECT name,status,target_hash,range_start,range_end,selected_text,source_path FROM _meta_comments';
const query = (compiler, project) => child(cli(compiler), ['query', '-p', project, '-s', sql]);
function resolveComment(compiler, project, row) {
  const before = readFileSync(row.source_path, 'utf8'), body = before.slice(before.indexOf('\n---\n') + 5);
  child(cli(compiler), ['update', '-p', project, '-c', '_meta_comments', '-i', JSON.stringify([{ name: row.name, fields: { status: 'resolved' } }])]);
  assert.equal(query(compiler, project).meta_comments.pending, 0);
  const after = readFileSync(row.source_path, 'utf8'); assert.equal(after.slice(after.indexOf('\n---\n') + 5), body);
}
async function previewSelection(compiler, mode, text, status) {
  const project = fixture(`${compiler} ${mode}`, text), before = inventory(project);
  const running = spawn(gui(compiler, true), ['--project', project, '--assets', assets, '--file', 'README.md', '--frames', '240', '--exercise-selection', mode], { stdio: ['ignore', 'pipe', 'pipe'], env: guiEnvironment });
  let output = '', error = '';
  running.stderr.on('data', value => { error += value; });
  const ready = new Promise((resolveReady, reject) => {
    running.stdout.on('data', value => { output += value; if (output.includes('"event":"selection-ready"')) resolveReady(); });
    running.on('error', reject);
    running.on('close', code => { if (!output.includes('"event":"selection-ready"')) reject(new Error(`Selection driver ${code}: ${error}`)); });
  });
  const done = new Promise((resolveDone, reject) => {
    running.on('error', reject); running.on('close', code => code === 0 ? resolveDone() : reject(new Error(`Selection driver ${code}: ${error}`)));
  });
  done.catch(() => {});
  const timer = setTimeout(() => running.kill(), 30000);
  try {
    await ready;
    const other = compiler === 'cjit' ? 'zig' : 'cjit';
    const inspected = child(gui(other), ['--project', project, '--inspect']);
    assert.equal(inspected.selection.view, 'preview'); assert.equal(inspected.selection.status, status);
    const context = JSON.parse(inspected.selection.rendered_context);
    assert.equal(context.sections.map(section => Array.from(section.text).slice(section.highlight_start, section.highlight_end).join('')).join('\n'), inspected.selection.text);
    assert.equal(inspected.document.dirty, mode === 'context');
    if (status === 'context') assert.deepEqual([inspected.selection.range_start, inspected.selection.range_end], [null, null]);
    else assert(context.sections.length >= 3 && inspected.selection.range_end > 0);
    await done;
    const after = inventory(project), target = join(project, 'README.md');
    for (const [path, digest] of Object.entries(before))
      if (path !== join(project, '.pi/attendant.tables') && !(mode === 'context' && path === target)) assert.equal(after[path], digest);
    assert.equal(readFileSync(target, 'utf8'), text + (mode === 'context' ? '\n' : ''));
    const pending = query(compiler, project); assert.equal(pending.meta_comments.pending, 1);
    const row = pending.rows[0]; assert.equal(row.target_hash, hash(readFileSync(target)));
    if (status === 'context') assert.deepEqual([row.range_start, row.range_end, row.selected_text], [0, 0, '']);
    else assert.equal(row.selected_text, Array.from(text).slice(row.range_start, row.range_end).join(''));
    const comment = readFileSync(row.source_path, 'utf8'); assert(comment.includes(inspected.selection.rendered_context));
    resolveComment(compiler, project, row);
    report.cases.push({ compiler, name: `preview ${mode}: cross-compiler inspection, owned context, Save & Send and explicit resolution` });
  } finally { clearTimeout(timer); if (running.exitCode === null) running.kill(); }
}
try {
  for (const compiler of ['cjit', 'zig']) {
    verifyBuild(compiler); verifyBuild(compiler, true);
    synchronous(compiler);
    await previewSelection(compiler, 'multi', 'plain **bold**\n\nsecond *emphasis* and [link](https://example.invalid)\n\n```c\ncode\n```', 'mapped');
    await previewSelection(compiler, 'context', 'plain  \nnext', 'context');
  }
  report.ok = true;
} finally { writeFileSync(join(root, 'build/gui-check-results.json'), JSON.stringify(report, null, 2) + '\n'); }
console.log(JSON.stringify({ ok: report.ok, cases: report.cases.length, run: report.run }));
