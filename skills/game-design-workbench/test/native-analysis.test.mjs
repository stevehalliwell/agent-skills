import test from 'node:test';
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { mkdtemp, mkdir, writeFile, readFile, rm } from 'node:fs/promises';
import { join } from 'node:path';
import { tmpdir } from 'node:os';

const executable = process.env.GD_NATIVE_EXE;
async function fixture(t) {
  const root = await mkdtemp(join(tmpdir(), 'gd-analysis-'));
  t.after(() => rm(root, { recursive: true, force: true }));
  for (const folder of ['axes', 'actors', 'rules']) await mkdir(join(root, folder));
  for (const [name, values] of [['Life', 'alive, dead'], ['Form', 'small, tall'], ['Contact', 'touches']])
    await writeFile(join(root, `axes/${name}.md`), `---\nvalue_type: enum\nallowed_values: [${values}]\nclosed: true\n---\nExplicit vocabulary.`);
  await writeFile(join(root, 'axes/Amount.md'), '---\nvalue_type: integer\nunit: life\nminimum: 0\nmaximum: 3\n---\nDiagnostic bounds.');
  await writeFile(join(root, 'actors/hero.md'), '---\naxes: [axes/Life, axes/Form]\n---\nActor.');
  await writeFile(join(root, 'actors/lives.md'), '---\naxes: [axes/Amount]\n---\nActor.');
  const args = ['rules', '--mechanics', `rules=${join(root, 'rules')}`, '--nouns', `actors=${join(root, 'actors')}`,
    '--axes', `axes=${join(root, 'axes')}`, '--json'];
  return {
    root,
    async source(name, body, header = '') {
      await writeFile(join(root, `rules/${name}.md`), `${header}\n\`\`\`gd-rule\n${body}\n\`\`\`\n`);
    },
    run(extra = [], status = 0) {
      const result = spawnSync(executable, [...args, ...extra], { encoding: 'utf8', timeout: 15000, maxBuffer: 16 * 1024 * 1024 });
      assert.equal(result.status, status, result.stderr || result.error?.message);
      return JSON.parse(result.stdout);
    },
  };
}
const actor = (name, requires, effects) => `    ${name}:\n        noun: actors/hero\n        requires: ${requires}\n        other_axes: ANY\n${effects}`;

test('headless scenarios expand explicit calls, preserve whole-case OTHER and read one immutable starting state',
  { skip: !executable && 'Set GD_NATIVE_EXE' }, async t => {
    const f = await fixture(t);
    await f.source('helper', `case change:\n    context: ANY\n${actor('Victim', 'alive AND small', '        becomes: dead\n        preserve: OTHER')}\n    Reserve:\n        noun: actors/lives\n        requires: ANY\n        set:\n            Amount: this - 1[life]`);
    await f.source('root', `case direct:\n    context: ANY\n${actor('Hero', 'alive AND small', '        becomes: dead, tall\n        preserve: OTHER')}\n    Budget:\n        noun: actors/lives\n        requires: ANY\n        set:\n            Amount: this - 1[life]\ncase composed:\n    context: ANY\n${actor('Hero', 'alive AND small', '        becomes: tall\n        preserve: OTHER\n        causes: rules/helper#change with Victim=Hero, Reserve=Budget')}\n    Budget:\n        noun: actors/lives\n        requires: ANY\n        preserve: OTHER`);
    const before = await readFile(join(f.root, 'rules/root.md'), 'utf8');
    const value = f.run(['--case', 'rules/root#composed', '--assume', 'Hero:axes/Life=alive', '--assume', 'Hero:axes/Form=small',
      '--assume', 'Budget:axes/Amount=3', '--analyze', '--fields', 'analysis']);
    assert.deepEqual(Object.keys(value.data), ['analysis']);
    const result = value.data.analysis.scenario;
    assert.equal(result.applicability, 'true');
    assert.equal(result.constraints_complete, true);
    assert.equal(result.expansions, 2);
    assert.equal(result.conflicts, 0);
    assert.equal(result.writes.length, 3);
    assert.deepEqual(result.writes.filter(row => row.state).map(row => row.state).sort(), ['dead', 'tall']);
    const quantity = result.writes.find(row => row.axis === 'axes/Amount');
    assert.equal(quantity.number, 2);
    assert.equal(quantity.via.length, 2);
    assert.equal(quantity.origin.record, 'rules/helper');
    const pair = value.data.analysis.comparisons.find(row => row.first.record === 'rules/root' && row.second.record === 'rules/root');
    assert.equal(pair.relation, 'redundant');
    assert.equal(pair.numeric_domains_unexamined, true);
    assert.equal(pair.independent_corroboration, false);
    assert.equal(await readFile(join(f.root, 'rules/root.md'), 'utf8'), before);
    const unknown = f.run(['--case', 'rules/root#composed', '--fields', 'analysis']).data.analysis.scenario;
    assert.equal(unknown.applicability, 'unknown');
    assert.ok(unknown.writes.every(row => row.conditional));
    assert.equal(unknown.writes.find(row => row.axis === 'axes/Amount').number, null);
    f.run(['--case', 'rules/root#composed', '--assume', 'Hero:axes/Life=alive', '--assume', 'Hero:axes/Life=dead'], 2);
    f.run(['--case', 'rules/root#composed', '--assume', 'Hero:axes/Amount=3'], 2);
    f.run(['--assume', 'Hero:axes/Life=alive'], 2);
  });

