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
    assert.equal(value.coverage.condition_count, 3);
    assert.equal(value.coverage.condition_bindings_complete, true);
    assert.equal(item.participants[0].requires_condition.state, 'parsed');
    assert.deepEqual(item.participants[0].requires_condition.program.map(row => row.op),
      ['state', 'state', 'state', 'or', 'and', 'state', 'not', 'and']);
    assert.deepEqual(item.participants[0].requires_condition.program.filter(row => row.op === 'state').map(row => row.target.state),
      ['alive', 'tall', 'spark', 'protected']);
    assert.equal(item.participants[0].other_axes_mode, 'any');
    assert.equal(item.participants[0].skip_if_condition.state, 'unspecified');
    assert.deepEqual(item.participants[0].skip_if_condition.program, []);
    assert.equal(item.participants[0].other_axes, 'ANY');
    assert.equal(item.participants[0].preserve, 'OTHER');
    assert.equal(item.participants[0].set[0].expression, '{control-vars/safety-window}');
    assert.equal(value.coverage.effect_bindings_complete, true);
    assert.equal(value.coverage.state_effect_count, 2);
    assert.deepEqual(item.participants[0].state_effects.map(row => row.target), [
      { resolution: 'resolved', axis: 'axes/Form', state: 'small' },
      { resolution: 'resolved', axis: 'axes/Protection', state: 'protected' },
    ]);
    assert.deepEqual(item.participants[0].set[0].target,
      { resolution: 'resolved', axis: 'axes/ProtectionTime', state: null });
    assert.deepEqual(item.participants[1].state_effects, []);
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

