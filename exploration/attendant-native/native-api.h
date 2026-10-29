#ifndef ATTENDANT_NATIVE_API_H
#define ATTENDANT_NATIVE_API_H
#include <stddef.h>
#include <stdint.h>
/* C99, single UI thread. No CLI process or SQLite projection needed for source I/O.
 * Input strings: caller-owned, NUL-terminated UTF-8, at most 4 MiB each.
 * Result pointers belong to session; invalidated by next operation or close.
 * Session owns one 128 MiB request arena; close releases all backing memory. */
typedef struct attendant_session attendant_session;
typedef struct attendant_result {
    int ok;
    const char *code, *message, *path, *hash, *text;
    int written_count;
    const char *const *written;
} attendant_result;
typedef struct attendant_feedback {
    const char *project, *target_path, *target_hash;
    uint64_t range_start, range_end; /* Unicode code points, zero-based/end-exclusive. */
    const char *selected_text, *comment;
} attendant_feedback;
typedef struct attendant_reference {
    const char *field, *value, *target_path; /* NULL target: unresolved, never guessed. */
} attendant_reference;
typedef struct attendant_document_info {
    const char *path, *name, *collection;
    int invalid, pending, reference_count;
    attendant_reference *references;
} attendant_document_info;
typedef struct attendant_workspace {
    const char *root;
    int document_count, diagnostic_count;
    attendant_document_info *documents;
} attendant_workspace;
attendant_session *attendant_open(void);
void attendant_close(attendant_session *session);
/* Lists configured collection records plus immediate workspace Markdown files.
 * Never corrects source or creates projection/config. Views share result lifetime. */
attendant_result attendant_read_workspace(attendant_session *session, const char *project, attendant_workspace *workspace);
attendant_result attendant_read_document(attendant_session *session, const char *path);
attendant_result attendant_save_document(attendant_session *session, const char *path, const char *expected_hash, const char *text);
/* First Send creates/registers owned _meta_comments, publishing config last.
 * Does not save target, repair unrelated records, refresh DB, or consume feedback.
 * Failed bootstrap rolls back owned files; written reports any paths left behind. */
attendant_result attendant_send_feedback(attendant_session *session, const attendant_feedback *feedback);
#endif
