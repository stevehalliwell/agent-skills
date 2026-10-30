import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtempSync, mkdirSync, readFileSync, writeFileSync, rmSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';
import { parseOptions, runChecks } from '../../tools/real-data-check.mjs';
import { inventory } from '../../tools/dataset-files.mjs';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
test('workflow CLI requires manifest and validates options before execution', () => {
  for (const args of [[], ['--manifest'], ['--unknown', 'x'], ['--manifest', 'x', '--manifest', 'y']]) assert.throws(() => parseOptions(args));
  assert.equal(parseOptions(['--manifest', 'capture.json']).manifest, resolve('capture.json'));
  assert.equal(parseOptions(['--manifest', 'x', '--zig', 'native.exe']).zig, resolve('native.exe'));
});
test('invalid workflow CLI returns structured exit 2 without running tests', () => {
  const result = spawnSync(process.execPath, [join(root, 'tools/real-data-check.mjs')], { encoding: 'utf8' });
  assert.equal(result.status, 2); assert.equal(JSON.parse(result.stdout).error.code, 'invalid-arguments');
});
test('workflow help has no manifest requirement or test side effects', () => {
  const result = spawnSync(process.execPath, [join(root, 'tools/real-data-check.mjs'), '--help'], { encoding: 'utf8' });
  assert.equal(result.status, 0); assert.match(JSON.parse(result.stdout).safety, /no live\/pristine CLI/);
});
test('changed capture rejected before runner discovery or invocation', () => {
  const project = mkdtempSync(join(tmpdir(), 'attendant-capture-check-'));
  try {
    mkdirSync(join(project, '.pi')); mkdirSync(join(project, 'records'));
    writeFileSync(join(project, '.pi/attendant.tables'), 'records as tasks\n');
    writeFileSync(join(project, 'records/.schema.md'), '---\nstatus: [todo, done]\n---\n');
    const captured = inventory(project), manifest = join(project, 'manifest.json');
    writeFileSync(manifest, JSON.stringify({ datasets: [{ pristine: project, ...captured }] }));
    writeFileSync(join(project, 'records/.schema.md'), '---\nstatus: [changed]\n---\n');
    const before = readFileSync(join(project, 'records/.schema.md'));
    assert.throws(() => runChecks({ manifest, cjit: 'nonexistent-native', zig: 'nonexistent-native' }), /Capture differs from manifest/);
    assert.deepEqual(readFileSync(join(project, 'records/.schema.md')), before);
  } finally { rmSync(project, { recursive: true, force: true }); }
});
