// Validate shipped Markdown directly; no generated model or fixture mirror.
// Run: node --test skills/game-design-workbench/test/authoring.test.mjs
// Control tuning test pipes native-validated items into public Attendant, with no JS evaluator.
// Optional public Attendant integration: set GD_ATTENDANT_SKILL to installed skill.
// Also set GD_NATIVE_EXE to crosscheck native matrix/skill/challenge identities and links.
// GD_NATIVE_DRAFT_REPORT: optional coordinate rank-4/self-prerequisite desktop report
// from manual viewer use or a retained earlier probe; checks explicit agent-side handoff.
import test from 'node:test';
import assert from 'node:assert/strict';
import { cp, mkdtemp, readFile, readdir, rm, writeFile } from 'node:fs/promises';
import { join, resolve, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';

const home = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const example = join(home, 'examples/crate-puzzle');
const plumber = join(home, 'examples/little-plumber-guy-nes');
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

test('large plumber example contains authored records rather than copied tutorial or empty bodies', async () => {
  const expected = { nouns: 26, verbs: 12, interactions: 49, mechanics: 24, systems: 6, dynamics: 8,
    aesthetics: 6, 'skill-atoms': 16, challenges: 19, levels: 2, 'control-vars': 20, resources: 4,
    experiments: 3, glossary: 7 };
  let count = 0;
  for (const role of roles) {
    const folder = join(plumber, 'game-design', role);
    const records = (await readdir(folder)).filter(name => !name.startsWith('.') && name.endsWith('.md'));
    assert.equal(records.length, expected[role], role);
    const template = normalized(await text(join(folder, '.template.md'))).trim();
    for (const name of records) {
      const body = normalized(await text(join(folder, name))).split('\n---\n')[1]?.trim();
      assert.ok(body && body !== template, `${role}/${name}: unfilled template`);
      assert.doesNotMatch(body, /Describe the intended feeling or experiential quality/);
    }
    count += records.length;
  }
  assert.equal(count, 202);
  assert.match(await text(join(plumber, 'game-design/concept.md')), /^Working mode: game-design$/m);
  assert.match(await text(join(plumber, 'README.md')), /Approximate instruction/);
});

test('public Attendant and native consumers read the large plumber example', { skip: !process.env.GD_ATTENDANT_SKILL && 'Set GD_ATTENDANT_SKILL for integration' }, async t => {
  const root = await temporary(t);
  await cp(join(plumber, 'game-design'), join(root, 'game-design'), { recursive: true });
  await cp(join(plumber, '.pi'), join(root, '.pi'), { recursive: true });
  const runner = join(resolve(process.env.GD_ATTENDANT_SKILL), 'scripts/attendant.mjs');
  const result = spawnSync(process.execPath, [runner, 'validate', '--strict', '--project', root], { encoding: 'utf8', timeout: 30000, maxBuffer: 4*1024*1024, windowsHide: true });
  assert.equal(result.status, 0, result.stderr);
  const validation = JSON.parse(result.stdout);
  assert.equal(validation.records, 202);
  assert.deepEqual(validation.diagnostics, []);
  assert.deepEqual(validation.corrections, []);
  if (!process.env.GD_NATIVE_EXE) return;
  const binding = role => `${role}=${join(root, 'game-design', role)}`;
  const native = args => {
    const run = spawnSync(resolve(process.env.GD_NATIVE_EXE), args, { encoding: 'utf8', timeout: 30000, maxBuffer: 4*1024*1024, windowsHide: true });
    assert.equal(run.status, 0, `${run.stderr}\n${run.stdout}`);
    const value = JSON.parse(run.stdout);
    assert.equal(value.ok, true);
    return value.data;
  };
  const controls = native(['controls', '--control-vars', binding('control-vars'), '--json']).controls;
  assert.equal(controls.length, 20);
  for (const [name, value] of [['walk-tile-time', 0.5], ['run-tile-time', 0.25], ['coin-bank-capacity', 49]])
    assert.equal(controls.find(row => row.reference === `control-vars/${name}`).value, value);
  assert.equal(controls.find(row => row.reference === 'control-vars/clock-runs-in-cellar').value, true);
  const matrix = native(['matrix', '--nouns', binding('nouns'), '--verbs', binding('verbs'), '--interactions', binding('interactions'), '--related', binding('mechanics'), '--json']);
  assert.equal(matrix.cases.length, 49);
  assert.equal(matrix.cells.length, 26);
  const skills = native(['skill-atoms', '--skill-atoms', binding('skill-atoms'), '--related', binding('mechanics'), '--json']);
  assert.equal(skills.skills.length, 16);
  assert.deepEqual(skills.findings, []);
  const routes = native(['scaffolding', '--skill-atoms', binding('skill-atoms'), '--challenges', binding('challenges'), '--related', binding('mechanics'), '--json']);
  assert.equal(routes.challenges.length, 19);
  assert.equal(routes.checks.length, 18);
  assert.equal(routes.checks.filter(row => row.coverage === 'absent').length, 1);
  assert.equal(routes.checks.find(row => row.skill === 'skill-atoms/finish-approach').coverage, 'absent');
  const mda = native(['mda', '--mechanics', binding('mechanics'), '--systems', binding('systems'), '--dynamics', binding('dynamics'), '--aesthetics', binding('aesthetics'), '--related', binding('experiments'), '--json']);
  assert.equal(mda.nodes.length, 44);
  assert.equal(mda.links.length, 74);
});

test('Markdown framework index links all 18 provisional guides', async () => {
  const index = await text(join(home, 'frameworks/index.md'));
  const guides = [...index.matchAll(/\[[^\]]+\]\(([a-z0-9-]+\.md)\)/g)].map(match => match[1]);
  assert.equal(guides.length, 18);
  assert.equal(new Set(guides).size, 18);
  assert.match(index, /provisional/);
  assert.deepEqual((await readdir(join(home, 'frameworks'))).filter(name => name.endsWith('.md') && name !== 'index.md').sort(), [...guides].sort());
  for (const name of guides) {
    const guide = await text(join(home, 'frameworks', name));
    assert.match(guide, /^# [^\n]+\n/);
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
  const experiments = query('SELECT name, stage FROM experiments ORDER BY name');
  assert.deepEqual(experiments, [
    { name: 'energy-encourages-planning', stage: 'proposed' },
    { name: 'route-cost', stage: 'completed' },
  ]);
  assert.equal(query('SELECT status FROM verbs WHERE name = \'pull\'')[0].status, 'rejected');
  let nativeSkills, nativeRoutes, routeBaseline;
  if (process.env.GD_NATIVE_EXE) {
    const native = spawnSync(resolve(process.env.GD_NATIVE_EXE), [
      'matrix', '--nouns', `nouns=${join(root, 'game-design/nouns')}`,
      '--verbs', `verbs=${join(root, 'game-design/verbs')}`,
      '--interactions', `interactions=${join(root, 'game-design/interactions')}`, '--json',
    ], { encoding: 'utf8', timeout: 30000, windowsHide: true });
    assert.equal(native.status, 0, native.stderr);
    const matrix = JSON.parse(native.stdout);
    assert.equal(matrix.ok, true);
    for (const role of ['nouns', 'verbs', 'interactions']) {
      const nativeRecords = matrix.data[role === 'interactions' ? 'cases' : role];
      const expected = query(`SELECT name, id, status FROM "${role}" ORDER BY name`);
      assert.deepEqual(nativeRecords.map(({ name, id, status }) => ({ name, id, status })), expected);
    }
    for (const row of query('SELECT name, coverage, nouns, verb, context_nouns, mechanics FROM interactions ORDER BY name')) {
      const interaction = matrix.data.cases.find(record => record.name === row.name);
      assert.equal(interaction.coverage, row.coverage);
      for (const field of ['nouns', 'verb', 'context_nouns', 'mechanics']) {
        const expected = field === 'verb' ? (row[field] ? [row[field]] : [])
          : Array.isArray(row[field]) ? row[field] : row[field] ? JSON.parse(row[field]) : [];
        assert.deepEqual(interaction.relationships.filter(edge => edge.field === field).map(edge => edge.reference), expected);
      }
    }
    nativeSkills = () => {
      const result = spawnSync(resolve(process.env.GD_NATIVE_EXE), ['skill-atoms', '--skill-atoms', `skill-atoms=${join(root, 'game-design/skill-atoms')}`, '--json'], { encoding: 'utf8', timeout: 30000, windowsHide: true });
      assert.equal(result.status, 0, result.stderr);
      const parsed = JSON.parse(result.stdout);
      assert.equal(parsed.ok, true);
      return parsed;
    };
    const skills = nativeSkills();
    const expectedAtoms = query('SELECT name, id, status, rank, prerequisites, mechanics FROM "skill-atoms" ORDER BY name');
    assert.deepEqual(skills.data.skills.map(({ name, id, status, authored_rank: rank }) => ({ name, id, status, rank })), expectedAtoms.map(({ name, id, status, rank }) => ({ name, id, status, rank })));
    for (const row of expectedAtoms) for (const field of ['prerequisites', 'mechanics']) {
      const expected = Array.isArray(row[field]) ? row[field] : row[field] ? JSON.parse(row[field]) : [];
      assert.deepEqual(skills.data.skills.find(record => record.name === row.name).relationships.filter(edge => edge.field === field).map(edge => edge.reference), expected);
    }
    assert.deepEqual(skills.data.findings.map(finding => finding.code), ['skill.rank_conflict']);
    nativeRoutes = () => {
      const result = spawnSync(resolve(process.env.GD_NATIVE_EXE), ['scaffolding', '--skill-atoms', `skill-atoms=${join(root, 'game-design/skill-atoms')}`, '--challenges', `challenges=${join(root, 'game-design/challenges')}`, '--json'], { encoding: 'utf8', timeout: 30000, windowsHide: true });
      assert.equal(result.status, 0, result.stderr);
      const parsed = JSON.parse(result.stdout); assert.equal(parsed.ok, true); return parsed;
    };
    routeBaseline = nativeRoutes();
    const expectedChallenges = query('SELECT name, id, status, entry, introduces, practises, requires, tests_mastery, next FROM challenges ORDER BY name');
    assert.deepEqual(routeBaseline.data.challenges.map(({ reference, id, status, entry }) => ({ reference, id, status, entry })), expectedChallenges.map(row => ({ reference: `challenges/${row.name}`, id: row.id, status: row.status, entry: row.entry === true || row.entry === 1 })));
    for (const row of expectedChallenges) for (const field of ['introduces', 'practises', 'requires', 'tests_mastery', 'next']) {
      const expected = Array.isArray(row[field]) ? row[field] : row[field] ? JSON.parse(row[field]) : [];
      assert.deepEqual(routeBaseline.data.challenges.find(record => record.reference === `challenges/${row.name}`).relationships.filter(edge => edge.field === field).map(edge => edge.reference), expected);
    }
    assert.equal(routeBaseline.coverage.check_count, 10);
    assert.equal(routeBaseline.data.checks.find(row => row.challenge === 'challenges/combined-room' && row.skill === 'skill-atoms/plan-push').coverage, 'absent');
    const optional = routeBaseline.data.checks.find(row => row.challenge === 'challenges/combined-room' && row.skill === 'skill-atoms/route-switch');
    assert.equal(optional.coverage, 'some_routes');
    assert.deepEqual(routeBaseline.data.witnesses.find(row => row.check === optional.index && !row.supported).path, ['challenges/move-demo', 'challenges/push-room', 'challenges/combined-room']);
  }
  const before = new Map();
  for (const role of roles) for (const name of await readdir(join(root, 'game-design', role))) {
    const path = join(root, 'game-design', role, name);
    before.set(path, await text(path));
  }
  const selected = 'SELECT id, name, created_at, desc, tags, status, prerequisites, rank, mechanics FROM "skill-atoms" WHERE name = \'coordinate\'';
  const selectedBefore = query(selected)[0];
  attendant(['update', '--collection', 'skill-atoms', '--items', JSON.stringify([{ name: 'coordinate', fields: { rank: 3 } }])]);
  if (nativeSkills) {
    const updated = nativeSkills();
    const coordinate = updated.data.skills.find(record => record.name === 'coordinate');
    assert.equal(coordinate.authored_rank, 3);
    assert.equal(coordinate.minimum_rank, 3);
    assert.equal(coordinate.display_rank, 3);
    assert.equal(coordinate.prerequisite_depth, 3);
    assert.deepEqual(updated.data.findings, []);
    assert.equal(updated.data.skills.find(record => record.name === 'plan-push').authored_rank, null);
    const updatedRoutes = nativeRoutes();
    assert.deepEqual(updatedRoutes.data.checks, routeBaseline.data.checks, 'rank edits must not change declared routes/opportunities');
    assert.deepEqual(updatedRoutes.data.findings.map(row => row.code), routeBaseline.data.findings.map(row => row.code).filter(code => code !== 'skill.rank_conflict'));
  }
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

test('agent applies actual viewer JSON through public Attendant on a private copy', {
  skip: !(process.env.GD_ATTENDANT_SKILL && process.env.GD_NATIVE_EXE && process.env.GD_NATIVE_DRAFT_REPORT) && 'Set Attendant/native executable and desktop draft report paths',
}, async t => {
  const report = JSON.parse(await text(resolve(process.env.GD_NATIVE_DRAFT_REPORT)));
  assert.equal(report.ok, true);
  assert.equal(report.data.applied, false);
  assert.equal(report.data.changes.length, 1);
  const change = report.data.changes[0];
  assert.equal(change.reference, 'skill-atoms/coordinate');
  assert.equal(change.current_disk_available, true);
  const root = await temporary(t);
  await cp(join(example, 'game-design'), join(root, 'game-design'), { recursive: true });
  await cp(join(example, '.pi'), join(root, '.pi'), { recursive: true });
  const runner = join(resolve(process.env.GD_ATTENDANT_SKILL), 'scripts/attendant.mjs');
  const attendant = args => {
    const result = spawnSync(process.execPath, [runner, ...args, '--project', root], { encoding: 'utf8', timeout: 30000, maxBuffer: 4*1024*1024, windowsHide: true });
    assert.equal(result.status, 0, `${result.stderr}\n${result.stdout}`);
    return JSON.parse(result.stdout);
  };
  assert.deepEqual(attendant(['validate', '--no-correct']).corrections, []);
  attendant(['schema']);
  const update = fields => attendant(['update', '-c', 'skill-atoms', '-i', JSON.stringify([{ name: 'coordinate', fields }])]);
  // Deliberately reproduce external disk change before consuming the report.
  update({ rank: 5 });
  const current = attendant(['query', '--sql', 'SELECT id, rank, prerequisites FROM "skill-atoms" WHERE name = \'coordinate\'']).rows[0];
  assert.equal(current.id, change.id);
  assert.equal(current.rank, change.fields.rank.current_disk);
  const prerequisites = Array.isArray(current.prerequisites) ? current.prerequisites : JSON.parse(current.prerequisites);
  assert.deepEqual(prerequisites, change.fields.prerequisites.current_disk);
  const target = join(root, 'game-design/skill-atoms/coordinate.md');
  const body = normalized(await text(target)).split('\n---\n')[1];
  const others = new Map();
  for (const role of roles) for (const name of await readdir(join(root, 'game-design', role))) {
    const path = join(root, 'game-design', role, name);
    if (path !== target) others.set(path, await text(path));
  }
  // This explicit test consumer owns the write; viewer never applies it.
  update(Object.fromEntries(Object.entries(change.fields).map(([field, values]) => [field, values.edited])));
  assert.deepEqual(attendant(['validate', '--no-correct']).diagnostics, []);
  assert.equal(normalized(await text(target)).split('\n---\n')[1], body);
  for (const [path, before] of others) assert.equal(await text(path), before, `collateral handoff write: ${path}`);
  const native = spawnSync(resolve(process.env.GD_NATIVE_EXE), ['skill-atoms', '--skill-atoms', `skill-atoms=${join(root, 'game-design/skill-atoms')}`, '--json'], { encoding: 'utf8', timeout: 30000, windowsHide: true });
  assert.equal(native.status, 0, native.stderr);
  const result = JSON.parse(native.stdout);
  const coordinate = result.data.skills.find(row => row.name === 'coordinate');
  assert.equal(coordinate.authored_rank, 4);
  assert.equal(coordinate.cycle, true);
  assert.equal(result.data.skills.find(row => row.name === 'plan-push').authored_rank, null);
  assert.deepEqual(result.data.findings.map(row => row.code), report.data.findings.map(row => row.code));
});

test('game-tweaked exports update only native-planned literals through public Attendant', { skip: !(process.env.GD_ATTENDANT_SKILL && process.env.GD_NATIVE_EXE) && 'Set GD_ATTENDANT_SKILL and GD_NATIVE_EXE for tuning integration' }, async t => {
  const root = await temporary(t);
  await cp(join(example, 'game-design'), join(root, 'game-design'), { recursive: true });
  await cp(join(example, '.pi'), join(root, '.pi'), { recursive: true });
  const runner = join(resolve(process.env.GD_ATTENDANT_SKILL), 'scripts/attendant.mjs');
  const attendant = args => {
    const result = spawnSync(process.execPath, [runner, ...args, '--project', root], { encoding: 'utf8', timeout: 30000, maxBuffer: 4*1024*1024, windowsHide: true });
    assert.equal(result.status, 0, `${result.stderr}\n${result.stdout}`);
    return JSON.parse(result.stdout);
  };
  const schema = attendant(['schema']).collections.find(collection => collection.name === 'control-vars');
  assert.ok(schema);
  assert.match(schema.usage, /named game configuration/i);
  assert.match(JSON.stringify(schema.fields), /value/);
  const folder = join(root, 'game-design/control-vars');
  assert.equal(resolve(schema.directory), resolve(folder));
  const native = (command, options = [], status = 0) => {
    const result = spawnSync(resolve(process.env.GD_NATIVE_EXE), ['controls', ...command, '--control-vars', `control-vars=${folder}`, ...options], { encoding: 'utf8', timeout: 30000, maxBuffer: 4*1024*1024, windowsHide: true });
    assert.equal(result.status, status, `${result.stderr}\n${result.stdout}`);
    return JSON.parse(result.stdout);
  };
  const target = join(folder, 'walk-speed.md');
  const body = normalized(await text(target)).split('\n---\n')[1];
  const values = native(['export'], ['--as', 'json']);
  values['walk-speed'] = 4;
  const input = join(root, 'game-tweaks.json');
  await writeFile(input, JSON.stringify(values));
  const invocation = ['--input', input, '--json'];
  const preview = native(['import'], invocation);
  assert.equal(preview.data.applied, false);
  assert.equal(preview.data.ready, true);
  assert.deepEqual(preview.data.items, [{ name: 'walk-speed', fields: { value: 4 } }]);
  assert.equal(preview.data.derived.find(row => row.reference === 'control-vars/walk-duration').candidate, 0.25);
  // An intervening source edit invalidates old preview even when values stay equal.
  attendant(['update', '-c', 'control-vars', '-i', JSON.stringify([{ name: 'walk-speed', fields: { desc: 'External edit before tuning adoption' } }])]);
  const fresh = native(['import'], invocation);
  assert.notEqual(fresh.data.source_digest, preview.data.source_digest);
  assert.deepEqual(fresh.data.items, preview.data.items);
  // Test explicitly adopts the fresh preview, not the stale one. Application has
  // no domain decisions: pass native's complete validated item array unchanged.
  const before = new Map();
  for (const role of roles) for (const name of await readdir(join(root, 'game-design', role))) {
    const path = join(root, 'game-design', role, name);
    before.set(path, await text(path));
  }
  const rechecked = native(['import'], invocation);
  assert.equal(rechecked.data.source_digest, fresh.data.source_digest);
  assert.deepEqual(rechecked.data.items, fresh.data.items);
  attendant(['update', '-c', 'control-vars', '-i', JSON.stringify(rechecked.data.items)]);
  assert.deepEqual(attendant(['validate', '--no-correct']).diagnostics, []);
  for (const [path, original] of before) if (path !== target) assert.equal(await text(path), original, `collateral tuning write: ${path}`);
  assert.equal(normalized(await text(target)).split('\n---\n')[1], body);
  const current = native([], ['--json']).data.controls;
  assert.equal(current.find(row => row.reference === 'control-vars/walk-speed').value, 4);
  assert.equal(current.find(row => row.reference === 'control-vars/walk-duration').value, 0.25);
  assert.equal(current.find(row => row.reference === 'control-vars/push-duration').value, 0.5);
  assert.equal(current.find(row => row.reference === 'control-vars/walk-speed').overridden, false);
  const row = attendant(['query', '--sql', 'SELECT id, value, expression, unit, value_type, "desc" FROM "control-vars" WHERE name = \'walk-speed\'']).rows[0];
  assert.equal(row.id, fresh.data.changes[0].id);
  assert.equal(row.value, 4);
  assert.equal(row.expression, '');
  assert.equal(row.unit, 'm/s');
  assert.equal(row.value_type, 'number');
  assert.equal(row.desc, 'External edit before tuning adoption');
  // Re-export, edit again, and prove formula conflicts cannot expose update items.
  const invalid = native(['export'], ['--as', 'json']);
  invalid['walk-speed'] = 8; invalid['walk-duration'] = 0.3;
  await writeFile(input, JSON.stringify(invalid));
  const rejected = native(['import'], invocation, 2);
  assert.equal(rejected.data.ready, false);
  assert.deepEqual(rejected.data.items, []);
  assert.ok(rejected.errors.some(error => error.code === 'controls.import_formula'));
  assert.equal(native(['export'], ['--as', 'json'])['walk-speed'], 4);
});
