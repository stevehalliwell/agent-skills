import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtemp, cp, readFile, writeFile, readdir, rm, mkdir, symlink } from 'node:fs/promises';
import { join, resolve, dirname, basename } from 'node:path';
import { fileURLToPath } from 'node:url';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';
import { planSetup as planProject, applySetup as applyProject } from '../scripts/setup.mjs';
import { runQuery, resolveTerm } from '../scripts/queries.mjs';
import { snapshot, projectContext, attendantHome, attendantFingerprint, bundle, hash } from '../scripts/lib.mjs';
import { catalog, saveReport, showReport } from '../scripts/reports.mjs';
import { model } from '../scripts/model.mjs';
import { guidance } from '../scripts/pack-guidance.mjs';
const home = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const roots = [];
// Existing scenarios explicitly choose exploration; production has no inferred default.
const planSetup = (root, options = {}) => planProject(root, { mode: 'game-explorer', ...options });
const applySetup = (root, id, options = {}) => applyProject(root, id, { mode: 'game-explorer', ...options });
async function temporary(fixture = false) {
  const root = await mkdtemp(join(tmpdir(), 'gd-test space-é-')); roots.push(root);
  if (fixture) await cp(join(home, 'examples/crate-puzzle'), root, { recursive: true, filter: source => !['.attendant', 'game-design-results', 'tools', 'AGENTS.md'].includes(basename(source)) });
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
  assert.equal(plan.new_collections.length, 16);
  assert.equal(plan.conflicts.length, 0);
  assert.deepEqual((await readdir(root)).sort(), ['.gitignore', 'AGENTS.md']);
  await assert.rejects(applySetup(root, undefined, {}), { code: 'APPROVAL_REQUIRED' });
  const applied = await applySetup(root, plan.plan_id, {});
  assert.ok(applied.changed.length > 57);
  assert.match(await readFile(join(root, 'AGENTS.md'), 'utf8'), /Keep this instruction/);
  assert.match(await readFile(join(root, '.gitignore'), 'utf8'), /local-only\//);
  assert.equal((await planSetup(root)).files.length, 0);
  const localScript = join(root, 'tools/game-design-workbench/scripts/workbench.mjs');
  const result = cli(root, ['query', 'skill-atoms'], localScript);
  assert.equal(result.status, 0, JSON.stringify(result.json));
  assert.deepEqual(result.json.data.rows, []);
  assert.equal(cli(root, ['query', 'skill-atoms', '--fields', 'missing_field'], localScript).json.errors[0].code, 'FIELD_UNKNOWN');
  // Ordinary Windows Git checkout must not invalidate owned-file/guidance hashes.
  for (const args of [['init', '-q'], ['-c', 'core.autocrlf=true', 'add', '.'], ['-c', 'user.name=Workbench Test', '-c', 'user.email=workbench@example.invalid', 'commit', '-qm', 'fixture'], ['-c', 'core.autocrlf=true', 'checkout-index', '-af']]) {
    const command = spawnSync('git', ['-C', root, ...args], { encoding: 'utf8' });
    assert.equal(command.status, 0, command.stderr);
  }
  const afterCheckout = await planSetup(root);
  assert.deepEqual(afterCheckout.conflicts, []);
  assert.equal(afterCheckout.files.length, 0);
  assert.equal(cli(root, ['query', 'skill-atoms'], localScript).status, 0);
  await writeFile(localScript, `${await readFile(localScript, 'utf8')}\n// User local edit.\n`);
  const conflict = await planSetup(root);
  assert.ok(conflict.conflicts.includes('tools/game-design-workbench/scripts/workbench.mjs'));
  await assert.rejects(applySetup(root, conflict.plan_id, {}), { code: 'SETUP_CONFLICT' });
  assert.match(await readFile(localScript, 'utf8'), /User local edit/);
  const manifestPath = join(root, 'tools/game-design-workbench/manifest.json');
  const manifest = JSON.parse(await readFile(manifestPath, 'utf8'));
  manifest.roles.skills = manifest.roles['skill-atoms']; delete manifest.roles['skill-atoms'];
  await writeFile(manifestPath, JSON.stringify(manifest));
  await assert.rejects(planSetup(root), { code: 'COLLECTION_RENAME_REQUIRED' });
  delete manifest.roles.skills; manifest.roles['skill-atoms'] = 'skill-atoms';
  manifest.roles.hypotheses = 'hypotheses';
  await writeFile(manifestPath, JSON.stringify(manifest));
  const beforeLegacyPlan = (await snapshot(root)).metadata.fingerprint;
  await assert.rejects(planSetup(root), { code: 'EXPERIMENT_MIGRATION_REQUIRED' });
  assert.equal((await snapshot(root)).metadata.fingerprint, beforeLegacyPlan);
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
  await writeFile(join(root, 'old-design.md'), source);
  await mkdir(join(root, '.pi')); await writeFile(join(root, '.pi/attendant.tables'), '');
  const plan = await planSetup(root);
  assert.deepEqual(plan.candidate_sources.sort(), ['notes/design.md', 'old-design.md']);
  await assert.rejects(applySetup(root, plan.plan_id, {}), { code: 'MIGRATION_DECISION_REQUIRED' });
  assert.equal(await readFile(join(root, 'notes/design.md'), 'utf8'), source);
  assert.equal((await planSetup(root, { fresh: true })).fresh_acknowledged, true);
});

