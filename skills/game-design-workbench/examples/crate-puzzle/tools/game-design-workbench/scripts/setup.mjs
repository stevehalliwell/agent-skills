import { readdir, readFile, mkdtemp, mkdir, writeFile, rm } from 'node:fs/promises';
import { join, relative, resolve, isAbsolute } from 'node:path';
import { tmpdir } from 'node:os';
import { projected } from './queries.mjs';
import { model } from './model.mjs';
import { conceptPath, describeConcept, withMode } from './concept.mjs';
import { home, optional, safePath, collections, bundle, hash, slash, fail, attendant, attendantFingerprint, atomicWrite, removeManaged } from './lib.mjs';
const roles = Object.keys(model);
async function templateContracts(mapping) {
  const temporary = await mkdtemp(join(tmpdir(), 'gd-contracts-'));
  const files = new Map();
  try {
    await mkdir(join(temporary, '.pi'));
    await writeFile(join(temporary, '.pi/attendant.tables'), roles.map(role => `${role} as ${mapping[role]}\n`).join(''));
    for (const role of roles) {
      await mkdir(join(temporary, role));
      for (const name of ['.schema.md', '.usage.md', '.template.md']) {
        let bytes = await readFile(join(home, 'templates', role, name));
        // Substitute known reference tokens in authored distribution templates only.
        if (name === '.schema.md') bytes = Buffer.from(bytes.toString('utf8').replace(/(\??ref(?:\[\])?:)([A-Za-z0-9_-]+)/g, (_, prefix, target) => `${prefix}${mapping[target] ?? target}`));
        files.set(`${role}/${name}`, bytes);
        await writeFile(join(temporary, role, name), bytes);
      }
    }
    return { files, contracts: attendant(temporary, ['schema']).collections };
  } finally { await rm(temporary, { recursive: true, force: true }); }
}
async function candidates(root, configured, excluded) {
  const found = [];
  async function visit(directory, depth) {
    for (const entry of await readdir(directory, { withFileTypes: true })) {
      if (entry.name.startsWith('.') || ['node_modules', 'tools', 'vendor', 'build'].includes(entry.name)) continue;
      const path = await safePath(root, join(directory, entry.name));
      if (excluded.includes(slash(relative(root, path)))) continue;
      if (configured.some(collection => process.platform === 'win32' ? collection.absolute.toLowerCase() === path.toLowerCase() : collection.absolute === path)) continue;
      if (entry.isDirectory() && depth < 2) await visit(path, depth + 1);
      else if (entry.isFile() && entry.name.endsWith('.md')) {
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
  const dependency_hash = await attendantFingerprint();
  const files = await bundle();
  const packageHash = hash(files.map(file => `${file.path}:${hash(file.bytes)}`).join('\n'));
  const target = 'tools/game-design-workbench';
  const manifestPath = `${target}/manifest.json`;
  const priorBytes = await optional(await safePath(root, manifestPath));
  const prior = priorBytes ? JSON.parse(priorBytes) : null;
  if (Object.hasOwn(prior?.roles ?? {}, 'skills')) fail('COLLECTION_RENAME_REQUIRED', 'Legacy skills collection role needs explicit migration to skill-atoms before setup/update. Preserve record IDs and rename references; do not create a second empty collection. Read references/setup.md.');
  if (['hypotheses', 'expectations', 'observations'].some(role => Object.hasOwn(prior?.roles ?? {}, role))) fail('EXPERIMENT_MIGRATION_REQUIRED', 'Consolidate legacy investigation collections into experiments before setup/update. Preserve claims, criteria, observations, provenance, and record identities; read references/setup.md#experiment-consolidation.');
  const mapping = { ...Object.fromEntries(roles.map(role => [role, role])), ...(prior?.roles ?? {}), ...(options.mapping ?? {}) };
  if (Object.keys(mapping).some(role => !roles.includes(role)) || new Set(Object.values(mapping)).size !== roles.length || Object.values(mapping).some(alias => typeof alias !== 'string' || !/^[A-Za-z0-9][A-Za-z0-9_-]*$/.test(alias))) fail('MAPPING_INVALID', 'Map distinct known roles to safe collection aliases.');
  const configured = await collections(root);
  const conceptName = conceptPath(options.concept ?? prior?.concept_path);
  const comparable = value => process.platform === 'win32' ? value.toLowerCase() : value;
  const isReadme = comparable(conceptName) === comparable('README.md');
  if ([...configured.map(collection => collection.directory), ...roles.map(role => `game-design/${mapping[role]}`)].some(directory => comparable(conceptName) === comparable(directory) || comparable(conceptName).startsWith(`${comparable(directory)}/`))) fail('CONCEPT_PATH_INVALID', 'Concept must be outside record collections; keep one project-level document.');
  const currentConcept = await optional(await safePath(root, conceptName));
  let conceptBytes = currentConcept;
  if (options.mode !== undefined) {
    conceptBytes = currentConcept ? withMode(currentConcept, options.mode) : withMode(Buffer.from((await readFile(join(home, 'templates/concept.md'), 'utf8')).replace('{{mode}}', options.mode)), options.mode);
  }
  const context = describeConcept(root, conceptName, conceptBytes);
  if (context.status !== 'ready') fail(context.issue?.code ?? 'CONCEPT_MODE_REQUIRED', context.issue?.message ?? 'Request existing concept material or offer template; choose --mode game-design or game-explorer. No mode is inferred from document presence.');
  const candidate_sources = await candidates(root, configured, [conceptName, prior?.concept_path].filter(Boolean));
  const templates = await templateContracts(mapping);
  const actualContracts = configured.length ? (await projected(root, context => context.schema)).data.collections : [];
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
  const readmePath = await safePath(root, 'README.md');
  const readmeBytes = await optional(readmePath);
  const readme = readmeBytes ? new TextDecoder('utf-8', { fatal: true, ignoreBOM: true }).decode(readmeBytes) : '';
  expect['README.md'] = readmeBytes ? hash(readmeBytes) : null;
  const linkMarker = /<!-- game-design-workbench:concept:start -->[\s\S]*?<!-- game-design-workbench:concept:end -->/;
  const existingLink = linkMarker.exec(readme)?.[0];
  const desiredLink = isReadme ? '' : `<!-- game-design-workbench:concept:start -->\n## Game concept\n\n[Concept and working mode](${conceptName.split('/').map(encodeURIComponent).join('/')})\n<!-- game-design-workbench:concept:end -->`;
  const linkOwned = existingLink && prior?.concept_link_hash === hash(existingLink.replaceAll('\r\n', '\n'));
  const linkUnchanged = existingLink?.replaceAll('\r\n', '\n') === desiredLink;
  if (existingLink && !linkOwned && !linkUnchanged) conflicts.push('Locally edited README.md concept link');
  let conceptLinkHash = null;
  if (isReadme) {
    if (existingLink && linkOwned) conceptBytes = Buffer.from(conceptBytes.toString('utf8').replace(existingLink, ''));
  } else if (existingLink || !readme.includes(`](${conceptName.split('/').map(encodeURIComponent).join('/')})`)) {
    conceptLinkHash = hash(desiredLink);
    if (!existingLink || linkOwned || linkUnchanged) {
      const newline = readme.includes('\r\n') ? '\r\n' : '\n';
      const rendered = desiredLink.replaceAll('\n', newline);
      const opening = /^(?:\uFEFF)?(?:---\r?\n[\s\S]*?\r?\n---(?:\r?\n|$))?(?:\r?\n)*(?:# [^\r\n]*(?:\r?\n|$))?/.exec(readme)[0].length;
      const head = readme.slice(0, opening);
      const next = existingLink ? readme.replace(existingLink, () => rendered) : `${head}${head && !head.endsWith('\n') ? newline : ''}${newline}${rendered}${newline}${newline}${readme.slice(opening)}`;
      await propose('README.md', Buffer.from(next), true);
    }
  }
  await propose(conceptName, conceptBytes, Boolean(options.mode || isReadme && linkOwned));
  for (const role of roles) {
    const alias = mapping[role];
    const existing = configured.find(c => c.alias === alias);
    const expectedFields = templates.contracts.find(collection => collection.name === alias).fields;
    if (existing) {
      const path = `${existing.directory}/.schema.md`;
      const bytes = await readFile(await safePath(root, path)); expect[path] = hash(bytes);
      const actual = actualContracts.find(collection => collection.name === alias).fields;
      const usagePath = `${existing.directory}/.usage.md`;
      const usage = await optional(await safePath(root, usagePath)); expect[usagePath] = usage ? hash(usage) : null;
      if (expectedFields.some(field => JSON.stringify(actual.find(candidate => candidate.name === field.name)) !== JSON.stringify(field)) || !usage?.toString('utf8').trim()) conflicts.push(`Incompatible schema or missing usage: ${existing.directory}`);
      reused.push({ role, alias, directory: existing.directory, usage: usagePath });
      continue;
    }
    const directory = `game-design/${alias}`;
    let entries = [];
    try { entries = await readdir(await safePath(root, directory)); } catch (error) { if (error.code !== 'ENOENT') throw error; }
    if (entries.length) { conflicts.push(`Nonempty unconfigured target requires migration: ${directory}`); continue; }
    newCollections.push({ role, alias, directory });
    for (const name of ['.schema.md', '.usage.md', '.template.md']) {
      const bytes = templates.files.get(`${role}/${name}`);
      await propose(`${directory}/${name}`, bytes);
    }
  }
  for (const file of files) {
    const path = `${target}/${file.path}`;
    const current = await optional(await safePath(root, path));
    const base = prior?.files?.[file.path];
    await propose(path, file.bytes, Boolean(current && base && hash(current) === base));
  }
  for (const [name, base] of Object.entries(prior?.files ?? {})) {
    if (isAbsolute(name) || name.includes('\\') || name.split('/').some(part => !part || part === '.' || part === '..')) fail('MANIFEST_INVALID', `Unsafe managed path: ${name}`);
    if (files.some(file => file.path === name)) continue;
    const path = `${target}/${name}`;
    const current = await optional(await safePath(root, path));
    expect[path] = current ? hash(current) : null;
    if (!current) continue;
    if (hash(current) !== base) { conflicts.push(`Locally edited retired file: ${path}`); continue; }
    changes.push({ path, operation: 'delete', before: base, sha256: null });
  }
  let ignore = (await optional(await safePath(root, '.gitignore')))?.toString('utf8') ?? '';
  const lines = ignore.split(/\r?\n/).map(line => line.trim());
  for (const pattern of ['.attendant/', 'game-design-results/*/artifacts/']) if (!lines.some(line => line === pattern || line === `/${pattern}` || (pattern === '.attendant/' && line === '**/.attendant/'))) ignore += `${ignore && !ignore.endsWith('\n') ? '\n' : ''}${pattern}\n`;
  await propose('.gitignore', Buffer.from(ignore), true);
  const block = `<!-- game-design-workbench:start -->\n### Game design workbench\n\nFor game-design operations, load tools/game-design-workbench/SKILL.md.\n<!-- game-design-workbench:end -->`;
  const agents = (await optional(await safePath(root, 'AGENTS.md')))?.toString('utf8') ?? '';
  const managed = /<!-- game-design-workbench:start -->[\s\S]*?<!-- game-design-workbench:end -->/.exec(agents);
  const normalizedManaged = managed?.[0].replaceAll('\r\n', '\n');
  if (managed && normalizedManaged !== block && prior?.guidance_hash !== hash(normalizedManaged)) conflicts.push('Locally edited AGENTS.md workbench block');
  else {
    let next;
    const newline = agents.includes('\r\n') ? '\r\n' : '\n';
    const renderedBlock = block.replaceAll('\n', newline);
    if (managed) next = agents.replace(managed[0], () => renderedBlock);
    else {
      const section = /^## Attendant\s*\r?$/m.exec(agents);
      if (section) {
        const rest = agents.slice(section.index + section[0].length);
        const boundary = /^## /m.exec(rest);
        const end = boundary ? section.index + section[0].length + boundary.index : agents.length;
        next = `${agents.slice(0, end).trimEnd()}${newline}${newline}${renderedBlock}${newline}${newline}${agents.slice(end)}`;
      } else next = `${agents.trimEnd()}${agents.trim() ? newline + newline : ''}## Attendant${newline}${newline}${renderedBlock}${newline}`;
    }
    await propose('AGENTS.md', Buffer.from(next), true);
  }
  const manifest = { schema_version: 1, release: '0.1.0', distribution_revision: packageHash, model_version: 2, report_version: 1, concept_path: conceptName, concept_link_hash: conceptLinkHash, roles: mapping, files: Object.fromEntries(files.map(file => [file.path, hash(file.bytes)])), guidance_hash: hash(block), capabilities: ['guided-framework-review', 'attendant-sql-queries', 'term-resolution', 'dated-review-storage'], unavailable: ['native-c99-analysis', 'native-viewer', 'control-evaluation-export', 'simulation-runtime'], dependencies: { node: '>=24.10.0', attendant: 'required installed skill; GD_ATTENDANT_SKILL points to skill directory' } };
  await propose(manifestPath, Buffer.from(`${JSON.stringify(manifest, null, 2)}\n`), true);
  const publicPlan = { project: root, distribution_revision: packageHash, attendant_fingerprint: dependency_hash, concept: { path: conceptName, mode: context.mode, commit_required: changes.some(change => change.path === conceptName) }, roles: mapping, new_collections: newCollections, reused_collections: reused, candidate_sources, candidate_scan: { maximum_directory_depth: 2, excludes: ['hidden paths', 'node_modules', 'tools', 'vendor', 'build', 'configured collections'] }, fresh_acknowledged: Boolean(options.fresh), conflicts, files: changes.map(({ bytes, ...change }) => change), expected: expect, dependencies: manifest.dependencies };
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
      if (change.operation === 'delete') {
        await removeManaged(root, change.path, change.before, resolve(root, 'tools/game-design-workbench'));
        applied.push(`${change.path} (removed retired managed file)`);
        continue;
      }
      let expected = change.before;
      const created = plan.new_collections.some(c => change.path.startsWith(`${c.directory}/`));
      if (created) {
        const empty = change.path.endsWith('/.schema.md') ? '---\n---\n' : '';
        expected = hash(Buffer.from(empty));
      }
      await atomicWrite(root, change.path, change.bytes, expected);
      applied.push(change.path);
    }
    return { plan_id: approved, changed: applied, collections: plan.roles, concept: plan.concept };
  } catch (error) { error.data = { applied, cause: error.data ?? null, recovery: 'Inspect listed paths. No automatic reset/rollback was attempted.' }; throw error; }
}
