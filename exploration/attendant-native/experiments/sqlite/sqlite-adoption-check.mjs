// Current native/CJIT/Node + matched late-populate and WAL/OFF comparisons.
// Usage: node sqlite-adoption-check.mjs --manifest <capture/manifest.json> [--runs 18] [--warmup 2] [--first-search-only]
// Prebuild normal binaries and wal-off/full/full-wal-off via sqlite-adoption-build.sh.
import assert from 'node:assert/strict';
import { readFileSync, writeFileSync } from 'node:fs';
import { createHash } from 'node:crypto';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { parseOptions, runBenchmark } from '../../tools/benchmark.mjs';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
try {
  const args = process.argv.slice(2);
  if (args.includes('--help')) {
    console.log(JSON.stringify({ usage: 'node sqlite-adoption-check.mjs --manifest <capture/manifest.json> [--runs 18] [--warmup 2] [--first-search-only]', note: '6 runners, 11 cases; current native plus fair late/full and WAL/OFF variants; compilation excluded.' })); process.exit(0);
  }
  assert.ok(args.filter(arg => arg === '--first-search-only').length <= 1, 'Repeated --first-search-only');
  const firstOnly = args.includes('--first-search-only');
  if (firstOnly) args.splice(args.indexOf('--first-search-only'), 1);
  if (!args.includes('--runs')) args.push('--runs', '18');
  if (!args.includes('--warmup')) args.push('--warmup', '2');
  const options = parseOptions(args);
  assert.ok(!process.env.ATTENDANT_EXPERIMENT_PHASE && !process.env.ATTENDANT_EXPERIMENT_SIGNAL, 'Unset pause variables before timing');
  const nativeRunners = [{ name: 'cjit', path: options.cjit }, { name: 'zig', path: options.zig }, ...['wal-off', 'full', 'full-wal-off'].map(name => ({ name, path: join(root, `build/projection-zig-adoption-${name}.exe`) }))];
  const result = runBenchmark({ ...options, nativeRunners, searchLifecycle: true, onlyScenarios: firstOnly ? ['first-search'] : ['fresh-sync', 'unchanged-sync', 'unchanged-query', 'unchanged-search', 'body-edit-sync', 'mtime-only-sync', 'query-before-search', 'first-search', 'search-after-edit', 'sync-before-search', 'body-edit-before-search'] });
  const hash = path => createHash('sha256').update(readFileSync(path)).digest('hex');
  const sourceArtifacts = Object.fromEntries(['apps/cli/main.c', 'src/core/core.c', 'src/core/prepare.h', 'apps/cli/read.h', 'tools/build-inputs.sh', 'experiments/sqlite/sqlite-adoption-source.mjs', 'experiments/sqlite/sqlite-adoption-build.sh', 'tools/benchmark.mjs'].map(name => [name, hash(join(root, name))]));
  const generatedSources = Object.fromEntries(['wal-off', 'full', 'full-wal-off'].map(profile => [profile, Object.fromEntries(['apps/cli/main.c', 'src/core/core.c', 'src/core/prepare.h', 'apps/cli/read.h'].map(name => [name, hash(join(root, 'build/sqlite-adoption', profile, name))]))]));
  const output = { complete: true, ...result, sourceArtifacts, generatedSources, note: 'Normal first search populates in place; full variants replace cache. Population validates before commit for rollback; full validates private committed DB before publication. Same mmap and persistent-index policy. WAL/OFF remains experimental.' };
  writeFileSync(join(result.directory, 'sqlite-adoption.json'), JSON.stringify(output, null, 2) + '\n');
  writeFileSync(join(root, firstOnly ? 'build/sqlite-population-first-search.json' : 'build/sqlite-population.json'), JSON.stringify(output, null, 2) + '\n');
  console.log(JSON.stringify(output));
} catch (error) {
  console.log(JSON.stringify({ error: { code: 'sqlite-adoption-failed', message: error.message, directory: error.benchmarkDirectory } }));
  console.error(error.message); process.exitCode = 1;
}
