// Source-contract acceptance; generated copies only. Node >=24.10 for lossless JSON.
// Usage: node tests/core/source-check.mjs <native-executable> [...]
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { existsSync, mkdirSync, readFileSync, readdirSync, rmSync, writeFileSync } from 'node:fs';
import { basename, dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { DatabaseSync } from 'node:sqlite';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
if (process.argv.length < 3 || process.argv.includes('--help')) {
  console.log('Usage: node tests/core/source-check.mjs <native-executable> [...]'); process.exit(process.argv.includes('--help') ? 0 : 2);
}
function decode(text) {
  return JSON.parse(text, (key, value, context) => typeof value === 'number' && !Number.isSafeInteger(value) && /^-?\d+$/.test(context.source) ? BigInt(context.source) : value);
}
function hashes(path) {
  return Object.fromEntries(readdirSync(path, { withFileTypes: true }).flatMap(entry => {
    const target = join(path, entry.name);
    return entry.isDirectory() ? [[target, '<directory>'], ...Object.entries(hashes(target))] : [[target, createHash('sha256').update(readFileSync(target)).digest('hex')]];
  }));
}
const report = [];
for (const executable of process.argv.slice(2).map(path => resolve(path))) for (const variant of ['relative', 'absolute']) {
  assert.ok(existsSync(executable), `Executable not found: ${executable}`);
  const project = join(root, 'build/review', `${basename(executable, '.exe')}-${variant} source contract café Ω`);
  rmSync(project, { recursive: true, force: true }); mkdirSync(join(project, '.pi'), { recursive: true });
  writeFileSync(join(project, '.pi/attendant.tables'), '');
  function native(args, expected = 0) {
    const result = spawnSync(executable, [...args, '-p', project], { timeout: 30_000, maxBuffer: 24 * 1024 * 1024 });
    const text = new TextDecoder('utf-8', { fatal: true }).decode(result.stdout);
    assert.equal(result.status, expected, `${args.join(' ')}\n${text}\n${result.stderr}\n${result.error ?? ''}`);
    return decode(text);
  }
  const cases = [];
  const table = native(['add-table', '-d', 'records/values', '-a', 'values']);
  const configured = variant === 'absolute' ? table.directory.replaceAll('\\', '/') + '/./' : 'records/./values/';
  const configText = `${configured}\t as\t values\r\n`;
  writeFileSync(table.configPath, configText);
  const schemaText = '---\ncount: 0\npayload: null\nidentifier: ""\noccurred: ?datetime\ndue: ?date\nmode: [9007199254740993, 9007199254740992.0]\nmixed: ["none", 1000000000000000100, 1000000000000000128.0, 0.12345678901234567]\n---\n';
  writeFileSync(table.schemaPath, schemaText);
  const body = '# Body café Ω 日本語 😀\r\n\r\nKeep decomposed e\u0301 and composed é.\r\n';
  writeFileSync(table.templatePath, body);
  const made = native(['create', '-c', 'values', '-i', '[{"name":"exact","fields":{"count":9007199254740993,"payload":[18446744073709551615,-9223372036854775808],"identifier":"18446744073709551616","occurred":"2028-02-29T23:59:59.123Z","due":"2028-02-29"}}]']);
  const path = made.records[0].path;
  assert.equal(made.records[0].fields.count, 9007199254740993n);
  assert.deepEqual(made.records[0].fields.payload, [18446744073709551615n, -9223372036854775808n]);
  assert.equal(readFileSync(table.configPath, 'utf8'), configText);
  assert.ok(readFileSync(path, 'utf8').endsWith(body));
  let row = native(['query', '-s', 'SELECT count, typeof(count) storage, payload, identifier, mode FROM "values"']).rows[0];
  assert.equal(row.count, 9007199254740993n); assert.equal(row.mode, 9007199254740993n); assert.equal(row.storage, 'integer');
  assert.deepEqual(decode(row.payload), [18446744073709551615n, -9223372036854775808n]);
  assert.equal(row.identifier, '18446744073709551616'); cases.push('exact i64 and JSON-backed u64 across create source projection and result');
  for (const integer of ['-9223372036854775808', '9223372036854775807', '9007199254740992', '9007199254740993']) {
    native(['update', '-c', 'values', '-i', `[{"name":"exact","fields":{"count":${integer}}}]`]);
    assert.equal(native(['query', '-s', 'SELECT count FROM "values"']).rows[0].count, BigInt(integer));
    assert.equal(native(['query', '-s', 'SELECT :number value, typeof(:number) storage', '-P', `{"number":${integer}}`]).rows[0].value, BigInt(integer));
  }
  cases.push('signed limits and SQL parameters bypass JavaScript safe-integer restriction');
  native(['update', '-c', 'values', '-i', '[{"name":"exact","fields":{"mode":9007199254740992.0}}]']);
  assert.equal(native(['query', '-s', 'SELECT mode FROM "values"']).rows[0].mode, 9007199254740992n);
  const enumBefore = hashes(project);
  native(['update', '-c', 'values', '-i', '[{"name":"exact","fields":{"mode":9007199254740994}}]'], 1);
  assert.deepEqual(hashes(project), enumBefore); cases.push('mixed integer float enum equality stays exact');
  for (const [literal, rounded] of [['1000000000000000100', '1000000000000000128'], ['1000000000000000128.0', '1.0e+18'], ['0.12345678901234567', '0.123456789012346']]) {
    native(['update', '-c', 'values', '-i', `[{"name":"exact","fields":{"mixed":${literal}}}]`]);
    assert.equal(native(['sync']).state, 'reused');
    const selected = native(['query', '-s', 'SELECT mixed FROM "values" WHERE name=\'exact\'']).rows[0].mixed;
    assert.equal(decode(selected), literal.includes('.') ? Number(literal) : BigInt(literal));
    const legacy = new DatabaseSync(join(project, '.attendant/attendant.sqlite'));
    legacy.prepare('UPDATE "values" SET mixed=? WHERE name=\'exact\'').run(rounded); legacy.close();
    assert.equal(native(['sync']).state, 'rebuilt');
    assert.equal(native(['query', '-s', 'SELECT mixed FROM "values" WHERE name=\'exact\'']).rows[0].mixed, selected);
    assert.equal(native(['sync']).state, 'reused');
  }
  for (const literal of ['1e20', '-1e20', '1e19', '1.8446744073709552e19', '5e-324', '0.1', '1e308', '1000000000000000128.0', '-9223372036854775808.0']) {
    native(['update', '-c', 'values', '-i', `[{"name":"exact","fields":{"count":${literal}}}]`]);
    assert.equal(native(['query', '-s', `SELECT count=${literal} same FROM "values"`]).rows[0].same, 1);
    assert.equal(native(['query', '-s', `SELECT ${literal} value`]).rows[0].value, Number(literal));
  }
  native(['create', '-c', 'values', '-i', '[{"name":"floating","fields":{"count":1e20,"payload":[1e20,-1e20]}}]']);
  assert.equal(native(['query', '-s', 'SELECT typeof(count) storage FROM "values" WHERE name=\'floating\'']).rows[0].storage, 'real');
  for (const literal of ['1000000000000000128.0', '-9223372036854775808.0']) {
    const name = literal.startsWith('-') ? 'float-minimum' : 'float-rounding';
    const floating = native(['create', '-c', 'values', '-i', `[{"name":"${name}","fields":{"count":${literal},"payload":[${literal}]}}]`]);
    assert.equal(floating.records[0].fields.payload[0], Number(literal));
    const row = native(['query', '-s', 'SELECT count,payload FROM "values" WHERE name=:name', '-P', JSON.stringify({ name })]).rows[0];
    assert.equal(BigInt(row.count), BigInt(Number(literal)));
    assert.equal(decode(row.payload)[0], Number(literal));
    native(['update', '-c', 'values', '-i', `[{"name":"${name}","fields":{"payload":${row.payload}}}]`]);
    assert.equal(native(['query', '-s', `SELECT ${literal} value`]).rows[0].value, Number(literal));
  }
  cases.push('finite f64 and integer-looking large float YAML round-trip');
  for (const literal of ['9223372036854775808', '18446744073709551615', '18446744073709551616', '-9223372036854775809', '1e999', '1e-999']) {
    const before = hashes(project);
    native(['update', '-c', 'values', '-i', `[{"name":"exact","fields":{"count":${literal}}}]`], 1);
    native(['create', '-c', 'values', '-i', `[{"name":"rejected","fields":{"count":${literal}}}]`], 1);
    native(['query', '-s', 'SELECT :number', '-P', `{"number":${literal}}`], 1);
    assert.deepEqual(hashes(project), before);
  }
  cases.push('overflow unsigned scalar and float range failures preserve tree');
  for (const date of ['1601-01-01T00:00:00.000Z', '9999-12-31T23:59:59.999Z']) {
    native(['update', '-c', 'values', '-i', JSON.stringify([{ name: 'exact', fields: { occurred: date } }])]);
    assert.equal(native(['query', '-s', 'SELECT occurred FROM "values" WHERE name=\'exact\'']).rows[0].occurred, date);
  }
  for (const date of ['2027-02-29', '2028-2-29', '2028-02-29T00:00:00.000Z']) {
    const before = hashes(project);
    native(['update', '-c', 'values', '-i', JSON.stringify([{ name: 'exact', fields: { due: date } }])], 1);
    assert.deepEqual(hashes(project), before);
  }
  const invalidDates = ['2028-02-29T00:00:00Z', '2028-02-29T00:00:00.1Z', '2028-02-29T00:00:00.1234Z', '2028-02-29T00:00:00.000z', '2028-02-29T00:00:00.000+00:00', '2028-02-29T00:00Z', '2028-02-29t00:00:00.000Z', '2027-02-29T00:00:00.000Z', '2028-02-29T24:00:00.000Z', '2028-02-29T00:00:60.000Z'];
  for (const date of invalidDates) {
    const before = hashes(project);
    native(['update', '-c', 'values', '-i', JSON.stringify([{ name: 'exact', fields: { occurred: date } }])], 1);
    assert.deepEqual(hashes(project), before);
  }
  native(['update', '-c', 'values', '-i', '[{"name":"exact","fields":{"occurred":"2028-02-29T00:00:00.000Z"}}]']);
  cases.push('canonical UTC timestamp only with calendar checks');
  const unicode = 'café Ω 日本語 😀 e\u0301 é a\u0085b\u2028c\u2029d';
  const metadata = made.records[0].fields;
  const authored = `---\r\n# Comment ${unicode}\r\nid: "${metadata.id}"\r\nname: exact\r\ncreated_at: "${metadata.created_at}"\r\ndesc: "${unicode}" # quoted UTF-8 stays\r\ntags: []\r\ncount: 9223372036854775807\r\npayload: [18446744073709551615]\r\nmode: 9007199254740993\r\nmixed: none\r\nidentifier: "unchanged"\r\n---\r\n${body}`;
  writeFileSync(path, authored);
  native(['validate', '--strict']);
  native(['update', '-c', 'values', '-i', '[{"name":"exact","fields":{"identifier":"changed"}}]']);
  assert.equal(readFileSync(path, 'utf8'), authored.replace('identifier: "unchanged"', 'identifier: "changed"'));
  assert.equal(native(['query', '-s', 'SELECT desc FROM "values" WHERE name=\'exact\'']).rows[0].desc, unicode);
  assert.ok(native(['search', '-q', 'café', '-c', 'values']).rows.some(row => row.name === 'exact'));
  for (const scalar of [unicode, `'${unicode}'`, `|-\r\n  ${unicode}`]) {
    writeFileSync(path, authored.replace(`desc: "${unicode}" # quoted UTF-8 stays`, `desc: ${scalar}`));
    native(['update', '-c', 'values', '-i', '[{"name":"exact","fields":{"identifier":"changed"}}]']);
    assert.equal(native(['query', '-s', 'SELECT desc FROM "values" WHERE name=\'exact\'']).rows[0].desc, unicode);
    assert.equal(readFileSync(path, 'utf8'), authored.replace(`desc: "${unicode}" # quoted UTF-8 stays`, `desc: ${scalar}`).replace('identifier: "unchanged"', 'identifier: "changed"'));
  }
  writeFileSync(path, authored.replace('identifier: "unchanged"', 'identifier: "changed"'));
  native(['sync']);
  cases.push('literal Unicode line breaks remain content and span edits preserve source');
  // Same bytes/hash/mtime with old lossy decoded cache must not be reused.
  const db = new DatabaseSync(join(project, '.attendant/attendant.sqlite'));
  db.prepare('UPDATE __attendant_cache SET front_matter=? WHERE source_path=?').run('{"count":9223372036854776000}', path); db.close();
  assert.equal(native(['sync']).state, 'rebuilt');
  assert.equal(native(['query', '-s', 'SELECT count FROM "values" WHERE name=\'exact\'']).rows[0].count, 9223372036854775807n);
  native(['update', '-c', 'values', '-i', '[{"name":"exact","fields":{"count":1000000000000000100}}]']);
  const collision = new DatabaseSync(join(project, '.attendant/attendant.sqlite'));
  collision.exec('UPDATE "values" SET count=1000000000000000128 WHERE name=\'exact\''); collision.close();
  // All cache JSON/hash/stat remain correct; old f64 digits collide with exact i64.
  assert.equal(native(['sync']).state, 'rebuilt');
  assert.equal(native(['query', '-s', 'SELECT count FROM "values" WHERE name=\'exact\'']).rows[0].count, 1000000000000000100n);
  native(['update', '-c', 'values', '-i', '[{"name":"exact","fields":{"count":9223372036854775807}}]']);
  cases.push('old lossy cache interpretation triggers rebuild without source change');
  const validSource = readFileSync(path, 'utf8');
  const missingCount = validSource.replace('count: 9223372036854775807\r\n', '');
  assert.ok(!missingCount.includes('count: '));
  writeFileSync(table.schemaPath, schemaText.replace('count: 0', 'count: 1000000000000000100'));
  writeFileSync(path, missingCount); native(['sync']);
  const defaultCollision = new DatabaseSync(join(project, '.attendant/attendant.sqlite'));
  defaultCollision.exec('UPDATE "values" SET count=1000000000000000128 WHERE name=\'exact\''); defaultCollision.close();
  assert.equal(native(['sync']).state, 'rebuilt');
  const defaultCheck = new DatabaseSync(join(project, '.attendant/attendant.sqlite'));
  const defaultStatement = defaultCheck.prepare('SELECT count FROM "values" WHERE name=\'exact\''); defaultStatement.setReadBigInts(true);
  assert.equal(defaultStatement.get().count, 1000000000000000100n); defaultCheck.close();
  assert.equal(readFileSync(path, 'utf8'), missingCount);
  writeFileSync(table.schemaPath, schemaText); writeFileSync(path, validSource);
  for (const date of invalidDates.slice(0, 5)) {
    const wrong = validSource.replace(metadata.created_at, date); writeFileSync(path, wrong);
    assert.ok(native(['validate', '--strict'], 1).diagnostics.some(item => item.code === 'record-created-at'));
    native(['validate']); assert.equal(readFileSync(path, 'utf8'), wrong);
  }
  writeFileSync(path, validSource);
  for (const literal of ['9223372036854775808', '18446744073709551616', '-9223372036854775809']) {
    const wrong = validSource.replace('count: 9223372036854775807', 'count: ' + literal); writeFileSync(path, wrong);
    const checked = native(['validate', '--strict'], 1);
    assert.ok(checked.diagnostics.some(item => item.code === 'record-number-range' || item.code === 'record-yaml'));
    native(['sync']); assert.equal(readFileSync(path, 'utf8'), wrong);
    assert.equal(native(['query', '-s', 'SELECT count(*) n FROM "values" WHERE name=\'exact\'']).rows[0].n, 0);
  }
  writeFileSync(path, validSource);
  for (const declaration of ['count: 18446744073709551615', 'count: [0, 18446744073709551615]']) {
    writeFileSync(table.schemaPath, '---\n' + declaration + '\n---\n');
    assert.ok(native(['validate', '--strict'], 1).diagnostics.some(item => item.code === 'schema-number-range'));
  }
  writeFileSync(table.schemaPath, schemaText);
  cases.push('invalid authored dates and numbers diagnosed never guessed or repaired');
  const bytes = Buffer.from(validSource); bytes[bytes.indexOf('Comment')] = 0xff; writeFileSync(path, bytes);
  const invalidBytes = hashes(project); native(['sync'], 1); assert.deepEqual(hashes(project), invalidBytes); writeFileSync(path, validSource);
  native(['sync']); const final = hashes(project); native(['validate', '--strict']); native(['doctor']);
  assert.deepEqual(hashes(project), final); assert.equal(readFileSync(table.configPath, 'utf8'), configText);
  cases.push('invalid UTF-8 rejected and configured path spelling unchanged');
  const result = { executable, variant, cases, invalidDatetimeForms: invalidDates.length };
  report.push(result); console.log(JSON.stringify(result));
}
writeFileSync(join(root, 'build/review/source-results.json'), JSON.stringify(report, null, 2) + '\n');
