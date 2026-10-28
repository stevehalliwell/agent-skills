# Pre-adoption SQLite experiment base

Frozen `projection.c`, `prepare.h`, `read.h` from incremental WAL/NORMAL implementation, before production lazy-FTS/mmap/integrity-policy changes. `sqlite-experiment-source.mjs` uses these inputs so historical variants retain defined meaning when current implementation changes.

Other source-contract/write/doctor headers and vendor code remain shared with exploration root. No standalone runtime or second production implementation. Historical artifact hashes remain authority for exact measured binaries; shared-header changes can change later rebuilds.

See `../../SQLITE-DISPOSABLE.md`. New production-path comparisons must label this old eager baseline separately from current native defaults.
