#!/usr/bin/env node
// Bootstrap CLI: reuses Attendant, rather than implementing the later C99 core.
import { writeFile, mkdir } from 'node:fs/promises';
import { resolve, dirname, relative, isAbsolute } from 'node:path';
import { inspect } from 'node:util';
import { collections, snapshot, safePath, projectRoot, optional, fail } from './lib.mjs';
import { planSetup, displayPlan, applySetup } from './setup.mjs';
import { projected, runQuery, resolveTerm } from './queries.mjs';
import { catalog, saveReport, showReport } from './reports.mjs';
const help = {
  release: '0.1.0 guided workbench; Node.js >=24.10.0',
  commands: ['setup plan [--mapping map.json] [--fresh]', 'setup apply --approve <plan-id> [same mapping/fresh options]', 'capabilities', 'lenses', 'query <matrix|skills|routes|mda|decisions|glossary|controls|evidence>', 'query records --collection <alias>', 'resolve --term <text>', 'validate', 'snapshot [--include <comma-separated-extra-source-paths>]', 'report save --lens <id> --input <findings.json> --snapshot <snapshot.json>', 'report show <run-id>'],
  common: '--project <root> --json --format json|text --fields <comma-separated-row-fields> --output <new-file> --limit <1..10000>',
  contract: 'stdout=data; stderr=diagnostics; exit 0 completed, 1 validation findings, 2 invalid input/runtime/approval. GD_AGENT=1 selects noninteractive JSON. No prompts or implicit consent.',
  unavailable: ['native C99 checks/views', 'formula evaluation/control exports', 'simulation/sweep/replay execution'],
  query_safety: 'Queries run Attendant against temporary source copy; live Markdown is not mechanically corrected. Source fingerprints detect intervening edits.'
};
async function readInput(path) {
  const bytes = await optional(resolve(path));
  if (bytes === null) fail('INPUT_MISSING', `Missing JSON input: ${path}`);
  try { return JSON.parse(bytes.toString('utf8')); }
  catch { fail('JSON_INVALID', `Invalid JSON input: ${path}`); }
}
let options = {};
let exit = 0;
try {
  const major = process.versions.node.split('.').map(Number);
  if (major[0] < 24 || (major[0] === 24 && major[1] < 10)) fail('NODE_VERSION', 'Node.js >=24.10.0 required by vendored Attendant.');
  const tokens = [];
  const valued = new Set(['project', 'mapping', 'approve', 'input', 'snapshot', 'lens', 'term', 'limit', 'include', 'fields', 'output', 'format', 'collection']);
  const booleans = new Set(['json', 'help', 'fresh']);
  for (let index = 2; index < process.argv.length; index++) {
    const arg = process.argv[index];
    if (!arg.startsWith('--')) { tokens.push(arg); continue; }
    const key = arg.slice(2);
    if (key in options) fail('ARGUMENT_INVALID', `Duplicate option --${key}`);
    if (booleans.has(key)) options[key] = true;
    else if (valued.has(key) && process.argv[index + 1] && !process.argv[index + 1].startsWith('--')) options[key] = process.argv[++index];
    else fail('ARGUMENT_INVALID', `Unknown option or missing value: ${arg}`);
  }
  if (options.json && options.format && options.format !== 'json') fail('FORMAT_CONFLICT', '--json conflicts with non-JSON --format.');
  if (options.format && !['json', 'text'].includes(options.format)) fail('FORMAT_INVALID', 'Choose --format json or text.');
  if (options.fields && !options.fields.split(',').every(field => /^[A-Za-z_][A-Za-z0-9_]*$/.test(field))) fail('FIELDS_INVALID', 'Use comma-separated row field names.');
  if (options.limit && (!/^\d+$/.test(options.limit) || Number(options.limit) < 1 || Number(options.limit) > 10000)) fail('LIMIT_INVALID', 'Limit must be integer from 1 through 10000.');
  const [command = 'help', action, extra] = tokens;
  if (tokens.length > 3) fail('ARGUMENT_INVALID', 'Unexpected positional arguments.');
  const allowed = {
    help: [], capabilities: [], lenses: [], setup: ['mapping', 'approve', 'fresh'], query: ['collection', 'limit', 'fields'], resolve: ['term'], validate: [], snapshot: ['include'], report: ['lens', 'input', 'snapshot']
  };
  if (!Object.hasOwn(allowed, command) && !options.help) fail('CAPABILITY_UNAVAILABLE', 'Command is not installed in first release. Use --help for available queries/guidance; native/simulation tools are later slices.');
  for (const key of Object.keys(options)) if (!['project', 'json', 'format', 'output', 'help'].includes(key) && !(allowed[command] ?? []).includes(key)) fail('ARGUMENT_INVALID', `--${key} does not apply to ${command}.`);
  if (['help', 'capabilities', 'lenses', 'resolve', 'validate', 'snapshot'].includes(command) && action) fail('ARGUMENT_INVALID', 'Unexpected positional argument.');
  if (['setup', 'query'].includes(command) && extra) fail('ARGUMENT_INVALID', 'Unexpected positional argument.');
  const root = await projectRoot(options.project ?? process.cwd());
  let data;
  let provenance;
  let coverage = { status: 'complete', operation: command };
  if (options.help || command === 'help') data = help;
  else if (command === 'capabilities') data = { available: ['setup/update plans', 'read-only Attendant query recipes', 'declared term resolution', 'source snapshots', 'dated submitted reviews', '18 guided framework packs'], unavailable: help.unavailable, dependencies: help.release };
  else if (command === 'lenses') data = await catalog();
  else if (command === 'setup') {
    const setupOptions = { fresh: Boolean(options.fresh), mapping: options.mapping ? await readInput(options.mapping) : undefined };
    if (action === 'plan') data = displayPlan(await planSetup(root, setupOptions));
    else if (action === 'apply') data = await applySetup(root, options.approve, setupOptions);
    else fail('ARGUMENT_INVALID', 'Use setup plan or setup apply --approve <plan-id>.');
  } else if (command === 'snapshot') data = (await snapshot(root, options.include ? options.include.split(',') : [])).metadata;
  else if (command === 'query' || command === 'resolve') {
    if (command === 'resolve' && typeof options.term !== 'string') fail('ARGUMENT_INVALID', 'resolve requires --term.');
    const result = command === 'resolve' ? await resolveTerm(root, options.term) : await runQuery(root, action, { collection: options.collection, limit: Number(options.limit ?? 1000) });
    data = result.data; provenance = result.snapshot;
    coverage = { status: data.truncated ? 'partial' : 'complete', operation: command === 'query' ? `query.${action}` : 'resolve', limitation: 'Declared data only; no native rank/scaffolding/formula interpretation.', projection_corrections: result.projection_corrections };
    if (options.fields) data = { ...data, rows: data.rows.map(row => Object.fromEntries(options.fields.split(',').map(field => { if (!Object.hasOwn(row, field)) fail('FIELD_UNKNOWN', `Unknown row field: ${field}`); return [field, row[field]]; }))) };
  } else if (command === 'validate') {
    const result = await projected(root, async context => context.validation);
    data = result.data; provenance = result.snapshot;
    coverage = { status: 'partial', operation: 'validate', performed: ['Attendant flat schema, supplied reference/type checks'], not_performed: ['workbench semantic/alias/unit/prerequisite checks; later C99 core'] };
    if (data.diagnostics.length) exit = 1;
  } else if (command === 'report') {
    if (action === 'save') {
      if (!options.input || !options.snapshot || !options.lens || extra) fail('ARGUMENT_INVALID', 'report save requires --lens, --input, and --snapshot.');
      const input = await readInput(options.input);
      const raw = await readInput(options.snapshot);
      data = await saveReport(root, options.lens, input, raw.data ?? raw);
    } else if (action === 'show' && extra) data = await showReport(root, extra);
    else fail('ARGUMENT_INVALID', 'Use report save or report show <run-id>.');
  }
  const result = { schema_version: 1, ok: exit === 0, data, coverage, ...(provenance ? { provenance: { schema_version: provenance.schema_version, fingerprint: provenance.fingerprint, tool_hash: provenance.tool_hash, source_count: provenance.sources.length, includes: provenance.includes } } : {}), errors: exit ? [{ code: 'VALIDATION_FINDINGS', message: 'Source diagnostics found; no corrections applied.' }] : [] };
  const json = options.json || options.format === 'json' || (options.format !== 'text' && (process.env.GD_AGENT === '1' || !process.stdout.isTTY));
  const output = `${json ? JSON.stringify(result) : inspect(result, { depth: null, colors: false, maxArrayLength: null })}\n`;
  if (options.output) {
    const target = resolve(options.output);
    const rel = relative(root, target).replaceAll('\\', '/');
    const dirs = await collections(root);
    if (['AGENTS.md', '.gitignore'].includes(rel) || rel.startsWith('.pi/') || rel.startsWith('tools/game-design-workbench/') || dirs.some(c => rel === c.directory || rel.startsWith(`${c.directory}/`))) fail('OUTPUT_SOURCE_CONFLICT', 'Output cannot replace or create canonical source/config/tool files.');
    if (!isAbsolute(rel) && !rel.startsWith('../') && rel !== '..') await safePath(root, target);
    await mkdir(dirname(target), { recursive: true });
    await writeFile(target, output, { flag: 'wx' });
  } else process.stdout.write(output);
} catch (error) {
  process.stderr.write(`${error.code ?? 'RUNTIME_ERROR'}: ${error.message}\n`);
  process.stdout.write(`${JSON.stringify({ schema_version: 1, ok: false, data: error.data ?? null, coverage: { status: 'incomplete' }, errors: [{ code: error.code ?? 'RUNTIME_ERROR', message: error.message }] })}\n`);
  exit = 2;
}
process.exitCode = exit;
