// Real GUI acceptance in unique owned workspace copies; never uses live repos.
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { cpSync, mkdirSync, mkdtempSync, readFileSync, readdirSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { verifyBuild } from './build-manifest.mjs';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const stack = resolve(root, '../../spikes/cimgui-richmd-c99');
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
const report = { ok: false, run: mkdtempSync(join(root, 'build/gui-check/run-')), results: [] };
try {
  for (const compiler of ['cjit', 'zig']) {
    const production = verifyBuild(compiler), testing = verifyBuild(compiler, true);
    report.results.push({ compiler, provenance: { builtAt: production.builtAt, binaries: [...production.binaries, ...testing.binaries], stack: production.stack } });
    const project = join(report.run, `${compiler} caf\u00e9 \u03a9`);
    cpSync(join(root, 'fixtures/project'), project, { recursive: true });
    writeFileSync(join(project, 'README.md'), '# Workspace root file\n\nPlain Markdown also opens.\n');
    const first = join(project, 'records/tasks/first.md');
    writeFileSync(first, readFileSync(first, 'utf8').replace('depends_on: [second]', 'depends_on: [second, missing]'));
    const before = inventory(project), shot = join(report.run, `${compiler}.ppm`);
    const result = spawnSync(join(root, `build/gui/attendant-gui-${compiler}.exe`), ['--project', project, '--assets', join(stack, 'build/deps/richmd/assets'),
      '--file', 'records/tasks/first.md', '--frames', '30', '--shot', shot], { encoding: 'utf8', timeout: 60000 });
    assert.equal(result.status, 0, result.stderr + result.stdout);
    const output = JSON.parse(result.stdout); assert.equal(output.frames, 30); assert.equal(output.tabs, 1); assert(output.vertices > 1000);
    assert.deepEqual(inventory(project), before, 'Read-only GUI never prepares DB or changes source');
    report.results.push({ compiler, output, screenshot: bitmap(shot) });
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
  }
  report.ok = true;
} finally { writeFileSync(join(root, 'build/gui-check-results.json'), JSON.stringify(report, null, 2) + '\n'); }
console.log(JSON.stringify(report));