test('queries expose cases and authored ranks without correcting live source', async () => {
  const root = await temporary(true);
  const path = join(root, 'game-design/skill-atoms/move.md');
  const edited = (await readFile(path, 'utf8')).replace('desc: Grid movement\n', '');
  await writeFile(path, edited);
  const before = (await snapshot(root)).metadata.fingerprint;
  const matrix = await runQuery(root, 'matrix');
  assert.equal(matrix.data.rows.length, 6);
  assert.equal(matrix.data.rows.filter(row => row.noun === 'nouns/crate' && row.verb === 'verbs/push').length, 3);
  assert.equal(matrix.data.rows.find(row => row.interaction === 'interactions/wall-activation').coverage, 'not_applicable');
  const skillAtoms = await runQuery(root, 'skill-atoms');
  assert.equal(skillAtoms.data.rows.find(row => row.name === 'plan-push').rank, null);
  assert.equal(skillAtoms.data.rows.find(row => row.name === 'coordinate').rank, 2);
  assert.equal(skillAtoms.data.rows.find(row => row.name === 'move').rank, 0);
  assert.ok(skillAtoms.data.rows.every(row => row.source_path.startsWith('game-design/skill-atoms/')));
  assert.ok(skillAtoms.projection_corrections.some(item => item.path.endsWith('move.md')));
  assert.equal((await snapshot(root)).metadata.fingerprint, before);
  assert.equal(await readFile(path, 'utf8'), edited);
  assert.ok(!(await readdir(root)).includes('.attendant'));
});

