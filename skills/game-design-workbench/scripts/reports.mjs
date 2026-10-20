import { readFile, mkdir, writeFile } from 'node:fs/promises';
import { join } from 'node:path';
import { randomBytes } from 'node:crypto';
import { spawnSync } from 'node:child_process';
import { home, snapshot, safePath, hash, fail, atomicWrite } from './lib.mjs';
export async function catalog() { return JSON.parse(await readFile(join(home, 'references/frameworks/catalog.json'), 'utf8')); }
const kinds = new Set(['invalid_broken', 'missing_required_relationship', 'unknown_unresolved', 'possible_opportunity', 'conflicting_claims', 'unverified_hypothesis', 'deliberately_excluded']);
const text = value => String(value).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/[\\`*_[\]]/g, '\\$&');
function git(root, args) { const result = spawnSync('git', ['-C', root, ...args], { encoding: 'utf8', timeout: 5000, windowsHide: true }); return result.status === 0 ? result.stdout.trim() : null; }
export async function saveReport(root, lens, input, baseline) {
  const pack = (await catalog()).find(item => item.id === lens);
  if (!pack) fail('LENS_REQUIRED', 'Choose one bundled lens using lenses; no all-lenses selector.');
  if (!baseline || !Array.isArray(baseline.includes) || !/^[a-f0-9]{64}$/.test(baseline.fingerprint ?? '')) fail('SNAPSHOT_REQUIRED', 'Capture snapshot before review; pass its saved metadata.');
  const current = (await snapshot(root, baseline.includes)).metadata;
  if (current.fingerprint !== baseline.fingerprint) fail('SOURCE_CHANGED', 'Source/tool state changed since review snapshot. Reinspect and capture new snapshot; do not relabel old findings as current.');
  if (!input || !Array.isArray(input.findings) || input.findings.length > 10000 || !Array.isArray(input.scope) || !input.scope.every(item => typeof item === 'string')) fail('REPORT_INVALID', 'Report needs scope string array and findings array (at most 10000).');
  if (input.coverage !== undefined && (!input.coverage || typeof input.coverage !== 'object' || Array.isArray(input.coverage))) fail('REPORT_INVALID', 'coverage must be an object.');
  if (input.limitations !== undefined && (!Array.isArray(input.limitations) || !input.limitations.every(item => typeof item === 'string'))) fail('REPORT_INVALID', 'limitations must be a string array.');
  const ids = new Set();
  const findings = [];
  for (const finding of input.findings) {
    if (!finding || !/^[a-z][a-z0-9_.-]+$/.test(finding.code ?? '') || !['agent', 'deterministic'].includes(finding.origin) || !['error', 'warning', 'info'].includes(finding.severity) || !kinds.has(finding.kind) || !Array.isArray(finding.records) || !finding.records.every(record => typeof record === 'string' && /^[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}$/.test(record)) || !Array.isArray(finding.sources) || typeof finding.reason !== 'string' || typeof finding.suggested_investigation !== 'string') fail('REPORT_INVALID', 'Each finding needs code/origin/kind/severity/records/sources/reason/suggested_investigation.');
    for (const source of finding.sources) {
      if (!source || typeof source.path !== 'string' || (typeof source.field !== 'string' && typeof source.section !== 'string')) fail('REPORT_INVALID', 'Each finding source needs project-relative path and field or section.');
      if (/^(?:[A-Za-z]:|[\\/])/.test(source.path)) fail('REPORT_INVALID', 'Finding source paths must be project-relative.');
      await safePath(root, source.path);
      if (!current.sources.some(item => item.path === source.path)) fail('SOURCE_NOT_CAPTURED', `Finding source is not in frozen inventory: ${source.path}. Include it before reviewing.`);
    }
    const identity = hash(JSON.stringify([finding.code, [...new Set(finding.records)].sort(), finding.sources.map(source => [source.field ?? '', source.section ?? '']).sort(), finding.trigger_key ?? ''])).slice(0, 24);
    if (ids.has(identity)) fail('REPORT_INVALID', 'Duplicate finding identity; use distinct trigger_key for distinct triggers.');
    ids.add(identity);
    findings.push({ ...finding, id: identity });
  }
  const status = input.coverage?.status ?? 'partial';
  if (!['partial', 'complete'].includes(status)) fail('REPORT_INVALID', 'Coverage status must be partial or complete within declared scope.');
  const created_at = new Date().toISOString();
  const run_id = `${created_at.replace(/[-:.]/g, '')}-${randomBytes(4).toString('hex')}`;
  const directory = await safePath(root, `game-design-results/${run_id}`);
  await mkdir(await safePath(root, 'game-design-results'), { recursive: true });
  await mkdir(directory); // Exclusive run directory; never reuse/overwrite.
  const gitStatus = git(root, ['status', '--porcelain']);
  const metadata = { schema_version: 1, run_id, created_at, lens, pack_version: pack.version, scope: input.scope, source_state: current, git_commit: git(root, ['rev-parse', 'HEAD']), git_dirty: gitStatus === null ? null : Boolean(gitStatus), status: 'writing' };
  const metadataPath = `game-design-results/${run_id}/run.json`;
  const pending = Buffer.from(`${JSON.stringify(metadata, null, 2)}\n`);
  await writeFile(join(directory, 'run.json'), pending, { flag: 'wx' });
  const report = { schema_version: 1, ...metadata, status: 'complete', findings, coverage: { ...input.coverage, status, native_c99_checks: 'unavailable in this release' }, limitations: input.limitations ?? [], interpretation: 'Submitted findings retain their declared evidence origin; saving validates structure/source state, not design truth.' };
  await writeFile(join(directory, 'review.json'), `${JSON.stringify(report, null, 2)}\n`, { flag: 'wx' });
  const body = `# ${text(pack.title)} review\n\nRun: ${run_id}\nDate: ${created_at}\nScope: ${input.scope.map(text).join(', ')}\nCoverage: ${status}. Historical report; source match is checked separately.\n\n${findings.length ? findings.map(finding => `## ${finding.code}\n\n${finding.origin}; ${finding.severity}; ${finding.kind}\n\n${text(finding.reason)}\n\nInvestigate: ${text(finding.suggested_investigation)}\n\n${finding.sources.map(source => `- [${text(source.path)}](../../${source.path.split(/[\\/]/).map(encodeURIComponent).join('/')}) — ${text(source.field ?? source.section)}`).join('\n')}`).join('\n\n') : 'No findings submitted; this is not completeness evidence.'}\n\n## Coverage and limitations\n\n${text(JSON.stringify(report.coverage))}\n\n${(report.limitations ?? []).map(item => `- ${text(item)}`).join('\n')}\n`;
  await writeFile(join(directory, 'review.md'), body, { flag: 'wx' });
  if ((await snapshot(root, current.includes)).metadata.fingerprint !== current.fingerprint) fail('SOURCE_CHANGED', `Source changed while saving ${run_id}; run remains incomplete.`);
  await atomicWrite(root, metadataPath, Buffer.from(`${JSON.stringify({ ...metadata, status: 'complete' }, null, 2)}\n`), hash(pending));
  return { run_id, paths: [`${metadataPath}`, `game-design-results/${run_id}/review.json`, `game-design-results/${run_id}/review.md`], findings: findings.length };
}
export async function showReport(root, run) {
  if (!/^\d{8}T\d{9}Z-[a-f0-9]{8}$/.test(run)) fail('RUN_INVALID', 'Use saved timestamped run ID, not a path.');
  const metadata = JSON.parse(await readFile(await safePath(root, `game-design-results/${run}/run.json`), 'utf8'));
  if (metadata.status !== 'complete') fail('REPORT_INCOMPLETE', 'Report write did not complete. Inspect run directory; do not treat as a completed review.');
  const report = JSON.parse(await readFile(await safePath(root, `game-design-results/${run}/review.json`), 'utf8'));
  const current = (await snapshot(root, report.source_state.includes)).metadata;
  return { report, historical: true, matches_current_source: current.fingerprint === report.source_state.fingerprint, freshly_run: false };
}
