// Provenance guards; restore every deliberately changed file before returning.
import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync, writeFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { verifyBuild } from './build-manifest.mjs';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
test('GUI acceptance rejects stale C input before launching', () => {
  const path = resolve(root, 'gui/workspace.c'), original = readFileSync(path);
  try {
    verifyBuild('zig'); writeFileSync(path, Buffer.concat([original, Buffer.from('\n/* Deliberate test drift. */\n')]));
    assert.throws(() => verifyBuild('zig'), /GUI build inputs changed/);
  } finally { writeFileSync(path, original); }
  verifyBuild('zig');
});
test('GUI acceptance rejects substituted runtime DLL', () => {
  const path = resolve(root, 'build/gui/cimgui_richmd.dll'), original = readFileSync(path), changed = Buffer.from(original);
  try {
    changed[changed.length - 1] ^= 1; writeFileSync(path, changed);
    assert.throws(() => verifyBuild('zig'), /Copied GUI DLL changed/);
  } finally { writeFileSync(path, original); }
  verifyBuild('zig');
});
