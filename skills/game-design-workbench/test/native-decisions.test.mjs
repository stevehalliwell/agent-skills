// Focused native CLI checks. Set GD_NATIVE_EXE to a fresh CJIT gd.exe.
import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtemp, writeFile, readFile, rm } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join, resolve } from 'node:path';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { fileURLToPath } from 'node:url';

const executable = process.env.GD_NATIVE_EXE && resolve(process.env.GD_NATIVE_EXE);
const enabled = { skip: !executable && 'Set GD_NATIVE_EXE for native decision-query checks' };
const deepEnabled = { skip: enabled.skip || (process.env.GD_DEEP_CHECKS !== '1' && 'Set GD_DEEP_CHECKS=1 for aggregate/text capacity probes') };
const invoke = (args, status = 0) => {
  const result = spawnSync(executable, args, { encoding: 'utf8', timeout: 20000, maxBuffer: 16 * 1024 * 1024 });
  assert.equal(result.status, status, result.stderr || result.error?.message || result.stdout);
  return result;
};
const query = (folder, args = [], status = 0) => JSON.parse(invoke(['query', '--question', 'decision-assessments', '--projection', 'challenges', '--challenges', `tasks=${folder}`, ...args, '--json'], status).stdout);
const block = text => '```decision-assessment\n' + text + '\n```\n';
const basic = 'id: sample\ntask: Choose response\ndemands:\n  - kind: knowledge\n';
const record = body => '---\nname: encounter\nid: fixture-id\nstatus: proposed\n---\n' + body;
async function fixture(t, body) {
  const folder = await mkdtemp(join(tmpdir(), 'gd-decisions-'));
  t.after(() => rm(folder, { recursive: true, force: true }));
  const path = join(folder, 'encounter.md');
  await writeFile(path, record(body), 'utf8');
  return { folder, path };
}

test('catalogue advertises exact CLI-only decision question and parameter restrictions', enabled, () => {
  const value = JSON.parse(invoke(['query', '--list', '--json']).stdout);
  const question = value.data.questions.find(item => item.name === 'decision-assessments');
  assert.equal(question.projection, 'challenges');
  assert.equal(question.ui_available, false);
  assert.equal(question.ipc_available, false);
  assert.deepEqual(question.fields, ['summary', 'assessments']);
  for (const args of [['query', '--list', '--intent', 'wanted'], ['query', '--question', 'outcomes', '--projection', 'rules', '--intent', 'wanted']]) {
    const failure = JSON.parse(invoke([...args, '--json'], 2).stdout);
    assert.equal(failure.ok, false);
  }
});

test('filter demand and intent within same entry and same contextual assessment', enabled, async t => {
  const { folder, path } = await fixture(t, block('id: first\ntask: First task\ncontext: State one\nevidence_status: authored-assessment\nrule_cases:\n  - "mechanics/example#case"\ndemands:\n  - kind: knowledge\n    intent: wanted\n  - kind: dexterity\n    intent: unwanted') + block('id: second\ntask: Second task\ndemands:\n  - kind: dexterity'));
  const before = createHash('sha256').update(await readFile(path)).digest('hex');
  assert.equal(query(folder, ['--demand', 'dexterity', '--intent', 'wanted']).data.summary.matched_assessments, 0);
  const value = query(folder, ['--demand', 'dexterity', '--intent', 'undecided']);
  assert.equal(value.data.summary.matched_assessments, 1);
  assert.equal(value.data.summary.matched_demand_entries, 1);
  assert.equal(value.data.assessments[0].record, 'tasks/encounter');
  assert.equal(value.data.assessments[0].record_id, 'fixture-id');
  assert.equal(value.data.assessments[0].assessment_id, 'second');
  assert.equal(value.data.assessments[0].context, null);
  assert.deepEqual(value.data.assessments[0].demands.map(({ kind, intent, matched }) => ({ kind, intent, matched })), [{ kind: 'dexterity', intent: 'undecided', matched: true }]);
  const first = query(folder, ['--intent', 'unwanted']).data.assessments[0];
  assert.equal(first.demands[0].matched, false);
  assert.equal(first.demands[1].matched, true);
  assert.deepEqual(first.rule_cases, ['mechanics/example#case']);
  assert.equal(first.evidence_status, 'authored-assessment');
  assert.equal(createHash('sha256').update(await readFile(path)).digest('hex'), before);
});

