// Focused native model and real Win32/D3D checks. All mutations use owned copies.
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
const gui = (compiler, testing = false) => join(root, guiDirectory, `attendant-gui-${compiler}${testing ? '-check' : ''}.exe`);
const cli = compiler => join(root, `build/attendant-${compiler}.exe`);
const hash = value => createHash('sha256').update(value).digest('hex');
mkdirSync(join(root, 'build/gui-check'), { recursive: true });
const report = { ok: false, run: mkdtempSync(join(root, 'build/gui-check/run-')), cases: [] };
function inventory(folder) {
  return Object.fromEntries(readdirSync(folder, { withFileTypes: true }).flatMap(entry => {
    const path = join(folder, entry.name);
    return entry.isDirectory() ? Object.entries(inventory(path)) : [[path, hash(readFileSync(path))]];
  }));
}
function child(command, args) {
  const result = spawnSync(command, args, { encoding: 'utf8', timeout: 60000, windowsHide: args.includes('--inspect') || args.includes('--reveal') });
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
function window(compiler, project, args, testing = true, file = 'README.md') {
  return child(gui(compiler, testing), ['--project', project, '--assets', assets, '--file', file, '--frames', '70', ...args]);
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
  const running = spawn(gui(compiler, true), ['--project', project, '--assets', assets, '--file', 'README.md', '--frames', '240', '--exercise-selection', mode], { stdio: ['ignore', 'pipe', 'pipe'] });
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
    const smoke = fixture(`${compiler} café Ω`), before = inventory(smoke), shot = join(report.run, `${compiler}.ppm`);
    const rendered = window(compiler, smoke, ['--shot', shot], false, 'records/tasks/first.md');
    assert(rendered.vertices > 1000 && rendered.tabs === 1);
    const pixels = readFileSync(shot), header = /^P6\n(\d+) (\d+)\n255\n/.exec(pixels.subarray(0, 80).toString('ascii'));
    assert(header); assert.equal(pixels.length - header[0].length, Number(header[1]) * Number(header[2]) * 3);
    assert.deepEqual(inventory(smoke), before);
    report.cases.push({ compiler, name: 'production render/ABI smoke without workspace writes' });
    const model = fixture(`${compiler} model`), first = join(model, 'records/tasks/first.md');
    writeFileSync(first, readFileSync(first, 'utf8').replace('depends_on: [second]', 'depends_on: [second, missing]'));
    writeFileSync(join(model, 'records/tasks/unreadable.md'), Buffer.from([0xff, 0xfe, 0]));
    const modeled = child(join(root, guiDirectory, `model-check-${compiler}.exe`), [model]);
    report.cases.push({ compiler, name: 'document/selection/IPC model, source conflicts and safe feedback', result: modeled });
    const diagnostics = fixture(`${compiler} diagnostics`);
    writeFileSync(join(diagnostics, 'records/unavailable'), 'Not a directory.\n');
    const tables = join(diagnostics, '.pi/attendant.tables'); writeFileSync(tables, readFileSync(tables, 'utf8') + '\nrecords/unavailable as unavailable\n');
    const diagnosticBefore = inventory(diagnostics), warned = window(compiler, diagnostics, [], true, 'records/tasks/first.md');
    assert(warned.issues_shown > 0 && warned.tabs === 1); assert.deepEqual(inventory(diagnostics), diagnosticBefore);
    report.cases.push({ compiler, name: 'partial discovery warning preserves healthy navigation and source' });
    const navigation = window(compiler, smoke, ['--exercise'], true, 'records/tasks/first.md');
    assert.equal(navigation.tabs, 2); assert.deepEqual(inventory(smoke), before);
    report.cases.push({ compiler, name: 'reference clicks and tab reuse do not write workspace' });
    const undo = fixture(`${compiler} undo`);
    writeFileSync(join(undo, 'undo-a###same.md'), 'undo original'); writeFileSync(join(undo, 'undo-b###same.md'), 'undo shared');
    window(compiler, undo, ['--exercise-selection', 'undo'], true, 'undo-a###same.md');
    for (const name of ['undo-a###same.md', 'undo-b###same.md']) assert.equal(readFileSync(join(undo, name), 'utf8'), 'undo shared');
    report.cases.push({ compiler, name: 'same-slot literal-path undo isolation and within-file undo/redo' });
    const editing = fixture(`${compiler} editing`), original = readFileSync(join(editing, 'README.md'), 'utf8');
    window(compiler, editing, ['--exercise-edit']);
    assert.equal(readFileSync(join(editing, 'README.md'), 'utf8'), original + ' GUI edit');
    const pending = query(compiler, editing); assert.equal(pending.meta_comments.pending, 1);
    assert.deepEqual(query(compiler, editing).rows, pending.rows); resolveComment(compiler, editing, pending.rows[0]);
    report.cases.push({ compiler, name: 'real edit, undo, dirty-close Cancel, source feedback and explicit resolution' });
    const failed = fixture(`${compiler} saved-not-queued`), conflict = join(failed, 'records/_meta_comments');
    mkdirSync(conflict); writeFileSync(join(conflict, 'user.md'), 'User-owned file.\n');
    const failedBefore = inventory(failed); window(compiler, failed, ['--exercise-save-fail']);
    assert.equal(readFileSync(join(failed, 'README.md'), 'utf8'), original + ' GUI edit');
    const failedAfter = inventory(failed); delete failedBefore[join(failed, 'README.md')]; delete failedAfter[join(failed, 'README.md')];
    assert.deepEqual(failedAfter, failedBefore);
    report.cases.push({ compiler, name: 'failed queue leaves successful save intact and user-owned files unchanged' });
    await previewSelection(compiler, 'multi', 'plain **bold**\n\nsecond *emphasis* and [link](https://example.invalid)\n\n```c\ncode\n```', 'mapped');
    await previewSelection(compiler, 'context', 'plain  \nnext', 'context');
  }
  report.ok = true;
} finally { writeFileSync(join(root, 'build/gui-check-results.json'), JSON.stringify(report, null, 2) + '\n'); }
console.log(JSON.stringify({ ok: report.ok, cases: report.cases.length, run: report.run }));
