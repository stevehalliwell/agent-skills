// Retained subprocess integration: source API feedback consumed/resolved by CLI.
// Direct source/fault/datetime assertions moved to test.sh and tests/core/c/.
// Usage: node tests/core/api-check.mjs (build API probes and both CLIs first).
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { mkdirSync, mkdtempSync, readFileSync, writeFileSync, readdirSync, renameSync } from 'node:fs';
import { dirname, join, resolve, basename } from 'node:path';
import { fileURLToPath } from 'node:url';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
const hash = text => createHash('sha256').update(text).digest('hex');
function inventory(folder) {
  return Object.fromEntries(readdirSync(folder, { withFileTypes: true }).filter(entry => entry.name !== '.attendant').flatMap(entry => {
    const path = join(folder, entry.name);
    return entry.isDirectory() ? Object.entries(inventory(path)) : [[path, hash(readFileSync(path))]];
  }));
}
function child(command, args) {
  const result = spawnSync(command, args, { encoding: 'utf8', timeout: 30000, env: { ...process.env, ATTENDANT_FEEDBACK_FAULT: '' } });
  assert.equal(result.status, 0, `${command}: ${result.error || result.stderr}\n${result.stdout}`);
  return JSON.parse(result.stdout);
}
mkdirSync(join(root, 'build/api-check'), { recursive: true });
const report = { ok: false, cases: [], run: mkdtempSync(join(root, 'build/api-check/run-')) };
try {
  for (const compiler of ['cjit', 'zig']) for (const spelling of ['relative', 'absolute']) {
    const suffix = process.platform === 'win32' ? '.exe' : '';
    const probe = join(root, `build/api-probe-${compiler}${suffix}`), cli = join(root, `build/attendant-${compiler}${suffix}`);
    const project = join(report.run, `${compiler}-${spelling} caf\u00e9 \u03a9`);
    mkdirSync(project);
    const target = join(project, 'document.md'), saved = '---\r\nname: file\r\n---\r\nHello caf\u00e9 \ud83d\ude00\r\nNext line.\r\nSaved before comment.\n';
    writeFileSync(target, saved);
    const selected = 'caf\u00e9 \ud83d\ude00\r\nNext', start = [...saved.slice(0, saved.indexOf('caf\u00e9'))].length, end = start + [...selected].length;
    const comment = 'Please check **this passage**.\r\n\r\nKeep the rest unchanged.\n';
    const send = () => child(probe, ['send', project, target, hash(saved), String(start), String(end), selected, comment, 'probe']);
    const first = send(), second = send();
    const alternate = join(project, 'feedback elsewhere');
    renameSync(join(project, 'records/_meta_comments'), alternate);
    const configText = '# Human config comment\r\n' + (spelling === 'absolute' ? alternate.replaceAll('\\', '/') : 'feedback elsewhere') + ' as _meta_comments\r\n';
    writeFileSync(join(project, '.pi/attendant.tables'), configText); send();
    const recordCase = name => report.cases.push({ compiler, spelling, name });
    const query = () => child(cli, ['query', '-p', project, '-s', 'SELECT name,status,target_path,range_start,range_end,selected_text FROM _meta_comments ORDER BY name']);
    const before = inventory(project), rows = query();
    assert.equal(rows.returned, 3); assert.equal(rows.meta_comments.pending, 3);
    for (const row of rows.rows) {
      assert.equal(row.selected_text, selected); assert.equal(row.range_start, start); assert.equal(row.range_end, end);
      assert.equal(resolve(row.target_path), resolve(target));
    }
    assert.deepEqual(query().rows, rows.rows); assert.deepEqual(inventory(project), before);
    recordCase('CLI reads API-created records/ranges without consuming or changing source');
    for (const row of rows.rows.slice(0, 2))
      child(cli, ['update', '-p', project, '-c', '_meta_comments', '-i', JSON.stringify([{ name: row.name, fields: { status: 'resolved' } }])]);
    assert.equal(query().meta_comments.pending, 1);
    for (const name of [basename(first.path), basename(second.path)]) assert(readFileSync(join(alternate, name), 'utf8').endsWith('---\n' + comment));
    recordCase('explicit CLI resolution preserves human body and decreases pending count');
    const contextual = join(report.run, `${compiler}-${spelling}-context`); mkdirSync(contextual);
    const contextTarget = join(contextual, 'preview.md'), contextBody = 'Context note: caf\u00e9 \ud83d\ude00.\r\n\nRendered context lives in note body.\n';
    writeFileSync(contextTarget, saved);
    const contextResult = child(probe, ['send-context', contextual, contextTarget, hash(saved), '0', '0', '', contextBody, 'probe']);
    const contextQuery = () => child(cli, ['query', '-p', contextual, '-s', 'SELECT name,status,range_start,range_end,selected_text FROM _meta_comments']);
    const contextRows = contextQuery(); assert.equal(contextRows.meta_comments.pending, 1);
    assert.deepEqual([contextRows.rows[0].range_start, contextRows.rows[0].range_end, contextRows.rows[0].selected_text], [0, 0, '']);
    assert.deepEqual(contextQuery().rows, contextRows.rows);
    recordCase('CLI reads API-created absent anchor without consuming');
    child(cli, ['update', '-p', contextual, '-c', '_meta_comments', '-i', JSON.stringify([{ name: contextRows.rows[0].name, fields: { status: 'resolved' } }])]);
    assert.equal(contextQuery().rows[0].status, 'resolved'); assert.equal(contextQuery().meta_comments.pending, 0);
    assert(readFileSync(contextResult.path, 'utf8').endsWith('---\n' + contextBody));
    recordCase('explicit CLI context resolution preserves body and absent anchor');
  }
  report.ok = true;
} finally { writeFileSync(join(root, 'build/api-check-results.json'), JSON.stringify(report, null, 2) + '\n'); }
console.log(JSON.stringify({ ok: report.ok, cases: report.cases.length, run: report.run }));