test('bounded comparisons distinguish equivalence, conflicts, disjoint gates, compatible partial overlap and open vocabularies',
  { skip: !executable && 'Set GD_NATIVE_EXE' }, async t => {
    const f = await fixture(t);
    const compare = async (a, b) => {
      await f.source('pair', `case first:\n    context: ANY\n${a}\ncase second:\n    context: ANY\n${b}`);
      return f.run(['--analyze', '--fields', 'analysis']).data.analysis.comparisons[0];
    };
    let result = await compare(actor('A', 'ANY', '        becomes: dead\n        preserve: OTHER'), actor('B', 'ANY', '        becomes: dead\n        preserve: OTHER'));
    assert.equal(result.relation, 'equivalent');
    assert.equal(result.checked_branches, 4);
    result = await compare(actor('A', 'ANY', '        becomes: dead\n        preserve: OTHER'), actor('B', 'ANY', '        becomes: alive\n        preserve: OTHER'));
    assert.equal(result.relation, 'conflict');
    assert.equal(result.conflicting_writes.length, 2);
    assert.equal(result.witness.length, 2);
    result = await compare(actor('A', 'alive', '        becomes: dead'), actor('B', 'dead', '        becomes: alive'));
    assert.equal(result.relation, 'disjoint');
    result = await compare(actor('A', 'ANY', '        becomes: dead'), actor('B', 'ANY', '        becomes: tall'));
    assert.equal(result.relation, 'compatible_overlap');
    assert.equal(result.complete_outcome_scope, false);
    await writeFile(join(f.root, 'axes/Life.md'), '---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: false\n---\nOpen.');
    result = await compare(actor('A', 'ANY', '        becomes: dead'), actor('B', 'ANY', '        becomes: dead'));
    assert.equal(result.relation, 'undetermined');
    assert.equal(result.closed_vocabularies, false);
  });

test('ordered context assumptions never reverse relations and explicit preservation remains a constraint',
  { skip: !executable && 'Set GD_NATIVE_EXE' }, async t => {
    const f = await fixture(t);
    await f.source('context', `case contact:\n    context: A touches B\n${actor('A', 'ANY', '        becomes: dead\n        preserve: Life, Form')}\n    B:\n        noun: actors/lives\n        requires: ANY\n        preserve: ALL`, '---\ncontext_axes: [axes/Contact]\n---');
    let result = f.run(['--case', 'rules/context#contact', '--context', 'B axes/Contact.touches A', '--fields', 'analysis']).data.analysis.scenario;
    assert.equal(result.context, 'unknown');
    result = f.run(['--case', 'rules/context#contact', '--context', 'A axes/Contact.touches B', '--assume', 'A:axes/Life=alive', '--fields', 'analysis']).data.analysis.scenario;
    assert.equal(result.context, 'true');
    assert.equal(result.conflicts, 1);
    assert.equal(result.constraints_complete, false);
    result = f.run(['--case', 'rules/context#contact', '--context-not', 'A touches B', '--fields', 'analysis']).data.analysis.scenario;
    assert.equal(result.applicability, 'false');
    assert.deepEqual(result.writes, []);
    f.run(['--case', 'rules/context#contact', '--context', 'A axes/Contact.touches B', '--context-not', 'A axes/Contact.touches B'], 2);
    f.run(['--case', 'rules/context#contact', '--context', 'A axes/Contact.unknown B'], 2);
  });

