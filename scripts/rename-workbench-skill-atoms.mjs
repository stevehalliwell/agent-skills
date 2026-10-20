#!/usr/bin/env node
// Maintainer-only forward rename for this repository's canonical fixture/source.
// Usage: node scripts/rename-workbench-skill-atoms.mjs [--check|--apply]
// Default check lists changes. Apply preserves record UUIDs and frozen evidence;
// modifies only known source/docs, fixture config, and managed guidance metadata.
// Does not migrate arbitrary projects, rewrite history, stage, commit, or install.
import { readFile, readdir, rename, access } from 'node:fs/promises';
import { resolve, dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { hash, atomicWrite } from '../skills/game-design-workbench/scripts/lib.mjs';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const home = 'skills/game-design-workbench';
const example = `${home}/examples/crate-puzzle`;
const args = process.argv.slice(2);
if (args.length === 1 && args[0] === '--help') { console.log('Canonical repo rename: --check (default) or --apply. Preserves UUIDs and frozen reports/snapshots; only old default collection is supported. Review Git diff afterward, regenerate packs/templates, then use guarded setup to refresh installed tools. Not an arbitrary-project migration. Exit 0 success, 1 failure, 2 usage.'); process.exit(0); }
if (args.length > 1 || args.length && !['--check', '--apply'].includes(args[0])) { console.error('Use --help.'); process.exit(2); }
const exists = async path => { try { await access(join(root, path)); return true; } catch (error) { if (error.code === 'ENOENT') return false; throw error; } };
async function files(directory) {
  const result = [];
  for (const entry of await readdir(join(root, directory), { withFileTypes: true })) {
    if (entry.isSymbolicLink()) throw Error(`Refuse link: ${directory}/${entry.name}`);
    const path = `${directory}/${entry.name}`;
    if (entry.isDirectory()) result.push(...await files(path)); else result.push(path);
  }
  return result;
}
function transform(text, code) {
  text = text.replaceAll('game-design/skills', 'game-design/skill-atoms').replaceAll('templates/skills', 'templates/skill-atoms')
    .replace(/(?<![\w-])skills\/(?!game-design-workbench(?:\/|\b)|attendant(?:\/|\b)|unity-cli(?:\/|\b))/g, 'skill-atoms/')
    .replace(/(["'`])skills\1/g, '$1skill-atoms$1').replace(/\bas skills\b/g, 'as skill-atoms')
    .replace(/(ref(?:\[\])?:)skills\b/g, '$1skill-atoms').replace(/\bquery skills\b/g, 'query skill-atoms')
    .replaceAll('matrix|skills|routes', 'matrix|skill-atoms|routes').replaceAll('<skills-alias>', '<skill-atoms-alias>')
    .replace(/\bgd skills\b/g, 'gd skill-atoms').replace(/\bgd view skills\b/g, 'gd view skill-atoms')
    .replaceAll('skills.set-rank', 'skill-atoms.set-rank').replace(/^# skills\b/gm, '# skill-atoms').replace(/^- skills:/gm, '- skill-atoms:')
    .replace(/\bskills-(?=move\b|push\b|plan-push\b|read-switch\b|route-switch\b|coordinate\b)/g, 'skill-atoms-')
    .replace(/\b([Ss])ix skills\b/g, '$1ix skill atoms');
  if (code) text = text.replaceAll('m.skills', "m['skill-atoms']").replace(/\bskills:/g, "'skill-atoms':")
    .replace(/\bconst skills\b/g, 'const skillAtoms').replaceAll('of skills)', 'of skillAtoms)').replace(/\bskills\.(?=(?:data|rows|json|provenance|snapshot)\b)/g, 'skillAtoms.');
  return text;
}
try {
  const registry = await readFile(join(root, example, '.pi/attendant.tables'), 'utf8');
  if (registry.includes('game-design/skill-atoms as skill-atoms') && !registry.includes('game-design/skills as skills')) {
    const legacy = [`${home}/templates/skills`, `${example}/game-design/skills`, ...['move', 'push', 'plan-push', 'read-switch', 'route-switch', 'coordinate'].map(name => `${example}/game-design/glossary/skills-${name}.md`)];
    if ((await Promise.all(legacy.map(exists))).some(Boolean)) throw Error('Partial rename; inspect Git diff and reconcile legacy paths before continuing.');
    console.log(JSON.stringify({ schema_version: 1, already_renamed: true, changes: [], moves: [] }));
    process.exit(0);
  }
  if (!registry.includes('game-design/skills as skills')) throw Error('Expected canonical legacy registry; arbitrary projects are not supported.');
  const paths = [
    ...(await files(`${home}/scripts`)), ...(await files(`${home}/test`)), ...(await files(`${home}/templates`)),
    ...(await files(`${example}/game-design`)),
    `${home}/SKILL.md`, `${home}/README.md`, `${example}/README.md`, `${example}/review-input.json`,
    `${example}/AGENTS.md`, `${example}/.pi/attendant.tables`, 'todo.md',
    'scripts/check-workbench-export.mjs', 'reports/game-design-workbench-slice-1.html'
  ];
  for (const name of ['capture', 'cli', 'inspect', 'model', 'review', 'setup']) paths.push(`${home}/references/${name}.md`);
  for (const name of ['game-design-workbench-skill-outline.md', 'game-design-frameworks-reference.md']) if (await exists(name)) paths.push(name);
  const identities = {};
  for (const path of paths.filter(path => new RegExp(`${example}/game-design/(skills/[^/.][^/]*|glossary/skills-[^/]+)\\.md$`).test(path))) {
    const text = await readFile(join(root, path), 'utf8');
    const id = /^id: ([a-f0-9-]{36})$/m.exec(text)?.[1];
    const name = /^name: (.+)$/m.exec(text)?.[1];
    if (!id || !name) throw Error(`Missing fixture identity: ${path}`);
    const role = path.includes('/glossary/') ? 'glossary' : 'skill-atoms';
    identities[`${role}/${transform(name, false)}`] = id;
  }
  const changes = [];
  for (const path of paths) {
    const before = await readFile(join(root, path));
    let text = transform(before.toString('utf8'), path.endsWith('.mjs'));
    if (path === `${home}/scripts/fixture-data.mjs` && Object.keys(identities).length) text += `\n// Fixed tutorial identities survive collection/path renames.\nexport const fixtureIdentities = ${JSON.stringify(identities, null, 2)};\n`;
    if (!before.equals(Buffer.from(text))) changes.push({ path, before: hash(before), bytes: Buffer.from(text) });
  }
  const manifestPath = `${example}/tools/game-design-workbench/manifest.json`;
  const manifestBytes = await readFile(join(root, manifestPath));
  const manifest = JSON.parse(manifestBytes);
  if (Object.hasOwn(manifest.roles, 'skills')) {
    if (manifest.roles.skills !== 'skills') throw Error('Custom legacy role binding needs separate migration.');
    const guidancePath = `${example}/AGENTS.md`;
    const current = await readFile(join(root, guidancePath), 'utf8');
    const block = /<!-- game-design-workbench:start -->[\s\S]*?<!-- game-design-workbench:end -->/.exec(current)?.[0];
    if (!block || hash(block.replaceAll('\r\n', '\n')) !== manifest.guidance_hash) throw Error('Managed guidance edited; reconcile before migration.');
    manifest.roles = Object.fromEntries(Object.entries(manifest.roles).map(([key, value]) => key === 'skills' ? ['skill-atoms', 'skill-atoms'] : [key, value]));
    manifest.guidance_hash = hash(transform(block, false).replaceAll('\r\n', '\n'));
    changes.push({ path: manifestPath, before: hash(manifestBytes), bytes: Buffer.from(JSON.stringify(manifest, null, 2) + '\n') });
  }
  const moves = [[`${home}/templates/skills`, `${home}/templates/skill-atoms`], [`${example}/game-design/skills`, `${example}/game-design/skill-atoms`]];
  for (const name of ['move', 'push', 'plan-push', 'read-switch', 'route-switch', 'coordinate']) moves.push([`${example}/game-design/glossary/skills-${name}.md`, `${example}/game-design/glossary/skill-atoms-${name}.md`]);
  const pending = [];
  for (const [from, to] of moves) if (await exists(from)) { if (await exists(to)) throw Error(`Rename target exists: ${to}`); pending.push({ from, to }); }
  console.log(JSON.stringify({ schema_version: 1, apply: args[0] === '--apply', changes: changes.map(({ path, before, bytes }) => ({ path, before, after: hash(bytes) })), moves: pending, preserved_fixture_ids: Object.keys(identities).length }));
  if (args[0] === '--apply') {
    for (const change of changes) await atomicWrite(root, change.path, change.bytes, change.before);
    for (const move of pending) await rename(join(root, move.from), join(root, move.to));
  }
} catch (error) { console.error(error.message); process.exitCode = 1; }