test('native effect bindings keep scoped names, duplicates, open unknowns and explicit type errors',
  { skip: !executable && 'Set GD_NATIVE_EXE' }, async t => {
    const root = await mkdtemp(join(tmpdir(), 'gd-effects-'));
    t.after(() => rm(root, { recursive: true, force: true }));
    for (const folder of ['actors', 'dimensions', 'rules']) await mkdir(join(root, folder));
    const files = new Map([
      ['dimensions/Life.md', '---\nvalue_type: enum\nallowed_values: [alive, dead, dead]\nclosed: true\n---\nLife.'],
      ['dimensions/Shield.md', '---\nvalue_type: enum\nallowed_values: ' + JSON.stringify(['active', 'dead', '斧"\\']) + '\n---\nOpen states.'],
      ['dimensions/Time.md', '---\nvalue_type: number\nunit: s\n---\nDuration.'],
      ['actors/hero.md', '---\naxes: [dimensions/Life, dimensions/Shield, dimensions/Time, dimensions/Life]\n---\nActor.'],
      ['actors/empty.md', '---\naxes: []\n---\nEmpty current inventory.'],
      ['actors/unknown.md', 'Actor with unspecified axes.'],
    ]);
    for (const [path, source] of files) await writeFile(join(root, path), source);
    const args = ['rules', '--mechanics', `rules=${join(root, 'rules')}`, '--nouns', `actors=${join(root, 'actors')}`,
      '--axes', `dimensions=${join(root, 'dimensions')}`];
    const path = join(root, 'rules/contact.md');
    const check = async (fields, status = 0, noun = 'hero') => {
      const source = `\`\`\`gd-rule\ncase contact:\n    A:\n        noun: actors/${noun}\n${fields}\n\`\`\`\n`;
      await writeFile(path, source);
      const value = structured(args, status);
      assert.equal(await readFile(path, 'utf8'), source);
      return { value, actor: value.data.cases[0].participants[0] };
    };
    let result = await check('        becomes: Life.dead, Life.dead, Shield.active\n        set:\n            Time: this + 1[s]\n            dimensions/Time: this + 2[s]');
    assert.equal(result.value.coverage.effect_bindings_complete, true);
    assert.deepEqual(result.actor.state_effects.map(row => row.text), ['Life.dead', 'Life.dead', 'Shield.active']);
    assert.ok(result.actor.state_effects.every(row => row.target.resolution === 'resolved'));
    assert.deepEqual(result.actor.set.map(row => row.target.axis), ['dimensions/Time', 'dimensions/Time']);
    assert.deepEqual(result.actor.set.map(row => row.expression), ['this + 1[s]', 'this + 2[s]']);
    result = await check('        becomes: Shield.斧"\\');
    assert.equal(result.actor.state_effects[0].text, 'Shield.斧"\\');
    assert.equal(result.actor.state_effects[0].target.state, '斧"\\');
    result = await check('        becomes: dead', 2);
    assert.equal(result.value.errors[0].code, 'rules.effect_ambiguous');
    assert.equal(result.actor.state_effects[0].target.axis, null);
    result = await check('        becomes: Life.absent', 2);
    assert.equal(result.actor.state_effects[0].target.resolution, 'missing');
    assert.equal(result.value.errors[0].line, 5);
    result = await check('        becomes: Shield.future');
    assert.equal(result.actor.state_effects[0].target.resolution, 'undetermined');
    assert.equal(result.value.coverage.effect_bindings_complete, false);
    assert.ok(result.value.data.findings.some(row => row.code === 'rules.effect_undetermined'));
    result = await check('        becomes: Time.active', 2);
    assert.equal(result.actor.state_effects[0].target.resolution, 'wrong_type');
    result = await check('        set:\n            Life: this', 2);
    assert.equal(result.actor.set[0].target.resolution, 'wrong_type');
    result = await check('        becomes: alive', 2, 'empty');
    assert.equal(result.actor.state_effects[0].target.resolution, 'missing');
    result = await check('        becomes: alive', 0, 'unknown');
    assert.equal(result.actor.state_effects[0].target.resolution, 'undetermined');
    result = await check('        becomes: Life.dead, , Shield.active', 2);
    assert.equal(result.value.errors[0].code, 'rules.state_list');
    assert.deepEqual(result.actor.state_effects, []); // No valid-looking partial list.
    assert.equal(result.actor.becomes, 'Life.dead, , Shield.active');
    result = await check('        becomes: Life.', 2);
    assert.equal(result.value.errors[0].code, 'rules.state_list');
    assert.deepEqual(result.actor.state_effects, []);
    await writeFile(join(root, 'actors/hero.md'), '---\naxes: [dimensions/Life, unavailable/Axis]\n---\nIncomplete inventory.');
    result = await check('        becomes: alive');
    assert.equal(result.actor.state_effects[0].target.resolution, 'undetermined');
    result = await check('        becomes: dimensions/Life.alive');
    assert.equal(result.actor.state_effects[0].target.resolution, 'resolved');
    for (const [path, source] of files) if (path !== 'actors/hero.md') assert.equal(await readFile(join(root, path), 'utf8'), source);
    await writeFile(join(root, 'actors/hero.md'), '---\naxes: [dimensions/Time]\n---\nExplicit definition.');
    await writeFile(join(root, 'dimensions/Time.md'), '---\nvalue_type: number\nunit: s\nminimum: 2\nmaximum: 1\n---\nInvalid definition.');
    result = await check('        set:\n            dimensions/Time: this', 2);
    assert.equal(result.actor.set[0].target.resolution, 'invalid_definition');
    assert.ok(result.value.errors.some(row => row.code === 'rules.effect_definition'));
  });

