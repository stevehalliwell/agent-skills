// End-to-end CLI benchmark: process launch through exit/output capture.
// Prebuilt native executables only. Compile/reset/edit/check work is NOT timed.
// Warm filesystem, fresh child process per command, serial rotated runner order.
// Usage: node tools/benchmark.mjs --manifest <capture/manifest.json> [--runs 20] [--warmup 3]
import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import { appendFileSync, cpSync, mkdirSync, mkdtempSync, readFileSync, readdirSync, realpathSync, rmSync, statSync, utimesSync, writeFileSync } from 'node:fs';
import { basename, dirname, join, relative, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';
import { cpus, release, totalmem } from 'node:os';
import { DatabaseSync } from 'node:sqlite';
import { inventory } from './dataset-files.mjs';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const runnerScript = resolve(root, '../../skills/attendant/scripts/attendant.mjs');
const baseTime = new Date('2026-01-01T00:00:00.000Z');
const changedTime = new Date(baseTime.getTime() + 60_000);
const marker = 'attendantbenchprobe';
const bodyAppend = Buffer.from(`\n\nAttendant benchmark token: ${marker}.\n`);
const query = 'SELECT name,status,priority FROM tasks ORDER BY priority DESC,created_at,name LIMIT 25';
const scenarios = ['fresh-sync', 'unchanged-sync', 'unchanged-query', 'unchanged-search', 'body-edit-sync', 'mtime-only-sync'];
const sha256 = bytes => createHash('sha256').update(bytes).digest('hex');
const quote = name => '"' + name.replaceAll('"', '""') + '"';
const plain = rows => rows.map(row => ({ ...row }));
export function parseOptions(args) {
  const options = { runs: 20, warmup: 3, cjit: join(root, 'build/projection-cjit.exe'), zig: join(root, 'build/projection-zig.exe') };
  const seen = new Set();
  for (let i = 0; i < args.length; i++) {
    const key = args[i].replace(/^--/, '');
    assert.ok(args[i].startsWith('--') && ['manifest', 'runs', 'warmup', 'cjit', 'zig'].includes(key), `Unknown option: ${args[i]}`);
    assert.ok(!seen.has(key), `Repeated option: --${key}`); seen.add(key);
    assert.ok(args[i + 1] && !args[i + 1].startsWith('--'), `Missing value: --${key}`);
    const value = args[++i];
    if (key === 'runs' || key === 'warmup') {
      assert.match(value, /^\d+$/, `--${key} must be an integer`);
      options[key] = Number(value);
      assert.ok(Number.isSafeInteger(options[key]) && options[key] >= (key === 'runs' ? 1 : 0) && options[key] <= 1000, `--${key} outside supported range`);
    } else options[key] = resolve(value);
  }
  assert.ok(options.manifest, '--manifest is required');
  return options;
}
export function summarize(values) {
  assert.ok(values.length && values.every(value => Number.isFinite(value) && value >= 0), 'Invalid timing samples');
  const sorted = [...values].sort((a, b) => a - b);
  const percentile = fraction => {
    const index = (sorted.length - 1) * fraction, low = Math.floor(index), high = Math.ceil(index);
    return sorted[low] + (sorted[high] - sorted[low]) * (index - low);
  };
  return { samples: sorted.length, medianMs: percentile(0.5), p90Ms: percentile(0.9), minMs: sorted[0], maxMs: sorted.at(-1), meanMs: values.reduce((sum, value) => sum + value, 0) / values.length };
}
export function measure(command, args, cwd) {
  // Only this interval contributes timing samples. No shell or result parsing.
  const start = process.hrtime.bigint();
  const result = spawnSync(command, args, { cwd, timeout: 60_000, maxBuffer: 24 * 1024 * 1024 });
  const end = process.hrtime.bigint();
  return { result, elapsedMs: Number(end - start) / 1e6 };
}
function invoke(runner, args) {
  const measured = measure(runner.command, [...runner.prefix, ...args, '-p', runner.work], root);
  const { result } = measured;
  const stderr = (result.stderr ?? Buffer.alloc(0)).toString('utf8');
  const measurement = { elapsedMs: measured.elapsedMs, exitCode: result.status, signal: result.signal, stdoutBytes: result.stdout?.length ?? 0, stderr, command: runner.command, args: [...runner.prefix, ...args, '-p', runner.work] };
  try {
    const stdout = new TextDecoder('utf-8', { fatal: true }).decode(result.stdout ?? Buffer.alloc(0));
    assert.equal(result.status, 0, `${runner.name} ${args.join(' ')}: ${result.error?.message ?? stderr}\n${stdout}`);
    const data = JSON.parse(stdout);
    assert.ok(!data.error, JSON.stringify(data.error));
    assert.equal(data.diagnostics?.length ?? 0, 0, 'Unexpected diagnostics');
    assert.equal(data.corrections?.length ?? 0, 0, 'Unexpected source correction');
    return { data, ...measurement };
  } catch (error) { error.measurement = { ...measurement, stdoutBase64: result.stdout?.toString('base64') }; throw error; }
}
function database(runner, action) {
  const db = new DatabaseSync(join(runner.work, '.attendant/attendant.sqlite'), { readOnly: true });
  try { return action(db); } finally { db.close(); }
}
function normalizedSearch(data, work) {
  return { ...data, rows: data.rows.map(row => ({ ...row, source_path: relative(work, row.source_path).replaceAll('\\', '/') })) };
}
function checkSearch(actual, expected) {
  assert.equal(actual.rows.length, expected.rows.length, 'Search row count differs');
  for (let i = 0; i < actual.rows.length; i++) {
    const score = expected.rows[i].score;
    assert.ok(Math.abs(actual.rows[i].score - score) <= Number.EPSILON * 16 * Math.max(Math.abs(score), 1e-30), 'Search scores differ beyond compiler rounding allowance');
    actual.rows[i].score = score;
  }
  assert.deepEqual(actual, expected, 'Search results differ');
}
export function verifyCachedSource(db, sourcePath, expected) {
  const row = db.prepare('SELECT mtime_ms,size,source_hash FROM __attendant_cache WHERE source_path=?').get(sourcePath);
  assert.deepEqual({ ...row }, expected, 'Cached source metadata differs');
}
export function verifyPreparedSearch(db, state, target, expected, lazy, persistent = false) {
  assert.ok(['updated', 'rebuilt'].includes(state), 'Search setup did not update projection');
  verifyCachedSource(db, target.source_path, expected);
  assert.equal(db.prepare('SELECT source_hash FROM tasks WHERE id=?').get(target.id)?.source_hash, expected.source_hash, 'Search setup left stale projected row');
  if (persistent) {
    assert.equal(db.prepare("SELECT value FROM __attendant_metadata WHERE key='fts_enabled'").get()?.value, '1', 'Search setup disabled persistent indexes');
    assert.equal(db.prepare("SELECT count(*) n FROM sqlite_master WHERE name IN ('__attendant_fts_tasks','__attendant_trigram_tasks')").get().n, 2, 'Search setup lost persistent indexes');
  }
  if (lazy) {
    assert.equal(db.prepare("SELECT value FROM __attendant_metadata WHERE key='search_ready'").get()?.value, '0', 'Search setup did not invalidate lazy indexes');
    assert.equal(db.prepare("SELECT count(*) n FROM sqlite_master WHERE name IN ('__attendant_fts_tasks','__attendant_trigram_tasks')").get().n, 0, 'Search setup left lazy indexes');
  }
}
function verifySources(runner) {
  const actual = inventory(runner.work);
  assert.deepEqual(actual, runner.inventory, 'Benchmark changed source outside expected body edit');
}
export function benchmarkRunners(options) {
  const entries = options.nativeRunners ?? [{ name: 'cjit', path: options.cjit }, { name: 'zig', path: options.zig }];
  assert.ok(Array.isArray(entries) && entries.length > 0 && entries.length <= 16, 'Expected 1..16 native runners');
  const names = new Set(['mjs']);
  return entries.map(entry => {
    assert.match(entry.name, /^[a-z][a-z0-9-]{0,31}$/, 'Unsafe runner name');
    assert.ok(!names.has(entry.name), 'Repeated/reserved runner name'); names.add(entry.name);
    assert.equal(typeof entry.path, 'string', 'Missing executable path');
    return { name: entry.name, path: resolve(entry.path) };
  });
}
export function runBenchmark(options) {
  const lifecycleScenarios = [...scenarios, 'query-before-search', 'first-search', 'search-after-edit'];
  const selectedScenarios = options.onlyScenarios ?? (options.searchLifecycle ? lifecycleScenarios : scenarios);
  assert.ok(Array.isArray(selectedScenarios) && selectedScenarios.length && new Set(selectedScenarios).size === selectedScenarios.length, 'Invalid scenario selection');
  for (const scenario of selectedScenarios) assert.ok((options.searchLifecycle ? [...lifecycleScenarios, 'sync-before-search', 'body-edit-before-search'] : scenarios).includes(scenario), 'Unknown benchmark scenario');
  // Validate frozen input/config paths before launching any CLI. Never use the
  // source/working paths from manifest as mutation targets; create owned copies.
  const manifest = JSON.parse(readFileSync(options.manifest, 'utf8'));
  assert.ok(Array.isArray(manifest.datasets) && manifest.datasets.length, 'Manifest has no datasets');
  const labels = new Set();
  const datasets = manifest.datasets.map(dataset => {
    const pristine = realpathSync(dataset.pristine), label = basename(dirname(pristine));
    assert.ok(!labels.has(label.toLowerCase()), 'Repeated dataset label'); labels.add(label.toLowerCase());
    const captured = inventory(pristine);
    assert.deepEqual(captured, { collections: dataset.collections, files: dataset.files }, 'Captured source differs from manifest');
    assert.ok(captured.collections.some(table => table.name === 'tasks' && table.records > 0), 'Benchmark needs a nonempty tasks collection');
    return { label, pristine, inventory: captured };
  });
  const nativeRunners = benchmarkRunners(options);
  for (const { path } of nativeRunners) assert.ok(statSync(path).isFile(), `Prebuilt executable missing: ${path}`);
  const parent = join(root, 'build/bench'); mkdirSync(parent, { recursive: true });
  const directory = mkdtempSync(join(parent, 'run-'));
  const samplesPath = join(directory, 'samples.jsonl'), reportPath = join(directory, 'report.json');
  writeFileSync(samplesPath, '', { flag: 'wx' });
  const report = {
    complete: false, directory, startedAt: new Date().toISOString(), options,
    protocol: { clock: 'process.hrtime.bigint around spawnSync', includes: 'process launch, CLI startup, operation, stdout/stderr capture, exit and parent handoff', excludes: 'compilation, fixture reset/copy, source edits, JSON parsing and correctness checks', filesystem: 'warm; fixtures copied immediately before launches; no cache flush', order: 'serial; runner order rotates each round and scenario', fixedSourceMtime: baseTime.toISOString(), changedSourceMtime: changedTime.toISOString(), query, search: 'task, limit 20; search-after-edit uses marker after untimed sync', scenarios: selectedScenarios, percentiles: 'linear interpolation; descriptive sample statistics, not confidence intervals', builds: 'prebuilt current artifacts; no compiler launched by harness; optimization flags not embedded in executables' },
    environment: { node: process.version, nodeSqlite: process.versions.sqlite, platform: process.platform, arch: process.arch, osRelease: release(), cpu: cpus()[0]?.model, logicalCpus: cpus().length, totalMemoryBytes: totalmem(), relevantEnvironment: Object.fromEntries(['NODE_OPTIONS', 'NODE_NO_WARNINGS', 'LANG', 'LC_ALL', 'TZ'].map(key => [key, process.env[key] ?? null])) },
    artifacts: Object.fromEntries([['mjs', runnerScript], ...nativeRunners.map(entry => [entry.name, entry.path]), ['buildScript', join(root, 'build.sh')]].map(([name, path]) => [name, { path, bytes: statSync(path).size, sha256: sha256(readFileSync(path)) }])),
    datasets: [], summary: []
  };
  const samples = [];
  try {
    for (const dataset of datasets) {
      const runners = [['mjs', process.execPath, [runnerScript]], ...nativeRunners.map(entry => [entry.name, entry.path, []])].map(([name, command, prefix]) => ({ name, command, prefix, work: join(directory, dataset.label, name, 'work'), saved: join(directory, dataset.label, name, 'baseline'), coldSaved: join(directory, dataset.label, name, 'baseline-no-search') }));
      const facts = { label: dataset.label, sourceFiles: dataset.inventory.files.length, sourceBytes: dataset.inventory.files.reduce((sum, file) => sum + file.bytes, 0), sourceRecords: dataset.inventory.collections.reduce((sum, table) => sum + table.records, 0), runners: {} };
      let expectedQuery, expectedSearch, expectedRecords;
      for (const runner of runners) {
        for (const file of dataset.inventory.files) {
          const target = join(runner.work, file.path); mkdirSync(dirname(target), { recursive: true });
          writeFileSync(target, readFileSync(join(dataset.pristine, file.path)), { flag: 'wx' }); utimesSync(target, baseTime, baseTime);
        }
        runner.inventory = dataset.inventory;
        const strict = invoke(runner, ['validate', '--strict']).data;
        assert.equal(strict.records, facts.sourceRecords);
        const first = invoke(runner, ['sync']).data;
        assert.equal(first.state, 'rebuilt');
        const schema = invoke(runner, ['schema']).data;
        const tasks = schema.collections.find(table => table.name === 'tasks');
        assert.ok(tasks && ['status', 'priority'].every(name => tasks.fields.some(field => field.name === name)), 'Workload requires declared tasks.status and tasks.priority');
        assert.ok(tasks.usage !== null, 'Read collection usage before benchmark mutations');
        const rows = invoke(runner, ['query', '-s', query]).data;
        if (options.searchLifecycle) cpSync(join(runner.work, '.attendant'), runner.coldSaved, { recursive: true, errorOnExist: true, force: false });
        const search = normalizedSearch(invoke(runner, ['search', '-q', 'task', '-l', '20']).data, runner.work);
        assert.ok(rows.returned > 0 && search.returned > 0, 'Benchmark workload must not be empty');
        if (runner.name === 'mjs') { expectedQuery = rows; expectedSearch = structuredClone(search); expectedRecords = first.records; }
        else { assert.deepEqual(rows, expectedQuery); checkSearch(search, expectedSearch); assert.equal(first.records, expectedRecords); }
        const version = invoke(runner, ['query', '-s', 'SELECT sqlite_version() version']).data.rows[0].version;
        const info = database(runner, db => ({ target: { ...db.prepare('SELECT id,name,source_path FROM tasks ORDER BY name LIMIT 1').get() }, counts: schema.collections.map(table => ({ name: table.name, records: db.prepare(`SELECT count(*) records FROM ${quote(table.name)}`).get().records })), journalMode: db.prepare('PRAGMA journal_mode').get().journal_mode, pageSize: db.prepare('PRAGMA page_size').get().page_size, lazy: db.prepare("SELECT value FROM __attendant_metadata WHERE key='search_ready'").get() !== undefined, persistent: db.prepare("SELECT value FROM __attendant_metadata WHERE key='fts_enabled'").get() !== undefined }));
        runner.target = info.target; runner.lazy = info.lazy; runner.persistent = info.persistent;
        const targetRelative = relative(runner.work, runner.target.source_path);
        assert.ok(dataset.inventory.files.some(file => file.path === targetRelative), 'Selected target escapes owned source');
        runner.original = readFileSync(runner.target.source_path); runner.originalHash = sha256(runner.original);
        runner.edited = Buffer.concat([runner.original, bodyAppend]); runner.editedHash = sha256(runner.edited);
        runner.counts = info.counts;
        verifySources(runner);
        assert.equal(invoke(runner, ['sync']).data.state, 'reused', 'Baseline must be unchanged');
        cpSync(join(runner.work, '.attendant'), runner.saved, { recursive: true, errorOnExist: true, force: false });
        facts.runners[runner.name] = { sqliteVersion: version, journalMode: info.journalMode, pageSize: info.pageSize, projectedRecords: first.records, counts: info.counts, target: targetRelative, savedStateFiles: readdirSync(runner.saved).map(name => ({ name, bytes: statSync(join(runner.saved, name)).size })), preSearchStateFiles: options.searchLifecycle ? readdirSync(runner.coldSaved).map(name => ({ name, bytes: statSync(join(runner.coldSaved, name)).size })) : null };
      }
      writeFileSync(join(directory, dataset.label, 'expected.json'), JSON.stringify({ query: expectedQuery, search: expectedSearch }, null, 2) + '\n');
      report.datasets.push(facts);
      for (let scenarioIndex = 0; scenarioIndex < selectedScenarios.length; scenarioIndex++) {
        const scenario = selectedScenarios[scenarioIndex];
        console.error(`${dataset.label}: ${scenario} (${options.warmup} warmups + ${options.runs} samples per runner)`);
        for (let round = 0; round < options.warmup + options.runs; round++) {
          const phase = round < options.warmup ? 'warmup' : 'measured';
          for (let slot = 0; slot < runners.length; slot++) {
            const runner = runners[(round + scenarioIndex + slot) % runners.length];
            // Reset source and all DB/sidecar files with no live CLI connections.
            // Fixed integer-second mtimes make baseline freshness reproducible.
            writeFileSync(runner.target.source_path, runner.original); utimesSync(runner.target.source_path, baseTime, baseTime);
            rmSync(join(runner.work, '.attendant'), { recursive: true, force: true });
            if (scenario !== 'fresh-sync') cpSync(['query-before-search', 'first-search', 'sync-before-search', 'body-edit-before-search'].includes(scenario) ? runner.coldSaved : runner.saved, join(runner.work, '.attendant'), { recursive: true });
            const edited = ['body-edit-sync', 'body-edit-before-search', 'search-after-edit'].includes(scenario);
            if (edited) writeFileSync(runner.target.source_path, runner.edited);
            if (edited || scenario === 'mtime-only-sync') utimesSync(runner.target.source_path, changedTime, changedTime);
            assert.equal(statSync(runner.target.source_path).mtimeMs, edited || scenario === 'mtime-only-sync' ? changedTime.getTime() : baseTime.getTime());
            if (scenario === 'search-after-edit') {
              const prepared = invoke(runner, ['sync']).data; // Excluded: measure deferred search work, not update twice.
              database(runner, db => verifyPreparedSearch(db, prepared.state, runner.target, { mtime_ms: changedTime.getTime(), size: runner.edited.length, source_hash: runner.editedHash }, runner.lazy, runner.persistent));
            }
            const args = ['unchanged-query', 'query-before-search'].includes(scenario) ? ['query', '-s', query] : ['unchanged-search', 'first-search', 'search-after-edit'].includes(scenario) ? ['search', '-q', scenario === 'search-after-edit' ? marker : 'task', '-l', '20'] : ['sync'];
            const sample = { dataset: dataset.label, runner: runner.name, scenario, phase, round, slot, valid: false };
            try {
              const measured = invoke(runner, args);
              Object.assign(sample, { elapsedMs: measured.elapsedMs, exitCode: measured.exitCode, signal: measured.signal, stdoutBytes: measured.stdoutBytes, stderr: measured.stderr, command: measured.command, args: measured.args, state: measured.data.state ?? null });
              if (scenario === 'unchanged-query' || scenario === 'query-before-search') assert.deepEqual(measured.data, expectedQuery);
              else if (scenario === 'unchanged-search' || scenario === 'first-search') checkSearch(normalizedSearch(measured.data, runner.work), expectedSearch);
              else if (scenario === 'search-after-edit') {
                assert.deepEqual(measured.data.rows.map(row => row.id), [runner.target.id], 'Deferred search missed edited source');
                database(runner, db => verifyCachedSource(db, runner.target.source_path, { mtime_ms: changedTime.getTime(), size: runner.edited.length, source_hash: runner.editedHash }));
              } else {
                assert.equal(measured.data.records, expectedRecords); assert.equal(measured.data.collections, dataset.inventory.collections.length);
                assert.ok(['reused', 'updated', 'rebuilt'].includes(measured.data.state));
                if (scenario === 'fresh-sync') assert.equal(measured.data.state, 'rebuilt');
                if (scenario === 'unchanged-sync' || scenario === 'sync-before-search') assert.equal(measured.data.state, 'reused');
                if (edited) assert.ok(['updated', 'rebuilt'].includes(measured.data.state));
                database(runner, db => {
                  verifyCachedSource(db, runner.target.source_path, { mtime_ms: edited || scenario === 'mtime-only-sync' ? changedTime.getTime() : baseTime.getTime(), size: edited ? runner.edited.length : runner.original.length, source_hash: edited ? runner.editedHash : runner.originalHash });
                  assert.deepEqual(plain(db.prepare(query).all()), expectedQuery.rows);
                  for (const count of runner.counts) assert.equal(db.prepare(`SELECT count(*) records FROM ${quote(count.name)}`).get().records, count.records);
                  assert.equal(db.prepare('SELECT source_hash FROM tasks WHERE id=?').get(runner.target.id).source_hash, edited ? runner.editedHash : runner.originalHash);
                  const lazy = db.prepare("SELECT value FROM __attendant_metadata WHERE key IN ('search_ready','fts_enabled') AND value='0'").get() !== undefined;
                  if (lazy) assert.equal(db.prepare("SELECT count(*) n FROM sqlite_master WHERE name IN ('__attendant_fts_tasks','__attendant_trigram_tasks')").get().n, 0, 'Lazy indexes should be absent');
                  else {
                    const hits = db.prepare('SELECT record_id FROM __attendant_fts_tasks WHERE __attendant_fts_tasks MATCH ?').all(marker).map(row => row.record_id);
                    assert.deepEqual(hits, edited ? [runner.target.id] : [], 'Body edit was not reflected in FTS');
                  }
                });
              }
              if (options.searchLifecycle && !['unchanged-query', 'query-before-search', 'unchanged-search', 'first-search', 'search-after-edit'].includes(scenario)) {
                const verified = invoke(runner, ['search', '-q', marker, '-l', '20']).data;
                assert.deepEqual(verified.rows.map(row => row.id), edited ? [runner.target.id] : [], 'Post-timing search differs');
              }
              assert.equal(sha256(readFileSync(runner.target.source_path)), edited ? runner.editedHash : runner.originalHash, 'CLI changed source');
              sample.valid = true;
            } catch (error) { Object.assign(sample, error.measurement); sample.error = error.message; throw error; }
            finally { appendFileSync(samplesPath, JSON.stringify(sample) + '\n'); samples.push(sample); }
          }
        }
        for (const runner of runners) {
          writeFileSync(runner.target.source_path, runner.original); utimesSync(runner.target.source_path, baseTime, baseTime);
          verifySources(runner);
          const selected = samples.filter(sample => sample.dataset === dataset.label && sample.runner === runner.name && sample.scenario === scenario && sample.phase === 'measured');
          assert.equal(selected.length, options.runs); assert.ok(selected.every(sample => sample.valid));
          report.summary.push({ dataset: dataset.label, runner: runner.name, scenario, ...summarize(selected.map(sample => sample.elapsedMs)), states: selected.reduce((counts, sample) => (counts[sample.state ?? 'not-reported'] = (counts[sample.state ?? 'not-reported'] ?? 0) + 1, counts), {}) });
        }
      }
      assert.deepEqual(inventory(dataset.pristine), dataset.inventory, 'Pristine source changed');
    }
    for (const item of report.summary) item.speedupVsMjs = report.summary.find(other => other.dataset === item.dataset && other.scenario === item.scenario && other.runner === 'mjs').medianMs / item.medianMs;
    report.complete = true; report.finishedAt = new Date().toISOString();
    report.measuredSamples = samples.filter(sample => sample.phase === 'measured').length;
    report.warmupSamples = samples.filter(sample => sample.phase === 'warmup').length;
    writeFileSync(reportPath, JSON.stringify(report, null, 2) + '\n');
    return { directory, reportPath, samplesPath, measuredSamples: report.measuredSamples, warmupSamples: report.warmupSamples, summary: report.summary };
  } catch (error) {
    report.error = error.message; report.finishedAt = new Date().toISOString();
    writeFileSync(reportPath, JSON.stringify(report, null, 2) + '\n');
    error.benchmarkDirectory = directory;
    throw error;
  }
}
if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  if (process.argv.includes('--help')) console.log(JSON.stringify({ usage: 'node tools/benchmark.mjs --manifest <capture/manifest.json> [--runs 20] [--warmup 3] [--cjit <exe>] [--zig <exe>]', output: 'JSON summary; full report and every warmup/measured sample under ignored build/bench/run-*', timing: 'wall-clock child launch through exit/output capture; startup included, compilation excluded', safety: 'new owned copies only; no original/captured-source mutations; no compiler invocation' }));
  else {
    let options;
    try { options = parseOptions(process.argv.slice(2)); }
    catch (error) { console.log(JSON.stringify({ error: { code: 'invalid-arguments', message: error.message } })); process.exitCode = 2; }
    if (options) try { console.log(JSON.stringify(runBenchmark(options))); }
    catch (error) { console.log(JSON.stringify({ error: { code: 'benchmark-failed', message: error.message }, directory: error.benchmarkDirectory })); console.error(error.message); process.exitCode = 1; }
  }
}