test('all SQL recipes run on fixture and return declared data', async () => {
  const root = await temporary(true);
  for (const name of ['matrix', 'skill-atoms', 'routes', 'mda', 'decisions', 'glossary', 'controls', 'evidence']) {
    const result = await runQuery(root, name);
    assert.ok(Array.isArray(result.data.rows), name);
    assert.equal(result.data.truncated, false, name);
  }
  const decisions = await runQuery(root, 'decisions');
  assert.equal(decisions.data.rows.find(row => row.name === 'no-pulling').status, 'rejected');
  const evidence = (await runQuery(root, 'evidence')).data.rows;
  assert.equal(evidence.length, 2);
  assert.ok(evidence.every(row => row.stage === 'proposed'));
  for (const role of ['hypotheses', 'expectations', 'observations']) {
    assert.ok(!Object.hasOwn(model, role));
    await assert.rejects(runQuery(root, 'records', { collection: role }), { code: 'COLLECTION_UNKNOWN' });
  }
  const planning = await readFile(join(root, 'game-design/experiments/energy-encourages-planning.md'), 'utf8');
  assert.match(planning, /01a15175-3c00-74e8-81e5-dcc6330be146/);
  const timing = await readFile(join(root, 'game-design/experiments/route-cost.md'), 'utf8');
  for (const heading of model.experiments.sections) assert.ok(timing.includes(`# ${heading}`));
  assert.match(timing, /target 8 s; minimum_pass_rate 0.9/);
  assert.match(timing, /upper_target, target_control, and upper_target_control remain null/);
  assert.match(timing, /01a15175-3c00-7003-8c7e-5f8225a0df35/);
  // Progress vocabulary can grow without committing to a transition engine.
  await writeFile(join(root, 'game-design/experiments/route-cost.md'), timing.replace('"stage": "proposed"', '"stage": "awaiting-method-review"'));
  assert.equal((await runQuery(root, 'evidence')).data.rows.find(row => row.name === 'route-cost').stage, 'awaiting-method-review');
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
  const result = cli(root, ['query', 'skill-atoms', '--fields', 'name,rank', '--limit', '2']);
  assert.equal(result.status, 0);
  assert.equal(result.json.data.truncated, true);
  assert.equal(result.json.coverage.status, 'partial');
  assert.deepEqual(Object.keys(result.json.data.rows[0]), ['name', 'rank']);
  assert.deepEqual(result.json.data.columns, ['name', 'rank']);
  assert.equal(cli(root, ['simulate']).json.errors[0].code, 'CAPABILITY_UNAVAILABLE');
  assert.equal(cli(root, ['query', 'skill-atoms', '--limit', '0']).status, 2);
  assert.equal(cli(root, ['query', 'skill-atoms', '--format', 'text']).json.errors[0].code, 'FORMAT_CONFLICT');
  assert.equal(cli(root, ['validate']).status, 0);
  const skill = join(root, 'game-design/skill-atoms/push.md');
  await writeFile(skill, (await readFile(skill, 'utf8')).replace('skill-atoms/move', 'skill-atoms/missing'));
  const invalid = cli(root, ['validate']);
  assert.equal(invalid.status, 1);
  assert.ok(invalid.json.data.diagnostics.some(item => item.code === 'record-ref-missing'));
});

test('snapshot detects source changes and rejects escapes', async () => {
  const root = await temporary(true);
  const before = (await snapshot(root)).metadata;
  const path = join(root, 'game-design/skill-atoms/coordinate.md');
  await writeFile(path, (await readFile(path, 'utf8')).replace('rank: 2', 'rank: 3'));
  assert.notEqual((await snapshot(root)).metadata.fingerprint, before.fingerprint);
  await assert.rejects(snapshot(root, ['../private.txt']), { code: 'PATH_OUTSIDE_PROJECT' });
  assert.equal(cli(root, ['snapshot', '--output', join(root, 'game-design/skill-atoms/overwrite.md')]).json.errors[0].code, 'OUTPUT_SOURCE_CONFLICT');
});

