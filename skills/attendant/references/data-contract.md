# Source, projection and limits

Read for source types, write failure handling, projection behavior or resource limits. Markdown remains authoritative; `.attendant/` is disposable SQLite projection state.

## Source and data contract

- Flat YAML front matter: scalar values and scalar arrays only. Reject duplicate keys, nested structures, aliases, anchors, explicit tags/documents, embedded NUL and invalid UTF-8. Preserve Unicode without normalization; CR/LF are line breaks, NEL/LS/PS remain scalar content. Bodies remain human Markdown.
- Date: `YYYY-MM-DD`. Datetime: `YYYY-MM-DDTHH:mm:ss.sssZ`, valid calendar/time, years 1601–9999. No offset guessing or date normalization.
- Decimal integers retain exact `i64`/`u64`; decimal point/exponent selects finite `f64`. Reject overflow, nonfinite values and nonzero float underflow to zero. SQLite scalar values/parameters reject integers above `INT64_MAX`; JSON-backed arrays retain `u64`. Use lossless JSON parsing for large integer results.
- `create`/`update` accept nonempty arrays of `{name, fields}`. Only declared fields plus `desc`/`tags` are writable. Defaults/template apply to create. Reject invalid types/references, immutable fields, unsafe names, collisions and invalid prospective batches before publication.
- Update/correction splices changed value spans; preserve unrelated bytes, comments, newline style and body. Formatting inside replaced values may change. Parse/validate serialized values before staging.
- Flush exclusive sibling temporary files; compare captured source before replacement. Publication is atomic per file, not across a batch. Completed writes survive later failures and are reported as `written`; source remains committed if projection refresh fails. No universal concurrent-writer guarantee.
- Add-table/first feedback bootstrap publish owned seeds before config registration. On macOS, failed setup retains published files and reports written paths; staging and empty owned directories are cleaned. Windows removes unchanged owned files through share-denying handles, preserving changed/busy files. Inspect reported files before retrying; never adopt conflicting managed data.
- Native source API is single-threaded and recoverable; see [native source API](../native/src/native-api.h) for ownership, lifetimes and request/result contracts. GUI source reads/saves/feedback do not require preparing SQLite.

## Projection and limits

Full bounded source scan/hash/type validation remains required before cache reuse or changed-only reconciliation. Never reconstruct Markdown from cached JSON. Detect equal-stat edits, additions, deletions and hidden transitions. Cold caches defer FTS until first search; later edits maintain both indexes.

SQLite uses WAL/NORMAL, 2000 ms busy timeout and transactional projection updates. Recent cache commits may be lost after power failure; Markdown remains authoritative. Full rebuild uses temporary DB and safe replacement; active/nonempty sidecars are not blindly removed. Stop consumers before manual disposable-state removal.

After preparation, user SQL has a read-only connection and authorizer: one statement, no writes, attach, extensions or private cache/metadata access. Search combines full-text and trigram results with deterministic ties.

Bounds: 128 MiB caller arena plus 12 MiB JSON workspace; parser/SQLite allocate separately. At most 32 collections, 128 fields/schema, 256 keys/record, 1024 scalar array items, 4096 records and 4 MiB/source file. Combined arena may fail earlier. Query: 10,000 rows, 10 MiB serialized rows, 4 MiB SQL, 1024 columns, expression depth 100 and 10-million-VM-instruction budget. Search retains at most 4096 hits. Capacity failures are explicit; these are not hard wall-clock guarantees.
