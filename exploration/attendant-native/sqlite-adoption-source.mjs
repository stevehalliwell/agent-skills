// Current-policy comparison variants, generated only under ignored build/.
// Usage: node sqlite-adoption-source.mjs <wal-off|late|late-wal-off>
// late preserves current mmap/check/once-enabled policy; isolates population vs rebuild.
import assert from 'node:assert/strict';
import { readFileSync, writeFileSync, mkdirSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
const root = dirname(fileURLToPath(import.meta.url));
export const profiles = ['wal-off', 'late', 'late-wal-off'];
export function generate(profile) {
  assert.ok(profiles.includes(profile), 'Unknown adoption profile');
  const files = Object.fromEntries(['projection.c', 'prepare.h', 'read.h'].map(name => [name, readFileSync(join(root, name), 'utf8')]));
  function change(file, old, replacement) {
    assert.equal(files[file].split(old).length, 2, `Expected one ${file} fragment: ${old.slice(0, 100)}`);
    files[file] = files[file].replace(old, () => replacement);
  }
  if (profile !== 'late') change('prepare.h', 'PRAGMA synchronous=NORMAL;', 'PRAGMA synchronous=OFF;');
  if (profile.startsWith('late')) {
    const start = files['projection.c'].indexOf('    if (c->fts_enabled) {\n        sql(c, format_text(c, "CREATE VIRTUAL TABLE');
    const end = files['projection.c'].indexOf('\n    }', start) + 6;
    assert.ok(start > 0 && end > start);
    const block = files['projection.c'].slice(start, end);
    const creation = block.slice(block.indexOf('\n') + 1, block.lastIndexOf('\n'));
    change('projection.c', block, '    if (c->fts_enabled) make_search_tables(c, table);');
    change('projection.c', 'static void make_tables(context *c, collection *table) {', `static void make_search_tables(context *c, collection *table) {\n${creation}\n}\nstatic void make_tables(context *c, collection *table) {`);
    const indexStart = files['projection.c'].indexOf('    for (i = 0; c->fts_enabled && i < 2; i++) {\n        s = prepare(c, format_text(c, "INSERT INTO %s (record_id,content)');
    const indexEnd = files['projection.c'].indexOf('    c->projected++;', indexStart);
    assert.ok(indexStart > 0 && indexEnd > indexStart);
    const loop = files['projection.c'].slice(indexStart, indexEnd);
    change('projection.c', loop, '    index_record(c, r);\n');
    change('projection.c', 'static void project_record(context *c, record *r) {', `static void index_record(context *c, record *r) {
    collection *table = r->owner;
    sqlite3_stmt *s;
    int i;
    value *hidden = lookup(&r->front, "hidden");
    if (r->blocked || (hidden && hidden->number.real)) return;
${loop}}
static void project_record(context *c, record *r) {`);
    change('prepare.h', `    if (mode == PROJECTION_SEARCH && !c->fts_enabled) {
        /* Full private build once; later sync/create/update keep indexes current. */
        c->fts_enabled = 1;
        reuse = incremental = 0;
    }
`, '');
    change('prepare.h', 'static void print_sync(', `/* Experiment only: populate existing cache under same current connection policy. */
static void populate_search(context *c, const char *root) {
    int i;
    if (projection_open(c, full_path(c, root, ".attendant/attendant.sqlite"), SQLITE_OPEN_READWRITE) != SQLITE_OK) fail(c, "Cannot open search writer");
    projection_writer(c);
    if (!snapshot_matches(c)) fail(c, "Projection changed while preparing search; retry");
    if (!c->fts_enabled) {
        c->fts_enabled = 1;
        for (i = 0; i < c->collection_count; i++) make_search_tables(c, &c->collections[i]);
        for (i = 0; i < c->record_count; i++) index_record(c, &c->records[i]);
        sql(c, "UPDATE __attendant_metadata SET value='1' WHERE key='fts_enabled'; UPDATE __attendant_metadata SET value='2' WHERE key='layout_version'");
        for (i = 0; i < c->record_count; i++) c->records[i].projection_changed = 0;
    }
    sql(c, "COMMIT; PRAGMA optimize");
    /* Match private full build's post-commit validation, including flushed FTS segments. */
    if (!snapshot_integrity(c) || !snapshot_matches(c)) fail(c, "Populated search failed validation");
    if (sqlite3_close(c->db) != SQLITE_OK) fail(c, "Cannot close search writer");
    c->db = NULL;
}
static void print_sync(`);
    change('read.h', '    open_snapshot(c, &options);', '    if (!strcmp(options.command, "search") && !c->fts_enabled) populate_search(c, options.project);\n    open_snapshot(c, &options);');
  }
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
