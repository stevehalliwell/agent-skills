import { readFile, mkdir, writeFile } from 'node:fs/promises';
import { join } from 'node:path';
import { randomBytes } from 'node:crypto';
import { readGit } from './concept.mjs';
import { home, snapshot, safePath, hash, fail, atomicWrite } from './lib.mjs';
export async function catalog() { return JSON.parse(await readFile(join(home, 'references/frameworks/catalog.json'), 'utf8')); }
const kinds = new Set(['invalid_broken', 'missing_required_relationship', 'unknown_unresolved', 'possible_opportunity', 'conflicting_claims', 'unverified_hypothesis', 'deliberately_excluded']);
const text = value => String(value).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/[\\`*_[\]]/g, '\\$&');
const git = (root, args) => readGit(root, args)?.toString('utf8').trim() ?? null;
export async function saveReport(root, lens, input, baseline) {
  const pack = (await catalog()).find(item => item.id === lens);
  if (!pack) fail('LENS_REQUIRED', 'Choose one bundled lens using lenses; no all-lenses selector.');
  if (!baseline || !Array.isArray(baseline.includes) || !/^[a-f0-9]{64}$/.test(baseline.fingerprint ?? '')) fail('SNAPSHOT_REQUIRED', 'Capture snapshot before review; pass its saved metadata.');
  const comparison = baseline.project_context?.comparison;
  const comparisonOptions = comparison ? { intentRef: comparison.commit, intentPath: comparison.path } : {};
  const current = (await snapshot(root, baseline.includes, comparisonOptions)).metadata;
  if (current.fingerprint !== baseline.fingerprint) fail('SOURCE_CHANGED', 'Source/tool state changed since review snapshot. Reinspect and capture new snapshot; do not relabel old findings as current.');
  if (current.project_context.status !== 'ready') fail(current.project_context.issue?.code ?? 'CONCEPT_MODE_REQUIRED', 'Establish project concept and working mode before framework review; no mode or original intent is inferred.');
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
      if (source.revision !== undefined) {
        if (source.revision !== current.project_context.comparison?.commit || source.path !== current.project_context.comparison?.path) fail('SOURCE_NOT_CAPTURED', 'Historical source must match concept path/revision selected before review.');
      } else if (!current.sources.some(item => item.path === source.path)) fail('SOURCE_NOT_CAPTURED', `Finding source is not in frozen inventory: ${source.path}. Include it before reviewing.`);
    }
    if (finding.intent_relation !== undefined) {
      if (!['supports', 'contradicts', 'tension', 'unknown', 'outside_intent', 'possible_goal_drift'].includes(finding.intent_relation) || !finding.sources.some(source => source.path === current.project_context.path || source.revision === current.project_context.comparison?.commit && source.path === current.project_context.comparison?.path)) fail('REPORT_INVALID', 'Intent relation needs a supported label and a captured concept-section citation; it is declared interpretation, not proof.');
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
  const metadata = { schema_version: 1, run_id, created_at, lens, pack_version: pack.version, scope: input.scope, project_context: current.project_context, source_state: current, git_commit: git(root, ['rev-parse', 'HEAD']), git_dirty: gitStatus === null ? null : Boolean(gitStatus), status: 'writing' };
  const metadataPath = `game-design-results/${run_id}/run.json`;
  const pending = Buffer.from(`${JSON.stringify(metadata, null, 2)}\n`);
  await writeFile(join(directory, 'run.json'), pending, { flag: 'wx' });
  const report = { schema_version: 1, ...metadata, status: 'complete', findings, coverage: { ...input.coverage, status, native_c99_checks: 'unavailable in this release' }, limitations: input.limitations ?? [], interpretation: 'Submitted findings retain their declared evidence origin; saving validates structure/source state, not design truth.' };
  await writeFile(join(directory, 'review.json'), `${JSON.stringify(report, null, 2)}\n`, { flag: 'wx' });
  const body = `# ${text(pack.title)} review\n\nRun: ${run_id}\nDate: ${created_at}\nScope: ${input.scope.map(text).join(', ')}\nCoverage: ${status}. Historical report; source match is checked separately.\n\nWorking mode: ${text(current.project_context.mode)}\nConcept: ${text(current.project_context.path)}; ${text(current.project_context.revision.status)}; commit ${text(current.project_context.revision.commit ?? 'unavailable')}.\nComparison: ${text(current.project_context.comparison ? `${current.project_context.comparison.path} @ ${current.project_context.comparison.commit}` : 'none selected; original intent is not inferred')}.\n\n${findings.length ? findings.map(finding => `## ${finding.code}\n\n${finding.origin}; ${finding.severity}; ${finding.kind}\n\n${text(finding.reason)}\n\nInvestigate: ${text(finding.suggested_investigation)}\n\n${finding.intent_relation ? `Intent relation: ${text(finding.intent_relation)}\n\n` : ''}${finding.sources.map(source => source.revision ? `- ${text(source.path)} @ ${source.revision} — ${text(source.field ?? source.section)}` : `- [${text(source.path)}](../../${source.path.split(/[\\/]/).map(encodeURIComponent).join('/')}) — ${text(source.field ?? source.section)}`).join('\n')}`).join('\n\n') : 'No findings submitted; this is not completeness evidence.'}\n\n## Coverage and limitations\n\n${text(JSON.stringify(report.coverage))}\n\n${(report.limitations ?? []).map(item => `- ${text(item)}`).join('\n')}\n`;
  await writeFile(join(directory, 'review.md'), body, { flag: 'wx' });
  if ((await snapshot(root, current.includes, comparisonOptions)).metadata.fingerprint !== current.fingerprint) fail('SOURCE_CHANGED', `Source changed while saving ${run_id}; run remains incomplete.`);
  await atomicWrite(root, metadataPath, Buffer.from(`${JSON.stringify({ ...metadata, status: 'complete' }, null, 2)}\n`), hash(pending));
  return { run_id, paths: [`${metadataPath}`, `game-design-results/${run_id}/review.json`, `game-design-results/${run_id}/review.md`], findings: findings.length };
}
export async function showReport(root, run) {
  if (!/^\d{8}T\d{9}Z-[a-f0-9]{8}$/.test(run)) fail('RUN_INVALID', 'Use saved timestamped run ID, not a path.');
  const metadata = JSON.parse(await readFile(await safePath(root, `game-design-results/${run}/run.json`), 'utf8'));
  if (metadata.status !== 'complete') fail('REPORT_INCOMPLETE', 'Report write did not complete. Inspect run directory; do not treat as a completed review.');
  const report = JSON.parse(await readFile(await safePath(root, `game-design-results/${run}/review.json`), 'utf8'));
  const comparison = report.source_state.project_context?.comparison;
  let current;
  try { current = (await snapshot(root, report.source_state.includes, comparison ? { intentRef: comparison.commit, intentPath: comparison.path } : {})).metadata; }
  catch (error) {
    if (error.code !== 'CONCEPT_HISTORY_UNAVAILABLE') throw error;
    return { report, historical: true, matches_current_source: false, freshly_run: false, comparison_unavailable: error.message };
  }
  return { report, historical: true, matches_current_source: current.fingerprint === report.source_state.fingerprint, freshly_run: false };
}
