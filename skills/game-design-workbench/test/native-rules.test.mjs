// Read-only native declaration CLI checks. No JS domain parser/evaluator.
// GD_NATIVE_EXE=/path/to/gd.exe node --test skills/game-design-workbench/test/native-rules.test.mjs
import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdir, mkdtemp, readFile, rm, writeFile } from 'node:fs/promises';
import { join, resolve, dirname } from 'node:path';
import { tmpdir } from 'node:os';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';

const executable = process.env.GD_NATIVE_EXE && resolve(process.env.GD_NATIVE_EXE);
const home = resolve(dirname(fileURLToPath(import.meta.url)), '..');
function invoke(args, expected = 0) {
  const result = spawnSync(executable, args, { encoding: 'utf8', timeout: 30000,
    maxBuffer: 4 * 1024 * 1024, windowsHide: true, env: { ...process.env, GD_AGENT: '1' } });
  assert.equal(result.status, expected, `${result.stderr}\n${result.stdout}`);
  return result;
}
function structured(args, expected = 0) {
  const result = invoke(args, expected);
  const value = JSON.parse(result.stdout);
  assert.equal(value.ok, expected === 0);
  assert.equal(value.coverage.complete, expected === 0);
  if (!expected) assert.equal(result.stderr, '');
  return value;
}

test('native rule CLI exposes approved specimen without implying completed migration or semantics',
  { skip: !executable && 'Set GD_NATIVE_EXE' }, async () => {
    const folder = join(home, 'examples/little-plumber-guy-nes/game-design');
    const binding = role => `${role}=${join(folder, role)}`;
    const args = ['rules', '--interactions', binding('interactions'), '--mechanics', binding('mechanics'),
      '--nouns', binding('nouns'), '--verbs', binding('verbs'), '--control-vars', binding('control-vars'), '--axes', binding('axes')];
    const value = structured(args);
    assert.equal(value.coverage.operation, 'rules');
    assert.equal(value.coverage.source_record_count, 73);
    assert.equal(value.coverage.records_with_cases, 1);
    assert.equal(value.coverage.axis_count, 8);
    assert.equal(value.coverage.axis_definitions_complete, true);
    assert.equal(value.coverage.bindings_complete, false); // Remaining inventories stay explicitly unknown.
    assert.equal(value.data.records.filter(row => row.rule_block === 'absent').length, 72);
    assert.equal(value.data.cases.length, 1);
    const item = value.data.cases[0];
    assert.equal(item.record, 'interactions/touch-walker-powered');
    assert.equal(item.case, 'powered-side-contact');
    assert.equal(item.status, 'accepted');
    assert.equal(item.coverage, 'defined');
    assert.ok(value.data.records.find(row => row.record === item.record).relationships
      .some(edge => edge.field === 'mechanics' && edge.reference === 'mechanics/enemy-contact'));
    assert.equal(item.context, 'Player sideContacts Walker');
    assert.deepEqual(item.participants.map(row => row.name), ['Player', 'Walker', 'Lives']);
    assert.ok(item.participants.every(row => row.noun_resolution === 'resolved' && row.applicable_axes_complete));
    assert.deepEqual(value.data.actors.find(row => row.record === 'nouns/hero').axes.map(row => row.reference),
      ['axes/Life', 'axes/Form', 'axes/Protection', 'axes/ProtectionTime']);
    assert.equal(item.participants[0].requires, 'alive AND (tall OR spark) AND NOT protected');
    assert.equal(item.participants[0].other_axes, 'ANY');
    assert.equal(item.participants[0].preserve, 'OTHER');
    assert.equal(item.participants[0].set[0].expression, '{control-vars/safety-window}');
    assert.equal(item.participants[1].preserve, null);
    assert.equal(item.participants[1].becomes, null);
    assert.equal(item.participants[2].preserve, 'ALL');
    const lines = (await readFile(join(folder, 'interactions/touch-walker-powered.md'), 'utf8')).split('\n');
    assert.equal(lines[item.line - 1].trim(), `case ${item.case}:`);
    assert.equal(lines[item.participants[0].set[0].line - 1].trim(), 'ProtectionTime: {control-vars/safety-window}');
    assert.match(value.coverage.scope, /not resolved or evaluated/);
    const selected = structured([...args, '--fields', 'cases']);
    assert.deepEqual(Object.keys(selected.data), ['cases']);
    assert.equal(selected.coverage.source_record_count, 73);
    assert.match(invoke([...args, '--format', 'text']).stdout, /Declarations only/);
  });

