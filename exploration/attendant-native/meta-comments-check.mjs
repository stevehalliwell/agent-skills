// Pending-feedback discovery acceptance, generated fixtures only.
// Usage: node meta-comments-check.mjs <native-executable> [...]
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { cpSync, mkdirSync, readFileSync, readdirSync, rmSync, unlinkSync, writeFileSync } from 'node:fs';
import { basename, dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { DatabaseSync } from 'node:sqlite';
const root = dirname(fileURLToPath(import.meta.url));
const executables = process.argv.slice(2).map(path => resolve(path));
if (process.argv.includes('--help') || !executables.length) { console.log('Usage: node meta-comments-check.mjs <native-executable> [...]'); process.exit(process.argv.includes('--help') ? 0 : 2); }
const runner = resolve(root, '../../skills/attendant/scripts/attendant.mjs');
function hashes(directory) {
  return Object.fromEntries(readdirSync(directory, { withFileTypes: true }).filter(e => e.name !== '.attendant').flatMap(e => {
    const path = join(directory, e.name);
    return e.isDirectory() ? Object.entries(hashes(path)) : [[path, createHash('sha256').update(readFileSync(path)).digest('hex')]];
  }));
}
const schema = '---\nstatus: [pending, resolved]\ntarget_path: ""\ntarget_hash: ""\nrange_start: 0\nrange_end: 0\nrange_unit: [unicode-codepoint]\nselected_text: ""\n---\n';
const results = [];
for (const executable of executables) for (const variant of ['relative', 'absolute']) {
  const project = join(root, 'build/review', `${basename(executable, '.exe')}-${variant} meta comments café Ω`);
  rmSync(project, { recursive: true, force: true }); cpSync(join(root, 'fixtures/project'), project, { recursive: true });
  const config = join(project, '.pi/attendant.tables'), comments = join(project, 'records/_meta_comments');
  if (variant === 'absolute') writeFileSync(config, `${join(project, 'records/decisions').replaceAll('\\', '/')}/ as decisions\n${join(project, 'records/tasks').replaceAll('\\', '/')}/ as tasks\n`);
  const cases = [];
  const invoke = (args, expected = 0, command = executable, prefix = []) => {
    const child = spawnSync(command, [...prefix, ...args, '-p', project], { encoding: 'utf8', timeout: 20000 });
    assert.equal(child.status, expected, child.stderr + child.stdout);
    return { data: JSON.parse(child.stdout), stderr: child.stderr.replaceAll('\r\n', '\n') };
  };
  const query = (sql = 'SELECT name FROM tasks ORDER BY name') => invoke(['query', '-s', sql]);
  let before = hashes(project), actual = query();
  assert.equal(actual.data.meta_comments, undefined); assert.equal(actual.stderr, ''); assert.deepEqual(hashes(project), before);
  assert.equal(invoke(['search', '-q', 'lantern']).data.meta_comments, undefined); assert.deepEqual(hashes(project), before);
  cases.push('absent inbox preserves output and source without creating collection');
  mkdirSync(comments); writeFileSync(join(comments, '.schema.md'), '---\nstatus: [todo, done]\n---\n');
  writeFileSync(join(comments, '.usage.md'), 'Generated acceptance comments only.\n');
  writeFileSync(join(comments, '.template.md'), 'Feedbackmarkertoken: I do not think we need this part.\n');
  writeFileSync(config, readFileSync(config, 'utf8') + `${variant === 'absolute' ? comments.replaceAll('\\', '/') : 'records/_meta_comments'} as _meta_comments\n`);
  before = hashes(project); actual = query();
  assert.deepEqual(actual.data.meta_comments, { pending: null, error: 'incompatible-schema' });
  assert.match(actual.stderr, /reminder unavailable/); assert.deepEqual(hashes(project), before);
  cases.push('incompatible lifecycle reports unavailable rather than zero or overwriting schema');
  writeFileSync(join(comments, '.schema.md'), schema); before = hashes(project); actual = query();
  assert.deepEqual(actual.data.meta_comments, { pending: 0 }); assert.equal(actual.stderr, ''); assert.deepEqual(hashes(project), before);
  const db = new DatabaseSync(join(project, '.attendant/attendant.sqlite'), { readOnly: true });
  try { assert.equal(db.prepare("SELECT value FROM __attendant_metadata WHERE key='fts_enabled'").get().value, '1'); } finally { db.close(); }
  // Existing indexed mode was retained across contract rebuild. Separate cold test below.
  cases.push('compatible empty inbox adds zero count without notice');
  invoke(['create', '-c', '_meta_comments', '-i', JSON.stringify([{ name: 'comment-one' }, { name: 'comment-two' }])]);
  before = hashes(project); actual = query();
  assert.deepEqual(actual.data.meta_comments, { pending: 2 }); assert.equal(actual.stderr, '2 _meta_comments are waiting for you.\n');
  assert.deepEqual(actual.data.columns, ['name']); assert.ok(actual.data.returned > 0); assert.deepEqual(hashes(project), before);
  cases.push('ordinary query preserves result fields and exposes plural pending reminder');
  actual = invoke(['search', '-q', 'Feedbackmarkertoken', '-c', '_meta_comments']);
  assert.equal(actual.data.returned, 2); assert.deepEqual(actual.data.meta_comments, { pending: 2 }); assert.match(actual.stderr, /^2 _meta_comments/);
  const inbox = query("SELECT name,status FROM _meta_comments WHERE status='pending' ORDER BY name");
  assert.equal(inbox.data.returned, 2); assert.equal(inbox.data.meta_comments.pending, 2); assert.deepEqual(hashes(project), before);
  cases.push('search and inbox read report same pending count without consuming comments');
  const one = join(comments, 'comment-one.md'), originalBody = readFileSync(one, 'utf8').split(/\r?\n---\r?\n/).at(-1);
  const resolveComment = name => invoke(['update', '-c', '_meta_comments', '-i', JSON.stringify([{ name, fields: { status: 'resolved' } }])]);
  assert.equal(resolveComment('comment-one').data.meta_comments, undefined);
  actual = query(); assert.deepEqual(actual.data.meta_comments, { pending: 1 }); assert.equal(actual.stderr, '1 _meta_comments is waiting for you.\n');
  assert.equal(readFileSync(one, 'utf8').split(/\r?\n---\r?\n/).at(-1), originalBody);
  resolveComment('comment-two'); actual = query(); assert.equal(actual.data.meta_comments.pending, 0); assert.equal(actual.stderr, '');
  cases.push('explicit resolution decreases count, clears notice at zero and preserves body');
  const resolved = readFileSync(one, 'utf8'), pending = resolved.replace('"status": "resolved"', '"status": "pending"'); assert.notEqual(pending, resolved);
  writeFileSync(one, pending); assert.equal(query().data.meta_comments.pending, 1); resolveComment('comment-one');
  cases.push('direct Markdown status edits refresh pending count on next query');
  invoke(['create', '-c', '_meta_comments', '-i', '[{"name":"hidden-comment"}]']);
  const hidden = join(comments, 'hidden-comment.md');
  writeFileSync(hidden, readFileSync(hidden, 'utf8').replace(/\r?\n---\r?\n/, '\nhidden: true\n---\n'));
  actual = query(); assert.equal(actual.data.meta_comments.pending, 0); assert.equal(actual.stderr, '');
  const malformed = join(comments, 'malformed.md'); writeFileSync(malformed, '---\nstatus: [\n---\nNever repair.\n'); before = hashes(project);
  actual = query(); assert.equal(actual.data.meta_comments.pending, 0); assert.ok(actual.data.diagnostics.length); assert.deepEqual(hashes(project), before);
  unlinkSync(malformed); unlinkSync(hidden); query();
  cases.push('counts match valid visible inbox rows and malformed feedback remains diagnosed unchanged');
  for (const args of [['schema'], ['sync'], ['doctor'], ['validate', '--strict'], ['--help']]) {
    actual = invoke(args); assert.equal(actual.data.meta_comments, undefined); assert.equal(actual.stderr, '');
  }
  cases.push('schema sync doctor validate and help do not acquire reminder side effects');
  before = hashes(project);
  assert.equal(invoke(['sync'], 0, process.execPath, [runner]).data.state, 'reused'); assert.equal(query().data.meta_comments.pending, 0);
  assert.deepEqual(hashes(project), before); cases.push('legacy indexed projection remains interoperable');
  rmSync(join(project, '.attendant'), { recursive: true, force: true }); actual = query(); assert.equal(actual.data.meta_comments.pending, 0);
  const cold = new DatabaseSync(join(project, '.attendant/attendant.sqlite'), { readOnly: true });
  try { assert.equal(cold.prepare("SELECT value FROM __attendant_metadata WHERE key='fts_enabled'").get().value, '0'); } finally { cold.close(); }
  assert.deepEqual(hashes(project), before); cases.push('cold query reminder does not build FTS or change Markdown');
  results.push({ executable, variant, cases }); console.log(JSON.stringify(results.at(-1)));
}
writeFileSync(join(root, 'build/review/meta-comments-results.json'), JSON.stringify(results, null, 2) + '\n');
