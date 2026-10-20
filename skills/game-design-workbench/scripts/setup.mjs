import { readdir, readFile } from 'node:fs/promises';
import { join, relative } from 'node:path';
import { parseDocument, stringify } from '../vendor/attendant/scripts/yaml.mjs';
import { model } from './model.mjs';
import { home, optional, safePath, collections, bundle, hash, slash, fail, attendant, atomicWrite } from './lib.mjs';
const roles = Object.keys(model);
function schema(text) {
  const match = /^---\r?\n([\s\S]*?)^---\r?$/m.exec(text);
  if (!match) fail('SCHEMA_INVALID', 'Schema needs flat YAML frontmatter.');
  const document = parseDocument(match[1], { uniqueKeys: true });
  if (document.errors.length) fail('SCHEMA_INVALID', document.errors[0].message);
  return { fields: document.toJS() ?? {}, body: text.slice(match.index + match[0].length) };
}
async function candidates(root) {
  const found = [];
  async function visit(directory, depth) {
    for (const entry of await readdir(directory, { withFileTypes: true })) {
      if (entry.name.startsWith('.') || ['node_modules', 'tools', 'vendor', 'build'].includes(entry.name)) continue;
      const path = await safePath(root, join(directory, entry.name));
      if (entry.isDirectory() && depth < 2) await visit(path, depth + 1);
      else if (entry.isFile() && entry.name.endsWith('.md') && directory !== root) {
        const bytes = await optional(path);
        if (bytes === null) fail('SOURCE_CHANGED', `Candidate disappeared: ${path}`);
        if (/^---\r?\n/.test(bytes.toString('utf8'))) found.push(slash(relative(root, path)));
      }
    }
  }
  await visit(root, 0);
  return found;
}
export async function planSetup(root, options = {}) {
  const files = await bundle();
  const packageHash = hash(files.map(file => `${file.path}:${hash(file.bytes)}`).join('\n'));
  const target = 'tools/game-design-workbench';
  const manifestPath = `${target}/manifest.json`;
  const priorBytes = await optional(await safePath(root, manifestPath));
  const prior = priorBytes ? JSON.parse(priorBytes) : null;
  const mapping = { ...Object.fromEntries(roles.map(role => [role, role])), ...(prior?.roles ?? {}), ...(options.mapping ?? {}) };
  if (Object.keys(mapping).some(role => !roles.includes(role)) || new Set(Object.values(mapping)).size !== roles.length || Object.values(mapping).some(alias => typeof alias !== 'string' || !/^[A-Za-z0-9][A-Za-z0-9_-]*$/.test(alias))) fail('MAPPING_INVALID', 'Map distinct known roles to safe collection aliases.');
  const configured = await collections(root);
  const configBytes = await optional(await safePath(root, '.pi/attendant.tables'));
  const candidate_sources = configBytes === null ? await candidates(root) : [];
  const changes = [];
  const newCollections = [];
  const reused = [];
  const conflicts = [];
  const expect = {};
  async function propose(path, bytes, allowReplace = false) {
    const current = await optional(await safePath(root, path));
    expect[path] = current ? hash(current) : null;
    if (current?.equals(bytes)) return;
    if (current && !allowReplace) { conflicts.push(path); return; }
    changes.push({ path, bytes, before: current ? hash(current) : null, sha256: hash(bytes) });
  }
  const contextPaths = ['.pi/attendant.tables', 'AGENTS.md', '.gitignore', manifestPath];
  for (const path of contextPaths) { const bytes = await optional(await safePath(root, path)); expect[path] = bytes ? hash(bytes) : null; }
  for (const role of roles) {
    const alias = mapping[role];
    const existing = configured.find(c => c.alias === alias);
    const templateBytes = await readFile(join(home, 'templates', role, '.schema.md'));
    const parsed = schema(templateBytes.toString('utf8'));
    const adapted = Object.fromEntries(Object.entries(parsed.fields).map(([key, value]) => [key, typeof value === 'string' ? value.replace(/^(\??ref(?:\[\])?:)(.+)$/, (_, prefix, roleName) => `${prefix}${mapping[roleName] ?? roleName}`) : value]));
    if (existing) {
      const path = `${existing.directory}/.schema.md`;
      const bytes = await readFile(await safePath(root, path)); expect[path] = hash(bytes);
      const actual = schema(bytes.toString('utf8')).fields;
      const usagePath = `${existing.directory}/.usage.md`;
      const usage = await optional(await safePath(root, usagePath)); expect[usagePath] = usage ? hash(usage) : null;
      if (Object.entries(adapted).some(([field, declaration]) => JSON.stringify(actual[field]) !== JSON.stringify(declaration)) || !usage?.toString('utf8').trim()) conflicts.push(`Incompatible schema or missing usage: ${existing.directory}`);
      reused.push({ role, alias, directory: existing.directory, usage: usagePath });
      continue;
    }
    const directory = `game-design/${alias}`;
    let entries = [];
    try { entries = await readdir(await safePath(root, directory)); } catch (error) { if (error.code !== 'ENOENT') throw error; }
    if (entries.length) { conflicts.push(`Nonempty unconfigured target requires migration: ${directory}`); continue; }
    newCollections.push({ role, alias, directory });
    for (const name of ['.schema.md', '.usage.md', '.template.md']) {
      const bytes = name === '.schema.md' ? Buffer.from(`---\n${stringify(adapted, { lineWidth: 0 }).trimEnd()}\n---${parsed.body}`) : await readFile(join(home, 'templates', role, name));
      await propose(`${directory}/${name}`, bytes);
    }
  }
  for (const file of files) {
    const path = `${target}/${file.path}`;
    const current = await optional(await safePath(root, path));
    const base = prior?.files?.[file.path];
    await propose(path, file.bytes, Boolean(current && base && hash(current) === base));
  }
  let ignore = (await optional(await safePath(root, '.gitignore')))?.toString('utf8') ?? '';
  const lines = ignore.split(/\r?\n/).map(line => line.trim());
  for (const pattern of ['.attendant/', 'game-design-results/*/artifacts/']) if (!lines.some(line => line === pattern || line === `/${pattern}` || (pattern === '.attendant/' && line === '**/.attendant/'))) ignore += `${ignore && !ignore.endsWith('\n') ? '\n' : ''}${pattern}\n`;
  await propose('.gitignore', Buffer.from(ignore), true);
  const all = roles.map(role => [...reused, ...newCollections].find(collection => collection.role === role)).filter(Boolean);
  const block = `<!-- game-design-workbench:start -->\n### Game design workbench\n\nMarkdown is source of truth; .attendant/ is disposable. Configuration: .pi/attendant.tables. Read schema and usage before record operations. Local tools/references: tools/game-design-workbench/.\n\n${all.map(c => `- ${c.alias}: ${c.directory}/.schema.md; ${c.directory}/.usage.md`).join('\n')}\n<!-- game-design-workbench:end -->`;
  const agents = (await optional(await safePath(root, 'AGENTS.md')))?.toString('utf8') ?? '';
  const managed = /<!-- game-design-workbench:start -->[\s\S]*?<!-- game-design-workbench:end -->/.exec(agents);
  if (managed && managed[0] !== block && prior?.guidance_hash !== hash(managed[0])) conflicts.push('Locally edited AGENTS.md workbench block');
  else {
    let next;
    if (managed) next = agents.replace(managed[0], () => block);
    else {
      const section = /^## Attendant\s*\r?$/m.exec(agents);
      if (section) {
        const rest = agents.slice(section.index + section[0].length);
        const boundary = /^## /m.exec(rest);
        const end = boundary ? section.index + section[0].length + boundary.index : agents.length;
        next = `${agents.slice(0, end).trimEnd()}\n\n${block}\n\n${agents.slice(end)}`;
      } else next = `${agents.trimEnd()}${agents.trim() ? '\n\n' : ''}## Attendant\n\n${block}\n`;
    }
    await propose('AGENTS.md', Buffer.from(next), true);
  }
  const manifest = { schema_version: 1, release: '0.1.0', distribution_revision: packageHash, model_version: 1, report_version: 1, roles: mapping, files: Object.fromEntries(files.map(file => [file.path, hash(file.bytes)])), guidance_hash: hash(block), capabilities: ['guided-framework-review', 'attendant-sql-queries', 'term-resolution', 'dated-review-storage'], unavailable: ['native-c99-analysis', 'native-viewer', 'control-evaluation-export', 'simulation-runtime'], dependencies: { node: '>=24.10.0', attendant: 'vendored snapshot', yaml: '2.9.0 MIT' } };
  await propose(manifestPath, Buffer.from(`${JSON.stringify(manifest, null, 2)}\n`), true);
  const publicPlan = { project: root, distribution_revision: packageHash, roles: mapping, new_collections: newCollections, reused_collections: reused, candidate_sources, fresh_acknowledged: Boolean(options.fresh), conflicts, files: changes.map(({ bytes, ...change }) => change), expected: expect, dependencies: manifest.dependencies };
  return { ...publicPlan, plan_id: hash(JSON.stringify(publicPlan)), changes };
}
export function displayPlan(plan) { const { changes, ...publicPlan } = plan; return publicPlan; }
export async function applySetup(root, approved, options) {
  const plan = await planSetup(root, options);
  if (approved !== plan.plan_id) fail('APPROVAL_REQUIRED', 'Review current setup plan; pass its exact --approve plan_id. No files changed.', displayPlan(plan));
  if (plan.conflicts.length) fail('SETUP_CONFLICT', 'Resolve listed conflicts without overwriting local data.', plan.conflicts);
  if (plan.candidate_sources.length && !options.fresh) fail('MIGRATION_DECISION_REQUIRED', 'Inspect candidate records; use Attendant migration or explicitly approve a --fresh plan.', plan.candidate_sources);
  const applied = [];
  try {
    if (!await optional(await safePath(root, '.pi/attendant.tables'))) await atomicWrite(root, '.pi/attendant.tables', Buffer.from(''));
    for (const collection of plan.new_collections) {
      attendant(root, ['add-table', '--directory', collection.directory, '--alias', collection.alias]);
      applied.push(`${collection.directory}/ (empty Attendant collection)`);
    }
    for (const change of plan.changes) {
      let expected = change.before;
      const created = plan.new_collections.some(c => change.path.startsWith(`${c.directory}/`));
      if (created) {
        const empty = change.path.endsWith('/.schema.md') ? '---\n---\n' : '';
        expected = hash(Buffer.from(empty));
      }
      await atomicWrite(root, change.path, change.bytes, expected);
      applied.push(change.path);
    }
    return { plan_id: approved, changed: applied, collections: plan.roles };
  } catch (error) { error.data = { applied, cause: error.data ?? null, recovery: 'Inspect listed paths. No automatic reset/rollback was attempted.' }; throw error; }
}
