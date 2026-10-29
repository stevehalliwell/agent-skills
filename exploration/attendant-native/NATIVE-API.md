# Embedded source API

`native-api.h` exposes C99 source read/save and selected-text feedback. CLI stays independent; no GUI/C++ dependency added to CLI.

## Lifetime and errors

- Create session with `attendant_open()`, close with `attendant_close()`.
- Single UI thread; no concurrent/reentrant requests. One 128 MiB request arena per session.
- Caller owns NUL-terminated UTF-8 input strings, at most 4 MiB each. Do not pass borrowed previous result as next input; copy into document/UI storage first.
- Result pointers valid until next request or close. `ok=0` returns `code`, `message`, optional failed `path`, and any committed `written` paths left after cleanup.
- Explicit recovery boundary stays entirely inside C core. It never unwinds GUI callbacks or C++ frames. Expected I/O/validation failures do not exit GUI. Owned file enumeration/read handles, YAML parser/document resources, staged files and bootstrap rollback share cleanup.
- Upstream allocation aborts, invalid caller pointers and process termination are not universal recoverable-error guarantees.

## Operations

```c
attendant_result loaded = attendant_read_document(session, path);
/* Copy loaded text/path/hash into caller-owned document state. */
attendant_result saved = attendant_save_document(session, path, loaded_hash, edited_text);
/* Only after explicit Save & Send and successful save: */
attendant_result queued = attendant_send_feedback(session, &feedback);
```

Save checks exact current hash, stages/flushed bytes, and rechecks original bytes before atomic per-file replacement. External dirty conflict returns failure without overwrite.

Feedback requires saved target hash, nonempty selected range/text, and nonempty human comment. Range counts complete saved file's Unicode code points; CR and LF each count. Body retains exact supplied comment bytes. Feedback does not save target or refresh projection; ordinary CLI preparation discovers files later.

## Workspace discovery

`attendant_read_workspace(session, project, &workspace)` reads configured source records plus immediate nonhidden root `.md` files. Does not correct source, create projection or create feedback collection. Returns document paths/names/collection, invalid/pending markers, diagnostic count and schema-declared reference links; unresolved targets stay NULL. Output and nested strings/references use normal borrowed request lifetime. GUI copies snapshot before next API call; failed refresh retains previous GUI view.

## Managed collection

Canonical schema in `native-api.c`; alias `_meta_comments`, default `records/_meta_comments`. Complete declaration mapping must match v1; field order/comments need not match. Configured compatible alternate directory allowed.

First Send stages schema/template/usage/comment/config. Publishes owned files, then config last. On failure, remove only unchanged operation-owned bootstrap files. Cleanup opens without write/delete sharing, compares expected bytes and deletes through same handle; changed/busy files survive and remain in `written`. Remove only empty operation-created directories. Missing `.pi/attendant.tables` created only on first Send. Conflicts never overwrite existing collection files. Later Send publishes one comment only.

Reading never creates collection or consumes pending status. Human comment is contextual intent, not blanket permission to edit. Agent explicitly resolves with ordinary `update`; native and existing `.mjs` can read/resolve same source.

## Validation

```bash
bash build-api-check.sh cjit
bash build-api-check.sh zig
bash build.sh cjit
bash build.sh zig
node api-check.mjs
```

`api-probe.c` is test executable, not public CLI. Test build alone enables `ATTENDANT_TEST_FAULTS`; production API does not accept environment-triggered failures/edits. Generated fixtures and evidence under ignored `build/api-check/`; report `build/api-check-results.json`.
