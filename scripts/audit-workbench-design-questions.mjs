#!/usr/bin/env node
// Re-run source-backed Mario question probes. Native C99 owns all semantics.
// Usage: node scripts/audit-workbench-design-questions.mjs --output NEW_DIRECTORY
// Optional: --executable PATH. Refuses existing output; never edits source data.
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
const nativeDirectory = join(root, 'skills/game-design-workbench/native');
mkdirSync(output, { recursive: true });
const manifest = {
  commit: spawnSync('git', ['rev-parse', 'HEAD'], { cwd: root, encoding: 'utf8' }).stdout.trim(),
  executable, executableSha256: hash(readFileSync(executable)), example, sourceHashes: before,
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
  probe('matrix', ['matrix', ...matrix]);
  probe('connectivity', ['connectivity', ...matrix]);
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
  manifest.examplesUnchanged = JSON.stringify(before) === JSON.stringify(snapshot(example));
  if (!manifest.examplesUnchanged) process.exitCode = 1;
  writeFileSync(join(output, 'metadata.json'), `${JSON.stringify(manifest, null, 2)}\n`);
  console.log(JSON.stringify({ output, successfulProbes: manifest.results.filter(result => result.ok).length, examplesUnchanged: manifest.examplesUnchanged, failure: manifest.failure ?? null }));
}