test('native rule CLI handles empty, partial and invalid requests without source writes',
  { skip: !executable && 'Set GD_NATIVE_EXE' }, async t => {
    const root = await mkdtemp(join(tmpdir(), 'gd-rule-cli-'));
    t.after(() => rm(root, { recursive: true, force: true }));
    const empty = join(root, 'empty');
    const sources = join(root, 'space 雪');
    const related = join(root, 'related');
    for (const folder of [empty, sources, related]) await mkdir(folder);
    const args = ['rules', '--interactions', `cases=${sources}`, '--related', `other=${related}`];
    const bytes = new Map([
      [join(sources, 'good.md'), '---\nid: 019fff63-0000-7000-8000-000000000001\nstatus: proposed\ncoverage: unknown\n---\n```gd-rule\ncase same-start:\n    Actor:\n        requires: ANY\n        set:\n            Amount: this + 1[life]\n            Amount: this + 2[life]\n```\n'],
      [join(sources, 'bad.md'), '```gd-rule\ncase invalid:\n    Actor:\n        invented: wrong\n```\n'],
      [join(related, 'ignored.md'), '```gd-rule\nnot a rule declaration\n```\n'],
    ]);
    for (const [path, content] of bytes) await writeFile(path, content);
    const value = structured(args, 2);
    assert.equal(value.errors[0].code, 'rules.field');
    assert.equal(value.errors[0].line, 4);
    assert.equal(value.data.records.length, 2);
    assert.equal(value.data.records.find(row => row.record === 'cases/bad').rule_block, 'invalid');
    assert.equal(value.data.cases.length, 1);
    assert.equal(value.data.cases[0].record_id, '019fff63-0000-7000-8000-000000000001');
    assert.equal(value.data.cases[0].status, 'proposed');
    assert.equal(value.data.cases[0].coverage, 'unknown');
    assert.equal(value.data.cases[0].context, null);
    assert.equal(value.data.cases[0].participants[0].noun, null);
    assert.equal(value.data.cases[0].participants[0].set.length, 2); // Never last-write-wins.
    assert.equal(value.errors_omitted, 0);
    const blank = structured(['rules', '--mechanics', `mechanics=${empty}`, '--json']);
    assert.deepEqual(blank.data, { records: [], cases: [], axes: [], actors: [], findings: [] });
    for (const invalid of [['rules'], ['rules', '--nouns', `nouns=${empty}`],
      ['rules', '--interactions', `cases=${empty}`, '--fields', 'controls'],
      ['rules', '--interactions', `cases=${empty}`, '--from', 'cases/good'],
      ['rules', '--interactions', `cases=${empty}`, '--set', 'controls/test=3']]) {
      assert.equal(structured(invalid, 2).errors[0].code, 'cli.arguments');
    }
    for (const [path, content] of bytes) assert.equal(await readFile(path, 'utf8'), content);
  });

