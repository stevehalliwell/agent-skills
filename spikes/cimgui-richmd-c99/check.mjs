#!/usr/bin/env node
// Real window/D3D smoke, serial fresh processes. No benchmark claims.
// Requires build.sh first. Saves PPM and BMP evidence in ignored build/.
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { readFileSync, writeFileSync, statSync, rmSync, mkdirSync } from 'node:fs';
import { createHash } from 'node:crypto';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
process.chdir(dirname(fileURLToPath(import.meta.url)));
if (process.platform !== 'win32') throw new Error('Windows x64 interactive desktop required');
const common = ['--assets', resolve('build/deps/richmd/assets')];
const runners = [
  ['zig', resolve('build/bin/host-zig.exe'), []],
  ['cjit', process.env.CJIT || 'cjit', ['-q', '-DCIMGUI_DEFINE_ENUMS_AND_STRUCTS', '-DCIMGUI_NO_EXPORT', '-Ibuild/deps/cimgui',
    '-Lbuild/bin', '-lcimgui_richmd', '-L', resolve(process.env.SystemRoot || 'C:/Windows', 'System32'), '-luser32', 'host.c', '--']],
];
function screenshot(path) {
  const bytes = readFileSync(path);
  const header = /^P6\n(\d+) (\d+)\n255\n/.exec(bytes.subarray(0, 80).toString('ascii'));
  assert(header, 'PPM header');
  const width = +header[1], height = +header[2], rgb = bytes.subarray(header[0].length);
  assert.equal(rgb.length, width * height * 3);
  assert(width > 600 && height > 400);
  // Count bright pixels in preview half, independently of source editor.
  let bright = 0;
  for (let y = 80; y < height; ++y) for (let x = Math.floor(width * 0.52); x < width; ++x) {
    const index = (y * width + x) * 3;
    if (rgb[index] > 140 && rgb[index + 1] > 140 && rgb[index + 2] > 140) ++bright;
  }
  // Tool-readable, dependency-free 24-bit BMP, bottom-up rows.
  const stride = (width * 3 + 3) & ~3;
  const bitmap = Buffer.alloc(54 + stride * height);
  bitmap.write('BM'); bitmap.writeUInt32LE(bitmap.length, 2); bitmap.writeUInt32LE(54, 10);
  bitmap.writeUInt32LE(40, 14); bitmap.writeInt32LE(width, 18); bitmap.writeInt32LE(height, 22);
  bitmap.writeUInt16LE(1, 26); bitmap.writeUInt16LE(24, 28);
  for (let y = 0; y < height; ++y) for (let x = 0; x < width; ++x) {
    const source = (y * width + x) * 3, target = 54 + (height - y - 1) * stride + x * 3;
    bitmap[target] = rgb[source + 2]; bitmap[target + 1] = rgb[source + 1]; bitmap[target + 2] = rgb[source];
  }
  writeFileSync(path.replace(/\.ppm$/, '.bmp'), bitmap);
  return { width, height, brightPreviewPixels: bright, sha256: createHash('sha256').update(bytes).digest('hex') };
}
const results = [], interactions = [], selections = [];
const collision = 'text 308a0945c21a0320\ntext 3d806b964d5618d8'; // Actual ImGui CRC32C collision; host verifies equality.
const selectionCases = [
  { name: 'plain', text: 'echo echo', mode: 'all', range: [0, 9] },
  { name: 'unicode', text: 'café Ω 😀', mode: 'all', range: [0, Buffer.byteLength('café Ω 😀')] },
  { name: 'literal-entity', text: 'echo &amp; star', mode: 'all', range: [0, 15] },
  { name: 'decomposed', text: 'cafe\u0301', mode: 'all', range: [0, Buffer.byteLength('cafe\u0301')] },
  { name: 'forward', text: 'echo echo', mode: 'forward', range: [0, 9] },
  { name: 'reverse', text: 'echo echo', mode: 'reverse', range: [0, 9] },
  { name: 'repeated-word', text: 'echo echo', mode: 'word', range: [5, 9] },
  { name: 'link-label', text: '[echo](attendant://tasks/example)', mode: 'label', range: [1, 5] },
  { name: 'menu-copy', text: 'echo echo', mode: 'menu', range: [0, 9], copied: 9 },
  ...['switch', 'revision', 'change', 'empty'].map(mode => ({ name: mode, text: 'echo echo', mode, none: true, priorExact: true })),
  ...['collision', 'key-collision'].map(mode => ({ name: mode, text: collision, mode, none: true, priorExact: true })),
  { name: 'code-child', text: 'plain\n\n```c\nx\n```', mode: 'widget', none: true, priorExact: true, child: true },
  ...[
    ['bold-markup', '**echo**'], ['escape', 'echo \\* star'], ['entity', 'echo &nbsp; star'],
    ['soft-break', 'echo\nagain'], ['crlf-break', 'echo\r\nagain'], ['hard-break', 'echo  \nagain'],
    ['table', 'echo\n\n| a | b |\n|---|---|\n| c | d |'], ['code', 'echo\n\n```c\nint x;\n```'],
    ['inline-code', 'echo `x`'], ['hidden-details', 'echo\n\n<details><summary>Hidden</summary>secret</details>'],
    ['wrapped-line', 'echo '.repeat(200)],
  ].map(([name, text]) => ({ name, text, mode: 'all' })),
];
mkdirSync('build/selection-fixtures', { recursive: true });
const runStartedAt = new Date().toISOString();
rmSync('build/results.json', { force: true });
process.on('uncaughtException', error => {
  writeFileSync('build/results.json', JSON.stringify({ ok: false, runStartedAt, error: error.message, results, interactions, selections }, null, 2) + '\n');
  console.error(error);
  process.exitCode = 1;
});
const build = JSON.parse(readFileSync('build/build-manifest.json', 'utf8'));
const hash = path => createHash('sha256').update(readFileSync(path)).digest('hex');
assert.equal(build.dllSha256, hash('build/bin/cimgui_richmd.dll'), 'DLL matches verified build');
assert.equal(build.hostSha256, hash('build/bin/host-zig.exe'), 'Native host matches verified build');
for (const file of build.files) assert.equal(hash(file.path), file.sha256, `Build input changed: ${file.path}`);
for (const [name, command, prefix] of runners) {
  for (const preview of [false, true]) {
    const path = `build/${name}-${preview ? 'preview' : 'blank'}.ppm`;
    rmSync(path, { force: true });
    const args = [...prefix, ...common, '--frames', '30', '--shot', path, ...(preview ? [] : ['--no-preview'])];
    const processResult = spawnSync(command, args, { encoding: 'utf8', timeout: 60000, maxBuffer: 2 * 1024 * 1024 });
    writeFileSync(`build/${name}-${preview ? 'preview' : 'blank'}.txt`, processResult.stdout + processResult.stderr);
    assert.equal(processResult.status, 0, `${name}: ${processResult.error || processResult.stderr}`);
    const output = JSON.parse(processResult.stdout.trim());
    assert.equal(output.ok, true); assert.equal(output.imgui, '1.93.0'); assert.equal(output.richmd, '1.0.0');
    assert.equal(output.frames, 30); assert(output.vertices > 0);
    results.push({ name, preview, output, screenshot: screenshot(path) });
  }
  const blank = results.at(-2), rendered = results.at(-1);
  assert(rendered.output.vertices > blank.output.vertices + 1000, 'RichMd emits additional geometry');
  assert(rendered.screenshot.brightPreviewPixels > blank.screenshot.brightPreviewPixels + 1000, 'Rendered preview changes actual GPU pixels');
  rmSync(`build/${name}-interaction.ppm`, { force: true });
  const interactive = spawnSync(command, [...prefix, ...common, '--frames', '30', '--exercise', '--shot', `build/${name}-interaction.ppm`],
    { encoding: 'utf8', timeout: 60000, maxBuffer: 2 * 1024 * 1024 });
  writeFileSync(`build/${name}-interaction.txt`, interactive.stdout + interactive.stderr);
  assert.equal(interactive.status, 0, `${name}: ${interactive.error || interactive.stderr}`);
  const interaction = JSON.parse(interactive.stdout.trim());
  assert.equal(interaction.exercise, true); assert.equal(interaction.links, 1); assert(interaction.copied_bytes > 0);
  interactions.push({ name, output: interaction, screenshot: screenshot(`build/${name}-interaction.ppm`) });
  for (const test of selectionCases) {
    const source = `build/selection-fixtures/${test.name}.md`;
    writeFileSync(source, test.text);
    const shot = ['plain', 'code-child', 'menu-copy'].includes(test.name) ? `build/${name}-selection-${test.name}.ppm` : null;
    const run = spawnSync(command, [...prefix, ...common, '--frames', '30', '--raw-file', resolve(source), '--selection', test.mode,
      ...(shot ? ['--shot', shot] : [])], { encoding: 'utf8', timeout: 60000, maxBuffer: 2 * 1024 * 1024 });
    writeFileSync(`build/${name}-selection-${test.name}.txt`, run.stdout + run.stderr);
    assert.equal(run.status, 0, `${name}/${test.name}: ${run.error || run.stderr}`);
    const output = JSON.parse(run.stdout.trim());
    const row = { name, case: test.name, output, ...(shot ? { screenshot: screenshot(shot) } : {}) };
    selections.push(row);
    assert.equal(readFileSync(source, 'utf8'), test.text, 'Selection check leaves source file unchanged');
    if (test.range) {
      assert.equal(output.raw_status, 1, `${name}/${test.name}: exact original-input span`);
      assert.deepEqual([output.raw_begin, output.raw_end], test.range, `${name}/${test.name}: original byte endpoints`);
    } else {
      assert(test.none ? output.raw_status === 0 : output.raw_status !== 1, `${name}/${test.name}: never claim exact provenance`);
      assert.deepEqual([output.raw_begin, output.raw_end], [0, 0], 'Unclaimed output has no stale endpoints');
    }
    if (test.priorExact) assert.equal(output.had_exact, 1, `${name}/${test.name}: started with a genuine exact selection`);
    if (test.child) assert.equal(output.widget_hovered, 1, 'Actual code child received click');
    if (test.copied) assert.equal(output.copied_bytes, test.copied, 'Context-menu Copy keeps genuine selection');
  }
  const bad = spawnSync(command, [...prefix, ...common, '--frames', '0'], { encoding: 'utf8', timeout: 15000 });
  assert.equal(bad.status, 2, `${name}: invalid frame bound rejected`);
}
const dll = readFileSync('build/bin/cimgui_richmd.dll');
function imports(image) {
  const pe = image.readUInt32LE(0x3c), optional = pe + 24;
  assert.equal(image.readUInt16LE(optional), 0x20b, 'PE32+');
  const sections = pe + 24 + image.readUInt16LE(pe + 20), count = image.readUInt16LE(pe + 6);
  function offset(rva) {
    for (let i = 0; i < count; ++i) {
      const section = sections + i * 40, base = image.readUInt32LE(section + 12);
      if (rva >= base && rva < base + Math.max(image.readUInt32LE(section + 8), image.readUInt32LE(section + 16)))
        return image.readUInt32LE(section + 20) + rva - base;
    }
    throw new Error(`RVA outside sections: ${rva}`);
  }
  const names = [];
  for (let descriptor = offset(image.readUInt32LE(optional + 120)); image.readUInt32LE(descriptor + 12); descriptor += 20) {
    const start = offset(image.readUInt32LE(descriptor + 12));
    names.push(image.subarray(start, image.indexOf(0, start)).toString('ascii'));
  }
  return names;
}
const importedDlls = imports(dll);
assert(!importedDlls.some(name => /libstdc\+\+|libc\+\+|libgcc|libwinpthread|lua|python|node/i.test(name)), 'No external language/compiler runtime DLL');
for (const preview of [false, true]) {
  const a = results.find(row => row.name === 'zig' && row.preview === preview);
  const b = results.find(row => row.name === 'cjit' && row.preview === preview);
  assert.equal(a.screenshot.sha256, b.screenshot.sha256, 'Compiler hosts render identical screenshot bytes');
}
const cjitInfo = spawnSync(process.env.CJIT || 'cjit', ['--help'], { encoding: 'utf8', timeout: 15000 });
assert.equal(cjitInfo.status, 0, 'CJIT version probe');
const cjitVersion = (cjitInfo.stdout + cjitInfo.stderr).match(/^CJIT v([^\s]+)/m)?.[1];
assert(cjitVersion, 'CJIT version identified');
const report = { ok: true, runStartedAt, platform: process.platform, architecture: process.arch,
  build: { builtAt: build.builtAt, zig: build.zig, generatorZig: build.generatorZig, sourceTreeSha256: build.sourceTreeSha256 },
  cjit: cjitVersion,
  dependencies: build.dependencies, dll: { bytes: statSync('build/bin/cimgui_richmd.dll').size,
    sha256: createHash('sha256').update(dll).digest('hex'), imports: importedDlls }, checkerSha256: hash('check.mjs'), results, interactions, selections };
writeFileSync('build/results.json', JSON.stringify(report, null, 2) + '\n');
console.log(JSON.stringify(report, null, 2));
