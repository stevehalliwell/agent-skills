// Current-policy comparison variants, generated only under ignored build/.
// Usage: node sqlite-adoption-source.mjs <wal-off|full|full-wal-off>
// Normal path populates FTS in place. full isolates replacement of current cache.
import assert from 'node:assert/strict';
import { readFileSync, writeFileSync, mkdirSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
const root = dirname(fileURLToPath(import.meta.url));
export const profiles = ['wal-off', 'full', 'full-wal-off'];
export function generate(profile) {
  assert.ok(profiles.includes(profile), 'Unknown adoption profile');
  const files = Object.fromEntries(['projection.c', 'prepare.h', 'read.h'].map(name => [name, readFileSync(join(root, name), 'utf8')]));
  function change(file, old, replacement) {
    assert.equal(files[file].split(old).length, 2, `Expected one ${file} fragment: ${old.slice(0, 100)}`);
    files[file] = files[file].replace(old, () => replacement);
  }
  if (profile !== 'full') change('prepare.h', 'PRAGMA synchronous=NORMAL;', 'PRAGMA synchronous=OFF;');
  if (profile.startsWith('full')) change('prepare.h',
    '    if (mode == PROJECTION_SEARCH && !c->fts_enabled) populate_search(c, root);',
    `    if (mode == PROJECTION_SEARCH && !c->fts_enabled) {
        c->fts_enabled = 1;
        rebuild_projection(c, root); return "rebuilt";
    }`);
  const directory = join(root, 'build/sqlite-adoption', profile);
  mkdirSync(directory, { recursive: true });
  for (const [name, text] of Object.entries(files)) writeFileSync(join(directory, name), text);
  return join(directory, 'projection.c');
}
if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  if (process.argv[2] === '--help') console.log('Usage: sqlite-adoption-source.mjs <' + profiles.join('|') + '>');
  else try { console.log(generate(process.argv[2])); }
  catch (error) { console.error(error.message); process.exitCode = profiles.includes(process.argv[2]) ? 1 : 2; }
}
