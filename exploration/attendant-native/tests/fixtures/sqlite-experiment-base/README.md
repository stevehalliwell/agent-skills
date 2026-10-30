# Pre-adoption SQLite experiment base

Frozen `projection.c`, `prepare.h`, `read.h` from incremental WAL/NORMAL implementation, before production lazy-FTS/mmap/integrity-policy changes. `sqlite-experiment-source.mjs` uses these inputs so historical variants retain defined meaning when current implementation changes.

Other source-contract/write/doctor headers remain shared with `src/core/` and `apps/cli/`; base/vendor dependencies remain shared with native application. No standalone runtime or second production implementation. Historical artifact hashes remain authority for exact measured binaries; shared-header changes can change later rebuilds.

Current shared YAML/write headers expect parser/document and rollback ownership fields absent from this frozen context. Historical rebuild is unqualified and currently fails; folder cleanup does not repair or silently modernize baseline. Original measured artifacts remain authoritative.

See `../../../SQLITE-DISPOSABLE.md`. New production-path comparisons must label this old eager baseline separately from current native defaults.
