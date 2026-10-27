#!/usr/bin/env node
/** Review01 source migration. JSON input is a temporary proposal, not an authoring format.
 * Public Attendant owns metadata/IDs. This tool replaces/adds only readable gd-rule fences.
 * Default: plan only. --stage DIR copies source to a new directory and verifies it first.
 * --apply modifies examples. Batches are not a cross-file transaction; failures retain partial
 * source changes for inspection. Safe re-run skips unchanged fields/bodies. No source deletion,
 * experiment execution, frozen evidence rewrite, publication, implicit downloads or JSON evaluator.
 */
import { readFile, writeFile, cp, mkdir, access, rename, realpath } from 'node:fs/promises';
import { resolve, join, relative, isAbsolute } from 'node:path';
import { spawnSync } from 'node:child_process';

const help = `Usage: node scripts/migrate-workbench-actor-rules.mjs --root REPO --proposal FILE
  --attendant RUNNER --native GD_EXE [--stage NEW_DIR | --apply] [--json]
Plan only by default. --stage verifies copied source; --apply adopts authorized proposal.
All example interaction/mechanic records must be covered exactly once. Preserve original
prose, IDs, statuses and flat links. stdout: result data; failures exit 2 with partial flag.
No experiments run. No source writes outside selected example collections (or stage).
`;
const options = {};
let changed = 0, mutationStarted = false;
const json = process.argv.includes('--json') || !process.stdout.isTTY || process.env.CI || process.env.GD_AGENT === '1';
function run(executable, args, input) {
  const result = spawnSync(executable, args, { encoding: 'utf8', input, timeout: 120000, maxBuffer: 32 * 1024 * 1024 });
  if (result.status !== 0) throw new Error(`${args[0]} failed: ${result.stderr?.trim() || result.error?.message || result.stdout.slice(0, 500)}`);
  return JSON.parse(result.stdout);
}
const attendant = (project, command, ...args) => run(process.execPath, [options.attendant, command, '-p', project, ...args]);
const items = (project, command, collection, values) => {
  mutationStarted = true;
  return run(process.execPath, [options.attendant, command, '-p', project, '-c', collection, '-i', '-'], JSON.stringify(values));
};
const fieldsOf = row => Object.fromEntries(Object.entries(row).filter(([key]) => !['updated_at', 'source_path', 'source_hash'].includes(key)));
function normalize(value, array = false) { return array && typeof value === 'string' ? JSON.parse(value) : value; }
async function atomic(path, content) {
  if (await readFile(path, 'utf8') === content) return;
  const temporary = `${path}.review01-${process.pid}.tmp`;
  mutationStarted = true;
  await writeFile(temporary, content, { flag: 'wx' });
  await rename(temporary, path); ++changed;
}
function fence(body, block) {
  const matches = [...body.matchAll(/^(`{3,}|~{3,})gd-rule[ \t]*\r?\n[\s\S]*?^\1[ \t]*$/gm)];
  if (matches.length > 1) throw new Error('Multiple existing gd-rule fences; no automatic repair');
  const replacement = `\`\`\`gd-rule\n${block.trimEnd()}\n\`\`\``;
  return matches.length ? body.slice(0, matches[0].index) + replacement + body.slice(matches[0].index + matches[0][0].length)
    : `${body}${body.endsWith('\n\n') ? '' : body.endsWith('\n') ? '\n' : '\n\n'}${replacement}\n`;
}
async function appendBody(path, body) {
  const source = await readFile(path, 'utf8');
  const header = source.match(/^---\r?\n[\s\S]*?\r?\n---(?:\r?\n|$)/)?.[0];
  if (!header) throw new Error(`Expected public Attendant front matter: ${path}`);
  await atomic(path, header + body(source.slice(header.length)));
}
async function inspect(project, example) {
  attendant(project, 'validate', '--strict');
  const schema = attendant(project, 'schema');
  const collections = new Map(schema.collections.map(item => [item.name, item]));
  const rows = new Map();
  for (const name of ['axes', 'nouns', 'interactions', 'mechanics']) {
    const collection = collections.get(name);
    if (!collection?.usage || !collection.fields) throw new Error(`Missing schema/usage: ${name}`);
    const within = relative(await realpath(project), await realpath(collection.directory));
    if (within === '..' || within.startsWith(`..${process.platform === 'win32' ? '\\' : '/'}`) || isAbsolute(within))
      throw new Error(`Mutation collection escapes selected project: ${name}`);
    const result = attendant(project, 'query', '-s', `SELECT * FROM "${name}"`, '-l', '4096');
    if (result.truncated) throw new Error(`Truncated collection: ${name}`);
    rows.set(name, new Map(result.rows.map(row => [row.name, row])));
  }
  const existing = ['interactions', 'mechanics'].flatMap(name => [...rows.get(name).keys()].map(key => `${name}/${key}`));
  const proposed = example.rules.map(item => `${item.collection}/${item.name}`);
  if (new Set(proposed).size !== proposed.length || existing.length !== proposed.length || existing.some(name => !proposed.includes(name)))
    throw new Error(`Proposal must cover every existing live rule exactly once: ${example.name}`);
  for (const group of [example.axes, example.nouns, example.rules]) for (const item of group) {
    if (!/^[A-Za-z0-9_-]+$/.test(item.name)) throw new Error(`Unsafe record name: ${item.name}`);
  }
  for (const item of example.rules) {
    if (typeof item.block !== 'string' || !item.block.startsWith('case ') || /\$\{|```|~~~/u.test(item.block)) throw new Error(`Invalid fence/reference text: ${item.name}`);
    if (!Array.isArray(item.context_axes)) throw new Error(`Missing explicit context inventory: ${item.name}`);
  }
  for (const item of example.nouns) if (!rows.get('nouns').has(item.name)) throw new Error(`Missing original noun: ${item.name}`);
  return { collections, rows };
}
async function apply(project, example, state) {
  const directory = name => state.collections.get(name).directory;
  const update = (collection, values) => { if (values.length) { items(project, 'update', collection, values); changed += values.length; } };
  const creates = [], updates = [];
  for (const item of example.axes) {
    const fields = Object.fromEntries(['value_type', 'allowed_values', 'unit', 'minimum', 'maximum', 'closed'].map(key => [key, item[key]]));
    const old = state.rows.get('axes').get(item.name);
    if (!old) creates.push({ name: item.name, fields });
    else {
      const differences = Object.fromEntries(Object.entries(fields).filter(([key, value]) => JSON.stringify(key === 'closed' && typeof old[key] === 'number' ? old[key] !== 0 : normalize(old[key], key === 'allowed_values')) !== JSON.stringify(value) && !(key === 'unit' && !old[key] && !value)));
      if (Object.keys(differences).length) updates.push({ name: item.name, fields: differences });
    }
  }
  if (creates.length) { items(project, 'create', 'axes', creates); changed += creates.length; }
  update('axes', updates);
  for (const item of creates) {
    const definition = example.axes.find(axis => axis.name === item.name);
    await appendBody(join(directory('axes'), `${item.name}.md`), () => `${definition.body}\n`);
  }
  update('nouns', example.nouns.filter(item => JSON.stringify(normalize(state.rows.get('nouns').get(item.name).axes, true)) !== JSON.stringify(item.axes))
    .map(item => ({ name: item.name, fields: { axes: item.axes } })));
  for (const name of ['interactions', 'mechanics']) update(name, example.rules.filter(item => item.collection === name && item.context_axes.length > 0 &&
    JSON.stringify(normalize(state.rows.get(name).get(item.name).context_axes, true)) !== JSON.stringify(item.context_axes))
    .map(item => ({ name: item.name, fields: { context_axes: item.context_axes } })));
  for (const item of example.rules) await appendBody(join(directory(item.collection), `${item.name}.md`), body => fence(body, item.block));
  const validation = attendant(project, 'validate', '--strict');
  const after = await inspect(project, example);
  for (const [collection, originals] of state.rows) for (const [name, original] of originals) {
    const next = fieldsOf(after.rows.get(collection).get(name));
    const before = fieldsOf(original);
    const permitted = collection === 'axes' ? ['allowed_values', 'value_type', 'unit', 'minimum', 'maximum', 'closed'] : collection === 'nouns' ? ['axes'] : ['context_axes'];
    for (const key of permitted) { delete before[key]; delete next[key]; }
    if (JSON.stringify(before) !== JSON.stringify(next)) throw new Error(`Unrelated metadata/identity changed: ${collection}/${name}`);
  }
  const nativeArgs = ['rules', '--analyze', '--json'];
  for (const collection of after.collections.values()) {
    const option = { nouns: '--nouns', verbs: '--verbs', interactions: '--interactions', mechanics: '--mechanics', axes: '--axes', 'control-vars': '--control-vars' }[collection.name] || '--related';
    nativeArgs.push(option, `${collection.name}=${collection.directory}`);
  }
  const native = run(options.native, nativeArgs);
  if (native.coverage.records_with_cases !== example.rules.length) throw new Error(`Incomplete native migration: ${example.name}`);
  return { example: example.name, rules: example.rules.length, axes: native.data.axes.length,
    cases: native.coverage.case_count, validation: { records: validation.records, diagnostics: validation.diagnostics, corrections: validation.corrections, strict: validation.strict }, native_complete: native.coverage.complete,
    invocation_count: native.data.analysis.calls.length, comparison_limited: native.data.analysis.comparison_limited };
}
try {
  for (let i = 2; i < process.argv.length; ++i) {
    const argument = process.argv[i];
    if (argument === '--help') { process.stdout.write(help); process.exit(0); }
    if (argument === '--json') continue;
    if (argument === '--apply') { options.apply = true; continue; }
    if (!['--root', '--proposal', '--attendant', '--native', '--stage'].includes(argument) || !process.argv[i + 1] || process.argv[i + 1].startsWith('--') || options[argument.slice(2)]) throw new Error(`Unknown/repeated option: ${argument}`);
    options[argument.slice(2)] = resolve(process.argv[++i]);
  }
  for (const name of ['root', 'proposal', 'attendant', 'native']) if (!options[name]) throw new Error(`Missing --${name}; see --help`);
  if (options.stage && options.apply) throw new Error('--stage and --apply are mutually exclusive');
  const proposal = JSON.parse(await readFile(options.proposal, 'utf8'));
  if (!Array.isArray(proposal.examples) || proposal.examples.length !== 2 || new Set(proposal.examples.map(item => item.name)).size !== 2 ||
    proposal.examples.some(item => !['crate-puzzle', 'little-plumber-guy-nes'].includes(item.name))) throw new Error('Expected both Review01 example proposals exactly once');
  const plans = [];
  for (const example of proposal.examples) {
    const project = join(options.root, 'skills/game-design-workbench/examples', example.name);
    plans.push({ project, example, state: await inspect(project, example) });
  }
  if (options.stage) {
    try { await access(options.stage); throw new Error('Stage must be a new directory'); } catch (error) { if (error.code !== 'ENOENT') throw error; }
    await mkdir(options.stage, { recursive: true });
    for (const plan of plans) {
      const stage = join(options.stage, plan.example.name);
      await mkdir(join(stage, '.pi'), { recursive: true });
      await cp(join(plan.project, '.pi/attendant.tables'), join(stage, '.pi/attendant.tables'));
      await cp(join(plan.project, 'game-design'), join(stage, 'game-design'), { recursive: true });
      plan.project = stage; plan.state = await inspect(stage, plan.example);
    }
  }
  const data = [];
  for (const plan of plans) data.push(options.apply || options.stage ? await apply(plan.project, plan.example, plan.state)
    : { example: plan.example.name, rules: plan.example.rules.length, missing_axes: plan.example.axes.filter(item => !plan.state.rows.get('axes').has(item.name)).map(item => item.name) });
  const result = { ok: true, mode: options.apply ? 'applied' : options.stage ? 'staged' : 'plan_only', changed, data };
  process.stdout.write(json ? `${JSON.stringify(result)}\n` : `${result.mode}: ${data.map(item => `${item.example} ${item.rules} rules`).join('; ')}; ${changed} changes\n`);
} catch (error) {
  const result = { ok: false, error: { code: 'migration.failed', message: error.message }, partial_changes_possible: mutationStarted, changed };
  process.stdout.write(`${JSON.stringify(result)}\n`); process.stderr.write(`Migration failed: ${error.message}\n`); process.exitCode = 2;
}