test('runtime arithmetic failures are uncheckable writes, never compatible merely because programs match',
  { skip: !executable && 'Set GD_NATIVE_EXE' }, async t => {
    const f = await fixture(t);
    await f.source('numeric', 'case divide:\n    context: ANY\n    Budget:\n        noun: actors/lives\n        requires: ANY\n        set:\n            Amount: this / this * 1[life]\n            Amount: this / this * 1[life]');
    const result = f.run(['--case', 'rules/numeric#divide', '--assume', 'Budget:axes/Amount=0', '--fields', 'analysis']).data.analysis.scenario;
    assert.equal(result.constraints_complete, false);
    assert.equal(result.conflicts, 0);
    assert.equal(result.uncertain_overlap_pairs, 1);
    assert.equal(result.write_checks[0].conflict, 'unknown');
    assert.ok(result.writes.every(row => row.numeric_state === 'arithmetic_error'));
    await f.source('binding', 'case invalid-condition:\n    context: ANY\n    Budget:\n        noun: actors/lives\n        requires: ANY OR Amount >= 1[life] / 0\n        preserve: ALL');
    const binding = f.run(['--case', 'rules/binding#invalid-condition', '--assume', 'Budget:axes/Amount=1', '--fields', 'analysis'], 2).data.analysis.scenario;
    assert.equal(binding.applicability, 'true'); // Strong three-valued OR, not complete condition bindings.
    assert.equal(binding.constraints_complete, false);
  });

test('invocation compilation exhaustion cannot turn unseen effects into unconditional OTHER preservation',
  { skip: !executable && 'Set GD_NATIVE_EXE' }, async t => {
    const f = await fixture(t);
    await f.source('helper', `case change:\n    context: ANY\n${actor('Victim', 'ANY', '        becomes: dead\n        preserve: OTHER')}`);
    for (const name of ['a-long', 'b-long', 'c-long']) await f.source(name,
      `case long:\n    context: ANY\n${actor('A', 'ANY', `        causes: rules/helper#change with${' '.repeat(7900)}Victim=A`)}`);
    await f.source('z-late', `case late:\n    context: ANY\n${actor('A', 'ANY', '        causes: rules/helper#change\n        preserve: OTHER')}`);
    const result = f.run(['--case', 'rules/z-late#late', '--assume', 'A:axes/Life=alive', '--fields', 'analysis'], 2).data.analysis;
    assert.equal(result.invocation_limited, true);
    assert.equal(result.scenario.constraints_complete, false);
    assert.ok(result.scenario.writes.length > 0);
    assert.ok(result.scenario.writes.every(row => row.kind === 'preserve' && row.conditional));
    await f.source('zz-clean', `case clean:\n    context: ANY\n${actor('A', 'ANY', '        preserve: ALL')}`);
    const clean = f.run(['--case', 'rules/zz-clean#clean', '--assume', 'A:axes/Life=alive', '--assume', 'A:axes/Form=small', '--fields', 'analysis'], 2).data.analysis;
    assert.equal(clean.invocation_limited, true);
    assert.equal(clean.scenario.constraints_complete, true); // Unrelated exhausted calls do not erase checked scope.
    assert.ok(clean.scenario.writes.every(row => row.kind === 'preserve' && !row.conditional));
  });

test('comparison reports branch and pair caps rather than claiming complete coverage',
  { skip: !executable && 'Set GD_NATIVE_EXE' }, async t => {
    const f = await fixture(t);
    const axes = [];
    for (let i = 0; i < 8; ++i) {
      axes.push(`axes/Axis${i}`);
      await writeFile(join(f.root, `axes/Axis${i}.md`), `---\nvalue_type: enum\nallowed_values: [a${i}, b${i}]\nclosed: true\n---\nIndependent declared dimension, not a reachable scene claim.`);
    }
    await writeFile(join(f.root, 'actors/hero.md'), `---\naxes: [${axes.join(', ')}]\n---\nActor.`);
    await f.source('pair', `case first:\n    context: ANY\n${actor('A', 'ANY', '        preserve: ALL')}\ncase second:\n    context: ANY\n${actor('B', 'ANY', '        preserve: ALL')}`);
    let result = f.run(['--analyze', '--fields', 'analysis']).data.analysis;
    assert.equal(result.comparisons[0].checked_branches, 128);
    assert.equal(result.comparisons[0].unexamined_known_branches, 128);
    assert.equal(result.comparisons[0].relation, 'undetermined');
    await f.source('pair', Array.from({ length: 92 }, (_, i) => `case c${i}:\n    context: ANY`).join('\n'));
    result = f.run(['--analyze', '--fields', 'analysis']).data.analysis;
    assert.equal(result.compared_pairs, 4096);
    assert.equal(result.pairs_unexamined, 90);
    assert.equal(result.comparison_limited, true);
  });
