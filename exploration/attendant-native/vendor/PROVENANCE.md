# Native projection dependencies

## SQLite

- Source: <https://www.sqlite.org/2026/sqlite-amalgamation-3530000.zip>.
- Version: 3.53.0. `SQLITE_SOURCE_ID`: `2026-04-09 11:41:38 4525003a53a7fc63ca75c59b22c79608659ca12f0131f52c18637f829977f20b`.
- Retained unmodified `sqlite3.c` and `sqlite3.h`; shell and extension header not needed.
- Public domain dedication is in the source headers; see <https://www.sqlite.org/copyright.html>.
- SHA-256 of imported bytes:

```text
c94657c2a08ed5f3f61a25f0018eb2f64ec42fd053da26d741db9e52e6ffaa77  sqlite/sqlite3.c
8e43a79982422f4720685f62839718f0e41410c1b71355aa32af3615ccd9e31d  sqlite/sqlite3.h
```

Build flags enable FTS5, disable loadable extensions, disable compiler intrinsics for CJIT, and disable SQLite mutexes for this single-process sequential builder. Do not reuse that threading choice for a concurrent viewer without review.

## LibYAML

- Source: <https://github.com/yaml/libyaml>, commit `90a56d4500aa1a1798514c5cb55c3ad4cb095f94`.
- Version 0.2.5 from source configuration. Build passes version macros explicitly.
- Retained `src/`, `include/` and MIT `License`. Parser used for reading only; no document emission.
- Local patch: `src/yaml_private.h`, `IS_BREAK_AT` recognizes CR/LF only, matching YAML 1.2 treatment of NEL/LS/PS as content rather than normalized line breaks. Keeps literal UTF-8 values and source marks intact. No claim of general YAML 1.2 equivalence; Attendant still enforces its flat subset.

## json-build

Copied unchanged from `D:/Repos/proto-framework/vendor/json-build/` at repository commit `f0b848e2541903ac99696f1f8a71d93e72ef4c5b`. Retained header, MIT license and existing provenance, including its upstream pin and local fixed-buffer modifications. Header SHA-256: `20a95741624dd363caf79361b4e6ec4e4e3e8b4414d535bb4a0ac54d9bd20ffb`. Uses fixed-buffer writer.

## libpdjson5

Copied unchanged from `D:/Repos/proto-framework/vendor/libpdjson5/`: version 1.0.1, upstream commit `ed602821f2929930e5179cd9240d468abf210d60`. Retained C source, header, Unlicense and existing provenance. Native reads select strict JSON and non-streaming mode, with caller-arena allocation. Parser depth is compiled with `LIBPDJSON5_STACK_MAX=4`; parameter values themselves must be scalar/null. No fixed-shape `json.h` binding used.

## Fixture

`../fixtures/project/records/decisions/vendor-project-dependencies-by-default.md` copies the YAML spike's real fixture, originally from `D:/Repos/proto-framework/records/decisions/vendor-project-dependencies-by-default.md` at commit `f0b848e2541903ac99696f1f8a71d93e72ef4c5b`. Original local SHA-256: `f0997a1b0947d9b03a5feba8d0fe72b2a3ee4bb85f2d41cb9477216ec626b531`. Git checkout may change fixture line endings; tests freeze and compare actual copied bytes rather than rely on the original hash. Other fixtures are synthetic and carry no project task authority.
