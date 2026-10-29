// Headless feedback/source API acceptance. Generated owned workspaces only.
// Usage: node api-check.mjs; first build-api-check.sh and build.sh for both compilers.
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { mkdirSync, mkdtempSync, readFileSync, writeFileSync, readdirSync, existsSync, renameSync, chmodSync, rmSync } from 'node:fs';
import { dirname, join, resolve, basename } from 'node:path';
import { fileURLToPath } from 'node:url';
const root = dirname(fileURLToPath(import.meta.url));
const legacy = resolve(root, '../../skills/attendant/scripts/attendant.mjs');
const hash = text => createHash('sha256').update(text).digest('hex');
function inventory(folder) {
  return Object.fromEntries(readdirSync(folder, { withFileTypes: true }).filter(entry => entry.name !== '.attendant').flatMap(entry => {
    const path = join(folder, entry.name);
    return entry.isDirectory() ? Object.entries(inventory(path)) : [[path, hash(readFileSync(path))]];
  }));
}
function child(command, args, status = 0, environment = {}) {
  const result = spawnSync(command, args, { encoding: 'utf8', timeout: 30000, env: { ...process.env, ...environment } });
  assert.equal(result.status, status, `${command}: ${result.error || result.stderr}\n${result.stdout}`);
  return JSON.parse(result.stdout);
}
mkdirSync(join(root, 'build/api-check'), { recursive: true });
const report = { ok: false, cases: [], run: mkdtempSync(join(root, 'build/api-check/run-')) };
try {
  for (const compiler of ['cjit', 'zig']) for (const spelling of ['relative', 'absolute']) {
    const probe = join(root, `build/api-probe-${compiler}.exe`), cli = join(root, `build/projection-${compiler}.exe`);
    const project = join(report.run, `${compiler}-${spelling} caf\u00e9 \u03a9`);
    mkdirSync(project);
    const target = join(project, 'document.md'), original = '---\r\nname: file\r\n---\r\nHello caf\u00e9 \ud83d\ude00\r\nNext line.\r\n';
    writeFileSync(target, original);
    const recordCase = name => report.cases.push({ compiler, spelling, name });
    let snapshot = inventory(project), read = child(probe, ['read', target]);
    assert.equal(read.text, original); assert.equal(read.hash, hash(original)); assert.deepEqual(inventory(project), snapshot);
    assert(!existsSync(join(project, '.pi'))); recordCase('read does not bootstrap workspace');
    let failure = child(probe, ['save', target, '0'.repeat(64), 'replacement'], 1);
    assert.equal(failure.code, 'source-changed'); assert.equal(failure.recovered, true); assert.deepEqual(inventory(project), snapshot);
    recordCase('stale save recovers in same process without writes');
    const saved = original + 'Saved before comment.\n';
    let result = child(probe, ['save', target, hash(original), saved]);
    assert.equal(readFileSync(target, 'utf8'), saved); assert.equal(result.hash, hash(saved)); assert.equal(result.written.length, 1);
    recordCase('atomic save preserves intended UTF-8 and CRLF');
    const selected = 'caf\u00e9 \ud83d\ude00\r\nNext', start = [...saved.slice(0, saved.indexOf('caf\u00e9'))].length, end = start + [...selected].length;
    const comment = 'Please check **this passage**.\r\n\r\nKeep the rest unchanged.\n';
    const send = (options = {}) => child(probe, ['send', project, target, options.hash ?? hash(saved), String(options.start ?? start), String(options.end ?? end),
      options.selected ?? selected, options.comment ?? comment, 'probe'], options.status ?? 0, { ATTENDANT_FEEDBACK_FAULT: options.fault || '' });
    snapshot = inventory(project);
    for (const options of [{ hash: hash(original) }, { selected: 'wrong' }, { end: end + 1 }, { end: 18446744073709551615n }, { start: 0, end: 0 }, { comment: ' \r\n' }]) {
      failure = send({ ...options, status: 1 }); assert.equal(failure.recovered, true); assert.deepEqual(inventory(project), snapshot);
    }
    recordCase('hash, code-point text, bounds and empty comment preflight leave no files');
    for (const fault of ['staged', 'seeds-published', 'comment-published']) {
      failure = send({ fault, status: 1 }); assert.equal(failure.recovered, true); assert.deepEqual(failure.written, []);
      assert.deepEqual(inventory(project), snapshot); assert(!existsSync(join(project, '.pi'))); assert(!existsSync(join(project, 'records')));
    }
    recordCase('staging and partial-bootstrap publication failures roll back owned files/directories');
    failure = send({ fault: 'target-change', status: 1 });
    assert.equal(failure.code, 'feedback-stale'); assert.equal(failure.recovered, true);
    assert.equal(readFileSync(target, 'utf8'), saved + '\n# Concurrent external edit\n');
    assert(!existsSync(join(project, 'records'))); assert(!existsSync(join(project, '.pi')));
    writeFileSync(target, saved); recordCase('target edit after staging rejects comment and preserves external bytes');
    failure = send({ fault: 'config-change', status: 1 });
    assert.equal(failure.recovered, true); assert.deepEqual(failure.written, []);
    assert.equal(readFileSync(join(project, '.pi/attendant.tables'), 'utf8'), '\n# Concurrent external edit\n');
    assert(!existsSync(join(project, 'records')));
    rmSync(join(project, '.pi'), { recursive: true }); recordCase('external config publication wins; owned bootstrap rolls back without deleting external config');
    failure = send({ fault: 'published-edit', status: 1 });
    const preservedSeed = join(project, 'records/_meta_comments/.schema.md');
    assert.deepEqual(failure.written, [preservedSeed]); assert.match(failure.message, /preserved changed or busy files/);
    assert(readFileSync(preservedSeed, 'utf8').endsWith('\n# Concurrent external edit\n'));
    snapshot = inventory(project); send({ status: 1 }); assert.deepEqual(inventory(project), snapshot);
    rmSync(join(project, 'records'), { recursive: true }); recordCase('rollback preserves changed published file, reports it, and retry conflicts without overwrite');
    result = send(); assert.equal(result.written.length, 5);
    const first = result.path, config = join(project, '.pi/attendant.tables'), comments = join(project, 'records/_meta_comments');
    assert(readFileSync(first, 'utf8').endsWith('---\n' + comment)); assert.equal(readFileSync(target, 'utf8'), saved);
    assert(!existsSync(join(project, '.attendant'))); recordCase('first Send publishes managed seeds/comment then config, without DB preparation');
    const schema = readFileSync(join(comments, '.schema.md'), 'utf8');
    snapshot = inventory(project); const second = send();
    assert.equal(second.written.length, 1); delete snapshot[second.path];
    const after = inventory(project); delete after[second.path]; assert.deepEqual(after, snapshot);
    recordCase('later Send creates only one comment and never saves target');
    writeFileSync(join(comments, '.schema.md'), schema.replace('selected_text: ""', 'selected_text: 0'));
    snapshot = inventory(project); failure = send({ status: 1 }); assert.equal(failure.code, 'feedback-conflict'); assert.deepEqual(inventory(project), snapshot);
    writeFileSync(join(comments, '.schema.md'), schema); recordCase('full managed schema conflict is recoverable and unchanged');
    // Compatible nondefault collection directory; config comments/spelling preserved.
    const alternate = join(project, 'feedback elsewhere'); renameSync(comments, alternate);
    const configText = '# Human config comment\r\n' + (spelling === 'absolute' ? alternate.replaceAll('\\', '/') : 'feedback elsewhere') + ' as _meta_comments\r\n';
    writeFileSync(config, configText);
    result = send(); assert.equal(dirname(result.path), alternate); assert.equal(readFileSync(config, 'utf8'), configText);
    assert(!existsSync(comments)); recordCase('compatible alternate relative/absolute directory reused without config rewrite');
    const query = command => child(command === 'legacy' ? process.execPath : cli,
      [...(command === 'legacy' ? [legacy] : []), 'query', '-p', project, '-s', 'SELECT name,status,target_path,range_start,range_end,selected_text FROM _meta_comments ORDER BY name']);
    snapshot = inventory(project); let rows = query('native');
    assert.equal(rows.returned, 3); assert.equal(rows.meta_comments.pending, 3);
    for (const row of rows.rows) {
      assert.equal(row.selected_text, selected); assert.equal(row.range_start, start); assert.equal(row.range_end, end);
      assert.equal(resolve(row.target_path), resolve(target));
    }
    assert.deepEqual(query('legacy').rows, rows.rows); assert.deepEqual(inventory(project), snapshot);
    recordCase('native and existing mjs read identical records/ranges without consuming or changing source');
    for (const [index, command] of [[0, 'native'], [1, 'legacy']]) {
      const name = rows.rows[index].name;
      child(command === 'legacy' ? process.execPath : cli,
        [...(command === 'legacy' ? [legacy] : []), 'update', '-p', project, '-c', '_meta_comments', '-i', JSON.stringify([{ name, fields: { status: 'resolved' } }])]);
    }
    assert.equal(query('native').meta_comments.pending, 1);
    for (const name of [basename(first), basename(second.path)]) assert(readFileSync(join(alternate, name), 'utf8').endsWith('---\n' + comment));
    recordCase('explicit native/mjs resolution preserves human body and decreases pending count');
    chmodSync(target, 0o444);
    try { failure = child(probe, ['save', target, hash(saved), 'must not replace'], 1); assert.equal(failure.recovered, true); }
    finally { chmodSync(target, 0o666); }
    assert.equal(readFileSync(target, 'utf8'), saved);
    assert(!readdirSync(project).some(name => name.endsWith('.tmp'))); recordCase('failed source publication leaves original intact and removes staging');
    // Source already saved remains saved if comment publication fails.
    const later = saved + 'Another edit.\n'; child(probe, ['save', target, hash(saved), later]);
    failure = send({ hash: hash(later), fault: 'staged', status: 1 });
    assert.equal(readFileSync(target, 'utf8'), later); assert.deepEqual(failure.written, []);
    recordCase('saved-but-not-queued is truthful; failed Send never rolls back successful save');
    // Fresh conflicting directory must never be adopted or overwritten.
    const conflict = join(report.run, `${compiler}-${spelling}-conflict`); mkdirSync(join(conflict, 'records/_meta_comments'), { recursive: true });
    writeFileSync(join(conflict, 'records/_meta_comments/keep.md'), 'Do not overwrite.');
    snapshot = inventory(conflict);
    failure = child(probe, ['send', conflict, target, hash(later), String(start), String(end), selected, comment, 'probe'], 1);
    assert.equal(failure.code, 'feedback-conflict'); assert.deepEqual(inventory(conflict), snapshot); recordCase('unregistered nonempty managed directory conflicts without overwrite');
  }
  report.ok = true;
} finally { writeFileSync(join(root, 'build/api-check-results.json'), JSON.stringify(report, null, 2) + '\n'); }
console.log(JSON.stringify({ ok: true, cases: report.cases.length, run: report.run }));
