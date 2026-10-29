// Real GUI acceptance in unique owned workspace copies; never uses live repos.
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { cpSync, mkdirSync, mkdtempSync, readFileSync, readdirSync, rmSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { verifyBuild } from './build-manifest.mjs';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const stack = resolve(root, '../../spikes/cimgui-richmd-c99');
const legacy = resolve(root, '../../skills/attendant/scripts/attendant.mjs');
function childJSON(command, args) {
  const child = spawnSync(command, args, { encoding: 'utf8', timeout: 60000 });
  assert.equal(child.status, 0, child.stderr + child.stdout); return JSON.parse(child.stdout);
}
function inventory(folder) {
  return Object.fromEntries(readdirSync(folder, { withFileTypes: true }).flatMap(entry => {
    const path = join(folder, entry.name);
    return entry.isDirectory() ? Object.entries(inventory(path)) : [[path, createHash('sha256').update(readFileSync(path)).digest('hex')]];
  }));
}
function bitmap(path) {
  const bytes = readFileSync(path), header = /^P6\n(\d+) (\d+)\n255\n/.exec(bytes.subarray(0, 80).toString('ascii'));
  assert(header); const width = +header[1], height = +header[2], rgb = bytes.subarray(header[0].length);
  assert.equal(rgb.length, width * height * 3);
  const stride = (width * 3 + 3) & ~3, output = Buffer.alloc(54 + stride * height);
  output.write('BM'); output.writeUInt32LE(output.length, 2); output.writeUInt32LE(54, 10); output.writeUInt32LE(40, 14);
  output.writeInt32LE(width, 18); output.writeInt32LE(height, 22); output.writeUInt16LE(1, 26); output.writeUInt16LE(24, 28);
  let bright = 0;
  for (let y = 0; y < height; y++) for (let x = 0; x < width; x++) {
    const source = (y * width + x) * 3, target = 54 + (height - y - 1) * stride + x * 3;
    output[target] = rgb[source + 2]; output[target + 1] = rgb[source + 1]; output[target + 2] = rgb[source];
    if (x > width * 0.65 && rgb[source] > 140 && rgb[source + 1] > 140 && rgb[source + 2] > 140) bright++;
  }
  assert(bright > 1000, 'Preview emits actual text pixels');
  writeFileSync(path.replace('.ppm', '.bmp'), output);
  return { width, height, bright, sha256: createHash('sha256').update(bytes).digest('hex') };
}
mkdirSync(join(root, 'build/gui-check'), { recursive: true });
const report = { ok: false, checkerSha256: createHash('sha256').update(readFileSync(fileURLToPath(import.meta.url))).digest('hex'), run: mkdtempSync(join(root, 'build/gui-check/run-')), results: [] };
try {
  for (const compiler of ['cjit', 'zig']) {
    const production = verifyBuild(compiler), testing = verifyBuild(compiler, true);
    report.results.push({ compiler, provenance: { builtAt: production.builtAt, version: production.version, inputSha256: createHash('sha256').update(JSON.stringify(production.inputs)).digest('hex'), binaries: [...production.binaries, ...testing.binaries], stack: production.stack } });
    const project = join(report.run, `${compiler} caf\u00e9 \u03a9`);
    cpSync(join(root, 'fixtures/project'), project, { recursive: true });
    writeFileSync(join(project, 'README.md'), '# Workspace root file\r\n\r\nPlain Markdown caf\u00e9 \u03a9 \ud83d\ude00 also opens.\r\n');
    const first = join(project, 'records/tasks/first.md');
    writeFileSync(first, readFileSync(first, 'utf8').replace('depends_on: [second]', 'depends_on: [second, missing]'));
    writeFileSync(join(project, 'records/tasks/unreadable.md'), Buffer.from([0xff, 0xfe, 0x00]));
    const before = inventory(project), shot = join(report.run, `${compiler}.ppm`);
    const result = spawnSync(join(root, `build/gui/attendant-gui-${compiler}.exe`), ['--project', project, '--assets', join(stack, 'build/deps/richmd/assets'),
      '--file', 'records/tasks/first.md', '--frames', '30', '--shot', shot], { encoding: 'utf8', timeout: 60000 });
    assert.equal(result.status, 0, result.stderr + result.stdout);
    const output = JSON.parse(result.stdout); assert.equal(output.frames, 30); assert.equal(output.tabs, 1); assert(output.vertices > 1000);
    assert.deepEqual(inventory(project), before, 'Read-only GUI never prepares DB or changes source');
    report.results.push({ compiler, output, screenshot: bitmap(shot) });
    const badFile = childJSON(join(root, `build/gui/attendant-gui-${compiler}.exe`), ['--project', project, '--assets', join(stack, 'build/deps/richmd/assets'),
      '--file', 'records/tasks/unreadable.md', '--frames', '5']);
    assert.equal(badFile.tabs, 0); assert.equal(badFile.frames, 5); assert.deepEqual(inventory(project), before);
    report.results.push({ compiler, badFileRecovered: true });
    const interactionShot = join(report.run, `${compiler}-navigation.ppm`);
    const interaction = spawnSync(join(root, `build/gui/attendant-gui-${compiler}-check.exe`), ['--project', project, '--assets', join(stack, 'build/deps/richmd/assets'),
      '--file', 'records/tasks/first.md', '--frames', '30', '--shot', interactionShot, '--exercise'], { encoding: 'utf8', timeout: 60000 });
    assert.equal(interaction.status, 0, interaction.stderr + interaction.stdout);
    const interactionOutput = JSON.parse(interaction.stdout); assert.equal(interactionOutput.tabs, 2); assert.equal(interactionOutput.frames, 30);
    assert.deepEqual(inventory(project), before, 'Real reference clicks and tab reuse never mutate workspace');
    report.results.push({ compiler, interaction: interactionOutput, screenshot: bitmap(interactionShot) });
    const modelProject = join(report.run, `${compiler} model`); cpSync(project, modelProject, { recursive: true });
    writeFileSync(join(modelProject, '.gui-test-owned'), 'Generated GUI acceptance fixture.\n');
    const model = spawnSync(join(root, `build/gui/model-check-${compiler}.exe`), [modelProject], { encoding: 'utf8', timeout: 30000 });
    assert.equal(model.status, 0, model.stderr + model.stdout); report.results.push({ compiler, model: JSON.parse(model.stdout) });
    const editProject = join(report.run, `${compiler} edited`); cpSync(project, editProject, { recursive: true });
    // Missing-reference visibility uses separate fixture; resolution uses valid project.
    rmSync(join(editProject, 'records/tasks/unreadable.md'));
    const validFirst = join(editProject, 'records/tasks/first.md');
    writeFileSync(validFirst, readFileSync(validFirst, 'utf8').replace('depends_on: [second, missing]', 'depends_on: [second]'));
    writeFileSync(join(editProject, '.gui-test-owned'), 'Generated GUI acceptance fixture.\n');
    const originalSource = readFileSync(join(editProject, 'README.md'), 'utf8'), editedSource = originalSource + ' GUI edit';
    const editShot = join(report.run, `${compiler}-edited.ppm`);
    const editing = childJSON(join(root, `build/gui/attendant-gui-${compiler}-check.exe`), ['--project', editProject, '--assets', join(stack, 'build/deps/richmd/assets'),
      '--file', 'README.md', '--frames', '70', '--shot', editShot, '--exercise-edit']);
    assert.equal(editing.frames, 70); assert.equal(readFileSync(join(editProject, 'README.md'), 'utf8'), editedSource, 'Actual source edits preserve UTF-8, supplementary characters and CRLF');
    report.results.push({ compiler, editing, screenshot: bitmap(editShot) });
    const cli = join(root, `build/projection-${compiler}.exe`);
    const sql = 'SELECT name,status,target_path,target_hash,range_start,range_end,selected_text,range_unit,source_path FROM _meta_comments';
    const query = command => childJSON(command === 'legacy' ? process.execPath : cli, [...(command === 'legacy' ? [legacy] : []), 'query', '-p', editProject, '-s', sql]);
    const update = (command, name, status) => childJSON(command === 'legacy' ? process.execPath : cli, [...(command === 'legacy' ? [legacy] : []), 'update', '-p', editProject, '-c', '_meta_comments', '-i', JSON.stringify([{ name, fields: { status } }])]);
    const pending = query('native'); assert.equal(pending.meta_comments.pending, 1); assert.equal(pending.rows.length, 1);
    const row = pending.rows[0], commentPath = row.source_path, comment = readFileSync(commentPath, 'utf8');
    assert.equal(row.status, 'pending'); assert.equal(row.target_path, join(editProject, 'README.md'));
    assert.equal(row.target_hash, createHash('sha256').update(editedSource).digest('hex'));
    assert.deepEqual([row.range_start, row.range_end, row.selected_text, row.range_unit], [0, [...editedSource].length, editedSource, 'unicode-codepoint']);
    const body = text => text.slice(text.indexOf('\n---\n') + 5);
    assert.equal(body(comment), 'GUI context note\nSecond line.');
    assert.deepEqual(query('native').rows, pending.rows, 'Reading does not consume feedback');
    assert.deepEqual(query('legacy').rows, pending.rows);
    update('legacy', row.name, 'resolved'); assert.equal(query('native').meta_comments.pending, 0);
    assert.equal(body(readFileSync(commentPath, 'utf8')), body(comment));
    update('native', row.name, 'pending'); update('native', row.name, 'resolved'); assert.equal(query('native').meta_comments.pending, 0);
    assert.equal(body(readFileSync(commentPath, 'utf8')), body(comment));
    assert.equal(readFileSync(join(editProject, 'README.md'), 'utf8'), editedSource);
    report.results.push({ compiler, feedbackLoop: { pending: 1, resolved: true, nativeAndLegacy: true, bodyPreserved: true, fullSavedSourceCodePoints: [...editedSource].length } });
    const previewProject = join(report.run, `${compiler} preview`); cpSync(join(root, 'fixtures/project'), previewProject, { recursive: true });
    writeFileSync(join(previewProject, '.gui-test-owned'), 'Generated GUI acceptance fixture.\n');
    const previewFile = join(previewProject, 'preview.md'), previewPrefix = '\ufeff---\r\ntitle: Preview mapping\r\n---\r\n';
    const previewSource = previewPrefix + 'Preview café Ω 😀 from GUI.', savedPreview = previewSource + '!';
    writeFileSync(previewFile, previewSource);
    const previewBefore = inventory(previewProject), previewShot = join(report.run, `${compiler}-preview-feedback.ppm`);
    const previewEditing = childJSON(join(root, `build/gui/attendant-gui-${compiler}-check.exe`), ['--project', previewProject, '--assets', join(stack, 'build/deps/richmd/assets'),
      '--file', 'preview.md', '--frames', '70', '--shot', previewShot, '--exercise-preview']);
    assert.equal(readFileSync(previewFile, 'utf8'), savedPreview, 'Preview Save & Send preserves BOM/CRLF/Unicode source bytes');
    const previewAfter = inventory(previewProject);
    for (const [path, hash] of Object.entries(previewBefore)) if (path !== previewFile && path !== join(previewProject, '.pi/attendant.tables')) assert.equal(previewAfter[path], hash);
    for (const path of Object.keys(previewAfter)) if (!(path in previewBefore)) assert.equal(dirname(path), join(previewProject, 'records/_meta_comments'), 'GUI creates only managed source, never projection');
    const previewPending = childJSON(cli, ['query', '-p', previewProject, '-s', sql]);
    assert.equal(previewPending.meta_comments.pending, 1); assert.equal(previewPending.rows.length, 1);
    const previewRow = previewPending.rows[0], previewComment = readFileSync(previewRow.source_path, 'utf8');
    assert.equal(previewRow.target_path, previewFile); assert.equal(previewRow.target_hash, createHash('sha256').update(savedPreview).digest('hex'));
    assert.deepEqual([previewRow.range_start, previewRow.range_end, previewRow.selected_text, previewRow.range_unit],
      [[...previewPrefix].length, [...savedPreview].length, savedPreview.slice(previewPrefix.length), 'unicode-codepoint']);
    assert.equal(body(previewComment), 'GUI preview context note\nSecond line.');
    assert.deepEqual(childJSON(process.execPath, [legacy, 'query', '-p', previewProject, '-s', sql]).rows, previewPending.rows);
    childJSON(cli, ['update', '-p', previewProject, '-c', '_meta_comments', '-i', JSON.stringify([{ name: previewRow.name, fields: { status: 'resolved' } }])]);
    assert.equal(childJSON(cli, ['query', '-p', previewProject, '-s', sql]).meta_comments.pending, 0);
    assert.equal(body(readFileSync(previewRow.source_path, 'utf8')), body(previewComment));
    assert.equal(readFileSync(previewFile, 'utf8'), savedPreview);
    report.results.push({ compiler, previewEditing, previewFeedback: { rangeStart: previewRow.range_start, rangeEnd: previewRow.range_end, selectedText: previewRow.selected_text,
      bomCrLfUnicodePreserved: true, nativeAndLegacy: true, resolved: true, editorSelectionUnchanged: true }, screenshot: bitmap(previewShot) });
    const failedProject = join(report.run, `${compiler} saved-not-queued`); cpSync(project, failedProject, { recursive: true });
    writeFileSync(join(failedProject, '.gui-test-owned'), 'Generated GUI acceptance fixture.\n');
    const conflict = join(failedProject, 'records/_meta_comments'); mkdirSync(conflict); writeFileSync(join(conflict, 'user.md'), '# User-owned file\n');
    const failedBefore = inventory(failedProject), failedShot = join(report.run, `${compiler}-saved-not-queued.ppm`);
    const savedNotQueued = childJSON(join(root, `build/gui/attendant-gui-${compiler}-check.exe`), ['--project', failedProject, '--assets', join(stack, 'build/deps/richmd/assets'),
      '--file', 'README.md', '--frames', '70', '--shot', failedShot, '--exercise-save-fail']);
    assert.equal(savedNotQueued.frames, 70); assert.equal(readFileSync(join(failedProject, 'README.md'), 'utf8'), editedSource);
    const failedAfter = inventory(failedProject); delete failedBefore[join(failedProject, 'README.md')]; delete failedAfter[join(failedProject, 'README.md')];
    assert.deepEqual(failedAfter, failedBefore, 'Failed queue leaves target saved and every other file unchanged');
    report.results.push({ compiler, savedNotQueued, screenshot: bitmap(failedShot) });
  }
  report.ok = true;
} finally { writeFileSync(join(root, 'build/gui-check-results.json'), JSON.stringify(report, null, 2) + '\n'); }
console.log(JSON.stringify(report));
