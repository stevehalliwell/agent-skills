// Generate isolated experimental C variants; never rewrite normal native sources.
// Usage: node sqlite-experiment-source.mjs <profile>; compile printed projection.c.
// Exact replacement checks deliberately fail when upstream source changes.
import assert from 'node:assert/strict';
import { readFileSync, writeFileSync, mkdirSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
export const profiles = ['control', 'wal-off', 'bulk-off', 'memory', 'lazy', 'lazy-wal-off', 'no-quick-check', 'page8', 'page16', 'page32', 'fts-deferred', 'fts-optimize'];
export function generate(profile) {
assert.ok(profiles.includes(profile), 'Unknown experiment profile');
const files = Object.fromEntries(['projection.c', 'prepare.h', 'read.h'].map(name => [name, readFileSync(join(root, 'tests/fixtures/sqlite-experiment-base', name), 'utf8')]));
function change(file, old, replacement) {
  assert.equal(files[file].split(old).length, 2, `${file}: expected one match for ${old.slice(0, 100)}`);
  files[file] = files[file].replace(old, () => replacement);
}
const lazy = profile === 'lazy' || profile === 'lazy-wal-off';
const indexedSeparately = lazy || profile === 'fts-deferred';
if (['wal-off', 'bulk-off', 'lazy-wal-off', 'memory'].includes(profile)) {
  change('prepare.h', 'PRAGMA synchronous=NORMAL;', 'PRAGMA synchronous=OFF;');
}
if (profile === 'bulk-off' || profile === 'memory') {
  change('prepare.h', 's = prepare(c, "PRAGMA journal_mode=WAL");', profile === 'memory' ? 's = prepare(c, "PRAGMA journal_mode=MEMORY");' : 's = prepare(c, c->temporary ? "PRAGMA journal_mode=OFF" : "PRAGMA journal_mode=WAL");');
  change('prepare.h', '!strcmp((const char *)sqlite3_column_text(s, 0), "wal");', profile === 'memory' ? '!strcmp((const char *)sqlite3_column_text(s, 0), "memory");' : '!strcmp((const char *)sqlite3_column_text(s, 0), c->temporary ? "off" : "wal");');
  if (profile === 'bulk-off') {
    change('prepare.h', '    projection_writer(c);\n    sql(c, "CREATE TABLE', '    sql(c, "PRAGMA locking_mode=EXCLUSIVE; PRAGMA cache_size=-65536; PRAGMA cache_spill=OFF; PRAGMA temp_store=MEMORY");\n    projection_writer(c);\n    sql(c, "CREATE TABLE');
    change('prepare.h', 'sql(c, "COMMIT; PRAGMA optimize;");', 'sql(c, "COMMIT; PRAGMA optimize; PRAGMA journal_mode=WAL;");');
  }
}
if (profile.startsWith('page')) {
  const size = Number(profile.slice(4)) * 1024;
  change('prepare.h', '    projection_writer(c);\n    sql(c, "CREATE TABLE', `    sql(c, "PRAGMA page_size=${size}");\n    projection_writer(c);\n    sql(c, "CREATE TABLE`);
}
if (profile === 'no-quick-check') {
  // Doctor still performs real integrity check; only normal preparation skips it.
  change('prepare.h', '== SQLITE_OK && snapshot_integrity(c))', '== SQLITE_OK)');
  change('prepare.h', 'if (snapshot_integrity(c) && plan_projection_delta(c, delta))', 'if (plan_projection_delta(c, delta))');
}
if (indexedSeparately) {
  const start = files['projection.c'].indexOf('    for (i = 0; i < 2; i++) {\n        s = prepare(c, format_text(c, "INSERT INTO %s (record_id,content)');
  const end = files['projection.c'].indexOf('    c->projected++;', start);
  assert.ok(start > 0 && end > start);
  const loop = files['projection.c'].slice(start, end);
  const guard = lazy ? '!c->search_ready' : 'c->defer_fts';
  const helper = `static void index_record(context *c, record *r) {\n    collection *table = r->owner;\n    sqlite3_stmt *s;\n    int i;\n    value *hidden = lookup(&r->front, "hidden");\n    if (${guard} || r->blocked || (hidden && hidden->number.real)) return;\n${loop}}\n`;
  change('projection.c', loop, '    index_record(c, r);\n');
  change('projection.c', 'static void project_record(context *c, record *r) {', helper + 'static void project_record(context *c, record *r) {');
  change('projection.c', 'int record_count, projected,', `int ${lazy ? 'search_ready' : 'defer_fts'}, record_count, projected,`);
}
if (profile === 'fts-deferred') {
  change('prepare.h', '    c->projected = 0;\n    for (i = 0; i < c->record_count; i++) project_record', '    c->defer_fts = 1;\n    c->projected = 0;\n    for (i = 0; i < c->record_count; i++) project_record');
  change('prepare.h', '    make_edges(c);\n    sql(c, "COMMIT;', '    c->defer_fts = 0;\n    for (i = 0; i < c->record_count; i++) index_record(c, &c->records[i]);\n    make_edges(c);\n    sql(c, "COMMIT;');
}
if (profile === 'fts-optimize') {
  change('prepare.h', '    make_edges(c);\n    sql(c, "COMMIT;', '    make_edges(c);\n    for (i = 0; i < c->collection_count; i++) {\n        collection *table = &c->collections[i];\n        sql(c, format_text(c, "INSERT INTO %s(%s) VALUES(\'optimize\')", internal(c, "fts", table), internal(c, "fts", table)));\n        sql(c, format_text(c, "INSERT INTO %s(%s) VALUES(\'optimize\')", internal(c, "trigram", table), internal(c, "trigram", table)));\n    }\n    sql(c, "COMMIT;');
}
if (lazy) {
  const creationStart = files['projection.c'].indexOf('    sql(c, format_text(c, "CREATE VIRTUAL TABLE');
  const creationEnd = files['projection.c'].indexOf('\n}', creationStart);
  const creation = files['projection.c'].slice(creationStart, creationEnd);
  change('projection.c', creation, '    if (c->search_ready) make_search_tables(c, table);');
  change('projection.c', 'static void make_tables(context *c, collection *table) {', `static void make_search_tables(context *c, collection *table) {\n${creation}\n}\nstatic void make_tables(context *c, collection *table) {`);
  const functions = `/* Experimental lazy indexes: absent or complete, committed atomically. */
static int search_state(context *c) {
    if (snapshot_metadata_matches(c, "search_ready", "1")) { c->search_ready = 1; return 1; }
    c->search_ready = 0;
    return snapshot_metadata_matches(c, "search_ready", "0");
}
static int search_counts(context *c, collection *table, int count) {
    if (!c->search_ready) return 1;
    return snapshot_count_matches(c, format_text(c, "SELECT count(*) FROM %s", internal(c, "fts", table)), count) &&
        snapshot_count_matches(c, format_text(c, "SELECT count(*) FROM %s", internal(c, "trigram", table)), count);
}
static void invalidate_search(context *c) {
    int i;
    for (i = 0; i < c->collection_count; i++) {
        sql(c, format_text(c, "DROP TABLE IF EXISTS %s", internal(c, "fts", &c->collections[i])));
        sql(c, format_text(c, "DROP TABLE IF EXISTS %s", internal(c, "trigram", &c->collections[i])));
    }
    c->search_ready = 0;
    sql(c, "UPDATE __attendant_metadata SET value='0' WHERE key='search_ready'");
}
`;
  change('prepare.h', '/* JSON text can collide', functions + '/* JSON text can collide');
  change('prepare.h', '    int i, matches;\n    if (!snapshot_metadata_matches', '    int i, matches;\n    if (!search_state(c)) return 0;\n    if (!snapshot_metadata_matches');
  change('prepare.h', '    memset(delta, 0, sizeof(*delta));', '    memset(delta, 0, sizeof(*delta));\n    if (!search_state(c)) return 0;');
  change('prepare.h', 'if (!snapshot_count_matches(c, format_text(c, "SELECT count(*) FROM %s", internal(c, "fts", table)), visible) ||\n            !snapshot_count_matches(c, format_text(c, "SELECT count(*) FROM %s", internal(c, "trigram", table)), visible))', 'if (!search_counts(c, table, visible))');
  change('prepare.h', 'if (!snapshot_count_matches(c, format_text(c, "SELECT count(*) FROM %s", internal(c, "fts", table)), count) ||\n            !snapshot_count_matches(c, format_text(c, "SELECT count(*) FROM %s", internal(c, "trigram", table)), count))', 'if (!search_counts(c, table, count))');
  change('prepare.h', 'for (i = 0; i < 2; i++) {\n            s = prepare(c, format_text(c, "DELETE FROM %s WHERE record_id=?"', 'for (i = 0; c->search_ready && i < 2; i++) {\n            s = prepare(c, format_text(c, "DELETE FROM %s WHERE record_id=?"');
  change('prepare.h', '    /* Delete the whole affected set first:', '    if (changed) invalidate_search(c);\n    /* Delete the whole affected set first:');
  change('prepare.h', '    metadata(c, "layout_version", "2");', '    c->search_ready = 0;\n    metadata(c, "search_ready", "0");\n    metadata(c, "layout_version", "2");');
  const ensure = `static void ensure_search(context *c, const char *root) {
    int i;
    const char *target = full_path(c, root, ".attendant/attendant.sqlite");
    if (sqlite3_open_v2(target, &c->db, SQLITE_OPEN_READWRITE, NULL) != SQLITE_OK) fail(c, "Cannot open lazy search writer");
    projection_writer(c);
    if (!snapshot_integrity(c) || !snapshot_matches(c)) fail(c, "Projection changed while preparing search; retry");
    if (!c->search_ready) {
        invalidate_search(c);
        c->search_ready = 1;
        for (i = 0; i < c->collection_count; i++) make_search_tables(c, &c->collections[i]);
        for (i = 0; i < c->record_count; i++) index_record(c, &c->records[i]);
        sql(c, "UPDATE __attendant_metadata SET value='1' WHERE key='search_ready'");
    }
    sql(c, "COMMIT");
    if (sqlite3_close(c->db) != SQLITE_OK) fail(c, "Cannot close lazy search writer");
    c->db = NULL;
}
`;
  change('prepare.h', 'static void print_sync(', ensure + 'static void print_sync(');
  change('read.h', '    open_snapshot(c, &options);', '    if (!strcmp(options.command, "search") && !c->search_ready) ensure_search(c, options.project);\n    open_snapshot(c, &options);');
}
// Disabled unless fault-test runner supplies both phase and owned signal path.
change('projection.c', '#include "prepare.h"', `static void experiment_pause(context *c, const char *phase) {
    const char *wanted = getenv("ATTENDANT_EXPERIMENT_PHASE");
    const char *path = getenv("ATTENDANT_EXPERIMENT_SIGNAL");
    FILE *signal;
    if (!wanted || !path || strcmp(wanted, phase)) return;
    if (sqlite3_db_cacheflush(c->db) != SQLITE_OK) fail(c, "Cannot flush experimental dirty pages");
    signal = fopen(path, "wb");
    if (!signal) fail(c, "Cannot signal experimental pause");
    fputs(phase, signal);
    if (fclose(signal)) fail(c, "Cannot close experimental signal");
    Sleep(30000);
}
#include "prepare.h"`);
change('prepare.h', '    sql(c, "COMMIT");\n    return changed', '    experiment_pause(c, "delta");\n    sql(c, "COMMIT");\n    return changed');
change('prepare.h', '    make_edges(c);\n', '    make_edges(c);\n    experiment_pause(c, "build");\n');
if (lazy) change('prepare.h', '    sql(c, "COMMIT");\n    if (sqlite3_close(c->db) != SQLITE_OK) fail(c, "Cannot close lazy search writer");', '    experiment_pause(c, "search");\n    sql(c, "COMMIT");\n    if (sqlite3_close(c->db) != SQLITE_OK) fail(c, "Cannot close lazy search writer");');
const directory = join(root, 'build/sqlite-experiments', profile);
mkdirSync(directory, { recursive: true });
for (const [name, text] of Object.entries(files)) writeFileSync(join(directory, name), text);
return join(directory, 'projection.c');
}
if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  if (process.argv[2] === '--help') console.log('Usage: node sqlite-experiment-source.mjs <' + profiles.join('|') + '>');
  else try {
    assert.equal(process.argv.length, 3, 'Expected one profile; use --help');
    console.log(generate(process.argv[2]));
  } catch (error) {
    console.error(JSON.stringify({ error: { code: profiles.includes(process.argv[2]) ? 'generation-failed' : 'invalid-arguments', message: error.message } }));
    process.exitCode = profiles.includes(process.argv[2]) ? 1 : 2;
  }
}
