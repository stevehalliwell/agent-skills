// Native macOS bridge regression, independent of the still-Windows application.
// Usage: node tests/gui/bridge-check.mjs (requires explicitly built GUI stack).
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { readFileSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { stackManifest } from '../../tools/gui-build-manifest.mjs';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
assert.equal(process.platform, 'darwin', 'Bridge check requires native macOS');
const sha256 = path => createHash('sha256').update(readFileSync(path)).digest('hex');
const inputs = ['tests/gui/bridge-check.c', 'tests/gui/bridge-check.mjs', 'tools/check-gui-bridge.sh', 'tools/gui-build-manifest.mjs'];
const report = { ok: false, testedAt: new Date().toISOString(), platform: process.platform, architecture: process.arch,
  stack: stackManifest(), inputs: inputs.map(path => ({ path, sha256: sha256(join(root, path)) })), cases: [] };
try {
  for (const compiler of ['zig', 'cjit']) {
    const result = spawnSync('bash', ['tools/check-gui-bridge.sh', compiler], {
      cwd: root, encoding: 'utf8', timeout: 120000,
    });
    assert.equal(result.status, 0, `${compiler}: ${result.error || result.stderr}\n${result.stdout}`);
    const bridge = JSON.parse(result.stdout);
    assert.deepEqual(bridge, { ok: true, frames: 4, image_seen: true, live_images: 0 });
    const screenshot = join(root, `build/bridge-check/${compiler}.ppm`);
    const pixels = readFileSync(screenshot);
    const header = /^P6\n(\d+) (\d+)\n255\n/.exec(pixels.subarray(0, 80).toString('ascii'));
    assert(header, 'Expected top-down P6 screenshot');
    const width = Number(header[1]), height = Number(header[2]);
    assert(width >= 640 && height >= 480 && width <= 16384 && height <= 16384);
    assert.equal(width * 480, height * 640, 'Retina framebuffer preserves logical aspect ratio');
    const body = pixels.subarray(header[0].length);
    assert.equal(body.length, width * height * 3, 'Screenshot contains exactly RGB framebuffer bytes');
    const sample = (x, y) => {
      const offset = (Math.floor(y * height / 480) * width + Math.floor(x * width / 640)) * 3;
      return [...body.subarray(offset, offset + 3)];
    };
    assert.deepEqual(sample(20, 20), [255, 0, 0], 'Top marker must remain at top in P6');
    assert.deepEqual(sample(20, 460), [0, 0, 255], 'Bottom marker must remain at bottom in P6');
    const colors = new Set();
    for (let offset = 0; offset < body.length; offset += 3)
      colors.add(body[offset] * 65536 + body[offset + 1] * 256 + body[offset + 2]);
    assert(colors.size > 32, 'Screenshot must contain rendered content, not only clear color');
    const compilerCommand = process.env[compiler === 'zig' ? 'ZIG' : 'CJIT'] || compiler;
    const version = spawnSync(compilerCommand, [compiler === 'zig' ? 'version' : '--help'], { encoding: 'utf8', timeout: 10000 });
    assert.equal(version.status, 0, 'Compiler identity probe failed');
    const executable = join(root, `build/bridge-check/bridge-${compiler}`);
    report.cases.push({ compiler, compilerCommand, version: (version.stdout + version.stderr).trim(), bridge,
      executable: { path: executable, sha256: sha256(executable) },
      screenshot: { path: screenshot, sha256: sha256(screenshot) }, width, height, colors: colors.size });
  }
  report.ok = true;
} finally {
  writeFileSync(join(root, 'build/bridge-check-results.json'), JSON.stringify(report, null, 2) + '\n');
}
console.log(JSON.stringify({ ok: report.ok, cases: report.cases.length }));
