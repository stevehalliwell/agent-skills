/* Experimental adapter, not part of normal builds. Intercept application-level
   SQLite opens to test connection settings without changing production defaults.
   Build via sqlite-tuning-build.sh. Set ATTENDANT_TUNING_REPORT=1 only for untimed
   probes: stderr reports effective settings and peak SQLite allocator bytes.
   WAL/NORMAL, transaction rollback and normal locking remain unchanged. */
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef SQLITE_TUNING_PROFILE
#error Build with sqlite-tuning-build.sh
#endif
#if SQLITE_TUNING_PROFILE < 0 || SQLITE_TUNING_PROFILE > 5
#error Invalid SQLite tuning profile
#endif
static int tuning_reports;
static void tuning_memory(void) {
    fprintf(stderr, "{\"profile\":%d,\"sqlitePeakBytes\":%lld}\n", SQLITE_TUNING_PROFILE, (long long)sqlite3_memory_highwater(0));
}
static long long tuning_value(sqlite3 *db, const char *statement) {
    sqlite3_stmt *query = NULL;
    long long value = -1;
    if (sqlite3_prepare_v2(db, statement, -1, &query, NULL) == SQLITE_OK && sqlite3_step(query) == SQLITE_ROW) value = sqlite3_column_int64(query, 0);
    sqlite3_finalize(query); return value;
}
static int tuning_configure(sqlite3 *db) {
    int result = SQLITE_OK;
    const char *report = getenv("ATTENDANT_TUNING_REPORT");
#if SQLITE_TUNING_PROFILE == 1
    result = sqlite3_exec(db, "PRAGMA cache_size=-16384", NULL, NULL, NULL);
#elif SQLITE_TUNING_PROFILE == 2
    result = sqlite3_exec(db, "PRAGMA mmap_size=67108864", NULL, NULL, NULL);
#elif SQLITE_TUNING_PROFILE == 3 || SQLITE_TUNING_PROFILE == 5
    result = sqlite3_exec(db, "PRAGMA cache_size=-16384; PRAGMA mmap_size=67108864; PRAGMA temp_store=MEMORY", NULL, NULL, NULL);
#elif SQLITE_TUNING_PROFILE == 4
    result = sqlite3_exec(db, "PRAGMA cache_size=-500000; PRAGMA mmap_size=4294967296; PRAGMA temp_store=MEMORY", NULL, NULL, NULL);
#endif
    if (report && !strcmp(report, "1")) {
        if (!tuning_reports++) atexit(tuning_memory);
        fprintf(stderr, "{\"profile\":%d,\"cacheSize\":%lld,\"mmapSize\":%lld,\"tempStore\":%lld,\"pageSize\":%lld}\n", SQLITE_TUNING_PROFILE,
            tuning_value(db, "PRAGMA cache_size"), tuning_value(db, "PRAGMA mmap_size"), tuning_value(db, "PRAGMA temp_store"), tuning_value(db, "PRAGMA page_size"));
    }
    return result;
}
static int tuning_open(const char *path, sqlite3 **db) {
    int result = sqlite3_open(path, db);
    if (result == SQLITE_OK) result = tuning_configure(*db);
    return result;
}
static int tuning_open_v2(const char *path, sqlite3 **db, int flags, const char *vfs) {
    int result = sqlite3_open_v2(path, db, flags, vfs);
    if (result == SQLITE_OK) result = tuning_configure(*db);
    return result;
}
static int tuning_close(sqlite3 *db) {
#if SQLITE_TUNING_PROFILE == 5
    const char *path = sqlite3_db_filename(db, "main"), *name = path;
    int result;
    if (path) {
        const char *cursor;
        for (cursor = path; *cursor; cursor++) if (*cursor == '/' || *cursor == '\\') name = cursor + 1;
    }
    /* Full builds alone: private, committed native-PID.sqlite before publication.
       Recheckpoint after cleanup; never VACUUM the published incremental DB. */
    if (name && !strncmp(name, "native-", 7) && sqlite3_get_autocommit(db)) {
        result = sqlite3_exec(db, "VACUUM; ANALYZE; PRAGMA wal_checkpoint(TRUNCATE)", NULL, NULL, NULL);
        if (result != SQLITE_OK) return result;
    }
#endif
    return sqlite3_close(db);
}
#define sqlite3_open tuning_open
#define sqlite3_open_v2 tuning_open_v2
#define sqlite3_close tuning_close
#include "fixtures/sqlite-experiment-base/projection.c"
