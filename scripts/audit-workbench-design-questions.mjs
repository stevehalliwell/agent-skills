#!/usr/bin/env node
// Re-run typed Mario question probes and bounded cross-example comparisons. Native C99 owns all semantics.
// Usage: node scripts/audit-workbench-design-questions.mjs --output NEW_DIRECTORY
// Optional: --executable PATH. Refuses existing output; never edits source data.
import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import { existsSync, mkdirSync, readdirSync, readFileSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve, sep } from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';

const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const example = join(root, 'skills/game-design-workbench/examples/little-plumber-guy-nes/game-design');
let executable = join(root, 'skills/game-design-workbench/native/build/gd.exe');
let output;
for (let index = 2; index < process.argv.length; index++) {
  const option = process.argv[index];
  if (option === '--help') {
    console.log('Usage: node scripts/audit-workbench-design-questions.mjs --output NEW_DIRECTORY [--executable PATH]');
    process.exit(0);
  }
  if (!['--output', '--executable'].includes(option) || !process.argv[index + 1] || process.argv[index + 1].startsWith('--')) {
    throw new Error(`Invalid option or missing value: ${option}`);
  }
  const value = resolve(process.argv[++index]);
  if (option === '--output') output = value;
  else executable = value;
}
if (!output || existsSync(output) || !existsSync(executable)) {
  throw new Error('Require new --output directory and existing native --executable. Build native/test.ps1 first.');
}
const hash = bytes => createHash('sha256').update(bytes).digest('hex');
function snapshot(directory) {
  return readdirSync(directory, { withFileTypes: true }).sort((a, b) => a.name.localeCompare(b.name)).flatMap(entry => {
    const path = join(directory, entry.name);
    if (entry.isSymbolicLink()) throw new Error(`Refuse source symlink: ${path}`);
    return entry.isDirectory() ? snapshot(path) : [{ path, sha256: hash(readFileSync(path)) }];
  });
}
if (output.toLowerCase() === example.toLowerCase() || output.toLowerCase().startsWith(example.toLowerCase() + sep)) {
  throw new Error('Output must not be inside authored game-design source.');
}
const before = snapshot(example);
const additionalSources = ['crate-puzzle', 'forge-chain'].map(name => {
  const directory = join(root, 'skills/game-design-workbench/examples', name, 'game-design');
  return { directory, hashes: snapshot(directory) };
});
const nativeDirectory = join(root, 'skills/game-design-workbench/native');
mkdirSync(output, { recursive: true });
const manifest = {
  commit: spawnSync('git', ['rev-parse', 'HEAD'], { cwd: root, encoding: 'utf8' }).stdout.trim(),
  executable, executableSha256: hash(readFileSync(executable)), example, sourceHashes: before, additionalSources,
  nativeSourceHashes: readdirSync(nativeDirectory).filter(name => /\.(c|h|ps1)$/.test(name)).sort().map(name => ({ path: join(nativeDirectory, name), sha256: hash(readFileSync(join(nativeDirectory, name))) })),
  auditScriptSha256: hash(readFileSync(fileURLToPath(import.meta.url))), results: [],
  scope: 'Read-only native queries; no game-fact changes, evaluator duplication, experiment reruns or GUI input.',
};
function probe(name, arguments_) {
  const result = spawnSync(executable, [...arguments_, '--json'], { cwd: root, encoding: 'utf8', timeout: 30000, maxBuffer: 32 * 1024 * 1024 });
  writeFileSync(join(output, `${name}.json`), result.stdout ?? '');
  writeFileSync(join(output, `${name}.stderr.log`), result.stderr ?? '');
  let parsed;
  try { parsed = JSON.parse(result.stdout); } catch { /* Retain malformed stdout for diagnosis. */ }
  manifest.results.push({ name, arguments: [...arguments_, '--json'], exitCode: result.status, ok: parsed?.ok === true, errors: parsed?.errors ?? [], failure: result.error?.message });
  if (result.status !== 0 || parsed?.ok !== true) throw new Error(`${name} failed; inspect retained output.`);
  return parsed;
}
const bindings = names => names.flatMap(name => [`--${name}`, `${name}=${join(example, name)}`]);
const matrix = bindings(['nouns', 'verbs', 'interactions']);
const rules = [...bindings(['interactions', 'mechanics', 'nouns', 'verbs', 'axes', 'resources', 'control-vars']), '--related', `systems=${join(example, 'systems')}`];
const ruleCase = (name, assumptions, context) => [...rules, '--case', name, ...assumptions.flatMap(value => ['--assume', value]), '--context', context, '--fields', 'analysis'];
try {
  probe('query-catalogue', ['query', '--list']);
  const matrixReport = probe('matrix', ['matrix', ...matrix]);
  const connectivity = probe('connectivity', ['connectivity', ...matrix]);
  assert.equal(matrixReport.data.cases.length, 49);
  assert.equal(matrixReport.data.cells.find(row => row.noun === 'nouns/brick' && row.verb === 'verbs/bump').cases.length, 2);
  // Zero flat target-incidence does not mean a noun is absent from conditional rules.
  assert.equal(connectivity.data.nouns.find(row => row.reference === 'nouns/gap').case_count, 0);
  for (const question of ['direct-connections', 'role-shapes', 'adjacency']) {
    const result = probe('matrix-' + question, ['query', '--question', question, '--projection', 'matrix', ...matrix]);
    assert.equal(result.coverage.query_complete, true);
    assert.equal(result.data.scope.incidence, true);
  }
  probe('skills', ['skill-atoms', ...bindings(['skill-atoms']), '--from', 'skill-atoms/fall-stomp', '--known', 'skill-atoms/stomp-walker', '--known', 'skill-atoms/control-fall']);
  probe('scaffolding', ['scaffolding', ...bindings(['skill-atoms', 'challenges'])]);
  probe('mda', ['mda', ...bindings(['mechanics', 'systems', 'dynamics', 'aesthetics']), '--from', 'mechanics/enemy-contact', '--direction', 'forward']);
  probe('rules', ['rules', ...rules]);
  probe('powered-tall', ['rules', ...ruleCase('interactions/touch-walker-powered#powered-side-contact', [
    'Player:axes/Life=alive', 'Player:axes/Form=tall', 'Player:axes/Protection=unprotected', 'Walker:axes/Life=alive',
  ], 'Player axes/Contact.sideContacts Walker')]);
  probe('small-walker', ['rules', ...ruleCase('interactions/touch-walker-small#lethal-side', [
    'Hero:axes/Form=small', 'Hero:axes/Protection=unprotected',
  ], 'Hero axes/Contact.sideContacts Walker')]);
  probe('gap-tall', ['rules', ...ruleCase('interactions/touch-gap-bottom#gap-lethal', ['Hero:axes/Form=tall'], 'Hero axes/Contact.crossesLowerBoundary Gap')]);
  const resources = probe('resources', ['resources', ...rules]);
  const facet = (caseName, participant, axis) => {
    const matches = resources.data.nodes.filter(node => node.kind === 'facet' && node.axis === axis &&
      node.representative_declaration.source.record === 'interactions/touch-coin' &&
      node.representative_declaration.case === caseName && node.representative_declaration.participant === participant);
    if (matches.length !== 1) throw new Error(`Expected unique authored facet: ${caseName}/${participant}/${axis}`);
    return matches[0].id;
  };
  const bank = facet('below-threshold', 'Bank', 'axes/CoinAmount');
  const coin = facet('below-threshold', 'Coin', 'axes/Availability');
  const lives = facet('threshold', 'Lives', 'axes/Amount');
  const score = facet('threshold', 'Score', 'axes/ScoreAmount');
  const query = (name, ...selectors) => probe(name, ['resources', ...rules, ...selectors, '--fields', 'routes']);
  query('coin-upstream', '--to', bank);
  query('coin-enables-credit', '--from', coin, '--to', bank);
  query('coin-accepted-only', '--to', bank, '--accepted-only');
  query('capped-life-frontier', '--to', lives);
  query('cross-case-identity-gap', '--from', bank, '--to', score);
  query('unbound-coin-role', '--to', 'resource_role:resources%2Fcoin-progress');
  // Current surfaces missing from historical baseline. Read-only native queries;
  // successful extraction is not human acceptance of the broader design question.
  probe('hero-bindings', ['query', '--question', 'object-bindings', '--projection', 'actors', ...rules, '--object', 'nouns/hero']);
  probe('hero-form-variants', ['query', '--question', 'actor-states', '--projection', 'actors', ...rules,
    '--case', 'interactions/touch-walker-small#lethal-side', '--participant', 'Hero', '--axis', 'axes/Form']);
  // Q1/Q3: compare native side effects across cases, not just selected-axis arrows.
  // Both enum assumptions apply to one exact Hero binding; contexts/Life/clock stay unknown.
  const heroHazards = probe('hero-form-protection-hazards', ['query', '--question', 'state-chart', '--projection', 'actors', ...rules,
    '--object', 'nouns/hero', '--axis', 'axes/Form', '--with-axis', 'axes/Protection']);
  assert.equal(heroHazards.coverage.state_query_complete, true);
  assert.equal(heroHazards.coverage.state_chart.retention_limited, false);
  assert.equal(heroHazards.coverage.state_chart.unretained_hypotheses, 0);
  assert.equal(heroHazards.coverage.final_outcomes_proven, false);
  const hazardScenario = (record, caseName, form, protection) => {
    const matches = heroHazards.data.scenarios.filter(row => row.source.record === record && row.source.case === caseName &&
      row.actor.participant === 'Hero' && row.from === form && row.companion_value === protection);
    assert.equal(matches.length, 1); return matches[0];
  };
  const proposal = (scenario, participant, axis, kind, state) => scenario.proposals.some(row =>
    row.affected.participant === participant && row.proposal.axis === axis && row.proposal.kind === kind &&
    (state === undefined || row.proposal.state === state));
  for (const form of ['small', 'tall', 'spark']) for (const protection of ['unprotected', 'protected']) {
    const selectedCase = protection === 'protected' ? 'walker-protected' : form === 'small' ? 'walker-small' : 'walker-powered';
    for (const caseName of ['walker-small', 'walker-powered', 'walker-protected']) {
      const scenario = hazardScenario('mechanics/enemy-contact', caseName, form, protection);
      if (caseName !== selectedCase) { assert.equal(scenario.applicability, 'false'); assert.deepEqual(scenario.proposals, []); continue; }
      assert.equal(scenario.applicability, 'unknown');
      assert(scenario.proposals.every(row => row.proposal.conditional));
      if (caseName === 'walker-small') assert(proposal(scenario, 'Hero', 'axes/Life', 'state', 'dead'));
      if (caseName === 'walker-protected') {
        assert(proposal(scenario, 'Hero', 'axes/Life', 'preserve'));
        assert(proposal(scenario, 'Hero', 'axes/Form', 'preserve', form));
      }
      if (caseName === 'walker-powered') {
        assert(proposal(scenario, 'Hero', 'axes/Form', 'state', 'small'));
        assert(proposal(scenario, 'Hero', 'axes/Protection', 'state', 'protected'));
        assert(proposal(scenario, 'Lives', 'axes/Amount', 'preserve'));
        assert(!scenario.proposals.some(row => row.affected.participant === 'Hero' && row.proposal.axis === 'axes/Life'));
      }
    }
    for (const [record, caseName] of [['interactions/touch-gap-bottom', 'gap-lethal'], ['mechanics/active-clock', 'expiry']]) {
      const scenario = hazardScenario(record, caseName, form, protection);
      assert.equal(scenario.applicability, 'unknown');
      assert(proposal(scenario, 'Hero', 'axes/Life', 'state', 'dead'));
      assert(scenario.proposals.some(row => row.affected.participant === 'Lives' && row.proposal.expression === 'this - 1[life]'));
      assert.deepEqual(scenario.transitions, []); // No Form arrow does not mean no Life effect.
      assert(scenario.proposals.every(row => row.proposal.conditional));
    }
  }
  // Same flat noun/verb pair contains materially different authored state responses.
  assert(proposal(hazardScenario('mechanics/brick-response', 'small-bump', 'small', 'unprotected'), 'Brick', 'axes/BrickState', 'preserve'));
  for (const form of ['tall', 'spark'])
    assert(proposal(hazardScenario('mechanics/brick-response', 'powered-break', form, 'unprotected'), 'Brick', 'axes/BrickState', 'state', 'destroyed'));
  probe('shellback-state-chart', ['query', '--question', 'state-chart', '--projection', 'actors', ...rules,
    '--object', 'nouns/shellback', '--axis', 'axes/ShellState']);
  probe('shellback-joint-state-chart', ['query', '--question', 'state-chart', '--projection', 'actors', ...rules,
    '--object', 'nouns/shellback', '--axis', 'axes/ShellState', '--with-axis', 'axes/Life']);
  // Q8: environmental opportunity stays on exact local Block; no pool or spawn inferred.
  const supply = probe('environmental-supply-state-chart', ['query', '--question', 'state-chart', '--projection', 'actors', ...rules,
    '--object', 'nouns/supply-block', '--axis', 'axes/SupplyState']);
  assert(supply.data.scenarios.some(scenario => scenario.from === 'unspent' &&
    scenario.transitions.some(transition => transition.to === 'spent' && transition.conditional)));
  assert(supply.data.cases.some(entry => entry.declaration.case === 'growth-small' &&
    entry.declaration.unknown_effects.includes('spawn geometry/motion unconfigured')));
  const spent = probe('environmental-supply-spent', ['query', '--question', 'state-chart', '--projection', 'actors', ...rules,
    '--object', 'nouns/supply-block', '--axis', 'axes/SupplyState',
    '--case', 'mechanics/supply-release#spent-no-payout', '--participant', 'Block']);
  const unspentHypothesis = spent.data.scenarios.find(scenario => scenario.from === 'unspent');
  const spentHypothesis = spent.data.scenarios.find(scenario => scenario.from === 'spent');
  assert.equal(unspentHypothesis.applicability, 'false');
  assert.equal(unspentHypothesis.transitions.length, 0);
  assert(spentHypothesis.transitions.some(transition => transition.kind === 'preserve' && transition.to === 'spent'));
  const growth = probe('environmental-supply-growth', ['rules', ...ruleCase('mechanics/supply-release#growth-small', [
    'Hero:axes/Motion=rising', 'Hero:axes/Form=small',
    'Block:axes/SupplyState=unspent', 'Block:axes/SupplyContent=growth',
  ], 'Hero axes/Contact.undersideContacts Block')]);
  assert.equal(growth.data.analysis.scenario.context, 'true');
  assert.equal(growth.data.analysis.scenario.applicability, 'unknown');
  assert(growth.data.analysis.scenario.writes.some(write => write.actor.participant === 'Block' &&
    write.axis === 'axes/SupplyState' && write.state === 'spent' && write.conditional));
  probe('death-contributors', ['query', '--question', 'effect-contributors', '--projection', 'effects', ...rules, '--axis', 'axes/Life', '--value', 'dead']);
  const deaths = probe('hero-death-contributors', ['query', '--question', 'effect-contributors', '--projection', 'effects', ...rules,
    '--axis', 'axes/Life', '--value', 'dead', '--object', 'nouns/hero']);
  assert.equal(deaths.data.contributors.length, 11);
  assert(deaths.data.contributors.every(row => row.affected.noun_reference === 'nouns/hero'));
  assert.equal(deaths.data.contributors.filter(row => row.via.some(step => step.instantiated)).length, 2);
  assert(deaths.data.cases.some(row => row.declaration.unknown_conditions?.includes('deduplication')));
  const mdaBindings = bindings(['mechanics', 'systems', 'dynamics', 'aesthetics']);
  probe('mda-dominators', ['query', '--question', 'structural-dominators', '--projection', 'mda', ...mdaBindings, '--start', 'mechanics/enemy-contact']);
  probe('mda-dominators-backward', ['query', '--question', 'structural-dominators', '--projection', 'mda', ...mdaBindings,
    '--start', 'mechanics/enemy-contact', '--direction', 'backward']);
  probe('mda-declaration-gaps', ['query', '--question', 'declaration-gaps', '--projection', 'mda', ...mdaBindings]);
  const features = probe('mda-similarity-features', ['query', '--question', 'similarity-features', '--projection', 'mda', ...mdaBindings]);
  const selectedFeatures = ['retained-partners', 'source-loaded-role'];
  for (const id of selectedFeatures) {
    if (!features.data.features.some(feature => feature.id === id)) throw new Error(`Missing advertised similarity feature: ${id}`);
  }
  probe('mda-similarity-map', ['query', '--question', 'similarity-map', '--projection', 'mda', ...mdaBindings,
    ...selectedFeatures.flatMap(id => ['--feature', `${id}=1`])]);
  const assessments = probe('decision-assessments', ['query', '--question', 'decision-assessments', '--projection', 'challenges', ...bindings(['challenges'])]);
  assert.equal(assessments.data.summary.source_records, 19);
  assert.equal(assessments.data.summary.retained_assessments, 14);
  assert.equal(assessments.data.summary.retained_demand_entries, 11);
  const withLanding = probe('skill-and-with-landing', ['skill-atoms', ...bindings(['skill-atoms']),
    '--from', 'skill-atoms/fall-stomp', '--known', 'skill-atoms/stomp-walker', '--known', 'skill-atoms/control-fall', '--known', 'skill-atoms/land-on-target']);
  for (const reference of ['skill-atoms/fall-stomp', 'skill-atoms/jump-stomp'])
    assert.equal(withLanding.data.exploration.skills.find(row => row.reference === reference).eligibility, 'all_declared_prerequisites_supplied');
  // Full loaded declaration context. Incident role means field + endpoint side, never enemy/ally.
  const relationships = [...bindings(['skill-atoms', 'challenges', 'mechanics', 'nouns', 'verbs', 'interactions',
    'axes', 'systems', 'dynamics', 'aesthetics', 'resources', 'control-vars']),
    ...['levels', 'experiments', 'glossary'].flatMap(name => ['--related', `${name}=${join(example, name)}`])];
  for (const [name, from, to, lens] of [
    ['gap-same-role', 'challenges/first-gap', 'challenges/final-gap', 'same-role'],
    ['gap-changed-role', 'challenges/first-gap', 'challenges/final-gap', 'changed-role'],
    ['landing-changed-role', 'challenges/pipe-steps', 'challenges/first-gap', 'changed-role'],
    ['stomp-methods', 'skill-atoms/fall-stomp', 'skill-atoms/jump-stomp', 'any'],
    ['supply-encounters', 'challenges/first-supply', 'challenges/flower-shelf', 'any'],
    ['coin-encounters', 'challenges/coin-cellar', 'challenges/coin-bridge', 'any'],
    ['ending-routes', 'challenges/high-stair', 'challenges/low-path', 'any'],
  ]) {
    const result = probe('relationships-' + name, ['query', '--question', 'related-structure', '--projection', 'relationships',
      ...relationships, '--from', from, '--to', to, '--lens', lens]);
    assert.equal(result.coverage.query_complete, true);
    // "shared" means qualifies under chosen lens; changed-role matches are shared here.
    if (name === 'gap-changed-role') assert.equal(result.data.evidence.comparison.shared, 1);
    if (name === 'landing-changed-role') assert.equal(result.data.evidence.comparison.shared, 2);
  }
  const challengeBindings = bindings(['skill-atoms', 'challenges']); // Both required by advertised projection contract.
  const route = probe('challenge-route-connections', ['query', '--question', 'direct-connections', '--projection', 'challenges', ...challengeBindings]);
  assert.equal(route.coverage.query_complete, true);
  const routeFeatures = probe('challenge-similarity-features', ['query', '--question', 'similarity-features', '--projection', 'challenges', ...challengeBindings]);
  assert(routeFeatures.data.features.some(row => row.id === 'retained-partners'));
  probe('challenge-similarity-map', ['query', '--question', 'similarity-map', '--projection', 'challenges', ...challengeBindings,
    '--feature', 'retained-partners=1', '--feature', 'source-loaded-role=1',
    '--entry', 'challenges/first-gap', '--entry', 'challenges/final-gap', '--from', 'challenges/first-gap', '--to', 'challenges/final-gap']);
  const crate = additionalSources[0].directory;
  const crateBindings = names => names.flatMap(name => ['--' + name, `${name}=${join(crate, name)}`]);
  probe('crate-scaffolding', ['scaffolding', ...crateBindings(['skill-atoms', 'challenges'])]);
  const crateAssessments = probe('crate-assessments', ['query', '--question', 'decision-assessments', '--projection', 'challenges', ...crateBindings(['challenges'])]);
  assert.equal(crateAssessments.data.summary.source_records, 4);
  assert.equal(crateAssessments.data.summary.retained_assessments, 1);
  probe('crate-encounter-structure', ['query', '--question', 'related-structure', '--projection', 'relationships',
    ...crateBindings(['skill-atoms', 'challenges', 'mechanics', 'nouns', 'verbs', 'interactions']),
    '--from', 'challenges/push-room', '--to', 'challenges/combined-room']);
  // Select only native-typed declarations; do not infer causality, applicability or identity.
  const declarations = resources.data.relations;
  writeFileSync(join(output, 'declaration-index.json'), `${JSON.stringify({
    meaning: 'Source declaration inventory, not composed outcomes, applicable actions, frequency or balance.',
    explicitPoolFacets: resources.data.nodes.filter(node => node.identity_scope === 'explicit_pool_axis').length,
    deathStateWrites: declarations.filter(edge => edge.kind === 'state_write' && edge.target?.axis === 'axes/Life' && edge.target.state === 'dead'),
    knownQuantityWrites: declarations.filter(edge => edge.kind === 'quantity_write' && edge.delta?.known),
    unknownQuantityWrites: declarations.filter(edge => edge.kind === 'quantity_write' && !edge.delta?.known),
    guardedInvocations: declarations.filter(edge => edge.kind === 'invocation'),
  }, null, 2)}\n`);
} catch (error) {
  manifest.failure = error.message;
  process.exitCode = 1;
} finally {
  manifest.examplesUnchanged = JSON.stringify(before) === JSON.stringify(snapshot(example)) &&
    additionalSources.every(source => JSON.stringify(source.hashes) === JSON.stringify(snapshot(source.directory)));
  if (!manifest.examplesUnchanged) process.exitCode = 1;
  writeFileSync(join(output, 'metadata.json'), `${JSON.stringify(manifest, null, 2)}\n`);
  console.log(JSON.stringify({ output, successfulProbes: manifest.results.filter(result => result.ok).length, examplesUnchanged: manifest.examplesUnchanged, failure: manifest.failure ?? null }));
}