test('native effect symbol count and byte-work limits preserve raw declarations and checked portions',
  { skip: !executable && 'Set GD_NATIVE_EXE' }, async t => {
    const root = await mkdtemp(join(tmpdir(), 'gd-effect-limits-'));
    t.after(() => rm(root, { recursive: true, force: true }));
    for (const folder of ['actors', 'dimensions', 'rules']) await mkdir(join(root, folder));
    await writeFile(join(root, 'actors/hero.md'), '---\naxes: [dimensions/Life]\n---\nHero.');
    const axis = join(root, 'dimensions/Life.md'), rule = join(root, 'rules/case.md');
    const args = ['rules', '--interactions', `rules=${join(root, 'rules')}`, '--nouns', `actors=${join(root, 'actors')}`,
      '--axes', `dimensions=${join(root, 'dimensions')}`];
    const cases = counts => '```gd-rule\n' + counts.map((count, index) =>
      `case c${index}:\n    Actor:\n        noun: actors/hero\n        becomes: ${Array(count).fill('a').join(', ')}\n`).join('') + '```\n';
    await writeFile(axis, '---\nvalue_type: enum\nallowed_values: [a]\nclosed: true\n---\nClosed.');
    await writeFile(rule, cases([2000, 2000, 2000, 2000, 192]));
    const exact = structured(args);
    assert.equal(exact.coverage.state_effect_count, 8192);
    assert.equal(exact.coverage.effect_bindings_complete, true);
    const over = cases([2000, 2000, 2000, 2000, 193, 1]);
    await writeFile(rule, over);
    const limited = structured(args, 2);
    assert.equal(limited.errors[0].code, 'rules.state_effect_limit');
    assert.equal(limited.coverage.state_effect_count, 8000);
    assert.equal(limited.coverage.effect_bindings_complete, false);
    assert.deepEqual(limited.data.cases[4].participants[0].state_effects, []);
    assert.deepEqual(limited.data.cases[5].participants[0].state_effects, []);
    assert.equal(await readFile(rule, 'utf8'), over);
    const vocabulary = Array.from({ length: 64 }, (_, index) => `value${index}${'x'.repeat(980)}`);
    await writeFile(axis, `---\nvalue_type: enum\nallowed_values: ${JSON.stringify(vocabulary)}\n---\nOpen vocabulary.`);
    const expensive = '```gd-rule\ncase work:\n    Actor:\n        noun: actors/hero\n        becomes: ' + Array(500).fill('unknown').join(', ') + '\n```\n';
    await writeFile(rule, expensive);
    const bounded = structured(args, 2);
    assert.equal(bounded.errors[0].code, 'rules.symbol_work_limit');
    assert.equal(bounded.coverage.state_effect_count, 500);
    assert.equal(bounded.coverage.effect_bindings_complete, false);
    assert.ok(bounded.coverage.symbol_work <= bounded.coverage.limits.symbol_work);
    const effects = bounded.data.cases[0].participants[0].state_effects;
    assert.ok(effects.some(row => row.target.resolution === 'undetermined'));
    assert.equal(effects.at(-1).target.resolution, 'unavailable');
    assert.equal(await readFile(rule, 'utf8'), expensive);
    const mixed = '```gd-rule\ncase mixed:\n    First:\n        noun: actors/hero\n        requires: ' +
      Array(100).fill('unknown').join(' OR ') + '\n        becomes: ' + Array(500).fill('unknown').join(', ') +
      '\n    Later:\n        noun: actors/hero\n        requires: NOT unknown\n```\n';
    await writeFile(rule, mixed);
    const shared = structured(args, 2);
    assert.equal(shared.errors[0].code, 'rules.symbol_work_limit');
    assert.ok(shared.coverage.symbol_work <= shared.coverage.limits.symbol_work);
    assert.ok(shared.data.cases[0].participants[0].requires_condition.program.filter(row => row.op === 'state')
      .every(row => row.target.resolution === 'undetermined'));
    assert.equal(shared.data.cases[0].participants[0].state_effects.at(-1).target.resolution, 'unavailable');
    assert.equal(shared.data.cases[0].participants[1].requires_condition.program[0].target.resolution, 'unavailable');
    assert.equal(await readFile(rule, 'utf8'), mixed);
  });

