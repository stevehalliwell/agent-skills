// Project concept text and Git provenance; no YAML parser or mutable history store.
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
export const defaultConceptPath = 'game-design/concept.md';
export const workingModes = ['game-design', 'game-explorer'];
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const reject = (code, message) => { throw Object.assign(Error(message), { code }); };
export function conceptPath(value = defaultConceptPath) {
  const canonical = typeof value === 'string' && process.platform === 'win32' ? value.toLowerCase() : value;
  if (typeof value !== 'string' || !value.endsWith('.md') || value.includes('\\') || value.includes(':') || /[\u0000-\u001f\u007f]/.test(value) || value.startsWith('/') || value.split('/').some(part => !part || part.startsWith('.')) || canonical === (process.platform === 'win32' ? 'agents.md' : 'AGENTS.md') || canonical.startsWith('tools/')) reject('CONCEPT_PATH_INVALID', 'Concept must be an in-project Markdown document outside hidden paths, tools, and AGENTS.md.');
  return value;
}
function decode(bytes) {
  try { return new TextDecoder('utf-8', { fatal: true, ignoreBOM: true }).decode(bytes); }
  catch { reject('CONCEPT_ENCODING_INVALID', 'Concept must be UTF-8; no conversion or write attempted.'); }
}
function modeLine(text) {
  // Own a single plain-Markdown header line, not a new YAML dialect. Body/code
  // below first section cannot change project mode.
  const frontmatter = /^(?:\uFEFF)?---\r?\n[\s\S]*?\r?\n---(?:\r?\n|$)/.exec(text)?.[0] ?? '';
  let offset = frontmatter.length || (text.startsWith('\uFEFF') ? 1 : 0); let fence = null; const lines = [];
  for (const line of text.slice(offset).split(/(?<=\n)/)) {
    const raw = line.replace(/\r?\n$/, '');
    const marker = /^ {0,3}(`{3,}|~{3,})/.exec(raw)?.[1];
    if (marker && (!fence || marker[0] === fence[0] && marker.length >= fence.length)) fence = fence ? null : marker;
    else if (!fence) {
      if (/^##\s/.test(raw)) break;
      if (raw.startsWith('Working mode:')) lines.push({ line: raw, index: offset });
    }
    offset += line.length;
  }
  if (lines.length > 1) reject('CONCEPT_MODE_INVALID', 'Keep exactly one Working mode line before first ## section.');
  if (!lines.length) return null;
  const match = /^Working mode: (game-design|game-explorer)\s*$/.exec(lines[0].line);
  if (!match) reject('CONCEPT_MODE_INVALID', 'Working mode must be game-design or game-explorer.');
  return { ...lines[0], mode: match[1] };
}
export function withMode(bytes, mode) {
  if (!workingModes.includes(mode)) reject('CONCEPT_MODE_INVALID', 'Choose game-design or game-explorer; no default mode is inferred.');
  const text = decode(bytes); const current = modeLine(text);
  if (current?.mode === mode) return bytes;
  if (current) return Buffer.from(`${text.slice(0, current.index)}Working mode: ${mode}${text.slice(current.index + current.line.length)}`);
  const newline = text.includes('\r\n') ? '\r\n' : '\n';
  const start = /^(?:\uFEFF)?---\r?\n[\s\S]*?\r?\n---(?:\r?\n|$)/.exec(text)?.[0].length ?? (text.startsWith('\uFEFF') ? 1 : 0);
  return Buffer.from(`${text.slice(0, start)}Working mode: ${mode}${newline}${newline}${text.slice(start)}`);
}
export function readGit(root, args) {
  const env = { ...process.env, GIT_NO_LAZY_FETCH: '1', GIT_NO_REPLACE_OBJECTS: '1', GIT_TERMINAL_PROMPT: '0' };
  // --project, not ambient Git subprocess state, selects repository/worktree.
  for (const key of ['GIT_DIR', 'GIT_WORK_TREE', 'GIT_COMMON_DIR', 'GIT_INDEX_FILE', 'GIT_OBJECT_DIRECTORY', 'GIT_ALTERNATE_OBJECT_DIRECTORIES', 'GIT_NAMESPACE']) delete env[key];
  const child = spawnSync('git', ['--no-lazy-fetch', '--literal-pathspecs', '-C', root, ...args], { timeout: 5000, maxBuffer: 2 * 1024 * 1024, windowsHide: true, env });
  if (child.error || child.status !== 0) return null;
  return child.stdout;
}
const sameText = (a, b) => a.toString('latin1').replaceAll('\r\n', '\n') === b.toString('latin1').replaceAll('\r\n', '\n');
export function describeConcept(root, path, bytes, options = {}) {
  let mode = null; let issue = null;
  try { mode = bytes ? modeLine(decode(bytes))?.mode ?? null : null; }
  catch (error) { issue = { code: error.code, message: error.message }; }
  const prefixBytes = readGit(root, ['rev-parse', '--show-prefix']);
  const prefix = prefixBytes?.toString('utf8').replace(/\r?\n$/, '') ?? null;
  const log = prefix !== null ? readGit(root, ['log', '-1', '--format=%H', '--', path]) : null;
  const commit = log?.toString('utf8').trim() || null;
  const committed = commit ? readGit(root, ['show', `${commit}:${prefix}${path}`]) : null;
  const unavailable = prefix === null || log === null || Boolean(commit && !committed);
  const revision = { commit, status: !bytes ? 'missing' : unavailable ? 'git_unavailable' : !commit ? 'untracked' : committed && sameText(bytes, committed) ? 'committed' : 'working_draft', committed_sha256: committed ? sha(committed) : null };
  revision.working_draft = ['untracked', 'working_draft'].includes(revision.status) ? true : revision.status === 'committed' ? false : null;
  const result = { path, mode, status: issue ? 'invalid' : mode ? 'ready' : 'unconfigured', issue, sha256: bytes ? sha(bytes) : null, revision, comparison: null };
  if (options.includeText) result.current_text = bytes?.toString('utf8') ?? null;
  if (options.intentPath !== undefined && options.intentRef === undefined) reject('CONCEPT_REVISION_REQUIRED', '--intent-path requires --intent-ref.');
  if (options.intentRef !== undefined) {
    if (!/^(?:[a-f0-9]{40}|[a-f0-9]{64})$/.test(options.intentRef)) reject('CONCEPT_REVISION_INVALID', 'Use exact full Git commit ID, not HEAD, a branch, or an abbreviated revision.');
    const oldPath = conceptPath(options.intentPath === undefined ? path : options.intentPath);
    const resolved = readGit(root, ['rev-parse', '--verify', `${options.intentRef}^{commit}`])?.toString('utf8').trim();
    if (prefix === null || !resolved) reject('CONCEPT_HISTORY_UNAVAILABLE', 'Selected concept commit is unavailable locally; no fetch or commit attempted.');
    const historical = readGit(root, ['show', `${resolved}:${prefix}${oldPath}`]);
    if (!historical) reject('CONCEPT_HISTORY_UNAVAILABLE', 'Concept path is absent/oversized at selected commit. Use --intent-path for a former filename.');
    const historicalText = decode(historical);
    result.comparison = { path: oldPath, commit: resolved, sha256: sha(historical) };
    if (options.includeText) result.comparison.text = historicalText;
  }
  return result;
}