test('native axes use explicit actor references, typed metadata and open vocabularies',
  { skip: !executable && 'Set GD_NATIVE_EXE' }, async t => {
    const root = await mkdtemp(join(tmpdir(), 'gd-axis-cli-'));
    t.after(() => rm(root, { recursive: true, force: true }));
    for (const folder of ['nouns', 'dimensions', 'rules']) await mkdir(join(root, folder));
    const files = new Map([
      ['dimensions/Life.md', '---\nvalue_type: enum\nallowed_values: [alive, dead, alive]\n---\nOpen named-state vocabulary.'],
      ['dimensions/Amount.md', '---\nvalue_type: integer\nunit: life\nminimum: -0\nmaximum: 9\n---\nNo initial value inferred.'],
      ['dimensions/Time.md', '---\nvalue_type: number\nunit: s\n---\nNo invented bound.'],
      ['nouns/hero.md', '---\nstatus: proposed\naxes: [dimensions/Life, dimensions/Amount, dimensions/Amount]\n---\nExplicit applicability.'],
      ['nouns/empty.md', '---\naxes: []\n---\nNo axes currently declared.'],
      ['rules/contact.md', '```gd-rule\ncase contact:\n    First:\n        noun: actors/hero\n        requires: alive\n    Second:\n        noun: actors/hero\n        set:\n            Amount: this + 1[life]\n```\n'],
    ]);
    for (const [path, content] of files) await writeFile(join(root, path), content);
    const args = ['rules', '--interactions', `cases=${join(root, 'rules')}`, '--nouns', `actors=${join(root, 'nouns')}`,
      '--axes', `dimensions=${join(root, 'dimensions')}`];
    const value = structured(args);
    assert.equal(value.coverage.axis_count, 3);
    assert.equal(value.coverage.actor_count, 2);
    assert.equal(value.coverage.axis_definitions_complete, true);
    assert.equal(value.coverage.bindings_complete, true);
    const life = value.data.axes.find(row => row.record === 'dimensions/Life');
    assert.equal(life.closed, false);
    assert.equal(life.closed_line, 0);
    assert.deepEqual(life.allowed_values, ['alive', 'dead', 'alive']);
    assert.equal(life.definition_valid, true);
    assert.equal(value.data.findings[0].code, 'rules.axis_duplicate_value');
    const amount = value.data.axes.find(row => row.record === 'dimensions/Amount');
    assert.equal(amount.value_type, 'integer');
    assert.equal(amount.unit, 'life');
    assert.equal(amount.minimum, -0);
    assert.equal(amount.maximum, 9);
    assert.equal(amount.field_lines.minimum, 4);
    assert.equal(value.data.axes.find(row => row.record === 'dimensions/Time').minimum, null);
    const hero = value.data.actors.find(row => row.record === 'actors/hero');
    assert.equal(hero.status, 'proposed');
    assert.equal(hero.axes_declared, true);
    assert.deepEqual(hero.axes.map(row => row.reference), ['dimensions/Life', 'dimensions/Amount', 'dimensions/Amount']);
    assert.ok(hero.axes.every(row => row.resolution === 'resolved'));
    const participants = value.data.cases[0].participants;
    assert.deepEqual(participants.map(row => row.name), ['First', 'Second']);
    assert.ok(participants.every(row => row.noun_resolution === 'resolved' && row.applicable_axes_complete));
    assert.equal(participants[1].set[0].expression, 'this + 1[life]');
    const narrow = structured([...args, '--fields', 'axes,actors,findings']);
    assert.deepEqual(Object.keys(narrow.data), ['axes', 'actors', 'findings']);
    assert.match(invoke([...args, '--format', 'text']).stdout, /Typed axes: 3; actors: 2/);
    // Omitting explicit bindings does not infer roles from aliases or body mentions.
    const unloaded = structured(['rules', '--interactions', `cases=${join(root, 'rules')}`, '--related', `actors=${join(root, 'nouns')}`]);
    assert.equal(unloaded.data.cases[0].participants[0].noun_resolution, 'wrong_role');
    assert.equal(unloaded.coverage.bindings_complete, false);
    await writeFile(join(root, 'nouns/unknown.md'), '---\naxes: null\n---\nUnspecified, not an empty list.');
    const unknown = structured(args);
    assert.equal(unknown.coverage.bindings_complete, false);
    assert.equal(unknown.data.actors.find(row => row.record === 'actors/unknown').axes_declared, false);
    assert.ok(unknown.data.findings.some(row => row.code === 'rules.actor_axes_unspecified'));
    for (const [path, content] of files) assert.equal(await readFile(join(root, path), 'utf8'), content);
    await writeFile(join(root, 'dimensions/Time.md'), '---\nvalue_type: number\nunit: s+life\n---\nInvalid unit.');
    const bad = structured(args, 2);
    assert.equal(bad.errors[0].code, 'rules.axis_unit');
    assert.equal(bad.errors[0].line, 3);
    assert.equal(bad.data.axes.find(row => row.record === 'dimensions/Time').definition_valid, false);
    assert.equal(bad.coverage.axis_definitions_complete, false);
  });
