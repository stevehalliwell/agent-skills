#!/usr/bin/env node
// Verify staged workbench/example in a disposable Git-index export. No stage,
// commit, installation, or project-source writes. Only private temp files change.
// Usage: node scripts/check-workbench-export.mjs --run-id ID [--previous-run ID] [--json|--text]
// Requires Node >=24.10.0 and GD_ATTENDANT_SKILL. Exit 0 pass, 1 failure, 2 usage.
import { mkdtemp, rm } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { resolve, dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
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
    console.log('Verify staged workbench/example: --run-id <saved-run> [--previous-run <stale-run>] [--json|--text]. Stage intended source, installed snapshot, and saved run first. Requires installed Attendant via GD_ATTENDANT_SKILL; never installs, stages, or commits. Default JSON when piped/GD_AGENT=1, text at TTY. Private Git-index export is removed. Exit 0 pass, 1 failure, 2 usage.');
    process.exit(0);
  }
  exit = 2;
  for (let index = 0; index < args.length; index++) {
    const key = args[index];
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
  const project = join(temporary, 'skills/game-design-workbench/examples/crate-puzzle');
  const helper = join(project, 'tools/game-design-workbench/scripts/workbench.mjs');
  const plan = invoke(helper, project, ['setup', 'plan']);
  assert.equal(plan.files.length, 0, 'Exported setup has pending changes.');
  assert.equal(plan.conflicts.length, 0, 'Exported setup conflicts.');
  const skills = invoke(helper, project, ['query', 'skills', '--fields', 'name,rank']);
  assert.equal(skills.rows.length, 6);
  assert.equal(skills.rows.find(row => row.name === 'coordinate').rank, 2);
  assert.equal(skills.rows.find(row => row.name === 'plan-push').rank, null);
  const review = invoke(helper, project, ['report', 'show', options['--run-id']]);
  assert.equal(review.matches_current_source, true, 'Selected review is stale in export.');
  assert.equal(review.historical, true);
  assert.equal(review.freshly_run, false);
  let previous;
  if (options['--previous-run']) {
    previous = invoke(helper, project, ['report', 'show', options['--previous-run']]);
    assert.equal(previous.matches_current_source, false, 'Previous review should stay stale.');
    assert.deepEqual(review.report.findings.map(item => item.id).sort(), previous.report.findings.map(item => item.id).sort());
  }
  assert.equal(git(['ls-files', '--stage', '--', 'skills/game-design-workbench']), inventory, 'Workbench index changed during verification.');
  const data = { base_commit: git(['rev-parse', 'HEAD']), index_workbench_inventory_sha256: createHash('sha256').update(inventory).digest('hex'), platform: process.platform, node: process.versions.node, setup_changes: 0, conflicts: 0, skill_rows: skills.rows.length, saved_review: options['--run-id'], saved_review_source_match: true, historical: true, freshly_run: false, ...(previous ? { previous_review_source_match: false, stable_finding_ids: true } : {}), dependency_fingerprint: review.report.source_state.dependencies.attendant };
  await rm(temporary, { recursive: true, force: true }); temporary = undefined;
  console.log(json ? JSON.stringify({ schema_version: 1, ok: true, data, errors: [] }) : `Export passes: setup unchanged; six skills; saved review matches; required Attendant stays external.${previous ? ' Previous review stays stale; finding IDs stable.' : ''}`);
} catch (error) {
  console.error(error.message);
  if (json) console.log(JSON.stringify({ schema_version: 1, ok: false, data: null, errors: [{ code: error.code ?? 'CHECK_FAILED', message: error.message }] }));
  process.exitCode = exit;
} finally {
  if (temporary) await rm(temporary, { recursive: true, force: true });
}
