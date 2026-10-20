import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtemp, cp, readFile, writeFile, readdir, rm, mkdir } from 'node:fs/promises';
import { join, resolve, dirname, basename } from 'node:path';
import { fileURLToPath } from 'node:url';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';
import { planSetup, applySetup } from '../scripts/setup.mjs';
import { runQuery, resolveTerm } from '../scripts/queries.mjs';
import { snapshot, hash } from '../scripts/lib.mjs';
const home = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const roots = [];
async function temporary(fixture = false) {
  const root = await mkdtemp(join(tmpdir(), 'gd-test space-é-')); roots.push(root);
  if (fixture) await cp(join(home, 'examples/crate-puzzle'), root, { recursive: true, filter: source => basename(source) !== '.attendant' && basename(source) !== 'game-design-results' });
  return root;
}
function cli(root, args, script = join(home, 'scripts/workbench.mjs')) {
  const child = spawnSync(process.execPath, [script, ...args, '--project', root, '--json'], { encoding: 'utf8', timeout: 120000 });
  assert.equal(child.error, undefined, child.error?.message);
  let json;
  try { json = JSON.parse(child.stdout); } catch { assert.fail(`Non-JSON stdout: ${child.stdout}\n${child.stderr}`); }
  return { status: child.status, json, stderr: child.stderr };
}
test.after(async () => { for (const root of roots) await rm(root, { recursive: true, force: true }); });

