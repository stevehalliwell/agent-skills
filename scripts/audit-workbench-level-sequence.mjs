#!/usr/bin/env node
// Q14: replay one Mario ending proposal and two controls through native scaffolding.
// Native owns route semantics. Copies/edits only NEW output fixtures; never adopts records.
// Usage: node scripts/audit-workbench-level-sequence.mjs --output NEW_DIRECTORY --executable PATH
// Stdout: one JSON receipt; failures retain artifacts, emit structured error and exit 2.
import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import { cpSync, existsSync, mkdirSync, readdirSync, readFileSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve, sep } from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';

const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const example = join(root, 'skills/game-design-workbench/examples/little-plumber-guy-nes');
const source = join(example, 'game-design');
const hash = bytes => createHash('sha256').update(bytes).digest('hex');
let output, executable, before, manifest;
function snapshot(folder) {
  return readdirSync(folder, { withFileTypes: true }).sort((a, b) => a.name.localeCompare(b.name)).flatMap(entry => {
    const path = join(folder, entry.name);
    assert.ok(!entry.isSymbolicLink(), `Source link refused: ${path}`);
    return entry.isDirectory() ? snapshot(path) : [{ path, sha256: hash(readFileSync(path)) }];
  });
}
function probe(name, challenges) {
  const args = ['scaffolding', '--skill-atoms', `skill-atoms=${source}/skill-atoms`,
    '--challenges', `challenges=${challenges}`, '--json'];
  const result = spawnSync(executable, args, { encoding: 'utf8', timeout: 30000,
    maxBuffer: 32 * 1024 * 1024, windowsHide: true, env: { ...process.env, GD_AGENT: '1' } });
  writeFileSync(join(output, name + '.json'), result.stdout ?? '', 'utf8');
  writeFileSync(join(output, name + '.stderr.log'), result.stderr ?? '', 'utf8');
  manifest.commands.push({ name, args, exit: result.status, failure: result.error?.message });
  assert.equal(result.status, 0, `${name}: native failure; inspect retained artifacts`);
  const value = JSON.parse(result.stdout);
  assert.equal(value.ok, true);
  assert.equal(value.coverage.complete, true);
  assert.equal(value.data.findings_omitted, 0);
  assert.equal(value.coverage.routes_complete, true);
  assert.equal(value.coverage.requirements_complete, true);
  assert.equal(value.data.witnesses_omitted, 0);
  const check = value.data.checks.find(row => row.challenge === 'challenges/final-gap' && row.skill === 'skill-atoms/finish-approach');
  assert.ok(check);
  const witnesses = value.data.witnesses.filter(row => row.check === check.index);
  assert.ok(witnesses.every(row => !row.prefix_truncated));
  const summary = { name, challenges: value.coverage.challenge_count, checks: value.coverage.check_count,
    target: check, witnesses };
  manifest.results.push(summary);
  return summary;
}
function copy(name) {
  const folder = join(output, name, 'challenges');
  cpSync(join(source, 'challenges'), folder, { recursive: true, errorOnExist: true, force: false });
  return folder;
}
function replace(folder, name, oldText, newText) {
  const file = join(folder, name + '.md'), content = readFileSync(file, 'utf8');
  assert.equal(content.split(oldText).length, 2, `Expected unique source anchor: ${name}`);
  writeFileSync(file, content.replace(oldText, newText), 'utf8');
}
try {
  for (let index = 2; index < process.argv.length; ++index) {
    const key = process.argv[index];
    if (key === '--help') {
      console.log('Usage: node scripts/audit-workbench-level-sequence.mjs --output NEW_DIRECTORY --executable PATH\nRead-only source; private proposed fixtures only. Receipt JSON on stdout; exit 0 success / 2 failure.');
      process.exit(0);
    }
    assert.ok(['--output', '--executable'].includes(key) && process.argv[index + 1] && !process.argv[index + 1].startsWith('--'), 'Unknown option or missing value');
    if (key === '--output') { assert.ok(!output, 'Duplicate output'); output = resolve(process.argv[++index]); }
    else { assert.ok(!executable, 'Duplicate executable'); executable = resolve(process.argv[++index]); }
  }
  assert.ok(output && executable && existsSync(executable) && !existsSync(output), 'Require existing executable and new output directory');
  assert.ok(output.toLowerCase() !== example.toLowerCase() && !output.toLowerCase().startsWith(example.toLowerCase() + sep), 'Output must stay outside authored example');
  before = snapshot(source);
  // Atomic leaf creation refuses concurrent reuse; caller must supply existing parent.
  mkdirSync(output);
  manifest = { scope: 'Private hypothetical route declarations; no gameplay, adoption or learning proof',
    source, sourceHashes: before, executable, executableSha256: hash(readFileSync(executable)),
    scriptSha256: hash(readFileSync(fileURLToPath(import.meta.url))), commands: [], results: [] };
  const baseline = probe('baseline', join(source, 'challenges'));
  assert.equal(baseline.target.coverage, 'absent');
  assert.equal(baseline.challenges, 19);

  const upper = copy('upper-only');
  replace(upper, 'high-stair', 'requires:\n', 'introduces:\n  - skill-atoms/finish-approach\nrequires:\n');
  const upperResult = probe('upper-only', upper);
  assert.equal(upperResult.target.coverage, 'some_routes');
  assert.ok(upperResult.witnesses.some(row => !row.supported && row.path.includes('challenges/low-path') && !row.path.includes('challenges/high-stair')));

  const annotation = copy('annotation-only');
  replace(annotation, 'checkpoint-strip', 'introduces:\n', 'introduces:\n  - skill-atoms/finish-approach\n');
  const annotationResult = probe('annotation-only', annotation);
  assert.equal(annotationResult.target.coverage, 'all_routes');
  assert.equal(annotationResult.challenges, 19);

  const shared = copy('shared-rehearsal');
  for (const branch of ['high-stair', 'low-path']) replace(shared, branch,
    '  - challenges/final-gap\n', '  - challenges/approach-rehearsal\n');
  writeFileSync(join(shared, 'approach-rehearsal.md'), `---
name: approach-rehearsal
status: proposed
entry: false
next:
  - challenges/final-gap
introduces:
  - skill-atoms/finish-approach
practises:
  - skill-atoms/land-on-target
  - skill-atoms/manage-clock
requires:
  - skill-atoms/jump
mechanics:
  - mechanics/jump-launch
  - mechanics/solid-collision
  - mechanics/active-clock
nouns:
  - nouns/step
---
## Proposed encounter, not adopted source

Both branches rejoin at a broad approach strip. Read a noninteractive launch cue,
jump onto a low broad step above continuous floor, land, notice remaining clock,
and continue toward visible final approach. A missed landing can return to floor
rather than a lethal gap in intended geometry. No time/life/power refill, new
finish trigger or enemy is proposed. Clock remains active. Checkpoint rule unchanged.

Exact width, height, collision, sightline, timing and available clock are unknown.
This declaration does not prove physical feasibility, learning, lower difficulty
or player awareness. Cue is prose layout proposal, not a new gameplay noun/rule.
`, 'utf8');
  const sharedResult = probe('shared-rehearsal', shared);
  assert.equal(sharedResult.target.coverage, 'all_routes');
  assert.equal(sharedResult.challenges, 20);
  assert.ok(sharedResult.witnesses.some(row => row.supported && row.path.includes('challenges/approach-rehearsal')));
  assert.ok(!sharedResult.witnesses.some(row => !row.supported));
  assert.deepEqual(snapshot(source), before);
  manifest.sourceUnchanged = true;
  manifest.ok = true;
  console.log(JSON.stringify({ ok: true, output, sourceUnchanged: true,
    results: manifest.results.map(row => ({ name: row.name, challenges: row.challenges, checks: row.checks, coverage: row.target.coverage })),
    limits: 'Declaration opportunity only; annotation-only control also passes, not proof of improved design or learning' }));
} catch (error) {
  if (manifest) { manifest.ok = false; manifest.error = error.message; }
  console.error(error.message);
  console.log(JSON.stringify({ ok: false, error: { code: 'level_sequence.audit', message: error.message }, output }));
  process.exitCode = 2;
} finally {
  if (manifest) {
    try {
      manifest.sourceUnchanged = JSON.stringify(snapshot(source)) === JSON.stringify(before);
      if (!manifest.sourceUnchanged) { manifest.ok = false; process.exitCode = 2; console.error('Authored source changed during audit'); }
      writeFileSync(join(output, 'manifest.json'), JSON.stringify(manifest, null, 2) + '\n', 'utf8');
    } catch (error) { console.error(error.message); process.exitCode = 2; }
  }
}