test('native condition grammar resolves grouped scoped predicates without flattening unknowns or local skips',
  { skip: !executable && 'Set GD_NATIVE_EXE' }, async t => {
    const root = await mkdtemp(join(tmpdir(), 'gd-condition-'));
    t.after(() => rm(root, { recursive: true, force: true }));
    for (const folder of ['actors', 'axes', 'rules']) await mkdir(join(root, folder));
    await writeFile(join(root, 'axes/Life.md'), '---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: true\n---\nLife.');
    await writeFile(join(root, 'axes/Protection.md'), '---\nvalue_type: enum\nallowed_values: ' + JSON.stringify(['protected', '斧"\\']) + '\n---\nOpen protection.');
    await writeFile(join(root, 'axes/Time.md'), '---\nvalue_type: number\nunit: s\n---\nNumeric.');
    await writeFile(join(root, 'actors/hero.md'), '---\naxes: [axes/Life, axes/Protection, axes/Time]\n---\nSame noun, distinct local bindings.');
    const path = join(root, 'rules/contact.md');
    const args = ['rules', '--mechanics', `rules=${join(root, 'rules')}`, '--axes', `axes=${join(root, 'axes')}`,
      '--nouns', `actors=${join(root, 'actors')}`];
    const check = async (expression, status = 0, extra = '') => {
      const source = `\`\`\`gd-rule\ncase contact:\n    A:\n        noun: actors/hero\n        requires: ${expression}\n${extra}\n    B:\n        noun: actors/hero\n\`\`\`\n`;
      await writeFile(path, source);
      const value = structured(args, status);
      assert.equal(await readFile(path, 'utf8'), source);
      for (const token of value.data.cases[0].participants[0].requires_condition.program)
        assert.equal(Buffer.from(expression).subarray(token.offset, token.offset + Buffer.byteLength(token.text)).toString('utf8'), token.text);
      return { value, actor: value.data.cases[0].participants[0] };
    };
    let result = await check('alive aNd (NOT protected || Life.dead)', 0, '        skip_if: !alive\n        other_axes: \tAnY');
    let condition = result.actor.requires_condition;
    assert.equal(condition.state, 'parsed');
    assert.equal(condition.bindings_complete, true);
    assert.equal(result.actor.other_axes_mode, 'any');
    assert.deepEqual(condition.program.map(row => row.op), ['state', 'state', 'not', 'state', 'or', 'and']);
    assert.equal(condition.line, 5);
    for (const token of condition.program) assert.equal(Buffer.from(result.actor.requires).subarray(token.offset,
      token.offset + Buffer.byteLength(token.text)).toString('utf8'), token.text);
    assert.deepEqual(result.actor.skip_if_condition.program.map(row => row.op), ['state', 'not']);
    assert.equal(result.value.data.cases[0].participants[1].requires_condition.state, 'unspecified');
    assert.equal(result.value.data.cases[0].participants[1].skip_if_condition.state, 'unspecified');
    assert.equal(result.value.data.cases[0].participants[1].other_axes_mode, 'unspecified');
    result = await check('Protection.斧"\\ AND !protected');
    assert.equal(result.actor.requires_condition.program[0].target.state, '斧"\\');
    result = await check('ANY');
    assert.deepEqual(result.actor.requires_condition.program.map(row => row.op), ['any']);
    result = await check('alive OR protected AND Life.dead');
    assert.deepEqual(result.actor.requires_condition.program.map(row => row.op), ['state', 'state', 'state', 'and', 'or']);
    result = await check('!!alive');
    assert.deepEqual(result.actor.requires_condition.program.map(row => row.op), ['state', 'not', 'not']);
    result = await check('NOT Protection.future');
    assert.equal(result.actor.requires_condition.bindings_complete, false);
    assert.equal(result.actor.requires_condition.program[0].target.resolution, 'undetermined');
    assert.equal(result.value.coverage.condition_bindings_complete, false);
    result = await check('Life.Alive', 2);
    assert.equal(result.value.errors[0].code, 'rules.condition_missing'); // State names remain case-sensitive.
    result = await check('Time.active', 2);
    assert.equal(result.value.errors[0].code, 'rules.condition_type');
    result = await check('Time > 0[s]');
    assert.equal(result.actor.requires_condition.state, 'unsupported');
    assert.deepEqual(result.actor.requires_condition.program, []);
    assert.equal(result.value.coverage.condition_bindings_complete, false);
    assert.ok(result.value.data.findings.some(row => row.code === 'rules.condition_unsupported'));
    result = await check('ANY', 2, '        other_axes: NONE');
    assert.equal(result.actor.other_axes_mode, 'invalid');
    assert.equal(result.value.errors[0].code, 'rules.other_axes');
    for (const expression of ['(', ')', '()', 'alive dead', 'alive AND', 'AND alive', 'alive || || dead',
      '!', 'alive & dead', 'alive | dead', 'alive NOT dead', 'alive OR (dead', 'alive OR dead)', 'Life.']) {
      result = await check(expression, 2);
      assert.equal(result.value.errors[0].code, 'rules.condition_syntax', expression);
      assert.equal(result.actor.requires_condition.state, 'invalid');
      assert.deepEqual(result.actor.requires_condition.program, []); // Never return a partial Boolean program.
      assert.equal(result.actor.requires, expression);
    }
  });