test('setup approval, idempotency, local-only execution, and conflict preservation', async () => {
  const root = await temporary();
  await writeFile(join(root, 'AGENTS.md'), '# Custom\n\nKeep this instruction.\n');
  await writeFile(join(root, '.gitignore'), '# Existing ignore rules\nlocal-only/\n');
  const plan = await planSetup(root);
  assert.equal(plan.new_collections.length, 19);
  assert.equal(plan.conflicts.length, 0);
  assert.deepEqual((await readdir(root)).sort(), ['.gitignore', 'AGENTS.md']);
  await assert.rejects(applySetup(root, undefined, {}), { code: 'APPROVAL_REQUIRED' });
  const applied = await applySetup(root, plan.plan_id, {});
  assert.ok(applied.changed.length > 57);
  assert.match(await readFile(join(root, 'AGENTS.md'), 'utf8'), /Keep this instruction/);
  assert.match(await readFile(join(root, '.gitignore'), 'utf8'), /local-only\//);
  assert.equal((await planSetup(root)).files.length, 0);
  const localScript = join(root, 'tools/game-design-workbench/scripts/workbench.mjs');
  const result = cli(root, ['query', 'skills'], localScript);
  assert.equal(result.status, 0, JSON.stringify(result.json));
  assert.deepEqual(result.json.data.rows, []);
  await writeFile(localScript, `${await readFile(localScript, 'utf8')}\n// User local edit.\n`);
  const conflict = await planSetup(root);
  assert.ok(conflict.conflicts.includes('tools/game-design-workbench/scripts/workbench.mjs'));
  await assert.rejects(applySetup(root, conflict.plan_id, {}), { code: 'SETUP_CONFLICT' });
  assert.match(await readFile(localScript, 'utf8'), /User local edit/);
});

test('stale setup approval fails before creating configuration', async () => {
  const root = await temporary();
  const plan = await planSetup(root);
  await writeFile(join(root, 'AGENTS.md'), '# New instructions\n');
  await assert.rejects(applySetup(root, plan.plan_id, {}), { code: 'APPROVAL_REQUIRED' });
  assert.ok(!(await readdir(root)).includes('.pi'));
});

test('unconfigured frontmatter needs migration/fresh decision', async () => {
  const root = await temporary();
  await mkdir(join(root, 'notes'));
  const source = '---\nname: old-design\n---\nOriginal notes.\n';
  await writeFile(join(root, 'notes/design.md'), source);
  const plan = await planSetup(root);
  assert.deepEqual(plan.candidate_sources, ['notes/design.md']);
  await assert.rejects(applySetup(root, plan.plan_id, {}), { code: 'MIGRATION_DECISION_REQUIRED' });
  assert.equal(await readFile(join(root, 'notes/design.md'), 'utf8'), source);
  assert.equal((await planSetup(root, { fresh: true })).fresh_acknowledged, true);
});

test('queries expose cases and authored ranks without correcting live source', async () => {
  const root = await temporary(true);
  const path = join(root, 'game-design/skills/move.md');
  const edited = (await readFile(path, 'utf8')).replace('desc: Grid movement\n', '');
  await writeFile(path, edited);
  const before = (await snapshot(root)).metadata.fingerprint;
  const matrix = await runQuery(root, 'matrix');
  assert.equal(matrix.data.rows.length, 6);
  assert.equal(matrix.data.rows.filter(row => row.noun === 'nouns/crate' && row.verb === 'verbs/push').length, 3);
  assert.equal(matrix.data.rows.find(row => row.interaction === 'interactions/wall-activation').coverage, 'not_applicable');
  const skills = await runQuery(root, 'skills');
  assert.equal(skills.data.rows.find(row => row.name === 'plan-push').rank, null);
  assert.equal(skills.data.rows.find(row => row.name === 'coordinate').rank, 2);
  assert.equal(skills.data.rows.find(row => row.name === 'move').rank, 0);
  assert.ok(skills.data.rows.every(row => row.source_path.startsWith('game-design/skills/')));
  assert.ok(skills.projection_corrections.some(item => item.path.endsWith('move.md')));
  assert.equal((await snapshot(root)).metadata.fingerprint, before);
  assert.equal(await readFile(path, 'utf8'), edited);
  assert.ok(!(await readdir(root)).includes('.attendant'));
});

test('all SQL recipes run on fixture and return declared data', async () => {
  const root = await temporary(true);
  for (const name of ['matrix', 'skills', 'routes', 'mda', 'decisions', 'glossary', 'controls', 'evidence']) {
    const result = await runQuery(root, name);
    assert.ok(Array.isArray(result.data.rows), name);
    assert.equal(result.data.truncated, false, name);
  }
  const decisions = await runQuery(root, 'decisions');
  assert.equal(decisions.data.rows.find(row => row.name === 'no-pulling').status, 'rejected');
  assert.equal((await runQuery(root, 'records', { collection: 'observations' })).data.rows.length, 0);
});

test('term normalization, inactive disposition, collisions, and unknown synonyms', async () => {
  const root = await temporary(true);
  assert.equal((await resolveTerm(root, '  BoX  ')).data.state, 'resolved');
  assert.equal((await resolveTerm(root, 'shoves')).data.state, 'unknown');
  const crate = join(root, 'game-design/glossary/nouns-crate.md');
  await writeFile(crate, (await readFile(crate, 'utf8')).replace('status: accepted', 'status: rejected'));
  assert.equal((await resolveTerm(root, 'box')).data.matches[0].status, 'rejected');
  const wall = join(root, 'game-design/glossary/nouns-wall.md');
  await writeFile(wall, (await readFile(wall, 'utf8')).replace('aliases: []', 'aliases: [" BOX "]'));
  const ambiguous = await resolveTerm(root, 'box');
  assert.equal(ambiguous.data.state, 'ambiguous');
  assert.equal(ambiguous.data.collisions[0].code, 'term.collision');
});

test('CLI formats, limits, field selection, validation, unavailable commands, and errors', async () => {
  const root = await temporary(true);
  const result = cli(root, ['query', 'skills', '--fields', 'name,rank', '--limit', '2']);
  assert.equal(result.status, 0);
  assert.equal(result.json.data.truncated, true);
  assert.equal(result.json.coverage.status, 'partial');
  assert.deepEqual(Object.keys(result.json.data.rows[0]), ['name', 'rank']);
  assert.equal(cli(root, ['simulate']).json.errors[0].code, 'CAPABILITY_UNAVAILABLE');
  assert.equal(cli(root, ['query', 'skills', '--limit', '0']).status, 2);
  assert.equal(cli(root, ['query', 'skills', '--format', 'text']).json.errors[0].code, 'FORMAT_CONFLICT');
  assert.equal(cli(root, ['validate']).status, 0);
  const skill = join(root, 'game-design/skills/push.md');
  await writeFile(skill, (await readFile(skill, 'utf8')).replace('skills/move', 'skills/missing'));
  const invalid = cli(root, ['validate']);
  assert.equal(invalid.status, 1);
  assert.ok(invalid.json.data.diagnostics.some(item => item.code === 'record-ref-missing'));
});

test('snapshot detects source changes and rejects escapes', async () => {
  const root = await temporary(true);
  const before = (await snapshot(root)).metadata;
  const path = join(root, 'game-design/skills/coordinate.md');
  await writeFile(path, (await readFile(path, 'utf8')).replace('rank: 2', 'rank: 3'));
  assert.notEqual((await snapshot(root)).metadata.fingerprint, before.fingerprint);
  await assert.rejects(snapshot(root, ['../private.txt']), { code: 'PATH_OUTSIDE_PROJECT' });
  assert.equal(cli(root, ['snapshot', '--output', join(root, 'game-design/skills/overwrite.md')]).json.errors[0].code, 'OUTPUT_SOURCE_CONFLICT');
});
