// Repository artifact checks, not a runtime/workflow implementation.
// Run: node --test skills/game-design-workbench/test/authoring.test.mjs
// Optional public Attendant integration: set GD_ATTENDANT_SKILL to installed skill.
import test from 'node:test';
import assert from 'node:assert/strict';
import { cp, mkdtemp, readFile, readdir, rm, writeFile } from 'node:fs/promises';
import { join, resolve, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';
import { model, schemaFor, usageFor } from '../scripts/model.mjs';
import { records } from '../scripts/fixture-data.mjs';

const home = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const example = join(home, 'examples/crate-puzzle');
const generator = join(home, 'scripts/build-example.mjs');
const text = path => readFile(path, 'utf8');
const normalized = value => value.replaceAll('\r\n', '\n');
// Generated fixture frontmatter is JSON-compatible YAML; not a general YAML reader.
const frontmatter = source => JSON.parse(normalized(source).split('\n---\n')[0].slice(4));
function invoke(script, args) {
  return spawnSync(process.execPath, [script, ...args], { encoding: 'utf8', timeout: 30000, maxBuffer: 4 * 1024 * 1024, windowsHide: true });
}
async function fixture(t) {
  const root = await mkdtemp(join(tmpdir(), 'gd-authoring-'));
  t.after(() => rm(root, { recursive: true, force: true }));
  const result = invoke(generator, ['--output', root]);
  assert.equal(result.status, 0, result.stderr);
  assert.deepEqual(JSON.parse(result.stdout), { ok: true, output: root, collections: 14, records: 63 });
  return root;
}

test('fixture generation matches canonical records, templates, and authored concept', async t => {
  const root = await fixture(t);
  assert.equal(Object.keys(model).length, 14);
  assert.equal(records.length, 63);
  const ids = new Set();
  for (const role of Object.keys(model)) {
    const folder = join(root, 'game-design', role);
    assert.deepEqual(frontmatter(await text(join(folder, '.schema.md'))), schemaFor(role));
    assert.equal(await text(join(folder, '.usage.md')), usageFor(role));
    for (const name of ['.schema.md', '.usage.md', '.template.md']) {
      const generated = normalized(await text(join(folder, name)));
      assert.equal(generated, normalized(await text(join(home, 'templates', role, name))), `${role}/${name}: template drift`);
      assert.equal(generated, normalized(await text(join(example, 'game-design', role, name))), `${role}/${name}: example contract drift`);
    }
    const expected = records.filter(record => record.role === role).map(record => `${record.name}.md`).sort();
    assert.deepEqual((await readdir(join(example, 'game-design', role))).filter(name => !name.startsWith('.')).sort(), expected);
    for (const name of expected) {
      const generated = normalized(await text(join(folder, name)));
      const canonical = normalized(await text(join(example, 'game-design', role, name)));
      const record = frontmatter(generated);
      assert.equal(record.id, canonical.match(/^\s*"?id"?: "?([a-f0-9-]{36})"?,?$/m)?.[1], `${role}/${name}: identity drift`);
      assert.equal(generated.split('\n---\n')[1], canonical.split('\n---\n')[1], `${role}/${name}: body drift`);
      assert.match(record.id, /^[a-f0-9]{8}-[a-f0-9]{4}-7[a-f0-9]{3}-[89ab][a-f0-9]{3}-[a-f0-9]{12}$/);
      assert.ok(!ids.has(record.id), `duplicate UUID ${record.id}`);
      ids.add(record.id);
    }
  }
  assert.equal(ids.size, 63);
  const concept = await text(join(root, 'game-design/concept.md'));
  assert.equal(concept, await text(join(example, 'game-design/concept.md')));
  assert.match(concept, /^Working mode: game-explorer$/m);
  assert.match(concept, /Hypothetical player voice/);
  assert.equal(await text(join(root, '.pi/attendant.tables')), await text(join(example, '.pi/attendant.tables')));
});

test('fixture generator refuses existing content without changing it', async t => {
  const root = await mkdtemp(join(tmpdir(), 'gd-occupied-'));
  t.after(() => rm(root, { recursive: true, force: true }));
  await writeFile(join(root, 'notes.md'), 'preserve user notes\n');
  const result = invoke(generator, ['--output', root]);
  assert.notEqual(result.status, 0);
  assert.match(result.stderr, /Output must be empty/);
  assert.equal(await text(join(root, 'notes.md')), 'preserve user notes\n');
  assert.deepEqual(await readdir(root), ['notes.md']);
});

test('manual framework catalog indexes all 18 provisional guides, not executable recipes', async () => {
  const catalog = JSON.parse(await text(join(home, 'references/frameworks/catalog.json')));
  assert.equal(catalog.length, 18);
  assert.equal(new Set(catalog.map(item => item.id)).size, 18);
  const paths = catalog.map(item => `${item.id}.md`).sort();
  assert.deepEqual((await readdir(join(home, 'references/frameworks'))).filter(name => name.endsWith('.md')).sort(), paths);
  for (const item of catalog) {
    assert.equal(item.path, `references/frameworks/${item.id}.md`);
    assert.equal(item.status, 'provisional');
    const guide = await text(join(home, item.path));
    assert.ok(guide.startsWith(`# ${item.title}\n`));
    assert.match(guide, /## Sources/);
    assert.match(guide, /Inspection topics:/);
    assert.doesNotMatch(guide, /Available recipes:|workbench\.mjs/);
  }
});

test('external Attendant validates/queries fixture and updates only selected rank', { skip: !process.env.GD_ATTENDANT_SKILL && 'Set GD_ATTENDANT_SKILL for integration' }, async t => {
  const root = await fixture(t);
  const runner = join(resolve(process.env.GD_ATTENDANT_SKILL), 'scripts/attendant.mjs');
  const attendant = (args, project = root) => {
    const result = invoke(runner, [...args, '--project', project]);
    assert.equal(result.status, 0, `${result.stderr}\n${result.stdout}`);
    return JSON.parse(result.stdout);
  };
  const validation = attendant(['validate', '--strict']);
  assert.deepEqual(validation.diagnostics, []);
  assert.deepEqual(validation.corrections, []);
  assert.equal(attendant(['schema']).collections.length, 14);
  const canonical = await mkdtemp(join(tmpdir(), 'gd-canonical-'));
  t.after(() => rm(canonical, { recursive: true, force: true }));
  await cp(join(example, 'game-design'), join(canonical, 'game-design'), { recursive: true });
  await cp(join(example, '.pi'), join(canonical, '.pi'), { recursive: true });
  assert.deepEqual(attendant(['validate', '--strict'], canonical).diagnostics, []);
  const query = (sql, project = root) => attendant(['query', '--sql', sql, '--limit', '1000'], project).rows;
  for (const role of Object.keys(model)) {
    const fields = ['id', 'name', 'created_at', 'desc', 'tags', ...Object.keys(schemaFor(role))].map(field => `"${field}"`).join(', ');
    const sql = `SELECT ${fields} FROM "${role}" ORDER BY name`;
    assert.deepEqual(query(sql), query(sql, canonical), `${role}: canonical field drift`);
  }
  const atoms = query('SELECT id, name, rank FROM "skill-atoms" ORDER BY name');
  assert.equal(atoms.length, 6);
  assert.equal(atoms.find(row => row.name === 'coordinate').rank, 2);
  assert.equal(atoms.find(row => row.name === 'plan-push').rank, null);
  assert.equal(query('SELECT name FROM "control-vars"').length, 7);
  assert.ok(query('SELECT stage FROM experiments').every(row => row.stage === 'proposed'));
  assert.equal(query('SELECT status FROM verbs WHERE name = \'pull\'')[0].status, 'rejected');
  const before = new Map();
  for (const role of Object.keys(model)) for (const name of await readdir(join(root, 'game-design', role))) {
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
  assert.equal(query('SELECT rank FROM "skill-atoms" WHERE name = \'coordinate\'')[0].rank, 3);
  attendant(['update', '--collection', 'verbs', '--items', JSON.stringify([{ name: 'pull', fields: { status: 'disabled' } }])]);
  assert.equal(query('SELECT status FROM verbs WHERE name = \'pull\'')[0].status, 'disabled');
  assert.ok(!query('SELECT name FROM verbs WHERE status = \'accepted\'').some(row => row.name === 'pull'));
});
