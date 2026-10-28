// Disposable-cache/lazy-search experiment. Prebuild via sqlite-experiment-build.sh.
// Usage: node sqlite-experiment-check.mjs --manifest <capture/manifest.json> [--runs 13] [--warmup 2] [--unsearched-only]
// Mutations only in benchmark-owned copies. Normal application defaults unchanged.
import assert from 'node:assert/strict';
import { readFileSync, writeFileSync } from 'node:fs';
import { createHash } from 'node:crypto';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { profiles } from './sqlite-experiment-source.mjs';
import { parseOptions, runBenchmark } from './benchmark.mjs';
const root = dirname(fileURLToPath(import.meta.url));
try {
  const args = process.argv.slice(2);
  if (args.includes('--help')) { console.log(JSON.stringify({ usage: 'node sqlite-experiment-check.mjs --manifest <capture/manifest.json> [--runs 13] [--warmup 2] [--unsearched-only]', profiles, note: 'Unsafe settings isolated to experimental executables. Includes first search and search after edit.' })); process.exit(0); }
  assert.ok(args.filter(arg => arg === '--unsearched-only').length <= 1, 'Repeated --unsearched-only');
  const unsearchedOnly = args.includes('--unsearched-only');
  if (unsearchedOnly) args.splice(args.indexOf('--unsearched-only'), 1);
  assert.ok(!args.includes('--zig') && !args.includes('--cjit'), 'Fixed experimental binaries required');
  if (!args.includes('--runs')) args.push('--runs', '13');
  if (!args.includes('--warmup')) args.push('--warmup', '2');
  const options = parseOptions(args);
  assert.ok(!process.env.ATTENDANT_EXPERIMENT_PHASE && !process.env.ATTENDANT_EXPERIMENT_SIGNAL, 'Unset experimental pause variables before timing');
  const result = runBenchmark({ ...options, searchLifecycle: true, ...(unsearchedOnly ? { onlyScenarios: ['sync-before-search', 'body-edit-before-search'] } : {}), nativeRunners: profiles.map(name => ({ name, path: join(root, `build/projection-zig-experiment-${name}.exe`) })) });
  const hash = path => createHash('sha256').update(readFileSync(path)).digest('hex');
  const sourceArtifacts = Object.fromEntries(['sqlite-experiment-source.mjs', 'sqlite-experiment-build.sh', 'benchmark.mjs', 'build-inputs.sh'].map(name => [name, hash(join(root, name))]));
  const generatedSources = Object.fromEntries(profiles.map(profile => [profile, Object.fromEntries(['projection.c', 'prepare.h', 'read.h'].map(name => [name, hash(join(root, 'build/sqlite-experiments', profile, name))]))]));
  const output = { complete: true, ...result, sourceArtifacts, generatedSources, limitation: 'Timing/parity, not power-loss qualification. Experimental binaries are not production defaults.' };
  writeFileSync(join(result.directory, 'sqlite-experiments.json'), JSON.stringify(output, null, 2) + '\n');
  writeFileSync(join(root, unsearchedOnly ? 'build/sqlite-experiments-unsearched.json' : 'build/sqlite-experiments.json'), JSON.stringify(output, null, 2) + '\n');
  console.log(JSON.stringify(output));
} catch (error) {
  console.log(JSON.stringify({ error: { code: 'sqlite-experiment-failed', message: error.message, directory: error.benchmarkDirectory } }));
  console.error(error.message); process.exitCode = 1;
}
