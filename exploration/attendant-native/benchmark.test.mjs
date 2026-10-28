// Focused harness checks. Full workload smoke uses benchmark.mjs on captured copies.
import assert from 'node:assert/strict';
import { test } from 'node:test';
import { mkdirSync, mkdtempSync, rmSync, writeFileSync } from 'node:fs';
import { join, resolve } from 'node:path';
import { DatabaseSync } from 'node:sqlite';
import { parseOptions, summarize, measure, verifyCachedSource, verifyPreparedSearch, benchmarkRunners } from './benchmark.mjs';
import { inventory } from './dataset-files.mjs';

test('statistics retain outliers and use interpolated percentiles', () => {
  const input = [40, 10, 30, 20];
  assert.deepEqual(summarize(input), { samples: 4, medianMs: 25, p90Ms: 37, minMs: 10, maxMs: 40, meanMs: 25 });
  assert.deepEqual(input, [40, 10, 30, 20]);
  const outlier = summarize([1, 1, 1, 1, 100]);
  assert.equal(outlier.medianMs, 1); assert.equal(outlier.maxMs, 100); assert.equal(outlier.samples, 5);
});
test('singleton and invalid timing groups', () => {
  assert.deepEqual(summarize([7]), { samples: 1, medianMs: 7, p90Ms: 7, minMs: 7, maxMs: 7, meanMs: 7 });
  for (const values of [[], [-1], [NaN], [Infinity]]) assert.throws(() => summarize(values));
});
test('CLI requires explicit manifest and validates repeat counts', () => {
  assert.equal(parseOptions(['--manifest', 'capture.json']).runs, 20);
  assert.equal(parseOptions(['--manifest', 'capture.json']).warmup, 3);
  assert.equal(parseOptions(['--manifest', 'capture.json', '--warmup', '0', '--runs', '1']).warmup, 0);
  for (const args of [[], ['--unknown', 'x'], ['--manifest'], ['--manifest', 'x', '--runs', '0'], ['--manifest', 'x', '--runs', '-1'], ['--manifest', 'x', '--warmup', '1.5'], ['--manifest', 'x', '--runs', '1001'], ['--manifest', 'x', '--runs', '1', '--runs', '2']]) assert.throws(() => parseOptions(args));
});
test('experimental runner registry keeps defaults and rejects unsafe names', () => {
  assert.deepEqual(benchmarkRunners({ cjit: 'cjit.exe', zig: 'zig.exe' }).map(entry => entry.name), ['cjit', 'zig']);
  assert.deepEqual(benchmarkRunners({ nativeRunners: [{ name: 'cache', path: 'cache.exe' }, { name: 'mmap', path: 'mmap.exe' }] }).map(entry => entry.name), ['cache', 'mmap']);
  for (const entries of [[], [{ name: '../escape', path: 'x' }], [{ name: 'mjs', path: 'x' }], [{ name: 'same', path: 'x' }, { name: 'same', path: 'y' }]]) assert.throws(() => benchmarkRunners({ nativeRunners: entries }));
});
test('usage errors return structured JSON and exit 2', () => {
  const { result } = measure(process.execPath, ['exploration/attendant-native/benchmark.mjs', '--manifest', 'unused', '--runs', '0']);
  assert.equal(result.status, 2); assert.equal(JSON.parse(result.stdout).error.code, 'invalid-arguments');
});
test('mtime-only validation rejects stale cache even when size and hash match', () => {
  const db = new DatabaseSync(':memory:');
  try {
    db.exec('CREATE TABLE __attendant_cache(source_path TEXT,mtime_ms REAL,size INTEGER,source_hash TEXT)');
    db.prepare('INSERT INTO __attendant_cache VALUES(?,?,?,?)').run('record.md', 1000, 50, 'hash');
    const original = { mtime_ms: 1000, size: 50, source_hash: 'hash' }, changed = { ...original, mtime_ms: 61000 };
    verifyCachedSource(db, 'record.md', original);
    assert.throws(() => verifyCachedSource(db, 'record.md', changed), /Cached source metadata differs/);
    db.exec('UPDATE __attendant_cache SET mtime_ms=61000');
    verifyCachedSource(db, 'record.md', changed);
    for (const expected of [{ ...changed, size: 51 }, { ...changed, source_hash: 'stale' }]) assert.throws(() => verifyCachedSource(db, 'record.md', expected));
  } finally { db.close(); }
});
test('deferred-search setup rejects successful stale sync before timing', () => {
  const db = new DatabaseSync(':memory:');
  try {
    db.exec("CREATE TABLE __attendant_cache(source_path TEXT,mtime_ms REAL,size INTEGER,source_hash TEXT); CREATE TABLE tasks(id TEXT,source_hash TEXT); CREATE TABLE __attendant_metadata(key TEXT,value TEXT); INSERT INTO __attendant_cache VALUES('r.md',1000,50,'old'); INSERT INTO tasks VALUES('record','old'); INSERT INTO __attendant_metadata VALUES('search_ready','0')");
    const target = { id: 'record', source_path: 'r.md' }, expected = { mtime_ms: 61000, size: 60, source_hash: 'new' };
    assert.throws(() => verifyPreparedSearch(db, 'updated', target, expected, true), /Cached source metadata differs/);
    db.exec("UPDATE __attendant_cache SET mtime_ms=61000,size=60,source_hash='new'");
    assert.throws(() => verifyPreparedSearch(db, 'updated', target, expected, true), /stale projected row/);
    db.exec("UPDATE tasks SET source_hash='new'");
    verifyPreparedSearch(db, 'updated', target, expected, true);
    assert.throws(() => verifyPreparedSearch(db, 'reused', target, expected, true), /did not update/);
    db.exec("UPDATE __attendant_metadata SET value='1'");
    assert.throws(() => verifyPreparedSearch(db, 'updated', target, expected, true), /did not invalidate/);
    db.exec("UPDATE __attendant_metadata SET value='0'; CREATE TABLE __attendant_trigram_tasks(record_id TEXT)");
    assert.throws(() => verifyPreparedSearch(db, 'updated', target, expected, true), /left lazy indexes/);
    db.exec("INSERT INTO __attendant_metadata VALUES('fts_enabled','1'); CREATE TABLE __attendant_fts_tasks(record_id TEXT)");
    verifyPreparedSearch(db, 'updated', target, expected, false, true);
    db.exec("UPDATE __attendant_metadata SET value='0' WHERE key='fts_enabled'");
    assert.throws(() => verifyPreparedSearch(db, 'updated', target, expected, false, true), /disabled persistent/);
    db.exec("UPDATE __attendant_metadata SET value='1' WHERE key='fts_enabled'; DROP TABLE __attendant_fts_tasks");
    assert.throws(() => verifyPreparedSearch(db, 'updated', target, expected, false, true), /lost persistent/);
  } finally { db.close(); }
});
test('external wall clock includes child lifetime and captures complete output', () => {
  const measured = measure(process.execPath, ['-e', 'setTimeout(()=>process.stdout.write(JSON.stringify({ok:true})),30)']);
  assert.equal(measured.result.status, 0);
  assert.deepEqual(JSON.parse(measured.result.stdout), { ok: true });
  assert.ok(measured.elapsedMs >= 25);
});
test('child failures are returned, not converted into successful timing results', () => {
  const measured = measure(process.execPath, ['-e', 'process.stderr.write("failed");process.exit(2)']);
  assert.equal(measured.result.status, 2); assert.equal(measured.result.stderr.toString(), 'failed');
  assert.ok(measured.elapsedMs > 0);
});
test('shared capture/benchmark inventory refuses escaping collection paths', () => {
  const parent = resolve('exploration/attendant-native/build'); mkdirSync(parent, { recursive: true });
  const temporary = mkdtempSync(join(parent, 'benchmark-check-'));
  try {
    mkdirSync(join(temporary, '.pi')); writeFileSync(join(temporary, '.pi/attendant.tables'), '../outside as tasks\n');
    assert.throws(() => inventory(temporary), /inside source project/);
  } finally { rmSync(temporary, { recursive: true, force: true }); }
});
