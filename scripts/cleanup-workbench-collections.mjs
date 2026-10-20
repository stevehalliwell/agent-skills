#!/usr/bin/env node
// Canonical tutorial migration only, not an arbitrary-project converter.
// Usage: node scripts/cleanup-workbench-collections.mjs [--check|--apply]
// Default: show plan. Apply guards affected sources against 82a97f6, preserves
// identities, embeds measurements, moves existing proposals into domain records,
// and leaves installed tools/history untouched for subsequent guarded update.
import { readFile, readdir, mkdtemp, rm, unlink, rmdir } from 'node:fs/promises';
import { spawnSync } from 'node:child_process';
import { tmpdir } from 'node:os';
import { resolve, dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { model } from '../skills/game-design-workbench/scripts/model.mjs';
import { hash, optional, atomicWrite, safePath } from '../skills/game-design-workbench/scripts/lib.mjs';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const home = 'skills/game-design-workbench'; const example = `${home}/examples/crate-puzzle`;
const baseline = '82a97f6'; const retired = ['controls', 'measures', 'decisions'];
const args = process.argv.slice(2);
if (args.length === 1 && args[0] === '--help') { console.log('Canonical tutorial only: --check (default), --apply. Guards affected files against 82a97f6, rejects unknown retired files/custom bindings, preserves 63 record IDs plus embedded measure provenance. Renames controls to control-vars, embeds measures in route-cost, moves pulling/crate-color proposals to domain records, and updates status schemas. No staging/commits/reports/installation. After apply run guarded setup update. Exit 0 success, 1 failure, 2 usage.'); process.exit(0); }
if (args.length > 1 || args.length && !['--check', '--apply'].includes(args[0])) { console.error('Use --help.'); console.log(JSON.stringify({ ok: false, errors: [{ code: 'ARGUMENT_INVALID', message: 'Use --help.' }] })); process.exit(2); }
const applied = []; let temporary;
function git(args) { const child = spawnSync('git', ['-C', root, ...args], { timeout: 10000, maxBuffer: 2 * 1024 * 1024 }); return child.status === 0 ? child.stdout : null; }
try {
  const manifestPath = `${example}/tools/game-design-workbench/manifest.json`;
  const manifest = JSON.parse(await readFile(join(root, manifestPath)));
  if (manifest.model_version === 3) {
    for (const role of retired) for (const base of [`${home}/templates`, `${example}/game-design`]) if (await optional(join(root, base, role, '.schema.md'))) throw Error(`Partial cleanup: ${base}/${role}`);
    console.log(JSON.stringify({ ok: true, already_cleaned: true, changes: [] })); process.exit(0);
  }
  if (manifest.model_version !== 2 || !retired.every(role => manifest.roles[role] === role)) throw Error('Expected canonical model-2 aliases; custom projects need separately reviewed migration.');
  const changes = [];
  async function propose(path, bytes) {
    const fullPath = await safePath(root, path); const before = await optional(fullPath);
    if (bytes === null && before === null || bytes !== null && before?.equals(bytes)) return;
    const original = git(['show', `${baseline}:${path}`]);
    if (original ? !before?.equals(original) : before !== null) throw Error(`Changed source or occupied destination: ${path}`);
    changes.push({ path, before: before ? hash(before) : null, bytes, operation: bytes === null ? 'delete' : 'write' });
  }
  temporary = await mkdtemp(join(tmpdir(), 'gd-cleanup-'));
  const generated = join(temporary, 'fixture');
  const build = spawnSync(process.execPath, [join(root, home, 'scripts/build-example.mjs'), '--output', generated], { encoding: 'utf8', timeout: 30000 });
  if (build.status !== 0) throw Error(build.stderr || 'Fixture generation failed.');
  for (const role of Object.keys(model)) {
    for (const name of ['.schema.md', '.usage.md', '.template.md']) {
      const bytes = await readFile(join(generated, 'game-design', role, name));
      await propose(`${home}/templates/${role}/${name}`, bytes);
      await propose(`${example}/game-design/${role}/${name}`, bytes);
    }
    if (['control-vars', 'mechanics', 'interactions', 'experiments', 'resources'].includes(role)) for (const name of await readdir(join(generated, 'game-design', role))) {
      if (!name.startsWith('.')) await propose(`${example}/game-design/${role}/${name}`, await readFile(join(generated, 'game-design', role, name)));
    }
  }
  await propose(`${example}/game-design/verbs/pull.md`, await readFile(join(generated, 'game-design/verbs/pull.md')));
  for (const role of retired) for (const base of [`${home}/templates`, `${example}/game-design`]) {
    const directory = `${base}/${role}`;
    const tree = git(['ls-tree', '-r', '--name-only', baseline, '--', directory]);
    if (!tree) throw Error(`Cannot inspect baseline directory: ${directory}`);
    const allowed = tree.toString('utf8').trim().split('\n').map(path => path.slice(directory.length + 1));
    let entries;
    try { entries = await readdir(join(root, directory)); } catch (error) { if (error.code !== 'ENOENT') throw error; entries = []; }
    for (const name of entries) {
      if (!allowed.includes(name)) throw Error(`Unknown retired source: ${directory}/${name}`);
      await propose(`${directory}/${name}`, null);
    }
  }
  await propose(`${example}/.pi/attendant.tables`, await readFile(join(generated, '.pi/attendant.tables')));
  for (const role of retired) delete manifest.roles[role];
  manifest.roles['control-vars'] = 'control-vars'; manifest.model_version = 3;
  await propose(manifestPath, Buffer.from(JSON.stringify(manifest, null, 2) + '\n'));
  if (args[0] === '--apply') {
    for (const item of changes) { const bytes = await optional(await safePath(root, item.path)); if ((bytes ? hash(bytes) : null) !== item.before) throw Error(`Source changed: ${item.path}`); }
    for (const item of changes) {
      if (item.bytes !== null) await atomicWrite(root, item.path, item.bytes, item.before);
      else { if (hash(await readFile(await safePath(root, item.path))) !== item.before) throw Error(`Source changed: ${item.path}`); await unlink(join(root, item.path)); }
      applied.push(item.path);
    }
    for (const role of retired) for (const base of [`${home}/templates`, `${example}/game-design`]) await rmdir(join(root, base, role)).catch(error => { if (error.code !== 'ENOENT') throw error; });
  }
  console.log(JSON.stringify({ ok: true, apply: args[0] === '--apply', collections: 14, records: 63, changes: changes.map(({ bytes, ...item }) => ({ ...item, after: bytes ? hash(bytes) : null })) }));
} catch (error) { console.error(error.message); console.log(JSON.stringify({ ok: false, applied, recovery: 'Inspect Git diff and applied paths; no automatic rollback.', errors: [{ code: 'CLEANUP_FAILED', message: error.message }] })); process.exitCode = 1; }
finally { if (temporary) await rm(temporary, { recursive: true, force: true }); }