test('all 18 generated packs match catalog, queries, roles, and selected entrypoint links', async () => {
  const entries = await catalog();
  assert.equal(entries.length, 18);
  assert.equal(new Set(entries.map(entry => entry.id)).size, 18);
  const skill = await readFile(join(home, 'SKILL.md'), 'utf8');
  for (const entry of entries) {
    const body = await readFile(join(home, entry.path), 'utf8');
    assert.ok(skill.includes(`(${entry.path})`), entry.id);
    for (const heading of ['Contents', 'Working concepts', 'Apply to selected slice', 'Original prompts', 'Worked example', 'Evidence and counterchecks', 'Common traps']) assert.ok(body.includes(`## ${heading}`), `${entry.id}: ${heading}`);
    const detail = guidance[entry.id];
    assert.ok(detail.concepts.length >= 3 && detail.procedure.length >= 4 && detail.example.length >= 3 && detail.evidence.length >= 2 && detail.traps.length >= 2, entry.id);
    for (const text of Object.values(detail).flat(2)) assert.ok(body.includes(text), `${entry.id}: generated guidance omitted`);
    assert.ok(body.includes('Access checked 2026-10-19:'), entry.id);
    assert.equal(entry.status, 'provisional');
    assert.ok(!body.includes('../review.md'), `${entry.id}: no back-link to already-read parent workflow`);
    assert.ok(!body.includes('Read actual role mapping'), `${entry.id}: shared workflow is not repeated`);
    assert.ok(entry.required_roles.every(role => Object.hasOwn(model, role)), entry.id);
    assert.ok(entry.queries.every(query => ['matrix', 'skill-atoms', 'routes', 'mda', 'decisions', 'glossary', 'controls', 'evidence', 'records'].includes(query)), entry.id);
  }
  assert.ok(skill.indexOf('references/model.md') < skill.indexOf('references/capture.md'));
  for (const path of ['SKILL.md', 'README.md', 'references/setup.md', 'references/capture.md', 'references/inspect.md', 'references/review.md', 'references/cli.md', 'examples/crate-puzzle/README.md']) {
    const text = await readFile(join(home, path), 'utf8');
    assert.ok(!text.includes('>=24.10.0'), `${path}: dependency version belongs to runtime reference`);
    assert.ok(!text.includes('export GD_ATTENDANT_SKILL='), `${path}: environment setup belongs to runtime reference`);
  }
  for (const role of Object.keys(model)) {
    const usage = await readFile(join(home, 'templates', role, '.usage.md'), 'utf8');
    assert.ok(!usage.includes('## Operating rules'), `${role}: shared rules precede collection-specific usage`);
  }
  const brief = await readFile(join(home, 'examples/crate-puzzle/README.md'), 'utf8');
  assert.ok(brief.includes('game-design/concept.md'));
  const concept = await readFile(join(home, 'examples/crate-puzzle/game-design/concept.md'), 'utf8');
  assert.ok(concept.includes('Working mode: game-explorer'));
  assert.ok(concept.includes('## Player story'));
  assert.ok(model.aesthetics.purpose.includes('not the overall player experience'));
  const child = spawnSync(process.execPath, [join(home, 'scripts/build-packs.mjs'), '--check'], { encoding: 'utf8' });
  assert.equal(child.status, 0, child.stdout + child.stderr);
});

function rankFinding() {
  return { scope: ['skill-atoms/coordinate authored rank'], findings: [{ code: 'skill.rank-intent', origin: 'agent', kind: 'unknown_unresolved', severity: 'warning', records: ['01a15175-3c00-7529-8fb6-09255d506737'], sources: [{ path: 'game-design/skill-atoms/coordinate.md', field: 'rank' }], reason: 'Authored rank is before rank-3 prerequisite; interpretation, not native check.', suggested_investigation: 'Ask whether this authored order is intentional.', trigger_key: 'authored-order' }], coverage: { status: 'partial', performed: ['source inspection'] }, limitations: ['No player observations or native C99 checks.'] };
}

test('dated reports preserve history, stable finding identity, and source freshness', async () => {
  const root = await temporary(true);
  const baseline = (await snapshot(root)).metadata;
  const first = await saveReport(root, 'skill-atoms', rankFinding(), baseline);
  assert.match(first.run_id, /^\d{8}T\d{9}Z-[a-f0-9]{8}$/);
  const path = join(root, first.paths[1]);
  const original = await readFile(path, 'utf8');
  const shown = await showReport(root, first.run_id);
  assert.equal(shown.matches_current_source, true);
  assert.equal(shown.historical, true);
  assert.equal(shown.freshly_run, false);
  assert.equal(shown.report.coverage.native_c99_checks, 'unavailable in this release');
  const revised = rankFinding(); revised.findings[0].reason = 'Different prose; same stable source rule.';
  const second = await saveReport(root, 'skill-atoms', revised, baseline);
  assert.notEqual(second.run_id, first.run_id);
  assert.equal((await showReport(root, second.run_id)).report.findings[0].id, shown.report.findings[0].id);
  const skill = join(root, 'game-design/skill-atoms/coordinate.md');
  await writeFile(skill, (await readFile(skill, 'utf8')).replace('rank: 2', 'rank: 3'));
  assert.equal((await showReport(root, first.run_id)).matches_current_source, false);
  assert.equal(await readFile(path, 'utf8'), original);
  await assert.rejects(saveReport(root, 'skill-atoms', revised, baseline), { code: 'SOURCE_CHANGED' });
});

