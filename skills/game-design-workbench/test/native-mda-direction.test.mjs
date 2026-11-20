// CLI help/discovery and sourced MDA traversal parity; no causal or player-evidence inference.
// Run with GD_NATIVE_EXE pointing to current CLI. All inspection is source-preserving.
import test from 'node:test';
import assert from 'node:assert/strict';
import { readFile, readdir } from 'node:fs/promises';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';

const executable = process.env.GD_NATIVE_EXE && resolve(process.env.GD_NATIVE_EXE);
const home = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const game = join(home, 'examples/little-plumber-guy-nes/game-design');
const bindings = ['mechanics', 'systems', 'dynamics', 'aesthetics'].flatMap(role =>
  ['--' + role, `${role}=${join(game, role)}`]);
const root = 'mechanics/enemy-contact';
function invoke(args, status = 0) {
  const result = spawnSync(executable, args, { encoding: 'utf8', timeout: 30000,
    maxBuffer: 16 * 1024 * 1024, windowsHide: true, env: { ...process.env, GD_AGENT: '1' } });
  assert.equal(result.status, status, result.stderr + '\n' + result.stdout);
  if (status === 0) assert.equal(result.stderr, '');
  return result.stdout;
}
function report(args) {
  const value = JSON.parse(invoke([...args, '--json']));
  assert.equal(value.ok, true);
  return value;
}
const domain = direction => report(['mda', ...bindings, '--from', root, '--direction', direction]);
const structural = direction => report(['query', '--question', 'structural-dominators',
  '--projection', 'mda', ...bindings, '--from', root, '--direction', direction]);
const domainIds = value => value.data.nodes.map(node => node.reference).sort();
const structuralIds = value => value.data.evidence.path_nodes.filter(node => node.reachable)
  .map(node => value.data.nodes.find(row => row.index === node.index).id).sort();
async function snapshot(folder) {
  const result = [];
  for (const entry of (await readdir(folder, { withFileTypes: true })).sort((a, b) => a.name.localeCompare(b.name))) {
    assert.equal(entry.isSymbolicLink(), false, entry.name);
    const path = join(folder, entry.name);
    if (entry.isDirectory()) result.push(...await snapshot(path));
    else result.push([path, (await readFile(path)).toString('base64')]);
  }
  return result;
}

test('MDA help and discovery distinguish domain direction from structural endpoints and incidence', { skip: !executable }, () => {
  const help = invoke(['--help']);
  assert.match(help, /Structural forward follows projected link endpoints/);
  assert.match(help, /mda --from mechanics\/enemy-contact --direction forward/);
  assert.match(help, /--direction backward follows those incoming basis links/);
  assert.match(help, /Matrix\/Resources use undirected incidence/);
  const catalogue = report(['query', '--list']);
  for (const name of ['reachability', 'structural-dominators', 'support-bypass', 'related-paths']) {
    const question = catalogue.data.questions.find(row => row.name === name);
    assert.match(question.direction_semantics, /structural backward.*domain mda --direction forward/);
    assert.match(question.direction_semantics, /mechanics\/enemy-contact/);
    assert.match(question.direction_semantics, /Matrix\/Resources remain undirected incidence, not flow/);
  }
});

test('domain forward matches structural backward on unchanged MDA endpoints, not causal direction', { skip: !executable }, async () => {
  const before = await snapshot(game);
  try {
    const forward = domain('forward'), backward = domain('backward');
    const structuralForward = structural('forward'), structuralBackward = structural('backward');
    assert.equal(forward.coverage.selected_node_count, 9);
    assert.equal(forward.coverage.total_node_count, 44);
    assert.equal(forward.coverage.total_link_count, 74);
    assert.deepEqual(domainIds(forward), structuralIds(structuralBackward));
    assert.deepEqual(domainIds(backward), structuralIds(structuralForward));
    assert.deepEqual(domainIds(backward), [root]);
    assert.equal(structuralBackward.coverage.query_complete, true);
    assert.equal(structuralForward.coverage.final_outcomes_proven, false);
    assert.deepEqual(structuralForward.data.links, structuralBackward.data.links);
    assert.match(forward.coverage.scope, /not causation, observed dynamics, emotion/);
  } finally { assert.deepEqual(await snapshot(game), before); }
});

test('sourced explanation retains accepted rules, proposed prediction/intention and raw basis lines', { skip: !executable }, async () => {
  const value = domain('forward');
  for (const [reference, status] of [[root, 'accepted'], ['systems/power-ladder', 'accepted'],
    ['dynamics/power-protection', 'proposed'], ['aesthetics/manageable-tension', 'proposed']]) {
    assert.equal(value.data.nodes.find(node => node.reference === reference).status, status);
  }
  for (const [source, target, field] of [['systems/power-ladder', root, 'mechanics'],
    ['dynamics/power-protection', 'systems/power-ladder', 'systems'],
    ['aesthetics/manageable-tension', 'dynamics/power-protection', 'dynamics']]) {
    const link = value.data.links.find(row => row.source === source && row.target === target && row.field === field);
    assert.ok(link);
    assert.equal(link.resolution, 'resolved');
    const lines = (await readFile(link.source_path, 'utf8')).split(/\r?\n/);
    assert.equal(lines[link.line - 1].trim(), '- ' + target);
  }
  const path = value.data.paths.find(row => row.to === 'aesthetics/manageable-tension');
  assert.deepEqual(path.records, [root, 'dynamics/power-protection', 'aesthetics/manageable-tension']);
  assert.ok(path.steps.every(step => step.reversed));
  const prediction = await readFile(join(game, 'dynamics/power-protection.md'), 'utf8');
  assert.match(prediction, /Neither cautious nor aggressive behavior is an observed fact/);
  const experiment = await readFile(join(game, 'experiments/first-course-comprehension.md'), 'utf8');
  assert.match(experiment, /^stage: proposed$/m);
  assert.match(experiment, /None\. No build, participants, generated program or execution exists/);
});

test('MDA explicit text/automatic JSON output and argument failures retain existing contracts', { skip: !executable }, () => {
  const automatic = JSON.parse(invoke(['mda', ...bindings, '--from', root, '--direction', 'forward']));
  assert.equal(automatic.ok, true);
  assert.equal(automatic.coverage.selected_node_count, 9);
  const text = invoke(['mda', ...bindings, '--from', root, '--direction', 'forward', '--format', 'text']);
  assert.ok(text.includes(root));
  const failure = JSON.parse(invoke(['mda', ...bindings, '--direction', 'wrong', '--json'], 2));
  assert.equal(failure.ok, false);
  assert.equal(failure.errors[0].code, 'cli.arguments');
});
