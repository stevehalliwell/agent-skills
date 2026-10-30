#!/usr/bin/env node
/** Migrate only shipped example gd-rule bodies using native declaration lines.
 * Attendant owns metadata; public strict validation/schema inspection guard operation.
 * Native C owns rule parsing and equivalence evidence. No JS rule evaluator.
 * Plan by default. Stage/verify before apply; per-file replacement is atomic, not a batch
 * transaction. Preserve all other bytes and refuse stale source. Safe re-run skips cases
 * already anchored. No experiments, pool inference, metadata edits or frozen rewrites.
 */
import { readFile, writeFile, mkdir, cp, rename, realpath, readdir, mkdtemp, rm, open, lstat } from 'node:fs/promises';
import { resolve, join, relative, isAbsolute, dirname, basename } from 'node:path';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { isDeepStrictEqual } from 'node:util';

const examples = ['crate-puzzle', 'little-plumber-guy-nes'];
const roles = ['interactions', 'mechanics', 'nouns', 'axes', 'verbs', 'resources', 'control-vars'];
const help = `Usage: node scripts/migrate-workbench-primary-targets.mjs --root REPO --native GD_EXE --attendant RUNNER
  [--targets FILE] [--stage NEW_DIRECTORY | --apply] [--json] [--fields summary,actions]
Plan only by default. Targets JSON: {"EXAMPLE": {"ALIAS/RECORD#CASE": "Participant"}}.
Use null only for an explicitly deferred anchor. Unique authored nouns participant or
single participant follows approved migration convention; ambiguous cases require targets.
Already anchored cases and participant-free drafts stay unchanged. Stage verifies native
rule/Resource equivalence and all outside-fence bytes before apply. Exit 0 success, 2 failure;
JSON errors include partial/changed paths. stdout is data, stderr is diagnostics.
`;
const options = {};
const changed = [], temporaryFiles = new Set();
let mode = 'plan', stage;
let json = !process.stdout.isTTY || Boolean(process.env.GD_AGENT || process.env.CI);
function fail(code, message) { const error = new Error(message); error.code = code; throw error; }
function inside(parent, child) {
  const path = relative(parent, child);
  return path === '' || (!isAbsolute(path) && path !== '..' && !path.startsWith('../') && !path.startsWith('..\\'));
}
/* Resolve nearest existing ancestor before creating output. Lexical containment
   alone is unsafe when an external parent is a junction back into source. */
async function destination(path) {
  let current = resolve(path);
  const missing = [];
  for (;;) {
    try { return resolve(await realpath(current), ...missing); }
    catch (error) {
      if (error.code !== 'ENOENT' || dirname(current) === current) throw error;
      const entry = await lstat(current).catch(error => { if (error.code === 'ENOENT') return null; throw error; });
      if (entry) fail('migration.containment', `Existing destination ancestor cannot be canonicalized: ${current}`);
      missing.unshift(basename(current)); current = dirname(current);
    }
  }
}
const hash = bytes => createHash('sha256').update(bytes).digest('hex');
function run(executable, args) {
  const result = spawnSync(executable, args, { encoding: 'utf8', timeout: 120000, maxBuffer: 64 * 1024 * 1024 });
  if (result.status !== 0) fail('migration.command', `${basename(executable)} ${args[0]} failed: ${(result.stderr || result.error?.message || result.stdout).trim().slice(0, 1500)}`);
  try { return JSON.parse(result.stdout); } catch { fail('migration.output', `${args[0]} returned invalid JSON`); }
}
const attendant = (project, command, ...args) => run(process.execPath, [options.attendant, command, '-p', project, ...args]);
function native(project, operation) {
  return run(options.native, [operation, ...roles.flatMap(role => ['--' + role, `${role}=${join(project, 'game-design', role)}`]), '--json']);
}
async function preflight(project) {
  const validation = attendant(project, 'validate', '--strict');
  if (validation.diagnostics?.length || validation.corrections?.length) fail('migration.validation', `Pending source diagnostics/corrections: ${project}`);
  const schema = attendant(project, 'schema');
  for (const name of ['interactions', 'mechanics']) {
    const collection = schema.collections.find(item => item.name === name);
    if (!collection?.usage?.trim() || !collection.fields) fail('migration.schema', `Missing schema/usage: ${name}`);
    const directory = await realpath(join(project, 'game-design', name));
    if (!inside(await realpath(project), directory) || directory !== await realpath(collection.directory))
      fail('migration.containment', `Unexpected collection owner: ${name}`);
  }
  return validation.records;
}
async function files(root, prefix = '') {
  const result = new Map();
  for (const entry of await readdir(join(root, prefix), { withFileTypes: true })) {
    if (entry.name === '.attendant') continue;
    const name = prefix ? join(prefix, entry.name) : entry.name;
    if (entry.isSymbolicLink()) fail('migration.symlink', `Refuse example symlink: ${join(root, name)}`);
    if (entry.isDirectory()) for (const pair of await files(root, name)) result.set(...pair);
    else if (entry.isFile()) result.set(name, hash(await readFile(join(root, name))));
  }
  return result;
}
/* Compare native facts, excluding only source locations, new anchor provenance
   and measured construction work (anchor metadata adds mapping work).
   Do not remove identities, statuses, guards, conditions, expressions, resolutions,
   call maps, effect multiplicity, units, quantities or graph relations. */