test('report rejects all-lenses, uncaptured sources, invalid structure, and incomplete runs', async () => {
  const root = await temporary(true);
  const baseline = (await snapshot(root)).metadata;
  await assert.rejects(saveReport(root, 'all', rankFinding(), baseline), { code: 'LENS_REQUIRED' });
  const invalid = rankFinding(); invalid.findings[0].sources[0].path = 'private-not-captured.md';
  await assert.rejects(saveReport(root, 'skill-atoms', invalid, baseline), { code: 'SOURCE_NOT_CAPTURED' });
  await assert.rejects(saveReport(root, 'skill-atoms', { ...rankFinding(), coverage: [] }, baseline), { code: 'REPORT_INVALID' });
  await assert.rejects(saveReport(root, 'skill-atoms', { ...rankFinding(), limitations: 'wrong' }, baseline), { code: 'REPORT_INVALID' });
  const first = await saveReport(root, 'skill-atoms', rankFinding(), baseline);
  const metadata = join(root, first.paths[0]);
  await writeFile(metadata, JSON.stringify({ ...JSON.parse(await readFile(metadata, 'utf8')), status: 'writing' }));
  await assert.rejects(showReport(root, first.run_id), { code: 'REPORT_INCOMPLETE' });
});

test('CLI snapshot/output/report round trip and explicit extra-source capture', async () => {
  const root = await temporary(true);
  const snapshotPath = join(root, 'baseline.json');
  const inputPath = join(root, 'findings.json');
  await writeFile(join(root, 'notes.md'), '# Context\n');
  const command = spawnSync(process.execPath, [join(home, 'scripts/workbench.mjs'), 'snapshot', '--project', root, '--include', 'notes.md', '--output', snapshotPath, '--json'], { encoding: 'utf8' });
  assert.equal(command.status, 0, command.stderr);
  assert.equal(command.stdout, '');
  const saved = JSON.parse(await readFile(snapshotPath, 'utf8'));
  assert.ok(saved.data.sources.some(item => item.path === 'notes.md'));
  await writeFile(inputPath, JSON.stringify(rankFinding()));
  const report = cli(root, ['report', 'save', '--lens', 'skill-atoms', '--input', inputPath, '--snapshot', snapshotPath]);
  assert.equal(report.status, 0, JSON.stringify(report.json));
  assert.equal(cli(root, ['report', 'show', report.json.data.run_id]).json.data.matches_current_source, true);
  assert.equal(cli(root, ['snapshot', '--output', snapshotPath]).status, 2); // Exclusive output, no overwrite.
});

