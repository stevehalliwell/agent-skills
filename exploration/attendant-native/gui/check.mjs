// Real GUI acceptance in unique owned workspace copies; never uses live repos.
import assert from 'node:assert/strict';
import { spawn, spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { cpSync, mkdirSync, mkdtempSync, readFileSync, readdirSync, renameSync, rmSync, symlinkSync, unlinkSync, writeFileSync } from 'node:fs';
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
async function checkSelectionInspection(report, compiler) {
  const other = compiler === 'cjit' ? 'zig' : 'cjit';
  const cases = [
    ['plain-menu', 'plain caf\u00e9 \u03a9 \ud83d\ude00 words', 'exact'],
    ['markup-menu', 'plain **bold** trailing', 'unsupported'],
    ['widget', 'plain\n\n```c\nx\n```', 'unsupported']
  ];
  for (const [mode, text, status] of cases) {
    const project = join(report.run, `${compiler} inspect ${mode}`);
    cpSync(join(root, 'fixtures/project'), project, { recursive: true });
    writeFileSync(join(project, '.gui-test-owned'), 'Owned selection inspection fixture.\n');
    writeFileSync(join(project, 'README.md'), text); const before = inventory(project);
    const child = spawn(join(root, `build/gui/attendant-gui-${compiler}-check.exe`), ['--project', project,
      '--assets', join(stack, 'build/deps/richmd/assets'), '--file', 'README.md', '--frames', '180', '--exercise-selection', mode], { stdio: ['ignore', 'pipe', 'pipe'] });
    let output = '', error = '';
    const ready = new Promise((resolve, reject) => {
      child.stdout.on('data', value => { output += value; if (output.includes('"event":"selection-ready"')) resolve(); });
      child.on('error', reject); child.on('close', code => { if (!output.includes('"event":"selection-ready"')) reject(new Error(`Selection driver ${code}: ${error}`)); });
    });
    const actionReady = new Promise((resolve, reject) => {
      child.stdout.on('data', () => { if (output.includes('"event":"selection-action"')) resolve(); });
      child.on('error', reject); child.on('close', code => { if (!output.includes('"event":"selection-action"')) reject(new Error(`Selection action driver ${code}: ${error}`)); });
    });
    actionReady.catch(() => {});
    child.stderr.on('data', value => { error += value; });
    const done = new Promise((resolve, reject) => { child.on('error', reject); child.on('close', code => code === 0 ? resolve() : reject(new Error(error))); });
    done.catch(() => {}); const timer = setTimeout(() => child.kill(), 30000);
    try {
      await ready;
      const inspected = childJSON(join(root, `build/gui/attendant-gui-${other}.exe`), ['--project', project, '--inspect']);
      assert.equal(inspected.selection.view, 'preview'); assert.equal(inspected.selection.status, status);
      assert.equal(inspected.selection.text, status === 'exact' ? text : null);
      assert.equal(inspected.selection.range_start, status === 'exact' ? 0 : null);
      assert.equal(inspected.selection.range_end, status === 'exact' ? Array.from(text).length : null);
      await actionReady;
      const afterAction = childJSON(join(root, `build/gui/attendant-gui-${other}.exe`), ['--project', project, '--inspect']);
      assert.deepEqual(afterAction, inspected, 'Unified action/comment-input focus preserves document selection');
      await done; assert.deepEqual(inventory(project), before);
      report.results.push({ compiler, selectionInspection: { mode, clientCompiler: other, status, priorSourceSelectionReplaced: true, unifiedActionUsesLatestSelection: true, retainedDuringCommentInputFocus: status === 'exact', sourceAndProjectionUnchanged: true } });
    } finally { clearTimeout(timer); if (child.exitCode === null) child.kill(); }
  }
}
async function checkAgentReveal(report, compiler) {
  const other = compiler === 'cjit' ? 'zig' : 'cjit';
  const executable = (name, testing = false) => join(root, `build/gui/attendant-gui-${name}${testing ? '-check' : ''}.exe`);
  function control(project, args, expectedCode = null, testing = false) {
    const result = spawnSync(executable(other, testing), ['--project', project, ...args], {
      encoding: 'utf8', timeout: 10000, windowsHide: true, env: { ...process.env, ATTENDANT_GUI_TEST_REVEAL_HASH: '0'.repeat(64) }
    });
    if (expectedCode !== null) assert.equal(result.status, expectedCode, result.stderr + result.stdout);
    return { status: result.status, output: JSON.parse(result.stdout), stderr: result.stderr };
  }
  function receiver(project, mode, shot, name = compiler) {
    const child = spawn(executable(name, true), ['--project', project, '--assets', join(stack, 'build/deps/richmd/assets'),
      '--file', 'README.md', '--frames', '400', ...(shot ? ['--shot', shot] : []), '--exercise-reveal', mode], { stdio: ['ignore', 'pipe', 'pipe'] });
    const events = []; let buffer = '', stderr = '', readyResolve, readyReject;
    const ready = new Promise((resolve, reject) => { readyResolve = resolve; readyReject = reject; });
    child.stdout.setEncoding('utf8'); child.stderr.setEncoding('utf8');
    child.stdout.on('data', text => {
      buffer += text;
      for (;;) { const end = buffer.indexOf('\n'); if (end < 0) break;
        const line = buffer.slice(0, end); buffer = buffer.slice(end + 1);
        try { const event = JSON.parse(line); events.push(event); if (event.event === 'ready') readyResolve(); }
        catch (error) { readyReject(error); }
      }
    });
    child.stderr.on('data', text => { stderr += text; });
    const timer = setTimeout(() => child.kill(), 30000);
    const done = new Promise((resolve, reject) => {
      child.on('error', error => { clearTimeout(timer); readyReject(error); reject(error); });
      child.on('close', status => {
        clearTimeout(timer);
        if (status === 0) resolve(events);
        else { const error = new Error(`Reveal receiver ${mode} exited ${status}: ${stderr}`); readyReject(error); reject(error); }
      });
    });
    // Early failure must not leave an unhandled promise while waiting for readiness.
    done.catch(() => {});
    return { child, ready, done };
  }
  for (const mode of ['clean', 'dirty', 'stale', 'other', 'reparse']) {
    const project = join(report.run, `${compiler} reveal ${mode} caf\u00e9 \u03a9`);
    cpSync(join(root, 'fixtures/project'), project, { recursive: true });
    writeFileSync(join(project, '.gui-test-owned'), 'Generated reveal acceptance fixture.\n');
    writeFileSync(join(project, 'README.md'), '\ufeff# Agent marker test\r\n\r\n' + Array.from({ length: 180 }, (_, i) => `Line ${i + 1}: caf\u00e9 \u03a9 \ud83d\ude00 marker context.\r\n`).join(''));
    writeFileSync(join(project, 'invalid.md'), Buffer.from([0xff]));
    writeFileSync(join(project, 'other.md'), readFileSync(join(project, 'README.md'), 'utf8').replace('# Agent marker test', '# Other agent target'));
    const outside = join(report.run, `${compiler} outside ${mode}`), changing = join(project, 'changing'), backup = join(project, 'changing-backup');
    mkdirSync(outside); writeFileSync(join(outside, 'other.md'), readFileSync(join(project, 'other.md')));
    if (mode === 'reparse') { mkdirSync(changing); writeFileSync(join(changing, 'other.md'), readFileSync(join(project, 'other.md'))); }
    const outsideBefore = inventory(outside);
    const before = inventory(project), shot = join(report.run, `${compiler}-reveal-${mode}.ppm`);
    let swapped = false;
    if (mode === 'clean') {
      assert.equal(control(project, ['--reveal', 'README.md', '--line', '4'], 1).output.code, 'gui-not-running');
      assert.equal(control(project, ['--inspect'], 1).output.code, 'gui-not-running');
    }
    const running = receiver(project, mode, shot);
    try {
      await running.ready;
      const inspected = control(project, ['--inspect'], 0).output;
      const liveText = readFileSync(join(project, 'README.md'), 'utf8') + (mode === 'dirty' ? '!' : '');
      assert.equal(inspected.document.path, join(project, 'README.md')); assert.equal(inspected.document.dirty, mode === 'dirty');
      assert.equal(inspected.selection.status, 'exact'); assert.equal(inspected.selection.view, 'source');
      assert.equal(inspected.selection.text, liveText); assert.equal(inspected.selection.range_start, 0);
      assert.equal(inspected.selection.range_end, Array.from(liveText).length); assert.equal(inspected.selection.range_unit, 'unicode-codepoint');
      assert.deepEqual(control(project, ['--inspect'], 0).output, inspected, 'Inspection reads without consuming selection');
      if (mode === 'clean') {
        const negative = [
          [['--inspect', '--reveal', 'README.md'], 2, 'usage'],
          [['--inspect', '--line', '4'], 2, 'usage'],
          [['--inspect', '--assets', 'unused'], 2, 'usage'],
          [['--reveal', 'README.md', '--line', '0'], 2, 'usage'],
          [['--reveal', 'README.md', '--line', '-1'], 2, 'usage'],
          [['--reveal', 'README.md', '--line', '4x'], 2, 'usage'],
          [['--reveal', 'README.md', '--line', '4294967296'], 2, 'usage'],
          [['--reveal', 'README.md', '--line', '4194306'], 2, 'usage'],
          [['--reveal', 'README.md', '--line', '999'], 1, 'unmapped-line'],
          [['--reveal', '../outside.md'], 1, 'reveal-not-allowed'],
          [['--reveal', 'not-markdown.txt'], 1, 'reveal-not-allowed'],
          [['--reveal', 'missing.md'], 1, 'source-read-failed'],
          [['--reveal', 'invalid.md'], 1, 'source-read-failed'],
          [['--reveal', 'README.md', '--unknown'], 2, 'usage'],
          [['--reveal', 'README.md', '--assets', 'unused'], 2, 'usage'],
          [['--reveal', 'README.md', ...Array(40).fill('--bad')], 2, 'usage']
        ];
        for (const [args, status, code] of negative) assert.equal(control(project, args, status).output.code, code);
        const junction = join(project, 'linked'); symlinkSync(outside, junction, 'junction');
        try { assert.equal(control(project, ['--reveal', 'linked/other.md'], 1).output.code, 'unsupported-path'); }
        finally { unlinkSync(junction); }
      }
      const target = mode === 'reparse' ? 'changing/other.md' : mode === 'other' ? 'other.md' : 'README.md';
      const result = control(project, ['--reveal', target, '--line', mode === 'other' || mode === 'reparse' ? '42' : '4'], 0, mode === 'stale');
      if (mode === 'reparse') { renameSync(changing, backup); symlinkSync(outside, changing, 'junction'); swapped = true; }
      assert.deepEqual(result.output, { ok: true, hint_updated: true, persistent: false, focus_changed: false });
      const events = await running.done, complete = events.find(event => event.event === 'complete');
      assert(events.some(event => event.event === 'inspected' && event.documentSelectionScrollFocusPreserved));
      assert(events.some(event => event.event === 'received' && event.documentSelectionScrollFocusPreserved));
      assert(complete?.caretSelectionPreservedAfterGo); assert.equal(complete.dirty, mode === 'dirty');
      const marked = mode === 'clean' || mode === 'other';
      assert(marked ? complete.markerDraws > 0 : complete.markerDraws === 0);
      if (swapped) { unlinkSync(changing); renameSync(backup, changing); swapped = false; }
      assert.deepEqual(inventory(project), before, 'Reveal and explicit Go never write source/projection; dirty test buffer discarded');
      assert.deepEqual(inventory(outside), outsideBefore, 'Outside junction target stays unchanged');
      const image = bitmap(shot), bytes = readFileSync(shot); let amber = 0, outsidePixels = 0;
      const offset = /^P6\n\d+ \d+\n255\n/.exec(bytes.subarray(0, 80).toString('ascii'))[0].length;
      for (let i = offset; i < bytes.length; i += 3) if (bytes[i] > 220 && bytes[i + 1] > 120 && bytes[i + 1] < 200 && bytes[i + 2] < 110) {
        amber++;
        const pixel = (i - offset) / 3, x = pixel % image.width, y = Math.floor(pixel / image.width), [left, top, right, bottom] = complete.sourceClip;
        if (x < left || x >= right || y < top || y >= bottom) outsidePixels++;
      }
      assert(marked ? amber > 500 : amber === 0, `Separate marker pixels: ${mode} ${amber}`);
      assert.equal(outsidePixels, 0, 'Agent marker cannot spill into preview, controls or outside source editor');
      report.results.push({ compiler, reveal: { mode, clientCompiler: other, crossCompilerIPC: true, inspection: { dirty: inspected.document.dirty, sourceRangeAndTextVerified: true, repeatableWithoutConsumption: true }, events, amberPixels: amber, markerPixelsOutsideSource: outsidePixels, sourceAndProjectionUnchanged: true }, screenshot: image });
    } finally {
      if (running.child.exitCode === null) running.child.kill();
      if (swapped) { unlinkSync(changing); renameSync(backup, changing); }
    }
  }
  const project = join(report.run, `${compiler} ambiguous reveal`);
  cpSync(join(root, 'fixtures/project'), project, { recursive: true });
  writeFileSync(join(project, '.gui-test-owned'), 'Generated ambiguous reveal fixture.\n');
  writeFileSync(join(project, 'README.md'), '# Ambiguous workspace\n' + 'Long context line.\n'.repeat(180));
  const before = inventory(project), first = receiver(project, 'idle'), second = receiver(project, 'idle', undefined, other);
  try {
    await Promise.all([first.ready, second.ready]);
    assert.equal(control(project, ['--reveal', 'README.md'], 1).output.code, 'gui-ambiguous');
    assert.equal(control(project, ['--inspect'], 1).output.code, 'gui-ambiguous');
    await Promise.all([first.done, second.done]); assert.deepEqual(inventory(project), before);
    report.results.push({ compiler, ambiguousRevealRejected: true, sourceAndProjectionUnchanged: true });
  } finally { for (const item of [first, second]) if (item.child.exitCode === null) item.child.kill(); }
  for (const name of [compiler, other]) {
    assert(!readFileSync(executable(name)).includes(Buffer.from('ATTENDANT_GUI_TEST_REVEAL_HASH')), 'Production omits stale-hash hook');
    assert(!readFileSync(executable(name)).includes(Buffer.from('--exercise-reveal')), 'Production omits reveal driver');
  }
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
    await checkSelectionInspection(report, compiler);
    await checkAgentReveal(report, compiler);
  }
  report.ok = true;
} finally { writeFileSync(join(root, 'build/gui-check-results.json'), JSON.stringify(report, null, 2) + '\n'); }
console.log(JSON.stringify(report));
