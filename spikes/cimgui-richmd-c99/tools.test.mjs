// Guard regression checks; run after successful build/check. No GUI launched.
// Deliberate edits stay in spike-owned files and are always restored.
import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync, writeFileSync } from 'node:fs';
import { spawnSync } from 'node:child_process';
import { dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
process.chdir(dirname(fileURLToPath(import.meta.url)));
function node(args) { return spawnSync(process.execPath, args, { encoding: 'utf8', timeout: 30000 }); }
test('offline verification accepts pins and rejects tracked dependency edits', () => {
  assert.equal(node(['prepare.mjs', '--verify']).status, 0);
  const path = 'build/deps/richmd/README.md', original = readFileSync(path);
  try {
    writeFileSync(path, Buffer.concat([original, Buffer.from('\nDrift probe.\n')]));
    const result = node(['prepare.mjs', '--verify']);
    assert.notEqual(result.status, 0);
    assert.match(result.stderr, /Unexpected tracked dependency edit/);
  } finally { writeFileSync(path, original); }
});
test('changed build input invalidates previous smoke success without launching GUI', () => {
  const path = 'host.c', original = readFileSync(path), report = readFileSync('build/results.json');
  assert.equal(JSON.parse(report).ok, true, 'Run successful check.mjs first');
  try {
    writeFileSync(path, Buffer.concat([original, Buffer.from('\n/* Drift probe. */\n')]));
    const result = node(['check.mjs']);
    assert.notEqual(result.status, 0);
    const failure = JSON.parse(readFileSync('build/results.json', 'utf8'));
    assert.equal(failure.ok, false);
    assert.match(failure.error, /Build input changed: host.c/);
    assert.deepEqual(failure.results, []);
  } finally { writeFileSync(path, original); writeFileSync('build/results.json', report); }
});