test('custom role binding reuses existing contracts and preserves local guidance/extra fields', async () => {
  const root = await temporary(true);
  await rm(join(root, 'game-design/skill-atoms'), { recursive: true });
  await cp(join(home, 'examples/crate-puzzle/game-design/skill-atoms'), join(root, 'game-design/abilities'), { recursive: true });
  const configured = join(root, '.pi/attendant.tables');
  await writeFile(configured, (await readFile(configured, 'utf8')).replace('game-design/skill-atoms as skill-atoms', 'game-design/abilities as abilities'));
  // Reference declarations and actual references must change through explicit migration.
  for (const folder of await readdir(join(root, 'game-design'))) {
    if (folder.endsWith('.md')) continue;
    for (const name of await readdir(join(root, 'game-design', folder))) {
      const path = join(root, 'game-design', folder, name);
      await writeFile(path, (await readFile(path, 'utf8')).replaceAll('ref[]:skill-atoms', 'ref[]:abilities').replaceAll('skill-atoms/', 'abilities/'));
    }
  }
  const usage = join(root, 'game-design/abilities/.usage.md');
  await writeFile(usage, '# Local ability policy\nPreserve this guidance.\n');
  const schema = join(root, 'game-design/abilities/.schema.md');
  await writeFile(schema, (await readFile(schema, 'utf8')).replace('---\n', '---\nlocal_note: ""\n'));
  const options = { mapping: { 'skill-atoms': 'abilities' } };
  const plan = await planSetup(root, options);
  assert.equal(plan.conflicts.length, 0);
  assert.equal(plan.new_collections.length, 0);
  await applySetup(root, plan.plan_id, options);
  assert.equal(await readFile(usage, 'utf8'), '# Local ability policy\nPreserve this guidance.\n');
  assert.match(await readFile(schema, 'utf8'), /local_note/);
  assert.equal((await runQuery(root, 'skill-atoms')).data.rows.length, 6);
});

test('delivered example copy uses local workbench and required external Attendant on populated source', async () => {
  const root = await temporary();
  await cp(join(home, 'examples/crate-puzzle'), root, { recursive: true, filter: source => !['.attendant', 'game-design-results'].includes(basename(source)) });
  const script = join(root, 'tools/game-design-workbench/scripts/workbench.mjs');
  const before = (await snapshot(root)).metadata.fingerprint;
  assert.equal(cli(root, ['validate'], script).status, 0);
  const skillAtoms = cli(root, ['query', 'skill-atoms'], script);
  assert.equal(skillAtoms.json.data.rows.length, 6);
  assert.equal(skillAtoms.json.provenance.fingerprint, before);
  const matrix = cli(root, ['query', 'matrix'], script);
  assert.equal(matrix.json.data.rows.filter(row => row.noun === 'nouns/crate' && row.verb === 'verbs/push').length, 3);
  assert.equal(cli(root, ['resolve', '--term', 'BOX'], script).json.data.state, 'resolved');
  assert.equal(cli(root, ['lenses'], script).json.data.length, 18);
  assert.equal((await snapshot(root)).metadata.fingerprint, before);
  assert.ok(!(await readdir(root)).includes('.attendant'));
  const prior = await snapshot(root);
  const selected = join(root, 'game-design/skill-atoms/coordinate.md');
  const priorText = await readFile(selected, 'utf8');
  const update = spawnSync(process.execPath, [join(attendantHome(), 'scripts/attendant.mjs'), 'update', '-c', 'skill-atoms', '-i', JSON.stringify([{ name: 'coordinate', fields: { rank: 3 } }]), '--project', root], { encoding: 'utf8' });
  assert.equal(update.status, 0, update.stderr);
  const after = await snapshot(root);
  assert.deepEqual(after.metadata.sources.filter(item => prior.metadata.sources.find(old => old.path === item.path)?.sha256 !== item.sha256).map(item => item.path), ['game-design/skill-atoms/coordinate.md']);
  const afterText = await readFile(selected, 'utf8');
  assert.equal(afterText.split('\n---\n')[1], priorText.split('\n---\n')[1]);
  for (const field of ['id', 'name', 'created_at']) assert.equal(afterText.match(new RegExp(`^${field}: (.+)$`, 'm'))[1], priorText.match(new RegExp(`^${field}: (.+)$`, 'm'))[1]);
  const edited = cli(root, ['query', 'skill-atoms'], script).json.data.rows;
  assert.equal(edited.find(row => row.name === 'coordinate').rank, 3);
  assert.equal(edited.find(row => row.name === 'plan-push').rank, null);
});