test('empty source/empty demands are not absence proofs and output fields preserve envelope', enabled, async t => {
  const { folder, path } = await fixture(t, 'Prose mentions dexterity, but declares no assessment.\n');
  assert.equal(query(folder).data.summary.retained_assessments, 0);
  await writeFile(path, record(block('id: empty\ntask: Unclassified task\ndemands: []')), 'utf8');
  assert.equal(query(folder).data.summary.matched_assessments, 1);
  assert.equal(query(folder, ['--intent', 'undecided']).data.summary.matched_assessments, 0);
  const selected = query(folder, ['--fields', 'summary']);
  assert.deepEqual(Object.keys(selected.data), ['summary']);
  assert.equal(selected.question, 'decision-assessments');
  assert.equal(selected.coverage.classification_inferred, false);
  assert.equal(selected.coverage.read_only, true);
  assert.deepEqual(selected.errors, []);
  const text = invoke(['query', '--question', 'decision-assessments', '--projection', 'challenges', '--challenges', `tasks=${folder}`, '--format', 'text']);
  assert.match(text.stdout, /tasks\/encounter#empty/);
  assert.equal(text.stderr, '');
  await rm(path);
  assert.equal(query(folder).data.summary.source_records, 0);
});

test('fences track nested examples, tilde markers, CRLF and source lines', enabled, async t => {
  const ignored = '````markdown\n' + block(basic) + '````\n';
  const actual = '~~~decision-assessment\nid: actual\ntask: "répondre"\ndemands:\n  - kind: reaction\n    intent: wanted\n~~~\n';
  const { folder, path } = await fixture(t, ignored + actual);
  const source = (await readFile(path, 'utf8')).replaceAll('\n', '\r\n');
  await writeFile(path, source, 'utf8');
  const value = query(folder);
  assert.equal(value.data.summary.retained_assessments, 1);
  const row = value.data.assessments[0];
  assert.equal(row.assessment_id, 'actual');
  assert.equal(row.task, 'répondre');
  assert.equal(source.split('\r\n')[row.line - 1], '~~~decision-assessment');
  assert.equal(source.split('\r\n')[row.demands[0].line - 1], '  - kind: reaction');
});

test('bad selectors fail explicitly and unrelated query families reject decision selectors', enabled, async t => {
  const { folder } = await fixture(t, block(basic));
  const wrongProjection = JSON.parse(invoke(['query', '--question', 'decision-assessments', '--projection', 'actors', '--challenges', `tasks=${folder}`, '--json'], 2).stdout);
  assert.equal(wrongProjection.errors[0].code, 'query.projection_unavailable');
  const missingBinding = JSON.parse(invoke(['query', '--question', 'decision-assessments', '--projection', 'challenges', '--json'], 2).stdout);
  assert.equal(missingBinding.ok, false);
  const legacy = JSON.parse(invoke(['query', '--question', 'adjacency', '--projection', 'challenges', '--challenges', `tasks=${folder}`, '--json'], 2).stdout);
  assert.equal(legacy.ok, false, 'Challenge-only assessment loading must not loosen legacy graph source contracts');
  for (const args of [['--intent', 'maybe'], ['--intent', 'wanted', '--intent', 'unwanted'], ['--demand', 'a/b'], ['--demand', 'knowledge', '--demand', 'reaction'], ['--status', 'accepted'], ['--work-limit', '0'], ['--fields', 'unknown'], ['--from', 'tasks/encounter'], ['--object', 'nouns/player']]) {
    const value = query(folder, args, 2);
    assert.equal(value.ok, false);
    assert.equal(value.coverage.complete, false);
    assert.equal(value.data.summary ?? null, null);
    assert.deepEqual(value.data.assessments ?? [], []);
    assert.ok(value.errors.length);
  }
});

test('malformed or ambiguous assessment fails atomically without success-shaped rows', enabled, async t => {
  const { folder, path } = await fixture(t, '');
  const invalid = [
    basic + 'extra: value',
    basic + 'id: another',
    basic.replace('id: sample', 'id: 3'),
    basic.replace('kind: knowledge', 'kind: true'),
    basic + '    intent: null',
    basic + '    intent: wanted\n    intent: unwanted',
    basic + '  - kind: knowledge\n    intent: wanted',
    basic + 'rule_cases: wrong',
    basic + 'context: 123',
    ...['0o12', '0b11', '-.INF', '.NAN'].map(value => basic.replace('task: Choose response', `task: ${value}`)),
    basic + 'rule_cases: [3]',
    basic + '---\nid: other',
    basic.replace('task: Choose response', 'task: &anchor response'),
    basic.replace('task: Choose response', 'task: *alias'),
    basic.replace('task: Choose response', 'task: !!str response'),
    basic.replace('task: Choose response', 'task: "escaped\\0"'),
    'id: missing\ntask: Missing demands',
    '[]',
  ];
  for (const bad of invalid) {
    await writeFile(path, record(block(basic.replace('id: sample', 'id: good')) + block(bad)), 'utf8');
    const value = query(folder, [], 2);
    assert.equal(value.ok, false, bad);
    assert.equal(value.data.summary, null, bad);
    assert.deepEqual(value.data.assessments, [], bad);
  }
  await writeFile(path, record(block(basic) + block(basic)), 'utf8');
  assert.equal(query(folder, [], 2).errors[0].code, 'decision.duplicate_id');
  await writeFile(path, record('```decision-assessment\n' + basic), 'utf8');
  assert.equal(query(folder, [], 2).errors[0].code, 'decision.unclosed_block');
});

test('local, entry, block, depth and event bounds fail rather than truncate', enabled, async t => {
  const { folder, path } = await fixture(t, '');
  const cases = [
    [Array.from({ length: 17 }, (_, index) => block(basic.replace('sample', `item-${index}`))).join(''), 'decision.local_limit'],
    [block('id: many\ntask: Too many\ndemands:\n' + Array.from({ length: 17 }, (_, index) => `  - kind: kind-${index}\n`).join('')), 'decision.demand_limit'],
    [block(basic + 'rule_cases:\n' + '  - mechanics/example\n'.repeat(17)), 'decision.reference_limit'],
    [block(basic + '# ' + 'x'.repeat(65536)), 'decision.block_limit'],
    [block(basic + 'extra: ' + '['.repeat(9) + 'value' + ']'.repeat(9)), 'decision.yaml_contract'],
    [block(basic + 'extra: [' + Array.from({ length: 2100 }, () => 'value').join(',') + ']'), 'decision.event_limit'],
  ];
  for (const [body, code] of cases) {
    await writeFile(path, record(body), 'utf8');
    const value = query(folder, [], 2);
    assert.equal(value.errors[0].code, code);
    assert.equal(value.coverage.complete, false);
    assert.deepEqual(value.data.assessments, []);
  }
});

test('aggregate assessment, demand and reference limits accept boundary then fail atomically', deepEnabled, async t => {
  const { folder } = await fixture(t, 'No assessments here.\n');
  const assessment = (index, demands = 8, references = 8) => block(`id: item-${index}\ntask: Choose\nrule_cases:\n${'  - "mechanics/example#case"\n'.repeat(references)}demands:\n${Array.from({ length: demands }, (_, kind) => `  - kind: kind-${kind}\n`).join('')}`);
  const body = (demands = 8, references = 8) => Array.from({ length: 16 }, (_, index) => assessment(index, index === 15 ? demands : 8, index === 15 ? references : 8)).join('');
  const last = join(folder, 'row-255.md');
  for (let index = 0; index < 256; ++index) await writeFile(join(folder, `row-${String(index).padStart(3, '0')}.md`), record(body()).replace('name: encounter', `name: row-${String(index).padStart(3, '0')}`).replace('fixture-id', `fixture-${index}`), 'utf8');
  const summary = query(folder, ['--fields', 'summary']).data.summary;
  assert.equal(summary.retained_assessments, 4096);
  assert.equal(summary.retained_demand_entries, 32768);
  const extra = join(folder, 'zz-extra.md');
  await writeFile(extra, record(block(basic)).replace('name: encounter', 'name: zz-extra'), 'utf8');
  assert.equal(query(folder, ['--fields', 'summary'], 2).errors[0].code, 'decision.assessment_limit');
  await rm(extra);
  for (const [demands, references, code] of [[9, 8, 'decision.demand_limit'], [8, 9, 'decision.reference_limit']]) {
    await writeFile(last, record(body(demands, references)).replace('name: encounter', 'name: row-255'), 'utf8');
    const failure = query(folder, ['--fields', 'summary'], 2);
    assert.equal(failure.errors[0].code, code);
    assert.equal(failure.data.summary, null);
  }
});

test('retained text exhaustion is explicit and does not emit partial rows', deepEnabled, async t => {
  const { folder } = await fixture(t, 'No assessments here.\n');
  for (let index = 0; index < 65; ++index) {
    const body = Array.from({ length: 16 }, (_, item) => block(`id: item-${item}\ntask: Choose\ncontext: ${'x'.repeat(4095)}\ndemands: []`)).join('');
    await writeFile(join(folder, `row-${index}.md`), record(body).replace('name: encounter', `name: row-${index}`), 'utf8');
  }
  const value = query(folder, ['--fields', 'summary'], 2);
  assert.equal(value.errors[0].code, 'decision.text_limit');
  assert.equal(value.data.summary, null);
});

test('shipped examples expose only explicit authored pilot assessments', enabled, () => {
  const home = fileURLToPath(new URL('../', import.meta.url));
  for (const [example, count] of [['crate-puzzle', 1], ['little-plumber-guy-nes', 5]]) {
    const folder = join(home, 'examples', example, 'game-design', 'challenges');
    assert.equal(query(folder).data.summary.retained_assessments, count);
    assert.equal(query(folder, ['--demand', 'dexterity']).data.summary.matched_assessments, 0);
  }
});
