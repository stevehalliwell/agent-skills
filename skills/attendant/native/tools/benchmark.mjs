#!/usr/bin/env node
// Repeatable warm-filesystem CLI process timings on owned fixture copies.
// Usage: node tools/benchmark.mjs [runs=10], integer 1..1000.
// Excludes builds, fixture copy, warmup and JSON checking. Includes launch through exit.
// Small synthetic fixture only; not real-project performance or GUI footprint evidence.
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { cpSync, mkdirSync, mkdtempSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { performance } from 'node:perf_hooks';
if (process.argv.includes('--help')) {
  console.log('Usage: node tools/benchmark.mjs [runs=10]\nOwned synthetic copies; warm sync/query/search; CJIT and Zig; results under build/bench/.');
  process.exit(0);
}
const runs = Number(process.argv[2] ?? 10);
if (process.argv.length > 3 || !Number.isInteger(runs) || runs < 1 || runs > 1000) {
  console.error('Usage: node tools/benchmark.mjs [runs=10], integer 1..1000'); process.exit(2);
}
const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
mkdirSync(join(root, 'build/bench'), { recursive: true });
const run = mkdtempSync(join(root, 'build/bench/run-'));
const compilers = ['cjit', 'zig'];
const commands = { sync: ['sync'], query: ['query', '-s', 'SELECT name,status FROM tasks ORDER BY name'], search: ['search', '-q', 'projection'] };
const samples = [];
for (const compiler of compilers) cpSync(join(root, 'tests/fixtures/project'), join(run, compiler), { recursive: true });
function invoke(compiler, name) {
  const start = performance.now();
  const child = spawnSync(join(root, `build/attendant-${compiler}.exe`), [...commands[name], '-p', join(run, compiler)], { encoding: 'utf8', timeout: 30000, windowsHide: true });
  const milliseconds = performance.now() - start;
  assert.equal(child.status, 0, child.error?.message || child.stderr);
  const result = JSON.parse(child.stdout);
  if (name === 'sync') assert(['rebuilt', 'reused'].includes(result.state));
  else assert.equal(result.returned, name === 'query' ? 2 : 1);
  return milliseconds;
}
for (const compiler of compilers) for (const name of Object.keys(commands)) invoke(compiler, name);
for (let round = 0; round < runs; round++) for (let offset = 0; offset < compilers.length; offset++) {
  const compiler = compilers[(round + offset) % compilers.length];
  for (const name of Object.keys(commands)) samples.push({ round, compiler, command: name, milliseconds: invoke(compiler, name) });
}
const summary = compilers.flatMap(compiler => Object.keys(commands).map(command => {
  const values = samples.filter(sample => sample.compiler === compiler && sample.command === command).map(sample => sample.milliseconds).sort((a, b) => a - b);
  const middle = Math.floor(values.length / 2), median = values.length % 2 ? values[middle] : (values[middle - 1] + values[middle]) / 2;
  return { compiler, command, samples: values.length, minimumMs: values[0], medianMs: median, maximumMs: values.at(-1) };
}));
writeFileSync(join(run, 'results.json'), JSON.stringify({ runs, scope: 'warm synthetic fixture, process launch through exit; rotating serial compiler order', samples, summary }, null, 2) + '\n');
console.log(JSON.stringify({ run, summary }));