test('native condition nesting, per-field and total program limits retain explicit partial coverage',
  { skip: !executable && 'Set GD_NATIVE_EXE' }, async t => {
    const root = await mkdtemp(join(tmpdir(), 'gd-condition-limits-'));
    t.after(() => rm(root, { recursive: true, force: true }));
    const path = join(root, 'limit.md');
    const args = ['rules', '--interactions', `rules=${root}`];
    const single = expression => `\`\`\`gd-rule\ncase test:\n    A:\n        requires: ${expression}\n\`\`\`\n`;
    await writeFile(path, single('('.repeat(64) + 'ANY' + ')'.repeat(64)));
    assert.equal(structured(args).data.cases[0].participants[0].requires_condition.program.length, 1);
    await writeFile(path, single('('.repeat(65) + 'ANY' + ')'.repeat(65)));
    let value = structured(args, 2);
    assert.equal(value.errors[0].code, 'rules.condition_depth');
    assert.deepEqual(value.data.cases[0].participants[0].requires_condition.program, []);
    await writeFile(path, single('NOT '.repeat(255) + 'ANY'));
    assert.equal(structured(args).data.cases[0].participants[0].requires_condition.program.length, 256);
    await writeFile(path, single('NOT '.repeat(256) + 'ANY'));
    value = structured(args, 2);
    assert.equal(value.errors[0].code, 'rules.condition_tokens');
    assert.deepEqual(value.data.cases[0].participants[0].requires_condition.program, []);
    const expression = Array(128).fill('ANY').join(' AND '); // 255 input and compiled tokens.
    const prefix = '```gd-rule\ncase capacity:\n' + Array.from({ length: 257 }, (_, i) =>
      `    A${i}:\n        requires: ${expression}\n`).join('');
    const exact = prefix + '    Last:\n        requires: ANY\n```\n';
    await writeFile(path, exact);
    value = structured([...args, '--fields', 'records']);
    assert.equal(value.coverage.condition_token_count, 65536);
    assert.equal(value.coverage.condition_bindings_complete, true);
    const overflow = prefix + '    Last:\n        requires: ANY\n    Overflow:\n        requires: ANY\n    Later:\n        requires: ANY\n```\n';
    await writeFile(path, overflow);
    value = structured([...args, '--fields', 'records'], 2);
    assert.equal(value.errors[0].code, 'rules.condition_tokens');
    assert.equal(value.errors.length, 1);
    assert.equal(value.coverage.condition_token_count, 65536);
    assert.equal(value.coverage.condition_bindings_complete, false);
    assert.equal(value.coverage.limits.condition_tokens, 65536);
    assert.equal(await readFile(path, 'utf8'), overflow);
  });
