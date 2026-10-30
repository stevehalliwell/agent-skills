// Remaining native CLI acceptance. Generated datasets only; Node is test oracle.
// Usage: node tests/core/cli-check.mjs <native-executable> [...]
import assert from 'node:assert/strict';
import { spawn, spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { DatabaseSync } from 'node:sqlite';
import { existsSync, mkdirSync, readFileSync, readdirSync, rmSync, writeFileSync } from 'node:fs';
import { basename, dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
const runner = resolve(root, '../../skills/attendant/scripts/attendant.mjs');
const executables = process.argv.slice(2).map(path => resolve(path));
if (!executables.length || process.argv.includes('--help')) {
  console.log('Usage: node tests/core/cli-check.mjs <native-executable> [...]\nRequires Node >=24.10.0; generated data stays under build/review/.');
  process.exit(executables.length ? 0 : 2);
}
function run(command, args, status = 0, options = {}) {
  const result = spawnSync(command, args, { maxBuffer: 24 * 1024 * 1024, timeout: 30_000, ...options });
  const text = new TextDecoder('utf-8', { fatal: true }).decode(result.stdout);
  assert.equal(result.status, status, `${command} ${args.join(' ')}\n${result.error ?? ''}\n${result.stderr}\n${text}`);
  return JSON.parse(text);
}
function hashes(path) {
  return Object.fromEntries(readdirSync(path, { withFileTypes: true }).flatMap(entry => {
    const target = join(path, entry.name);
    return entry.isDirectory() ? [[target, '<directory>'], ...Object.entries(hashes(target))] : [[target, createHash('sha256').update(readFileSync(target)).digest('hex')]];
  }));
}
async function withFileLock(path, action) {
  const powershell = join(process.env.SystemRoot, 'System32/WindowsPowerShell/v1.0/powershell.exe');
  const child = spawn(powershell, ['-NoProfile', '-NonInteractive', '-ExecutionPolicy', 'Bypass', '-File', join(root, 'tools/hold-file.ps1'), '-Path', path]);
  let log = '';
  child.stderr.on('data', chunk => { log += chunk; });
  const exit = new Promise((resolve, reject) => { child.once('exit', resolve); child.once('error', reject); });
  try {
    await new Promise((resolve, reject) => {
      const timer = setTimeout(() => reject(new Error(`File lock setup timed out: ${log}`)), 10_000);
      let output = '';
      child.stdout.on('data', chunk => { output += chunk; if (output.includes('LOCKED')) { clearTimeout(timer); resolve(); } });
      child.once('error', error => { clearTimeout(timer); reject(error); });
      child.once('exit', code => { clearTimeout(timer); reject(new Error(`File lock helper exited ${code}: ${log}`)); });
    });
    action();
  } finally { child.stdin.end('release\n'); assert.equal(await exit, 0, log); }
}
function setBirthTime(path, time) {
  // Fixture-only Windows metadata setter; no shell interpolation of paths.
  assert.ok(path.startsWith(join(root, 'build/review')) && Number.isFinite(Date.parse(time)));
  const result = spawnSync(join(process.env.SystemRoot, 'System32/WindowsPowerShell/v1.0/powershell.exe'), ['-NoProfile', '-NonInteractive', '-Command', '[IO.File]::SetCreationTimeUtc($env:ATTENDANT_TEST_PATH, [DateTime]::Parse($env:ATTENDANT_TEST_TIME).ToUniversalTime())'], { env: { ...process.env, ATTENDANT_TEST_PATH: path, ATTENDANT_TEST_TIME: time }, timeout: 10_000 });
  assert.equal(result.status, 0, String(result.stderr));
}
const nativeEnvironment = { ...process.env };
for (const key of Object.keys(nativeEnvironment)) if (key.toLowerCase() === 'path') delete nativeEnvironment[key];
nativeEnvironment.PATH = join(process.env.SystemRoot, 'System32');
const report = [];
for (const executable of executables) assert.ok(existsSync(executable), `Native executable not found: ${executable}`);
for (const executable of executables) for (const variant of ['relative', 'absolute']) {
  const project = join(root, 'build/review', `${basename(executable, '.exe')}-${variant} CLI café Ω`);
  rmSync(project, { recursive: true, force: true }); mkdirSync(join(project, '.pi'), { recursive: true });
  writeFileSync(join(project, '.pi/attendant.tables'), '');
  const native = (...args) => run(executable, [...args, '-p', project]);
  const oracle = (...args) => run(process.execPath, [runner, ...args, '-p', project]);
  const cases = [];
  const frozenEmpty = hashes(project);
  assert.equal(run(executable, ['doctor', '-p', project], 1, { env: nativeEnvironment }).projection, 'missing');
  assert.deepEqual(hashes(project), frozenEmpty); assert.ok(!existsSync(join(project, '.attendant'))); cases.push('doctor missing projection read-only');
  const added = run(executable, ['add-table', '-d', 'records/notes', '-a', 'notes', '-p', project], 0, { env: nativeEnvironment });
  assert.equal(added.name, 'notes');
  assert.equal(readFileSync(added.schemaPath, 'utf8'), '---\n---\n');
  assert.equal(readFileSync(added.usagePath, 'utf8'), ''); assert.equal(readFileSync(added.templatePath, 'utf8'), '');
  assert.equal(readFileSync(added.configPath, 'utf8'), 'records/notes as notes\n');
  if (variant === 'absolute') writeFileSync(added.configPath, `${added.directory.replaceAll('\\', '/')}/ as notes\n`);
  assert.equal(native('schema').collections[0].fields.length, 0); cases.push('add-table empty schema and automatic preparation');
  for (const args of [['add-table', '-d', 'records/other', '-a', 'notes'], ['add-table', '-d', 'records/notes', '-a', 'other'], ['add-table', '-d', '../outside'], ['add-table', '-d', '.'], ['add-table', '-d', 'records/NUL'], ['add-table', '-d', 'records/a as b', '-a', 'ambiguous'], ['add-table', '-d', '#notes', '-a', 'comment'], ['add-table', '-d', '.attendant/source', '-a', 'state']]) {
    const before = hashes(project); run(executable, [...args, '-p', project], 1); assert.deepEqual(hashes(project), before);
  }
  cases.push('add-table rejects collisions traversal devices root ambiguous grammar and state');
  const beforeLockedConfig = hashes(project);
  await withFileLock(added.configPath, () => {
    const failure = run(executable, ['add-table', '-p', project, '-d', 'records/locked', '-a', 'locked'], 1);
    assert.equal(failure.error.code, 'add-table-write'); assert.equal(failure.failed, added.configPath);
    assert.deepEqual(hashes(project), beforeLockedConfig);
  });
  cases.push('add-table config publication failure cleans owned seed files and directories');
  writeFileSync(added.schemaPath, '---\nstatus: [open, done]\npriority: 0\nlabels: string[]\ndue: ?date\nrelated: ?ref:notes\n---\n');
  const body = '# Template café Ω\r\n\r\nKeep **body**, [links](alpha.md), --- and final newline.\r\n';
  writeFileSync(added.templatePath, '---\nignored: {nested: template metadata}\n---\n' + body);
  const items = [{ name: 'alpha', fields: { desc: 'First café Ω', tags: ['one'] } }, { name: 'beta', fields: { status: 'done', related: 'notes/alpha' } }];
  const input = join(project, 'items.json'); writeFileSync(input, JSON.stringify(items));
  const created = run(executable, ['create', '-c', 'notes', '-i', `@${input}`, '-p', project], 0, { env: nativeEnvironment });
  assert.deepEqual(created.records.map(row => row.fields.name), ['alpha', 'beta']);
  for (const record of created.records) {
    assert.match(record.fields.id, /^[0-9a-f]{8}-[0-9a-f]{4}-7[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$/);
    assert.equal(record.fields.priority, 0); assert.deepEqual(record.fields.labels, []);
    assert.ok(readFileSync(record.path, 'utf8').endsWith(body));
  }
  native('search', '-q', 'Template'); // Indexed format is the legacy interoperability boundary.
  assert.equal(oracle('sync').state, 'reused');
  assert.deepEqual(native('query', '-s', 'SELECT name,status,priority,labels FROM notes ORDER BY name'), oracle('query', '-s', 'SELECT name,status,priority,labels FROM notes ORDER BY name'));
  cases.push('create batch defaults UUID metadata templates and runner reuse');
  const unusual = 'a\u007fb\u0080c\u0085d\u009fe\u2028f\u2029g\ufffeh\uffff';
  const special = native('create', '-c', 'notes', '-i', JSON.stringify([{ name: 'unicode-breaks', fields: { desc: unusual, labels: [unusual] } }]));
  assert.equal(special.records[0].fields.desc, unusual); assert.deepEqual(special.records[0].fields.labels, [unusual]);
  native('update', '-c', 'notes', '-i', JSON.stringify([{ name: 'unicode-breaks', fields: { desc: unusual + ' updated', tags: [unusual] } }]));
  assert.deepEqual(native('query', '-s', "SELECT desc,tags,labels FROM notes WHERE name='unicode-breaks'"), oracle('query', '-s', "SELECT desc,tags,labels FROM notes WHERE name='unicode-breaks'"));
  assert.equal(native('query', '-s', "SELECT desc FROM notes WHERE name='unicode-breaks'").rows[0].desc, unusual + ' updated');
  cases.push('create and update preserve YAML line-break Unicode code points');
  const frozen = hashes(project);
  assert.deepEqual(run(executable, ['validate', '--strict', '-p', project], 0, { env: nativeEnvironment }), oracle('validate', '--strict'));
  assert.equal(native('doctor').projection, 'fresh'); assert.deepEqual(hashes(project), frozen); cases.push('strict validation parity and fresh doctor read-only');
  const alpha = created.records[0].path, beta = created.records[1].path;
  const meta = created.records[0].fields;
  const decorated = `---\r\n# Header stays café Ω\r\nid: '${meta.id}' # immutable ID\r\nname: alpha\r\ncreated_at: ${meta.created_at}\r\ndesc: |\r\n  Original line café Ω\r\n  Second line\r\n# Tags rationale stays\r\ntags:\r\n  - old # keep list comment\r\n# Status rationale stays\r\nstatus: open # keep status comment\r\npriority: 0 # keep priority comment\r\nlabels:\r\n  - old-label\r\n# Final rationale stays\r\n---\r\n${body}`;
  writeFileSync(alpha, decorated);
  const updates = [{ name: 'alpha', fields: { status: 'done', desc: 'New café Ω\nline', tags: ['new', 'Ω'], labels: ['new-label'] } }, { name: 'beta', fields: { priority: 3 } }];
  const updated = run(executable, ['update', '-p', project, '-c', 'notes', '-i', '-'], 0, { input: JSON.stringify(updates), env: nativeEnvironment });
  assert.deepEqual(updated.records.map(row => row.fields), updates.map(row => row.fields));
  const changed = readFileSync(alpha, 'utf8');
  assert.ok(changed.startsWith(`---\r\n# Header stays café Ω\r\nid: '${meta.id}' # immutable ID\r\nname: alpha\r\ncreated_at: ${meta.created_at}\r\n`));
  assert.ok(changed.includes('status: "done" # keep status comment\r\npriority: 0 # keep priority comment\r\n'));
  assert.equal(changed, decorated.replace('|\r\n  Original line café Ω\r\n  Second line\r\n', '"New café Ω\\nline"\r\n').replace('  - old #', '  ["new","Ω"] #').replace('status: open', 'status: "done"').replace('  - old-label', '  ["new-label"]'));
  assert.ok(changed.endsWith(body)); assert.equal(oracle('sync').state, 'reused');
  assert.deepEqual(native('query', '-s', 'SELECT desc,tags FROM notes WHERE name=\'alpha\'').rows, [{ desc: 'New café Ω\nline', tags: '["new","Ω"]' }]);
  cases.push('update batch block spans Unicode CRLF comments and exact body');
  const failures = [
    ['create', '-c', 'notes', '-i', '[{"name":"new"},{"name":"alpha"}]'],
    ['create', '-c', 'notes', '-i', '[{"name":"new","fields":{"missing":1}}]'],
    ['create', '-c', 'notes', '-i', '[{"name":"new"},{"name":"NEW"}]'],
    ['create', '-c', 'notes', '-i', '[{"name":"NUL"}]'],
    ['create', '-c', 'notes', '-i', '[{"name":"../escape"}]'],
    ['create', '-c', 'notes', '-i', '[{"name":"new","fields":{"status":"invalid"}}]'],
    ['update', '-c', 'notes', '-i', '[{"name":"alpha","fields":{"status":"open"}},{"name":"beta","fields":{"status":"invalid"}}]'],
    ['update', '-c', 'notes', '-i', '[{"name":"alpha","fields":{"id":"immutable"}}]'],
    ['update', '-c', 'notes', '-i', '[{"name":"alpha","fields":{}}]'],
    ['update', '-c', 'notes', '-i', '[{"name":"missing","fields":{"status":"done"}}]'],
    ['update', '-c', 'notes', '-i', '[{"name":"alpha","fields":{"related":"notes/missing"}}]'],
    ['update', '-c', 'notes', '-i', '[{"name":"alpha","fields":{"due":"2027-02-29"}}]'],
    ['create', '-c', 'notes', '-i', '{bad}'], ['create', '-c', 'notes', '-i', '[]'],
    ['create', '-c', 'notes', '-i', '[{"name":"new","fields":{"priority":{"nested":1}}}]'],
    ['create', '-c', 'notes', '-i', '[{"name":"new"},]'],
    ['create', '-c', 'notes', '-i', JSON.stringify([{ name: 'new', fields: { ['é'.repeat(2000)]: 1 } }])],
    ['create', '-c', 'notes', '-i', '[{"name":"new","fields":{"desc":"\\ud800"}}]'],
  ];
  for (const args of failures) { const before = hashes(project); run(executable, [...args, '-p', project], 1); assert.deepEqual(hashes(project), before, args.join(' ')); }
  cases.push('write batch preflight failures preserve source and DB');
  // Dry-run detects missing metadata/defaults without writing. Default validation
  // patches only missing/invalid mechanical metadata and leaves body bytes intact.
  const repair = join(added.directory, 'repair.md');
  const repairText = '---\r\n# Author comment café Ω\r\nname: repair # retain\r\nstatus: open\r\n---\r\n' + body;
  writeFileSync(repair, repairText);
  const beforeRepair = hashes(project);
  for (const params of ['{bad}', '{"x":[1]}']) {
    run(executable, ['query', '-s', 'SELECT :x', '-P', params, '-p', project], 1);
    assert.deepEqual(hashes(project), beforeRepair, 'Bad parameter transport must not apply pending corrections');
  }
  const strict = run(executable, ['validate', '--strict', '-p', project], 1);
  assert.equal(strict.corrected, false); assert.ok(strict.corrections.some(row => row.path === repair));
  native('validate', '--no-correct'); assert.deepEqual(hashes(project), beforeRepair);
  assert.ok(native('sync').diagnostics.some(row => row.path === repair));
  const diagnosed = hashes(project), diagnosedDoctor = native('doctor');
  assert.equal(diagnosedDoctor.projection, 'fresh'); assert.ok(diagnosedDoctor.diagnostics.some(row => row.path === repair));
  assert.deepEqual(hashes(project), diagnosed);
  const repaired = native('validate'); assert.equal(repaired.corrected, true); assert.equal(repaired.diagnostics.length, 0);
  const repairedText = readFileSync(repair, 'utf8');
  assert.ok(repairedText.startsWith('---\r\n# Author comment café Ω\r\nname: repair # retain\r\nstatus: open\r\n'));
  assert.ok(repairedText.endsWith(body)); assert.equal(native('validate', '--strict').corrections.length, 0);
  assert.equal(oracle('validate', '--strict').diagnostics.length, 0); cases.push('strict no-correct and default source-preserving correction');
  const missing = join(added.directory, 'autofix.md'); writeFileSync(missing, '---\nname: autofix\n---\nAutomatic correction body\n');
  const automatic = native('query', '-s', "SELECT name FROM notes WHERE name='autofix'");
  assert.equal(automatic.returned, 1); assert.ok(automatic.corrections.some(row => row.path === missing));
  assert.equal(oracle('sync').state, 'reused'); cases.push('read preparation reports automatic corrections');
  const bad = join(added.directory, 'bad.md');
  const invalidSources = [
    '---\nname: [unfinished\n---\nBroken body\n',
    '---\nvalue: ' + '['.repeat(5000) + 'deep' + ']'.repeat(5000) + '\n---\nDeep body\n',
    '---\n---\nEmpty record body\n',
    '---\n' + '- entry\n'.repeat(4096) + '---\nRoot sequence body\n',
  ];
  for (const malformed of invalidSources) {
    writeFileSync(bad, malformed);
    assert.ok(native('validate', '--no-correct').diagnostics.some(row => row.path === bad));
    assert.ok(native('sync').diagnostics.some(row => row.path === bad));
    const query = native('query', '-s', 'SELECT name FROM notes ORDER BY name');
    assert.ok(query.diagnostics.some(row => row.path === bad)); assert.ok(!query.rows.some(row => row.name === 'bad'));
    assert.equal(readFileSync(bad, 'utf8'), malformed); assert.equal(oracle('sync').state, 'reused');
    const cannotWrite = hashes(project);
    run(executable, ['create', '-c', 'notes', '-i', '[{"name":"blocked"}]', '-p', project], 1);
    assert.deepEqual(hashes(project), cannotWrite);
  }
  cases.push('malformed unsupported deep and empty YAML reported skipped never repaired and blocks writes');
  rmSync(bad);
  const boundaryHeader = 'id: 0197f3f1-1a2b-7000-8000-000000000099\nname: boundary\ncreated_at: 2025-01-01T00:00:00.000Z\ndesc: Boundary\ntags: []\nstatus: open\npriority: 0\nlabels: []\n';
  for (const style of ['flow', 'indentless', 'empty-block', 'empty-indentless']) for (const length of [1024, 1025]) {
    const array = style === 'flow' ? '[' + Array(length).fill('null').join(',') + ']\n' : '\n' + (style === 'indentless' ? '- item\n' : style === 'empty-block' ? '  -\n' : '-\n').repeat(length);
    const text = '---\n' + boundaryHeader + 'extras: ' + array + '---\nBoundary body\n';
    writeFileSync(bad, text);
    const validation = run(executable, ['validate', '--strict', '-p', project], length === 1024 ? 0 : 1);
    if (length === 1024) assert.equal(validation.diagnostics.length, 0);
    else assert.ok(validation.diagnostics.some(row => row.path === bad && row.message.includes('capacity exceeded before document load')));
    assert.equal(readFileSync(bad, 'utf8'), text);
  }
  for (const indent of ['', '  ']) {
    const text = '---\n' + boundaryHeader.replace('tags: []\n', 'tags:\n- old\n').split('\n').filter(Boolean).map(line => indent + line + '\n').join('') + '---\nBoundary body\n';
    writeFileSync(bad, text);
    native('update', '-c', 'notes', '-i', '[{"name":"bad","fields":{"tags":["new"]}}]');
    assert.equal(readFileSync(bad, 'utf8'), text.replace(indent + '- old\n', indent + '  ["new"]\n'));
  }
  rmSync(bad); cases.push('YAML event preflight counts implicit null and indentless arrays; indentless edits round-trip');
  const duplicate = join(added.directory, 'duplicate.md');
  writeFileSync(duplicate, readFileSync(beta, 'utf8'));
  setBirthTime(beta, '2002-01-01T00:00:00Z'); setBirthTime(duplicate, '2001-01-01T00:00:00Z');
  const duplicates = native('validate'); assert.ok(duplicates.corrections.some(row => row.path === duplicate || row.path === beta));
  assert.equal(native('validate', '--strict').diagnostics.length, 0);
  assert.equal(oracle('validate', '--strict').diagnostics.length, 0);
  assert.equal(native('query', '-s', "SELECT source_path FROM notes WHERE name='beta'").rows[0].source_path, duplicate);
  const unknown = join(added.directory, 'unknown-date.md'), betaText = readFileSync(beta, 'utf8');
  writeFileSync(unknown, betaText.replace(/^"created_at":.*$/m, '"created_at": "2099-01-01T00:00:00.000Z"'));
  writeFileSync(beta, betaText.replace(/^"created_at":.*\n/m, ''));
  native('validate');
  assert.equal(native('query', '-s', "SELECT source_path FROM notes WHERE name='beta (2)'").rows[0].source_path, unknown);
  assert.equal(oracle('validate', '--strict').diagnostics.length, 0);
  cases.push('duplicate ID and name repairs prefer valid source dates then filesystem creation time');
  const clock = join(added.directory, 'clock.md');
  writeFileSync(clock, '---\nid: 0197f3f1-1a2b-7000-8000-000000000088\nname: clock\ncreated_at: 2025-01-01T00:00:00.000Z\ndesc: Clock\ntags: []\nstatus: open\npriority: 0\nlabels: []\ndue: 2028-02-29\n---\nClock body\n');
  const staleBytes = hashes(project);
  assert.equal(run(executable, ['doctor', '-p', project], 1).projection, 'stale');
  assert.deepEqual(hashes(project), staleBytes);
  assert.deepEqual(native('query', '-s', "SELECT created_at,due FROM notes WHERE name='clock'").rows, [{ created_at: '2025-01-01T00:00:00.000Z', due: '2028-02-29' }]);
  assert.equal(oracle('sync').state, 'reused'); cases.push('stale doctor read-only canonical UTC and calendar validation');
  const schemaBefore = readFileSync(added.schemaPath, 'utf8');
  writeFileSync(added.schemaPath, schemaBefore.replace('priority: 0', 'priority: 0\nid: immutable'));
  const invalidSchemaBytes = hashes(project);
  assert.ok(native('validate', '--no-correct').diagnostics.some(row => row.code === 'schema-field-reserved'));
  assert.equal(run(executable, ['doctor', '-p', project], 1).ok, false);
  assert.deepEqual(hashes(project), invalidSchemaBytes); writeFileSync(added.schemaPath, schemaBefore);
  cases.push('invalid schema diagnostics without writes');
  const beforePartial = readFileSync(beta, 'utf8');
  await withFileLock(beta, () => {
    const failure = run(executable, ['update', '-p', project, '-c', 'notes', '-i', '[{"name":"alpha","fields":{"priority":7}},{"name":"beta","fields":{"priority":8}}]'], 1);
    assert.equal(failure.error.code, 'update-write'); assert.equal(failure.failed, beta); assert.ok(failure.written.includes(alpha)); assert.ok(!failure.written.includes(beta));
    assert.equal(readFileSync(beta, 'utf8'), beforePartial);
    assert.ok(!Object.keys(hashes(project)).some(path => /[\\/]\.attendant-[^\\/]+\.tmp$/.test(path)));
  });
  assert.equal(native('query', '-s', "SELECT priority FROM notes WHERE name='alpha'").rows[0].priority, 7);
  cases.push('partial update publication reports written paths and cleans staged files');
  const database = join(project, '.attendant/attendant.sqlite'); native('sync');
  const connection = new DatabaseSync(database); connection.exec('BEGIN EXCLUSIVE');
  let refresh;
  try { refresh = run(executable, ['update', '-p', project, '-c', 'notes', '-i', '[{"name":"alpha","fields":{"priority":9}}]'], 1); }
  finally { connection.exec('ROLLBACK'); connection.close(); }
  assert.equal(refresh.error.code, 'projection-refresh'); assert.ok(refresh.written.includes(alpha));
  assert.equal(native('query', '-s', "SELECT priority FROM notes WHERE name='alpha'").rows[0].priority, 9);
  cases.push('projection refresh failure reports authoritative committed source');
  assert.ok(!Object.keys(hashes(project)).some(path => /[\\/]\.attendant-[^\\/]+\.tmp$/.test(path)));
  const capacity = join(project, 'capacity'); mkdirSync(join(capacity, '.pi'), { recursive: true });
  const entries = [];
  for (let index = 0; index < 31; index++) {
    const directory = join(capacity, 'records', `c${index}`); mkdirSync(directory, { recursive: true });
    writeFileSync(join(directory, '.schema.md'), '---\n---\n');
    entries.push(`${variant === 'absolute' ? directory.replaceAll('\\', '/') : `records/c${index}`} as c${index}\n`);
  }
  writeFileSync(join(capacity, '.pi/attendant.tables'), entries.join(''));
  run(executable, ['add-table', '-p', capacity, '-d', 'records/c31', '-a', 'c31']);
  assert.equal(run(executable, ['validate', '--strict', '-p', capacity]).collections.length, 32);
  assert.equal(run(executable, ['schema', '-p', capacity]).collections.length, 32);
  const fullCapacity = hashes(capacity);
  run(executable, ['add-table', '-p', capacity, '-d', 'records/c32', '-a', 'c32'], 1);
  assert.deepEqual(hashes(capacity), fullCapacity); cases.push('add-table final supported collection succeeds and overflow preserves tree');
  const result = { executable, variant, cases, rejectedBatches: failures.length, runnerReused: true, nativeWithoutNode: true };
  report.push(result); console.log(JSON.stringify(result));
}
mkdirSync(join(root, 'build/review'), { recursive: true });
writeFileSync(join(root, 'build/review/cli-results.json'), JSON.stringify(report, null, 2) + '\n');