test('generic validation explicitly does not enforce null-declared scalar subtypes', async () => {
  const root = await temporary(true);
  const path = join(root, 'game-design/skill-atoms/coordinate.md');
  await writeFile(path, (await readFile(path, 'utf8')).replace('rank: 2', 'rank: not-an-integer'));
  const result = cli(root, ['validate']);
  assert.equal(result.status, 0);
  assert.equal(result.json.coverage.status, 'partial');
  assert.ok(result.json.coverage.not_performed[0].includes('semantic'));
  assert.equal((await runQuery(root, 'skill-atoms')).data.rows.find(row => row.name === 'coordinate').rank, 'not-an-integer');
});

test('required Attendant is explicit, never copied, and missing dependency cannot write', async () => {
  const root = await temporary();
  const previous = process.env.GD_ATTENDANT_SKILL;
  try {
    delete process.env.GD_ATTENDANT_SKILL;
    assert.equal(cli(root, ['help']).status, 0);
    assert.equal(cli(root, ['setup', 'plan']).json.errors[0].code, 'ATTENDANT_REQUIRED');
    assert.deepEqual(await readdir(root), []);
    await assert.rejects(planSetup(root), { code: 'ATTENDANT_REQUIRED' });
    for (const path of ['skills/attendant', join(root, 'missing')]) {
      process.env.GD_ATTENDANT_SKILL = path;
      await assert.rejects(planSetup(root), { code: 'ATTENDANT_REQUIRED' });
      assert.deepEqual(await readdir(root), []);
    }
    assert.ok((await bundle()).every(file => !file.path.startsWith('vendor/') && file.path !== 'scripts/pack-guidance.mjs'));
  } finally { if (previous === undefined) delete process.env.GD_ATTENDANT_SKILL; else process.env.GD_ATTENDANT_SKILL = previous; }
});

test('external dependency changes invalidate source/tool fingerprint', async () => {
  const root = await temporary(true);
  const dependency = await temporary();
  await mkdir(join(dependency, 'scripts'));
  await writeFile(join(dependency, 'SKILL.md'), '# Attendant test fixture\n');
  await writeFile(join(dependency, 'scripts/attendant.mjs'), '// Never executed by this fingerprint test.\n');
  const previous = process.env.GD_ATTENDANT_SKILL;
  try {
    process.env.GD_ATTENDANT_SKILL = dependency;
    const first = await snapshot(root);
    assert.equal(first.metadata.dependencies.attendant, await attendantFingerprint());
    await writeFile(join(dependency, 'scripts/attendant.mjs'), '// Changed external dependency.\n');
    const changed = await snapshot(root);
    assert.notEqual(changed.metadata.fingerprint, first.metadata.fingerprint);
    await mkdir(join(dependency, 'references'));
    await writeFile(join(dependency, 'references/commands.md'), '# Changed dependency guidance\n');
    assert.notEqual((await snapshot(root)).metadata.fingerprint, changed.metadata.fingerprint);
  } finally { if (previous === undefined) delete process.env.GD_ATTENDANT_SKILL; else process.env.GD_ATTENDANT_SKILL = previous; }
});

