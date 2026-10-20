#!/usr/bin/env node
// Verify staged workbench/example in a disposable Git-index export. No stage,
// commit, installation, or project-source writes. Only private temp files change.
// Usage: node scripts/check-workbench-export.mjs --run-id ID [--previous-run ID] [--expect-stale] [--json|--text]
// Requires Node >=24.10.0 and GD_ATTENDANT_SKILL. Exit 0 pass, 1 failure, 2 usage.
import { mkdtemp, rm, lstat } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { resolve, dirname, join, relative } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { spawnSync } from 'node:child_process';
import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const args = process.argv.slice(2);
const options = {};
let temporary;
let json = !process.stdout.isTTY || process.env.GD_AGENT === '1';
let exit = 1;
const fail = (code, message) => { throw Object.assign(Error(message), { code }); };
function git(parameters) {
  const child = spawnSync('git', ['-C', root, ...parameters], { encoding: 'utf8', timeout: 60000, maxBuffer: 16 * 1024 * 1024, windowsHide: true });
  if (child.status !== 0) fail('EXPORT_FAILED', child.error?.message ?? (child.stderr.trim() || (parameters[0] === 'diff' ? 'Stage intended workbench changes before verifying index export.' : `git ${parameters[0]} failed.`)));
  return child.stdout.trim();
}
function invoke(helper, project, parameters) {
  const child = spawnSync(process.execPath, [helper, ...parameters, '--project', project, '--json'], { encoding: 'utf8', timeout: 60000, maxBuffer: 16 * 1024 * 1024, windowsHide: true });
  let output;
  try { output = JSON.parse(child.stdout); } catch { fail('CHILD_FAILED', child.error?.message ?? (child.stderr.trim() || 'Workbench returned no JSON.')); }
  if (child.status !== 0 || !output.ok) fail('CHILD_FAILED', output.errors?.[0]?.message ?? (child.stderr.trim() || 'Workbench failed.'));
  return output.data;
}
try {
  if (args.length === 1 && args[0] === '--help') {
    console.log('Verify staged workbench/example: --run-id <saved-run> [--previous-run <stale-run>] [--expect-stale] [--json|--text]. --expect-stale verifies history after source changes without inventing a new review. Stage intended source, installed snapshot, and saved run first. Requires installed Attendant via GD_ATTENDANT_SKILL; never installs, stages, or commits. Default JSON when piped/GD_AGENT=1, text at TTY. Private Git-index export is removed. Exit 0 pass, 1 failure, 2 usage.');
    process.exit(0);
  }
  exit = 2;
  for (let index = 0; index < args.length; index++) {
    const key = args[index];
    if (key === '--expect-stale') {
      if (options[key]) fail('ARGUMENT_INVALID', 'Duplicate --expect-stale.');
      options[key] = true; continue;
    }
    if (['--json', '--text'].includes(key)) {
      if (options.format) fail('ARGUMENT_INVALID', 'Choose one output format.');
      options.format = key; json = key === '--json'; continue;
    }
    if (!['--run-id', '--previous-run'].includes(key) || options[key] || !/^\d{8}T\d{9}Z-[a-f0-9]{8}$/.test(args[index + 1] ?? '')) fail('ARGUMENT_INVALID', 'Use --help; require valid timestamped run IDs.');
    options[key] = args[++index];
  }
  if (!options['--run-id']) fail('ARGUMENT_INVALID', '--run-id is required; stage that saved run first.');
  exit = 1;
  git(['diff', '--exit-code', '--quiet', '--', 'skills/game-design-workbench']);
  const inventory = git(['ls-files', '--stage', '--', 'skills/game-design-workbench']);
  temporary = await mkdtemp(join(tmpdir(), 'gd-index-export-'));
  git(['checkout-index', '--all', `--prefix=${temporary.replaceAll('\\', '/')}/`]);
  const packageHome = join(temporary, 'skills/game-design-workbench');
  const { bundle } = await import(pathToFileURL(join(packageHome, 'scripts/lib.mjs')));
  let localLinkTargets = 0;
  for (const file of await bundle()) {
    if (!file.path.endsWith('.md')) continue;
    for (const match of file.bytes.toString('utf8').matchAll(/\[[^\]]*\]\(([^)]+)\)/g)) {
      const href = match[1];
      if (/^[a-z][a-z\d+.-]*:/i.test(href) || href.startsWith('#')) continue;
      const target = resolve(dirname(join(packageHome, file.path)), decodeURIComponent(href.split('#')[0]));
      if (relative(temporary, target).startsWith('..')) fail('DOC_LINK_INVALID', `${file.path}: target escapes export: ${href}`);
      if (!await lstat(target).catch(error => { if (error.code === 'ENOENT') return null; throw error; })) fail('DOC_LINK_INVALID', `${file.path}: missing target: ${href}`);
      localLinkTargets++;
    }
  }
  const project = join(packageHome, 'examples/crate-puzzle');
  const helper = join(project, 'tools/game-design-workbench/scripts/workbench.mjs');
  const plan = invoke(helper, project, ['setup', 'plan']);
  assert.equal(plan.files.length, 0, 'Exported setup has pending changes.');
  assert.equal(plan.conflicts.length, 0, 'Exported setup conflicts.');
  const context = invoke(helper, project, ['context']);
  assert.equal(context.mode, 'game-explorer');
  assert.equal(context.path, 'game-design/concept.md');
  assert.equal(context.revision.status, 'git_unavailable');
  assert.equal(context.comparison, null, 'Blank example concept must not become original intent.');
  assert.ok(context.current_text.includes('## Player story'));
  const skillAtoms = invoke(helper, project, ['query', 'skill-atoms', '--fields', 'name,rank']);
  assert.equal(skillAtoms.rows.length, 6);
  assert.equal(skillAtoms.rows.find(row => row.name === 'coordinate').rank, 2);
  assert.equal(skillAtoms.rows.find(row => row.name === 'plan-push').rank, null);
  assert.equal(Object.keys(plan.roles).length, 16);
  const experiments = invoke(helper, project, ['query', 'evidence']);
  assert.equal(experiments.rows.length, 2);
  assert.ok(experiments.rows.every(row => row.stage === 'proposed'));
  const review = invoke(helper, project, ['report', 'show', options['--run-id']]);
  assert.equal(review.matches_current_source, !options['--expect-stale'], options['--expect-stale'] ? 'Historical review must remain stale after source changes.' : 'Selected review is stale in export.');
  assert.equal(review.historical, true);
  assert.equal(review.freshly_run, false);
  let previous;
  if (options['--previous-run']) {
    previous = invoke(helper, project, ['report', 'show', options['--previous-run']]);
    assert.equal(previous.matches_current_source, false, 'Previous review should stay stale.');
    assert.deepEqual(review.report.findings.map(item => item.id).sort(), previous.report.findings.map(item => item.id).sort());
  }
  assert.equal(git(['ls-files', '--stage', '--', 'skills/game-design-workbench']), inventory, 'Workbench index changed during verification.');
  const data = { base_commit: git(['rev-parse', 'HEAD']), index_workbench_inventory_sha256: createHash('sha256').update(inventory).digest('hex'), platform: process.platform, node: process.versions.node, setup_changes: 0, conflicts: 0, local_link_targets_checked: localLinkTargets, working_mode: context.mode, concept_path: context.path, concept_history_status: context.revision.status, concept_sha256: context.sha256, concept_comparison: context.comparison, collections: Object.keys(plan.roles).length, experiment_rows: experiments.rows.length, skill_atom_rows: skillAtoms.rows.length, saved_review: options['--run-id'], saved_review_source_match: review.matches_current_source, expected_stale: Boolean(options['--expect-stale']), historical: true, freshly_run: false, ...(previous ? { previous_review_source_match: false, stable_finding_ids: true } : {}), dependency_fingerprint: plan.attendant_fingerprint, review_dependency_fingerprint: review.report.source_state.dependencies?.attendant ?? null };
  await rm(temporary, { recursive: true, force: true }); temporary = undefined;
  console.log(json ? JSON.stringify({ schema_version: 1, ok: true, data, errors: [] }) : `Export passes: setup unchanged; six skill atoms; two proposed investigations; saved review ${options['--expect-stale'] ? 'stays stale as expected' : 'matches'}; required Attendant stays external.${previous ? ' Previous review stays stale; finding IDs stable.' : ''}`);
} catch (error) {
  console.error(error.message);
  if (json) console.log(JSON.stringify({ schema_version: 1, ok: false, data: null, errors: [{ code: error.code ?? 'CHECK_FAILED', message: error.message }] }));
  process.exitCode = exit;
} finally {
  if (temporary) await rm(temporary, { recursive: true, force: true });
}