function comparable(value, parent = '') {
  if (Array.isArray(value)) return value.map(item => comparable(item, parent));
  if (value && typeof value === 'object') return Object.fromEntries(Object.entries(value)
    .filter(([key]) => !['source_path', 'line', 'field_lines', 'primary_target', 'target_origin'].includes(key) && !key.endsWith('_line') && !(parent === 'collections' && key === 'directory'))
    .filter(([key]) => !(parent === 'instantiation' && key === 'concrete_primary_target')
      && !(parent === 'coverage' && key === 'work'))
    .map(([key, item]) => [key, comparable(item, key)]));
  return value;
}
function outsideFence(text) {
  const pattern = /^(`{3,}|~{3,})gd-rule[ \t]*\r?\n[\s\S]*?^\1[ \t]*(?:\r?\n|$)/gm;
  const matches = [...text.matchAll(pattern)];
  if (matches.length !== 1) fail('migration.fence', 'Expected one ordinary authoritative rule fence');
  return text.slice(0, matches[0].index) + text.slice(matches[0].index + matches[0][0].length);
}
function choose(example, item, record, overrides) {
  const key = `${item.record}#${item.case}`;
  if (item.primary_target) {
    if (Object.hasOwn(overrides, key) && overrides[key] !== item.primary_target) fail('migration.stale_target', `Existing anchor differs from proposal: ${example}/${key}`);
    return { key, target: item.primary_target, reason: 'already_authored', skip: true };
  }
  if (!item.participants.length) return { key, target: null, reason: 'participant_free_draft', skip: true };
  let target, reason;
  if (Object.hasOwn(overrides, key)) { target = overrides[key]; reason = target === null ? 'explicitly_deferred' : 'authored_exception'; }
  else {
    const nouns = record.relationships.filter(edge => edge.field === 'nouns').map(edge => edge.reference);
    const candidates = item.participants.filter(actor => nouns.includes(actor.noun));
    if (candidates.length === 1) { target = candidates[0].name; reason = 'unique_authored_noun_participant'; }
    else if (item.participants.length === 1) { target = item.participants[0].name; reason = 'single_declared_participant'; }
    else fail('migration.ambiguous', `Author primary target before migration: ${example}/${key}`);
  }
  if (target !== null && (typeof target !== 'string' || !item.participants.some(actor => actor.name === target)))
    fail('migration.target', `Target is not a local participant: ${example}/${key}`);
  return { key, target, reason, skip: target === null };
}
function rewrite(source, cases, selections) {
  const lines = source.match(/[^\n]*\n|[^\n]+$/g) || [];
  const remove = new Set(), insert = new Map();
  for (const item of cases) {
    const selection = selections.get(`${item.record}#${item.case}`);
    if (selection.skip) continue;
    if (lines[item.line - 1]?.trim() !== `case ${item.case}:`) fail('migration.stale_lines', `Native case line changed: ${selection.key}`);
    const ending = lines[item.line - 1].endsWith('\r\n') ? '\r\n' : '\n';
    const block = [`    target: ${selection.target}${ending}`];
    const actor = item.participants.find(row => row.name === selection.target);
    const moves = [];
    for (const field of ['becomes', 'preserve', 'causes', 'set']) {
      const first = actor.field_lines[field];
      if (!first) continue;
      const last = field === 'set' ? Math.max(first, ...actor.set.map(row => row.line)) : first;
      for (let line = first; line <= last; ++line) {
        const text = lines[line - 1];
        if (typeof text !== 'string' || (!text.startsWith('        ') && text.trim() && !text.trimStart().startsWith('#')))
          fail('migration.stale_lines', `Unexpected result block: ${selection.key}:${line}`);
        if (remove.has(line)) fail('migration.overlap', `Overlapping source moves: ${selection.key}:${line}`);
        moves.push([line, text.startsWith('    ') ? text.slice(4) : text]); remove.add(line);
      }
    }
    moves.sort((a, b) => a[0] - b[0]);
    block.push(...moves.map(([, text]) => text)); insert.set(item.line, block);
  }
  const result = lines.flatMap((text, index) => [...(remove.has(index + 1) ? [] : [text]), ...(insert.get(index + 1) || [])]).join('');
  if (outsideFence(result) !== outsideFence(source)) fail('migration.prose', 'Migration changed bytes outside rule fence');
  return result;
}
function emit(result) {
  if (options.fields && result.ok) {
    const allowed = new Set(['summary', 'actions']);
    const requested = options.fields.split(',');
    if (requested.some(field => !allowed.has(field))) fail('cli.fields', 'Fields: summary,actions');
    result = { ok: true, ...Object.fromEntries(requested.map(field => [field, result[field]])) };
  }
  process.stdout.write(json ? JSON.stringify(result) + '\n' : `${mode}: ${result.ok ? 'verified' : 'failed'}; ${changed.length} live files changed.\n${JSON.stringify(result, null, 2)}\n`);
}
try {
  for (let i = 2; i < process.argv.length; ++i) {
    const flag = process.argv[i];
    if (flag === '--help' || flag === '-h') { process.stdout.write(help); process.exit(0); }
    if (flag === '--json') { json = true; continue; }
    if (flag === '--apply') { options.apply = true; continue; }
    if (!['--root', '--native', '--attendant', '--targets', '--stage', '--fields'].includes(flag) || !process.argv[i + 1] || process.argv[i + 1].startsWith('--')) fail('cli.arguments', `Unknown/missing option: ${flag}`);
    options[flag.slice(2)] = process.argv[++i];
  }
  if (!options.root || !options.native || !options.attendant || (options.apply && options.stage)) fail('cli.arguments', help.trim());
  for (const field of ['root', 'native', 'attendant']) options[field] = await realpath(resolve(options[field]));
  if (options.fields && options.fields.split(',').some(field => !['summary', 'actions'].includes(field))) fail('cli.fields', 'Fields: summary,actions');
  mode = options.apply ? 'apply' : options.stage ? 'stage' : 'plan';
  const overrides = options.targets ? JSON.parse(await readFile(options.targets, 'utf8')) : {};
  if (!overrides || typeof overrides !== 'object' || Array.isArray(overrides) || Object.keys(overrides).some(name => !examples.includes(name))) fail('migration.targets', 'Targets must be an object keyed by shipped example names');
  const states = [], actions = [];
  for (const example of examples) {
    const project = await realpath(join(options.root, 'skills/game-design-workbench/examples', example));
    const recordCount = await preflight(project), before = native(project, 'rules'), resources = native(project, 'resources');
    const original = await files(project), records = new Map(before.data.records.map(row => [row.record, row]));
    const selections = new Map();
    const explicit = Object.hasOwn(overrides, example) ? overrides[example] : {};
    if (!explicit || typeof explicit !== 'object' || Array.isArray(explicit)) fail('migration.targets', `Invalid target object: ${example}`);
    const keys = new Set(before.data.cases.map(row => `${row.record}#${row.case}`));
    if (Object.keys(explicit).some(key => !keys.has(key))) fail('migration.targets', `Unknown case in targets: ${example}`);
    for (const item of before.data.cases) {
      const selection = choose(example, item, records.get(item.record), explicit);
      selections.set(selection.key, selection); actions.push({ example, ...selection });
    }
    const updates = [];
    for (const record of records.values()) if (record.rule_block === 'parsed') {
      const path = await realpath(record.source_path);
      if (!['interactions', 'mechanics'].some(role => inside(join(project, 'game-design', role), path))) fail('migration.containment', `Rule path escapes source collections: ${path}`);
      const bytes = await readFile(path), source = bytes.toString('utf8');
      if (!bytes.equals(Buffer.from(source, 'utf8'))) fail('migration.encoding', `Source is not lossless UTF-8: ${path}`);
      const next = rewrite(source, before.data.cases.filter(row => row.record === record.record), selections);
      if (source !== next) updates.push({ path, name: relative(project, path), source, next });
    }
    states.push({ example, project, recordCount, before, resources, original, updates });
  }
  if (mode !== 'plan') {
    if (options.stage) {
      stage = await destination(options.stage);
      if (states.some(state => inside(state.project, stage))) fail('migration.containment', 'Stage directory must be outside example source');
      await mkdir(dirname(stage), { recursive: true }); await mkdir(stage); // Existing output is never reused.
    } else {
      const parent = await destination(join(options.root, 'skills/game-design-workbench/native/build'));
      if (states.some(state => inside(state.project, parent))) fail('migration.containment', 'Default stage directory resolves inside example source');
      await mkdir(parent, { recursive: true }); stage = await mkdtemp(join(parent, 'primary-target-stage-'));
    }
    for (const state of states) {
      const staged = join(stage, state.example);
      await cp(state.project, staged, { recursive: true, filter: path => basename(path) !== '.attendant' });
      for (const update of state.updates) await writeFile(join(staged, update.name), update.next);
      await preflight(staged);
      if (!isDeepStrictEqual(comparable(state.before), comparable(native(staged, 'rules'))) ||
          !isDeepStrictEqual(comparable(state.resources), comparable(native(staged, 'resources'))))
        fail('migration.equivalence', `Native rule/Resource facts changed: ${state.example}`);
      const stagedFiles = await files(staged);
      const expected = new Map(state.original);
      for (const update of state.updates) expected.set(update.name, hash(update.next));
      if (!isDeepStrictEqual(stagedFiles, expected)) fail('migration.immutability', `Unexpected staged file changes: ${state.example}`);
    }
    if (mode === 'apply') for (const state of states) {
      if (!isDeepStrictEqual(await files(state.project), state.original)) fail('migration.stale_source', `Source changed during staging: ${state.example}`);
      for (const update of state.updates) {
        if (await readFile(update.path, 'utf8') !== update.source) fail('migration.stale_source', `Source changed before replace: ${update.path}`);
        const temporary = `${update.path}.primary-${process.pid}.tmp`;
        let handle;
        try {
          handle = await open(temporary, 'wx'); temporaryFiles.add(temporary);
          await handle.writeFile(update.next, { encoding: 'utf8' });
          await handle.close(); handle = null;
          if (await readFile(update.path, 'utf8') !== update.source) fail('migration.stale_source', `Source changed before rename: ${update.path}`);
          await rename(temporary, update.path); temporaryFiles.delete(temporary); changed.push(update.path);
        } finally {
          if (handle) await handle.close().catch(() => {});
          if (temporaryFiles.has(temporary)) {
            try { await rm(temporary, { force: true }); temporaryFiles.delete(temporary); }
            catch (error) { fail('migration.cleanup', `Cannot remove owned temporary file ${temporary}: ${error.message}`); }
          }
        }
      }
      await preflight(state.project);
      if (!isDeepStrictEqual(comparable(state.before), comparable(native(state.project, 'rules'))) ||
          !isDeepStrictEqual(comparable(state.resources), comparable(native(state.project, 'resources')))) fail('migration.equivalence', `Post-apply native facts changed: ${state.example}`);
      const expected = new Map(state.original);
      for (const update of state.updates) expected.set(update.name, hash(update.next));
      if (!isDeepStrictEqual(await files(state.project), expected)) fail('migration.immutability', `Unexpected live file changes: ${state.example}`);
    }
  }
  emit({ ok: true, summary: { mode, stage: stage || null, changed: changed.length, proposed_files: states.reduce((sum, state) => sum + state.updates.length, 0), cases: actions.length,
    deferred: actions.filter(row => row.target === null).map(row => `${row.example}/${row.key}`), native_equivalence: mode !== 'plan', outside_fence_bytes: 'unchanged', source_scope: 'two shipped examples only' }, actions });
} catch (error) {
  process.stderr.write(`primary-target migration: ${error.message}\n`);
  emit({ ok: false, mode, stage: stage || null, partial: changed.length > 0 || temporaryFiles.size > 0, changed, temporary_files: [...temporaryFiles], errors: [{ code: error.code || 'migration.failed', message: error.message }] });
  process.exitCode = 2;
}
