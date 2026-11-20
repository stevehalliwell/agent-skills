import test from 'node:test';
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { mkdtemp, mkdir, writeFile, readFile, rm } from 'node:fs/promises';
import { join } from 'node:path';
import { tmpdir } from 'node:os';

const executable = process.env.GD_NATIVE_EXE;
test('rule quantities share typed math, retain proposed bounds violations and never supply an implicit starting value',
  { skip: !executable && 'Set GD_NATIVE_EXE' }, async t => {
    const root = await mkdtemp(join(tmpdir(), 'gd-quantities-'));
    t.after(() => rm(root, { recursive: true, force: true }));
    for (const folder of ['axes', 'actors', 'rules', 'controls']) await mkdir(join(root, folder));
    await writeFile(join(root, 'axes/Amount.md'), '---\nvalue_type: integer\nunit: life\nminimum: 0\nmaximum: 3\n---\nAmount.');
    await writeFile(join(root, 'axes/Time.md'), '---\nvalue_type: number\nunit: s\n---\nTime.');
    await writeFile(join(root, 'axes/Form.md'), '---\nvalue_type: enum\nallowed_values: [small, tall]\nclosed: true\n---\nForm.');
    await writeFile(join(root, 'actors/hero.md'), '---\naxes: [axes/Amount, axes/Time, axes/Form]\n---\nActor.');
    await writeFile(join(root, 'controls/form.md'), '---\nvalue_type: enum\nvalue: small\nallowed_values: [small, tall]\n---\nConfigured form.');
    await writeFile(join(root, 'controls/award.md'), '---\nvalue_type: integer\nvalue: 1\nunit: life\n---\nConfigured award.');
    await writeFile(join(root, 'controls/large.md'), '---\nvalue_type: number\nvalue: 9007199254740992\nunit: "1"\n---\nUnsafe integer-consumer operand.');
    await writeFile(join(root, 'controls/bad-cost.md'), '---\nvalue_type: integer\nexpression: 1[life] / 0[1]\nunit: life\n---\nFailed dependency.');
    await writeFile(join(root, 'controls/policy.md'), '---\nvalue_type: boolean\nvalue: true\n---\nNot a number.');
    const path = join(root, 'rules/quantity.md');
    const args = ['rules', '--mechanics', `rules=${join(root, 'rules')}`, '--axes', `axes=${join(root, 'axes')}`,
      '--nouns', `actors=${join(root, 'actors')}`, '--control-vars', `controls=${join(root, 'controls')}`, '--json'];
    const check = async (expression, { axis = 'Amount', requires = 'ANY', becomes = null, status = 0 } = {}) => {
      const source = `\`\`\`gd-rule\ncase inspect:\n    unknown_effects: No supplied actor starting values.\n    Actor:\n        noun: actors/hero\n        requires: ${requires}\n${becomes ? `        becomes: ${becomes}\n` : ''}        set:\n            ${axis}: ${expression}\n\`\`\`\n`;
      await writeFile(path, source);
      const result = spawnSync(executable, args, { encoding: 'utf8', timeout: 15000 });
      assert.equal(result.status, status, result.stderr || result.error?.message);
      const value = JSON.parse(result.stdout);
      assert.equal(await readFile(path, 'utf8'), source);
      return { value, assignment: value.data.cases[0].participants[0].set[0], actor: value.data.cases[0].participants[0] };
    };
    let result = await check('this + {controls/award}');
    assert.equal(result.assignment.quantity.state, 'parsed');
    assert.equal(result.assignment.quantity.inspection.state, 'unknown');
    assert.equal(result.assignment.quantity.inspection.number, null);
    assert.equal(result.assignment.quantity.inspection.checks_complete, true);
    assert.equal(result.value.data.cases[0].unknown_effects, 'No supplied actor starting values.');
    result = await check('0[life]', { requires: '{controls/policy}', becomes: '{controls/form}' });
    assert.equal(result.actor.requires_condition.program[0].op, 'control');
    assert.equal(result.actor.requires_condition.bindings_complete, true);
    assert.equal(result.actor.state_effects[0].target.state, 'small');
    assert.equal(result.actor.state_effects[0].control, 'controls/form');
    result = await check('0[life]', { requires: '{controls/award}', status: 2 });
    assert.ok(result.value.errors.some(row => row.code === 'rules.condition_control_type'));
    result = await check('0[life]', { becomes: '{controls/policy}', status: 2 });
    assert.ok(result.value.errors.some(row => row.code === 'rules.effect_control_type'));
    result = await check('4[life]');
    assert.equal(result.assignment.quantity.inspection.number, 4);
    assert.equal(result.assignment.quantity.inspection.bounds, 'outside_declared');
    assert.ok(result.value.data.findings.some(row => row.code === 'rules.quantity_bounds'));
    assert.equal(result.value.ok, true); // Diagnostic, not an implicit gameplay cap/reject policy.
    result = await check('1.5[life]', { status: 2 });
    assert.equal(result.assignment.quantity.inspection.number, 1.5); // Retain finite proposed value despite invalid integer type.
    assert.ok(result.value.errors.some(row => row.code === 'rules.quantity_arithmetic'));
    result = await check('1.00000000000000001[life]', { status: 2 });
    assert.equal(result.assignment.quantity.inspection.state, 'arithmetic_error');
    result = await check('0.5[life] * 2[1]');
    assert.equal(result.assignment.quantity.inspection.number, 1); // Genuine fractional intermediates remain allowed.
    result = await check('1[life] + 1e-17[life]');
    assert.equal(result.assignment.quantity.inspection.number, 1); // Binary64, not an exact-decimal/rational claim.
    for (const expression of ['0[life] * 9007199254740992[1]', '0[life] * {controls/large}',
      '({controls/large} - {controls/large}) * 1[life]']) {
      result = await check(expression, { status: 2 });
      assert.equal(result.assignment.quantity.inspection.state, 'arithmetic_error');
    }
    result = await check('0[life]', { requires: 'Amount >= {controls/bad-cost}', status: 2 });
    assert.equal(result.actor.requires_condition.program[0].quantity.bindings_complete, false);
    assert.equal(result.actor.requires_condition.bindings_complete, false);
    assert.equal(result.value.coverage.condition_bindings_complete, false);
    result = await check('this + 1[life]', { axis: 'Time', status: 2 });
    assert.equal(result.assignment.quantity.inspection.state, 'unit_error');
    assert.ok(result.value.errors.some(row => row.code === 'rules.quantity_unit'));
    result = await check('UNKNOWN');
    assert.equal(result.assignment.quantity.inspection.state, 'unknown');
    assert.equal(result.assignment.quantity.inspection.checks_complete, true);
    result = await check('1[life] / 0[1]', { status: 2 });
    assert.equal(result.assignment.quantity.inspection.state, 'arithmetic_error');
    result = await check('{controls/policy}', { status: 2 });
    assert.equal(result.assignment.quantity.inspection.state, 'arithmetic_error'); // No boolean-to-number coercion.
    result = await check('{rules/quantity}', { status: 2 });
    assert.ok(result.value.errors.some(row => row.code === 'rules.quantity_reference')); // Never bind a rule at control partition boundary.
    result = await check('${controls/award}', { status: 2 });
    assert.ok(result.value.errors.some(row => row.code === 'rules.reference_syntax'));
    assert.equal(result.assignment.quantity.token_count, 0);
    result = await check('1[life] +', { status: 2 });
    assert.equal(result.assignment.quantity.state, 'invalid');
    assert.equal(result.assignment.quantity.token_count, 0);
    result = await check('0[life]', { requires: 'Amount >= (1[life] + {controls/award}) AND NOT (Amount != 2[life])' });
    assert.deepEqual(result.actor.requires_condition.program.map(row => row.op), ['compare', 'compare', 'not', 'and']);
    assert.equal(result.actor.requires_condition.program[0].quantity.inspection.number, 2);
    result = await check('0[life]', { requires: 'Amount >= this', status: 2 });
    assert.ok(result.value.errors.some(row => row.code === 'rules.quantity_syntax'));
    assert.equal(result.actor.requires_condition.bindings_complete, false);
    assert.equal(result.actor.requires_condition.state, 'invalid');
    assert.deepEqual(result.actor.requires_condition.program, []);
    result = await check('0[life]', { requires: 'ANY OR Amount >= 1[life] +', status: 2 });
    assert.equal(result.actor.requires_condition.state, 'invalid');
    assert.deepEqual(result.actor.requires_condition.program, []); // ANY cannot hide malformed numeric syntax.
    result = await check('0[life]', { requires: 'ANY OR ${controls/policy}', status: 2 });
    assert.equal(result.actor.requires_condition.state, 'invalid');
    assert.deepEqual(result.actor.requires_condition.program, []);
    const human = spawnSync(executable, [...args.filter(argument => argument !== '--json'), '--format', 'text'], { encoding: 'utf8', timeout: 15000 });
    assert.match(human.stdout, /Shared numeric math inspects constants/);
    assert.doesNotMatch(human.stdout, /No resulting quantities evaluated|resulting quantities and composition are not resolved/);
    assert.equal(result.value.coverage.limits.quantities, 16384);
    assert.equal(result.value.coverage.limits.quantity_tokens, 32768);
    assert.equal(result.value.coverage.limits.quantity_work, 67108864);
  });
