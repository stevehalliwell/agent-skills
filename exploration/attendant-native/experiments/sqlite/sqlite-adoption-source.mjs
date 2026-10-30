// Current-policy comparison variants, generated only under ignored build/.
// Usage: node experiments/sqlite/sqlite-adoption-source.mjs <wal-off|full|full-wal-off>
// Copy current CLI/core layout so relative includes retain their production meaning.
import assert from 'node:assert/strict';
import { cpSync, readFileSync, writeFileSync, mkdirSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
export const profiles = ['wal-off', 'full', 'full-wal-off'];
export function generate(profile) {
  assert.ok(profiles.includes(profile), 'Unknown adoption profile');
  let prepare = readFileSync(join(root, 'src/core/prepare.h'), 'utf8');
  function change(old, replacement) {
    assert.equal(prepare.split(old).length, 2, `Expected one prepare fragment: ${old.slice(0, 100)}`);
    prepare = prepare.replace(old, () => replacement);
  }
  if (profile !== 'full') change('PRAGMA synchronous=NORMAL;', 'PRAGMA synchronous=OFF;');
  if (profile.startsWith('full')) change(
    '    if (mode == PROJECTION_SEARCH && !c->fts_enabled) populate_search(c, root);',
    `    if (mode == PROJECTION_SEARCH && !c->fts_enabled) {
        c->fts_enabled = 1;
        rebuild_projection(c, root); return "rebuilt";
    }`);
  const directory = join(root, 'build/sqlite-adoption', profile);
  mkdirSync(directory, { recursive: true });
  for (const folder of ['src', 'apps/cli', 'vendor']) cpSync(join(root, folder), join(directory, folder), { recursive: true });
  writeFileSync(join(directory, 'src/core/prepare.h'), prepare);
  return join(directory, 'apps/cli/main.c');
}
if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  if (process.argv[2] === '--help') console.log('Usage: node experiments/sqlite/sqlite-adoption-source.mjs <' + profiles.join('|') + '>');
  else try { console.log(generate(process.argv[2])); }
  catch (error) { console.error(error.message); process.exitCode = profiles.includes(process.argv[2]) ? 1 : 2; }
}
