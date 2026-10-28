// Interleaved safe SQLite connection profiles; same startup-inclusive harness.
// Build profiles separately; compilation/reset/edit/check work is NOT timed.
// Usage: node sqlite-tuning-check.mjs --manifest <capture/manifest.json> [--runs 21] [--warmup 3]
// Six native profiles plus Node: 21 rounds balance all seven ordering positions.
// Mutate only benchmark-owned copies. Never changes normal native defaults.
import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import { readFileSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';
import { parseOptions, runBenchmark } from './benchmark.mjs';
const root = dirname(fileURLToPath(import.meta.url));
const profiles = ['control', 'cache', 'mmap', 'combined', 'large', 'vacuum'];
const args = process.argv.slice(2);
if (args.includes('--help')) {
  console.log(JSON.stringify({ usage: 'node sqlite-tuning-check.mjs --manifest <capture/manifest.json> [--runs 21] [--warmup 3]', profiles, note: 'Prebuilt experimental binaries required. Defaults unchanged. Profiles interleaved with Node control.' }));
  process.exit(0);
}
try {
  assert.ok(!args.includes('--zig') && !args.includes('--cjit'), 'Tuning driver uses fixed experimental binaries');
  if (!args.includes('--runs')) args.push('--runs', '21');
  if (!args.includes('--warmup')) args.push('--warmup', '3');
  const options = parseOptions(args);
  assert.ok(!process.env.ATTENDANT_TUNING_REPORT, 'Unset ATTENDANT_TUNING_REPORT before timing');
  const nativeRunners = profiles.map(name => ({ name, path: join(root, `build/projection-zig-tuning-${name}.exe`) }));
  const sourceArtifacts = Object.fromEntries(['sqlite-tuning.c', 'sqlite-tuning-build.sh', 'build-inputs.sh', 'benchmark.mjs'].map(name => [name, createHash('sha256').update(readFileSync(join(root, name))).digest('hex')]));
  const result = runBenchmark({ ...options, nativeRunners });
  const report = JSON.parse(readFileSync(result.reportPath, 'utf8')), probes = [];
  for (const runner of nativeRunners) for (const dataset of report.datasets) {
    const work = join(result.directory, dataset.label, runner.name, 'work');
    const sync = spawnSync(runner.path, ['sync', '-p', work], { encoding: 'utf8', timeout: 60_000 });
    assert.equal(sync.status, 0, sync.stderr);
    const probe = spawnSync(runner.path, ['query', '-s', report.protocol.query, '-p', work], { encoding: 'utf8', timeout: 60_000, env: { ...process.env, ATTENDANT_TUNING_REPORT: '1' } });
    assert.equal(probe.status, 0, probe.stderr);
    probes.push({ profile: runner.name, dataset: dataset.label, operation: 'untimed unchanged query after cache alignment', measurements: probe.stderr.trim().split(/\r?\n/).filter(Boolean).map(JSON.parse) });
  }
  const output = { complete: true, ...result, probes, sourceArtifacts };
  writeFileSync(join(result.directory, 'sqlite-tuning.json'), JSON.stringify(output, null, 2) + '\n');
  const resultsPath = resolve(root, 'build/sqlite-tuning-interleaved.json');
  writeFileSync(resultsPath, JSON.stringify(output, null, 2) + '\n');
  console.log(JSON.stringify({ resultsPath, ...output }));
} catch (error) {
  console.log(JSON.stringify({ error: { code: 'sqlite-tuning-failed', message: error.message, directory: error.benchmarkDirectory } }));
  console.error(error.message); process.exitCode = 1;
}