test('update removes only unchanged retired managed files and guards edited legacy copies', async () => {
  const root = await temporary(true);
  let plan = await planSetup(root); await applySetup(root, plan.plan_id, {});
  const packageRoot = join(root, 'tools/game-design-workbench');
  const manifestPath = join(packageRoot, 'manifest.json');
  const legacy = 'vendor/attendant/legacy.mjs';
  const path = join(packageRoot, legacy);
  await mkdir(dirname(path), { recursive: true });
  await writeFile(path, 'legacy\n');
  const manifest = JSON.parse(await readFile(manifestPath, 'utf8'));
  manifest.files[legacy] = hash(Buffer.from('legacy\n'));
  await writeFile(manifestPath, JSON.stringify(manifest, null, 2) + '\n');
  await writeFile(path, 'local changes\n');
  plan = await planSetup(root);
  assert.ok(plan.conflicts.some(item => item.includes('Locally edited retired file')));
  await assert.rejects(applySetup(root, plan.plan_id, {}), { code: 'SETUP_CONFLICT' });
  assert.equal(await readFile(path, 'utf8'), 'local changes\n');
  await writeFile(path, 'legacy\n');
  plan = await planSetup(root);
  assert.ok(plan.files.some(item => item.operation === 'delete' && item.path.endsWith(legacy)));
  await applySetup(root, plan.plan_id, {});
  assert.ok(!(await readdir(packageRoot)).includes('vendor'));
  assert.equal((await planSetup(root)).files.length, 0);
  const owned = 'retired/managed.txt';
  await mkdir(join(packageRoot, 'retired'));
  await writeFile(join(packageRoot, owned), 'owned\n');
  await writeFile(join(packageRoot, 'retired/local.txt'), 'preserve unknown file\n');
  const updated = JSON.parse(await readFile(manifestPath, 'utf8'));
  updated.files[owned] = hash(Buffer.from('owned\n'));
  await writeFile(manifestPath, JSON.stringify(updated, null, 2) + '\n');
  plan = await planSetup(root); await applySetup(root, plan.plan_id, {});
  assert.deepEqual(await readdir(join(packageRoot, 'retired')), ['local.txt']);
  assert.equal(await readFile(join(packageRoot, 'retired/local.txt'), 'utf8'), 'preserve unknown file\n');
  updated.files['../outside.md'] = hash(Buffer.from('outside\n'));
  await writeFile(manifestPath, JSON.stringify(updated, null, 2) + '\n');
  await assert.rejects(planSetup(root), { code: 'MANIFEST_INVALID' });
});

test('fixture generator needs no YAML module or dependency install', async () => {
  const root = await temporary();
  const environment = { ...process.env }; delete environment.GD_ATTENDANT_SKILL;
  const generated = spawnSync(process.execPath, [resolve('skills/game-design-workbench/scripts/build-example.mjs'), '--output', root], { encoding: 'utf8', env: environment });
  assert.equal(generated.status, 0, generated.stderr);
  const result = await runQuery(root, 'skill-atoms');
  assert.equal(result.data.rows.length, 6);
  assert.equal(result.data.rows.find(row => row.name === 'coordinate').rank, 2);
  assert.equal(result.data.rows.find(row => row.name === 'plan-push').rank, null);
  assert.ok(Object.hasOwn(model, 'skill-atoms'));
  assert.ok(!Object.hasOwn(model, 'skills'));
  await assert.rejects(runQuery(root, 'skills'), { code: 'QUERY_UNKNOWN' });
  const records = await runQuery(root, 'records', { collection: 'skill-atoms' });
  assert.equal(records.data.rows.find(row => row.name === 'coordinate').id, '01a15175-3c00-7529-8fb6-09255d506737');
  const identities = async project => (await snapshot(project)).files.flatMap(file => [...file.bytes.toString('utf8').matchAll(/^\s*"?id"?: "?([a-f0-9-]{36})"?,?$/gm)].map(match => match[1])).sort();
  assert.deepEqual(await identities(root), await identities(join(home, 'examples/crate-puzzle')));
});

test('oversized source and out-of-project junction fail without touching target', async () => {
  const root = await temporary(true);
  const path = join(root, 'game-design/skill-atoms/large.md');
  await writeFile(path, Buffer.alloc(2 * 1024 * 1024 + 1));
  await assert.rejects(snapshot(root), { code: 'SOURCE_LIMIT' });
  await rm(path);
  const outside = await temporary();
  await writeFile(join(outside, 'keep.txt'), 'Do not touch.');
  const source = join(root, 'game-design/skill-atoms');
  await rm(source, { recursive: true });
  await symlink(outside, source, process.platform === 'win32' ? 'junction' : 'dir');
  await assert.rejects(snapshot(root), { code: 'SYMLINK_UNSUPPORTED' });
  assert.equal(await readFile(join(outside, 'keep.txt'), 'utf8'), 'Do not touch.');
});
