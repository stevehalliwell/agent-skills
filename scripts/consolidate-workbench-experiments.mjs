#!/usr/bin/env node
// Canonical fixture migration, not an arbitrary-project converter.
// Usage: node scripts/consolidate-workbench-experiments.mjs [--check|--apply]
// Default check shows exact hash-guarded writes/deletions. Apply requires pristine
// affected fixture files from ce0756b (ordinary Git recovery point). Keeps frozen
// reports/baselines and unaffected records. No staging, commits, or installation.
import { readFile, readdir, mkdtemp, rm, unlink, rmdir } from 'node:fs/promises';
import { spawnSync } from 'node:child_process';
import { tmpdir } from 'node:os';
import { resolve, dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { model } from '../skills/game-design-workbench/scripts/model.mjs';
import { hash, optional, atomicWrite } from '../skills/game-design-workbench/scripts/lib.mjs';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const home = 'skills/game-design-workbench'; const example = `${home}/examples/crate-puzzle`;
const args = process.argv.slice(2);
if (args.length === 1 && args[0] === '--help') { console.log('Canonical fixture only: --check (default), --apply. Reads pristine ce0756b sources; refuses affected fixture edits/unknown retired files; regenerates distribution templates. Preserves moved hypothesis UUID, embeds original criterion/UUID/nulls in timing experiment, retains frozen evidence. Updates shipped templates and fixture usage. After apply, run guarded setup update. Exit 0 success, 1 failure, 2 usage.'); process.exit(0); }
if (args.length > 1 || args.length && !['--check', '--apply'].includes(args[0])) { console.error('Use --help.'); console.log(JSON.stringify({ schema_version: 1, ok: false, errors: [{ code: 'ARGUMENT_INVALID', message: 'Use --help.' }] })); process.exit(2); }
let temporary;
const applied = [];
try {
  const manifestPath = `${example}/tools/game-design-workbench/manifest.json`;
  const manifestBytes = await readFile(join(root, manifestPath)); const manifest = JSON.parse(manifestBytes);
  if (manifest.model_version >= 2) {
    for (const role of ['hypotheses', 'expectations', 'observations']) if (await optional(join(root, example, 'game-design', role, '.schema.md'))) throw Error(`Partial migration: ${role}. Inspect Git diff before recovery.`);
    console.log(JSON.stringify({ schema_version: 1, already_consolidated: true, changes: [] })); process.exit(0);
  }
  if (!['hypotheses', 'expectations', 'observations'].every(role => manifest.roles[role] === role)) throw Error('Expected canonical legacy aliases; custom projects need separately reviewed migration.');
  const changes = [];
  async function propose(path, bytes, fixture = false) {
    const before = await optional(join(root, path));
    if (bytes !== null && before?.equals(bytes)) return;
    if (fixture) {
      const original = spawnSync('git', ['show', `ce0756b:${path}`], { cwd: root, maxBuffer: 2 * 1024 * 1024 });
      if (original.status === 0 ? !before?.equals(original.stdout) : before !== null) throw Error(`Fixture changed or destination occupied: ${path}`);
    }
    if (!before && bytes === null) return;
    changes.push({ path, before: before ? hash(before) : null, bytes, operation: bytes === null ? 'delete' : 'write' });
  }
  temporary = await mkdtemp(join(tmpdir(), 'gd-consolidate-'));
  const generated = join(temporary, 'fixture');
  const result = spawnSync(process.execPath, [join(root, home, 'scripts/build-example.mjs'), '--output', generated], { encoding: 'utf8', timeout: 30000 });
  if (result.status !== 0) throw Error(result.stderr || 'Fixture generation failed.');
  for (const role of Object.keys(model)) {
    for (const name of ['.schema.md', '.usage.md', '.template.md']) {
      const bytes = await readFile(join(generated, 'game-design', role, name));
      await propose(`${home}/templates/${role}/${name}`, bytes);
      if (name === '.usage.md' || ['dynamics', 'aesthetics', 'experiments'].includes(role)) await propose(`${example}/game-design/${role}/${name}`, bytes, true);
    }
  }
  for (const [role, name] of [['dynamics', 'careful-positioning'], ['aesthetics', 'deliberate-tension'], ['experiments', 'route-cost'], ['experiments', 'energy-encourages-planning']]) await propose(`${example}/game-design/${role}/${name}.md`, await readFile(join(generated, 'game-design', role, `${name}.md`)), true);
  for (const role of ['hypotheses', 'expectations', 'observations']) {
    for (const base of [`${home}/templates`, `${example}/game-design`]) {
      const allowed = ['.schema.md', '.usage.md', '.template.md', ...(base.endsWith('/game-design') ? role === 'hypotheses' ? ['energy-encourages-planning.md'] : role === 'expectations' ? ['route-time.md'] : [] : [])];
      for (const name of await readdir(join(root, base, role))) {
        if (!allowed.includes(name)) throw Error(`Unknown retired source: ${base}/${role}/${name}`);
        await propose(`${base}/${role}/${name}`, null, base.endsWith('/game-design'));
      }
    }
    delete manifest.roles[role];
  }
  await propose(`${example}/.pi/attendant.tables`, await readFile(join(generated, '.pi/attendant.tables')), true);
  manifest.model_version = 2;
  await propose(manifestPath, Buffer.from(JSON.stringify(manifest, null, 2) + '\n'), true);
  // Rewrite only role-array declarations; concepts remain hypotheses/observations.
  const packPath = `${home}/scripts/pack-data.mjs`; const packs = await readFile(join(root, packPath), 'utf8');
  const next = packs.replace(/roles: \[([^\]]*)\]/g, (_, body) => `roles: [${[...new Set([...body.matchAll(/'([a-z-]+)'/g)].map(match => ['hypotheses', 'observations', 'expectations'].includes(match[1]) ? 'experiments' : match[1]))].map(role => `'${role}'`).join(', ')}]`);
  await propose(packPath, Buffer.from(next));
  if (args[0] === '--apply') {
    // Preflight entire batch before first write; per-file guards check again.
    for (const item of changes) if (((await optional(join(root, item.path))) ? hash(await readFile(join(root, item.path))) : null) !== item.before) throw Error(`Source changed: ${item.path}`);
    for (const item of changes) {
      if (item.bytes !== null) await atomicWrite(root, item.path, item.bytes, item.before);
      else { if (hash(await readFile(join(root, item.path))) !== item.before) throw Error(`Source changed: ${item.path}`); await unlink(join(root, item.path)); }
      applied.push(item.path);
    }
    for (const role of ['hypotheses', 'expectations', 'observations']) for (const base of [`${home}/templates`, `${example}/game-design`]) await rmdir(join(root, base, role));
  }
  console.log(JSON.stringify({ schema_version: 1, apply: args[0] === '--apply', collections: 16, records: 67, changes: changes.map(({ bytes, ...item }) => ({ ...item, after: bytes ? hash(bytes) : null })) }));
} catch (error) { console.error(error.message); console.log(JSON.stringify({ schema_version: 1, ok: false, applied, recovery: 'Inspect Git diff and reported applied paths; no automatic rollback.', errors: [{ code: 'CONSOLIDATION_FAILED', message: error.message }] })); process.exitCode = 1; }
finally { if (temporary) await rm(temporary, { recursive: true, force: true }); }
