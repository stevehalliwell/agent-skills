// Provenance guards; restore every deliberately changed file before returning.
import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync, writeFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { verifyBuild, guiDirectory } from '../../tools/gui-build-manifest.mjs';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
test('GUI provenance records successful compiler version probes', () => {
  for (const compiler of ['cjit', 'zig']) {
    const manifest = verifyBuild(compiler);
    assert.equal(manifest.version.status, 0);
    assert.match(manifest.version.text, compiler === 'cjit' ? /^CJIT v/m : /^\d+\.\d+\.\d+/);
  }
});
test('GUI acceptance rejects stale C input before launching', () => {
  const path = resolve(root, 'apps/gui/workspace.c'), original = readFileSync(path);
  try {
    verifyBuild('zig'); writeFileSync(path, Buffer.concat([original, Buffer.from('\n/* Deliberate test drift. */\n')]));
    assert.throws(() => verifyBuild('zig'), /GUI build inputs changed/);
  } finally { writeFileSync(path, original); }
  verifyBuild('zig');
});
test('GUI acceptance tracks extracted test state and API fault headers', () => {
  for (const name of ['tests/gui/app-state.inc', 'tests/core/feedback-faults.h']) {
    const path = resolve(root, name), original = readFileSync(path);
    try {
      verifyBuild('zig'); writeFileSync(path, Buffer.concat([original, Buffer.from('\n/* Deliberate test drift. */\n')]));
      assert.throws(() => verifyBuild('zig'), /GUI build inputs changed/);
    } finally { writeFileSync(path, original); }
    verifyBuild('zig');
  }
});
test('GUI acceptance rejects substituted runtime DLL', () => {
  const path = resolve(root, guiDirectory, 'cimgui_richmd.dll'), original = readFileSync(path), changed = Buffer.from(original);
  try {
    changed[changed.length - 1] ^= 1; writeFileSync(path, changed);
    assert.throws(() => verifyBuild('zig'), /Copied GUI DLL changed/);
  } finally { writeFileSync(path, original); }
  verifyBuild('zig');
});
