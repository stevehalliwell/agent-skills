// Validate shipped Markdown directly; no generated model or fixture mirror.
// Run: node --test skills/game-design-workbench/test/authoring.test.mjs
// Optional public Attendant integration: set GD_ATTENDANT_SKILL to installed skill.
import test from 'node:test';
import assert from 'node:assert/strict';
import { cp, mkdtemp, readFile, readdir, rm } from 'node:fs/promises';
import { join, resolve, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';

const home = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const example = join(home, 'examples/crate-puzzle');
const text = path => readFile(path, 'utf8');
const normalized = value => value.replaceAll('\r\n', '\n');
const roles = (await readdir(join(home, 'templates'), { withFileTypes: true })).filter(entry => entry.isDirectory()).map(entry => entry.name).sort();
async function temporary(t) {
  const root = await mkdtemp(join(tmpdir(), 'gd-assets-'));
  t.after(() => rm(root, { recursive: true, force: true }));
  return root;
}

test('shipped templates and example contain readable collection contracts and concept', async () => {
  assert.equal(roles.length, 14);
  for (const role of roles) for (const name of ['.schema.md', '.usage.md', '.template.md']) {
    assert.ok((await text(join(home, 'templates', role, name))).trim(), `${role}/${name}: empty template`);
    assert.ok((await text(join(example, 'game-design', role, name))).trim(), `${role}/${name}: empty example contract`);
  }
  const concept = await text(join(example, 'game-design/concept.md'));
  assert.match(concept, /^Working mode: game-explorer$/m);
  assert.match(concept, /Hypothetical player voice/);
});

test('manual framework catalog indexes all 18 provisional guides', async () => {
  const catalog = JSON.parse(await text(join(home, 'frameworks/catalog.json')));
  assert.equal(catalog.length, 18);
  assert.equal(new Set(catalog.map(item => item.id)).size, 18);
  assert.deepEqual((await readdir(join(home, 'frameworks'))).filter(name => name.endsWith('.md')).sort(), catalog.map(item => `${item.id}.md`).sort());
  for (const item of catalog) {
    assert.equal(item.path, `frameworks/${item.id}.md`);
    assert.equal(item.status, 'provisional');
    const guide = await text(join(home, item.path));
    assert.ok(guide.startsWith(`# ${item.title}\n`));
    assert.equal(guide.match(/^## References$/gm)?.length, 1);
    assert.equal([...guide.matchAll(/^## (.+)$/gm)].at(-1)?.[1], 'References');
    assert.match(guide, /Inspection topics:/);
  }
});

test('external Attendant validates copied templates/example and preserves rank-only updates', { skip: !process.env.GD_ATTENDANT_SKILL && 'Set GD_ATTENDANT_SKILL for integration' }, async t => {
  const root = await temporary(t);
  await cp(join(example, 'game-design'), join(root, 'game-design'), { recursive: true });
  await cp(join(example, '.pi'), join(root, '.pi'), { recursive: true });
  const runner = join(resolve(process.env.GD_ATTENDANT_SKILL), 'scripts/attendant.mjs');
  const attendant = (args, project = root) => {
    const result = spawnSync(process.execPath, [runner, ...args, '--project', project], { encoding: 'utf8', timeout: 30000, maxBuffer: 4 * 1024 * 1024, windowsHide: true });
    assert.equal(result.status, 0, `${result.stderr}\n${result.stdout}`);
    return JSON.parse(result.stdout);
  };
  const validation = attendant(['validate', '--strict']);
  assert.deepEqual(validation.diagnostics, []);
  assert.deepEqual(validation.corrections, []);
  assert.deepEqual(attendant(['schema']).collections.map(collection => collection.name).sort(), roles);

  // Copy real templates as an agent would; do not synthesize schema or records.
  const empty = await temporary(t);
  await cp(join(home, 'templates'), join(empty, 'game-design'), { recursive: true });
  await cp(join(example, '.pi'), join(empty, '.pi'), { recursive: true });
  const emptyValidation = attendant(['validate', '--strict'], empty);
  assert.deepEqual(emptyValidation.diagnostics, []);
  assert.deepEqual(emptyValidation.corrections, []);
  assert.deepEqual(attendant(['schema'], empty).collections.map(collection => collection.name).sort(), roles);

  const query = (sql, project = root) => attendant(['query', '--sql', sql, '--limit', '1000'], project).rows;
  const identities = roles.flatMap(role => {
    assert.deepEqual(query(`SELECT id FROM "${role}"`, empty), []);
    return query(`SELECT id FROM "${role}"`).map(row => row.id);
  });
  assert.equal(identities.length, 63);
  assert.equal(new Set(identities).size, 63);
  for (const id of identities) assert.match(id, /^[a-f0-9]{8}-[a-f0-9]{4}-7[a-f0-9]{3}-[89ab][a-f0-9]{3}-[a-f0-9]{12}$/);
  const atoms = query('SELECT id, name, rank FROM "skill-atoms" ORDER BY name');
  assert.equal(atoms.length, 6);
  assert.equal(atoms.find(row => row.name === 'coordinate').rank, 2);
  assert.equal(atoms.find(row => row.name === 'plan-push').rank, null);
  assert.equal(query('SELECT name FROM "control-vars"').length, 7);
  const experiments = query('SELECT stage FROM experiments');
  assert.equal(experiments.length, 2);
  assert.ok(experiments.every(row => row.stage === 'proposed'));
  assert.equal(query('SELECT status FROM verbs WHERE name = \'pull\'')[0].status, 'rejected');
  const before = new Map();
  for (const role of roles) for (const name of await readdir(join(root, 'game-design', role))) {
    const path = join(root, 'game-design', role, name);
    before.set(path, await text(path));
  }
  const selected = 'SELECT id, name, created_at, desc, tags, status, prerequisites, rank, mechanics FROM "skill-atoms" WHERE name = \'coordinate\'';
  const selectedBefore = query(selected)[0];
  attendant(['update', '--collection', 'skill-atoms', '--items', JSON.stringify([{ name: 'coordinate', fields: { rank: 3 } }])]);
  const target = join(root, 'game-design/skill-atoms/coordinate.md');
  for (const [path, source] of before) {
    const after = await text(path);
    if (path === target) {
      assert.deepEqual(query(selected)[0], { ...selectedBefore, rank: 3 });
      assert.equal(normalized(after).split('\n---\n')[1], normalized(source).split('\n---\n')[1]);
    } else assert.equal(after, source, `unrequested change: ${path}`);
  }
  attendant(['update', '--collection', 'verbs', '--items', JSON.stringify([{ name: 'pull', fields: { status: 'disabled' } }])]);
  assert.equal(query('SELECT status FROM verbs WHERE name = \'pull\'')[0].status, 'disabled');
  assert.ok(!query('SELECT name FROM verbs WHERE status = \'accepted\'').some(row => row.name === 'pull'));
});
