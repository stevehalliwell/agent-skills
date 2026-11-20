// Inspect shipped source through native consumers; no JS rule evaluator or gameplay simulation.
// Run: node --test skills/game-design-workbench/test/forge-chain.test.mjs
// Optional integrations: GD_NATIVE_EXE and GD_ATTENDANT_EXE (explicit native executable paths).
import test from 'node:test';
import assert from 'node:assert/strict';
import { cp, mkdtemp, readFile, readdir, rm, writeFile } from 'node:fs/promises';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';

const home = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const example = join(home, 'examples/forge-chain');
const game = join(example, 'game-design');
const native = process.env.GD_NATIVE_EXE && resolve(process.env.GD_NATIVE_EXE);
const attendant = process.env.GD_ATTENDANT_EXE && resolve(process.env.GD_ATTENDANT_EXE);
const roles = ['nouns', 'axes', 'mechanics', 'resources', 'control-vars', 'systems'];
const text = path => readFile(path, 'utf8');

async function snapshot(folder) {
  const entries = (await readdir(folder, { withFileTypes: true })).sort((a, b) => a.name.localeCompare(b.name));
  const result = [];
  for (const entry of entries) {
    assert.equal(entry.isSymbolicLink(), false, entry.name);
    const path = join(folder, entry.name);
    if (entry.isDirectory()) result.push(...await snapshot(path));
    else result.push([path, (await readFile(path)).toString('base64')]);
  }
  return result;
}
async function temporary(t) {
  const root = await mkdtemp(join(tmpdir(), 'gd-forge-chain-'));
  t.after(() => rm(root, { recursive: true, force: true }));
  return root;
}
function invoke(executable, args) {
  const result = spawnSync(executable, args, {
    encoding: 'utf8', timeout: 30000, maxBuffer: 16 * 1024 * 1024,
    windowsHide: true, env: { ...process.env, GD_AGENT: '1' },
  });
  assert.equal(result.status, 0, result.stderr + '\n' + result.stdout);
  assert.equal(result.stderr, '');
  return JSON.parse(result.stdout);
}
function bindings(folder) {
  return [...['mechanics', 'nouns', 'axes', 'resources', 'control-vars'].flatMap(role =>
    ['--' + role, `${role}=${join(folder, role)}`]), '--related', `systems=${join(folder, 'systems')}`];
}
function resources(folder, extra = []) {
  const result = invoke(native, ['resources', ...bindings(folder), ...extra, '--json']);
  assert.equal(result.ok, true);
  assert.equal(result.coverage.complete, false);
  assert.deepEqual(result.data.findings ?? [], []);
  return result;
}
function structure(folder, question, extra = []) {
  const result = invoke(native, ['query', '--question', question, '--projection', 'resources',
    ...bindings(folder), ...extra, '--json']);
  assert.equal(result.ok, true);
  assert.equal(result.data.scope.incidence, true);
  assert.equal(result.data.scope.walk_direction, 'undirected incidence');
  assert.equal(result.coverage.final_outcomes_proven, false);
  return result;
}
function facet(report, pool) {
  const matches = report.data.nodes.filter(node => node.kind === 'facet' && node.pool === 'resources/' + pool);
  assert.equal(matches.length, 1, pool);
  assert.equal(matches[0].identity_scope, 'explicit_pool_axis');
  return matches[0].id;
}
function oreToKey(folder, overview) {
  return resources(folder, ['--from', facet(overview, 'ore-stock'), '--to', facet(overview, 'key-stock'),
    '--accepted-only', '--fields', 'routes']).data.routes;
}

test('forge-chain ships six readable partial-model contracts and 24 populated records', async () => {
  const expected = { nouns: 6, axes: 5, mechanics: 2, resources: 4, 'control-vars': 6, systems: 1 };
  const registry = await text(join(example, '.pi/attendant.tables'));
  const identities = new Set();
  for (const role of roles) {
    assert.ok(registry.includes(`game-design/${role} as ${role}`));
    for (const contract of ['.schema.md', '.usage.md', '.template.md'])
      assert.ok((await text(join(game, role, contract))).trim(), role + '/' + contract);
    const records = (await readdir(join(game, role))).filter(name => !name.startsWith('.') && name.endsWith('.md'));
    assert.equal(records.length, expected[role], role);
    const template = (await text(join(game, role, '.template.md'))).trim();
    for (const name of records) {
      const source = await text(join(game, role, name));
      const match = source.match(/^---\r?\n([\s\S]*?)\r?\n---\r?\n([\s\S]*)$/);
      assert.ok(match, name);
      assert.ok(match[2].trim() && match[2].trim() !== template, name + ': unfilled body');
      assert.doesNotMatch(match[1], /^\s*\{|:\s*\[\s*[^\]\s]/m, name + ': nonempty flow container');
      const id = match[1].match(/^"?id"?: "([0-9a-f-]{36})"$/m)?.[1];
      assert.ok(id && !identities.has(id), name + ': missing/duplicate identity');
      identities.add(id);
    }
  }
  assert.equal(identities.size, 24);
  assert.match(await text(join(game, 'concept.md')), /^Working mode: game-explorer$/m);
  assert.doesNotMatch(await text(join(game, 'mechanics/.schema.md')), /^verbs:/m);
});

test('forge-chain controls and native rule bindings resolve without changing authored bytes', { skip: !native }, async () => {
  const before = await snapshot(game);
  try {
    const controls = invoke(native, ['controls', '--control-vars', `control-vars=${join(game, 'control-vars')}`, '--json']);
    assert.equal(controls.ok, true);
    const expected = { 'ore-per-smelt': 2, 'fuel-per-smelt': 1, 'ingots-per-smelt': 1,
      'ingots-per-key': 1, 'fuel-per-key': 1, 'keys-per-forge': 1 };
    assert.equal(controls.data.controls.length, 6);
    for (const [name, value] of Object.entries(expected))
      assert.equal(controls.data.controls.find(row => row.reference === 'control-vars/' + name).value, value);
    const rules = invoke(native, ['rules', ...bindings(game), '--json']);
    assert.equal(rules.ok, true);
    assert.equal(rules.data.cases.length, 2);
    assert.equal(rules.coverage.pool_bindings_complete, true);
    assert.equal(rules.coverage.quantity_bindings_complete, true);
    assert.equal(rules.coverage.preservation_bindings_complete, true);
    assert.deepEqual(rules.data.findings, []);
  } finally { assert.deepEqual(await snapshot(game), before); }
});

test('ore-to-key witness retains shared intermediate, two fuel debits and nonconsumed readiness guards', { skip: !native }, async () => {
  const before = await snapshot(game);
  try {
    const overview = resources(game);
    assert.equal(overview.data.nodes.length, 12);
    assert.equal(overview.data.relations.length, 18);
    assert.equal(overview.data.nodes.filter(node => node.identity_scope === 'explicit_pool_axis').length, 4);
    const routes = oreToKey(game, overview);
    assert.equal(routes.status, 'conditional_witnesses');
    assert.equal(routes.limited, false);
    assert.equal(routes.witnesses.length, 1);
    const witness = routes.witnesses[0];
    assert.equal(witness.conditional, true);
    assert.equal(witness.feasibility, 'not_checked');
    assert.equal(witness.unresolved, false);
    assert.equal(witness.proposal_reliance, false);
    assert.deepEqual(witness.steps.map(step => step.meaning), [
      'checked_debit', 'checked_credit_not_final_outcome', 'checked_debit', 'checked_credit_not_final_outcome',
    ]);
    assert.deepEqual(witness.steps.map(step => step.declaration.delta.number), [-2, 1, -1, 1]);
    assert.equal(witness.nodes[2], facet(overview, 'ingot-stock'));
    assert.equal(routes.activations.length, 2);
    for (const [name, station] of [['smelt', 'Furnace'], ['forge', 'Forge']]) {
      const activation = routes.activations.find(row => row.whole_case_guards.name === name);
      assert.ok(activation);
      const guard = activation.whole_case_guards.participants.find(row => row.name === station);
      assert.equal(guard.requires, 'ready');
      assert.equal(guard.preserve, 'Readiness');
      const fuel = activation.whole_case_guards.participants.find(row => row.name === 'Fuel');
      assert.equal(fuel.requires, name === 'smelt'
        ? 'FuelAmount >= {control-vars/fuel-per-smelt}' : 'FuelAmount >= {control-vars/fuel-per-key}');
      const material = activation.whole_case_guards.participants.find(row => row.name === (name === 'smelt' ? 'Ore' : 'Ingot'));
      assert.equal(material.requires, name === 'smelt'
        ? 'OreAmount >= {control-vars/ore-per-smelt}' : 'IngotAmount >= {control-vars/ingots-per-key}');
      const side = activation.all_direct_relations_including_side_inputs;
      assert.ok(side.some(row => row.kind === 'quantity_write' && row.facet === facet(overview, 'fuel-stock') && row.delta.number === -1));
      assert.ok(side.some(row => row.kind === 'requirement' && row.target.axis === 'axes/Readiness'));
      assert.ok(side.some(row => row.kind === 'preservation' && row.target.axis === 'axes/Readiness'));
      assert.ok(!side.some(row => row.kind === 'quantity_write' && row.target.axis === 'axes/Readiness'));
    }
  } finally { assert.deepEqual(await snapshot(game), before); }
});

test('different explicit ingot pool does not join despite same noun, axis and participant label', { skip: !native }, async t => {
  const before = await snapshot(game);
  const root = await temporary(t);
  // Native-only fixture, not a managed project or checked-in alternative.
  await cp(game, root, { recursive: true });
  await writeFile(join(root, 'resources/ingot-reserve.md'),
    '---\nstatus: proposed\nnoun: nouns/ingot\n---\nSeparate proposed reserve pool for identity control.\n', 'utf8');
  const file = join(root, 'mechanics/forge.md');
  const source = await text(file);
  const binding = 'IngotAmount: resources/ingot-stock';
  assert.equal(source.split(binding).length, 2);
  await writeFile(file, source.replace(binding, 'IngotAmount: resources/ingot-reserve'), 'utf8');
  try {
    const overview = resources(root);
    const shared = facet(overview, 'ingot-stock');
    const reserve = facet(overview, 'ingot-reserve');
    assert.notEqual(shared, reserve);
    assert.ok(overview.data.nodes.some(node => node.id === reserve && node.axis === 'axes/IngotAmount'));
    const forge = overview.data.nodes.find(node => node.kind === 'case' && node.declaration.name === 'forge');
    const ingot = forge.declaration.participants.find(row => row.name === 'Ingot');
    assert.equal(ingot.noun, 'nouns/ingot');
    assert.equal(ingot.pools[0].identity_resolution, 'resolved');
    const routes = oreToKey(root, overview);
    assert.equal(routes.status, 'no_known_direct_witness');
    assert.equal(routes.limited, false);
    assert.deepEqual(routes.witnesses, []);
    // Shared fuel still connects incidence even though typed conversion witness is absent.
    const incidence = structure(root, 'reachability', [
      '--from', facet(overview, 'ore-stock'), '--to', facet(overview, 'key-stock'),
    ]);
    assert.equal(incidence.coverage.query_complete, true);
    assert.ok(incidence.data.evidence.support_length > 0);
  } finally { assert.deepEqual(await snapshot(game), before); }
});

test('unbound identical Ingot labels retain separate local facets rather than inferred inventory', { skip: !native }, async t => {
  const before = await snapshot(game);
  const root = await temporary(t);
  await cp(game, root, { recursive: true });
  for (const name of ['smelt', 'forge']) {
    const file = join(root, 'mechanics', name + '.md');
    const source = await text(file);
    const binding = '        pools:\n            IngotAmount: resources/ingot-stock\n';
    assert.equal(source.split(binding).length, 2);
    await writeFile(file, source.replace(binding, ''), 'utf8');
  }
  try {
    const overview = resources(root);
    const local = overview.data.nodes.filter(node => node.kind === 'facet' && node.axis === 'axes/IngotAmount');
    assert.equal(local.length, 2);
    assert.ok(local.every(node => node.pool === null && node.identity_scope === 'case_participant_axis'));
    assert.notEqual(local[0].id, local[1].id);
    const routes = oreToKey(root, overview);
    assert.equal(routes.status, 'no_known_direct_witness');
    assert.equal(routes.limited, false);
    assert.deepEqual(routes.witnesses, []);
  } finally { assert.deepEqual(await snapshot(game), before); }
});

test('declared uses and source witnesses do not become role-quality scores or alternative key recipes', { skip: !native }, async () => {
  const before = await snapshot(game);
  try {
    const overview = resources(game);
    for (const [pool, debits, credits] of [['ore-stock', 1, 0], ['fuel-stock', 2, 0],
      ['ingot-stock', 1, 1], ['key-stock', 0, 1]]) {
      const writes = overview.data.relations.filter(row => row.kind === 'quantity_write' && row.facet === facet(overview, pool));
      assert.ok(writes.every(row => row.delta.known && row.delta.checks_complete));
      assert.equal(writes.filter(row => row.delta.number < 0).length, debits, pool);
      assert.equal(writes.filter(row => row.delta.number > 0).length, credits, pool);
    }
    const fuel = resources(game, ['--from', facet(overview, 'fuel-stock'), '--to', facet(overview, 'key-stock'),
      '--accepted-only', '--fields', 'routes']).data.routes;
    assert.equal(fuel.limited, false);
    assert.equal(fuel.witnesses.length, 2);
    const forge = fuel.activations.find(row => row.whole_case_guards.name === 'forge');
    assert.ok(fuel.witnesses.every(witness => witness.nodes.includes(forge.id) && witness.feasibility === 'not_checked'));
    assert.deepEqual(fuel.witnesses.map(witness => witness.steps.length).sort((a, b) => a - b), [2, 4]);
    const keyCredits = overview.data.relations.filter(row => row.kind === 'quantity_write' &&
      row.facet === facet(overview, 'key-stock') && row.delta.number > 0);
    assert.equal(keyCredits.length, 1);
    assert.equal(keyCredits[0].source.record, 'mechanics/forge');
    assert.ok(forge.whole_case_guards.participants.some(row => row.name === 'Ingot' && row.requires.includes('ingots-per-key')));
  } finally { assert.deepEqual(await snapshot(game), before); }
});

test('incidence counts and pivotal paths remain distinct from directed resource witnesses', { skip: !native }, async () => {
  const before = await snapshot(game);
  try {
    const overview = resources(game);
    const from = facet(overview, 'ore-stock'), to = facet(overview, 'key-stock');
    const connections = structure(game, 'direct-connections');
    const metadata = connections.data.nodes.filter(node => node.id.startsWith('resource_role:'));
    assert.equal(metadata.length, 4);
    assert.ok(metadata.every(node => node.counts.distinct_partners === 1));
    const dominators = structure(game, 'structural-dominators', ['--from', from, '--to', to]);
    assert.equal(dominators.coverage.query_complete, true);
    const node = id => dominators.data.nodes.find(row => row.id === id).index;
    const ingot = facet(overview, 'ingot-stock'), fuel = facet(overview, 'fuel-stock');
    for (const id of [ingot, fuel])
      assert.equal(dominators.data.evidence.path_nodes.find(row => row.index === node(id)).dominates_target, false);
    for (const name of ['smelt', 'forge']) {
      const recipe = overview.data.nodes.find(row => row.kind === 'case' && row.declaration.name === name);
      assert.equal(dominators.data.evidence.path_nodes.find(row => row.index === node(recipe.id)).dominates_target, true);
    }
    // Inspect native-selected path rather than locking a particular tie-break order.
    const supportNodes = new Set(dominators.data.evidence.support_suffix.flatMap(arc => {
      const link = dominators.data.links.find(row => row.index === arc.link);
      return [link.from, link.to];
    }));
    const candidates = [fuel, ingot].filter(id => supportNodes.has(node(id)));
    assert.equal(candidates.length, 1);
    const bypass = structure(game, 'support-bypass', ['--from', from, '--to', to, '--avoid', candidates[0]]);
    assert.equal(bypass.data.evidence.bypass.valid, true);
    assert.equal(bypass.data.evidence.bypass.complete, true);
    assert.equal(bypass.data.evidence.bypass.found, true);
    // Native bypass only handles nonmandatory candidates on its selected support path.
    const offPath = [fuel, ingot].find(id => id !== candidates[0]);
    const unavailable = structure(game, 'support-bypass', ['--from', from, '--to', to, '--avoid', offPath]);
    assert.equal(unavailable.data.evidence.bypass, null);
    assert.match(unavailable.data.evidence.bypass_unavailable_reason, /selected support-path candidate/);
    assert.equal(oreToKey(game, overview).witnesses.length, 1);
  } finally { assert.deepEqual(await snapshot(game), before); }
});

test('public Attendant strictly validates all 24 records in a temporary managed copy', { skip: !attendant }, async t => {
  const root = await temporary(t);
  await cp(game, join(root, 'game-design'), { recursive: true });
  await cp(join(example, '.pi'), join(root, '.pi'), { recursive: true });
  const before = await snapshot(join(root, 'game-design'));
  const value = invoke(attendant, ['validate', '--strict', '-p', root]);
  assert.equal(value.records, 24);
  assert.deepEqual(value.diagnostics, []);
  assert.deepEqual(value.corrections, []);
  assert.deepEqual(await snapshot(join(root, 'game-design')), before);
});
