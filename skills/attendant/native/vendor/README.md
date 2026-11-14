# Native dependencies

Keep upstream pins, licenses and local patch rules here. Build outputs and acquired GUI checkouts live under ignored `build/`. No runtime dependency on historical spike or reference repositories.

## SQLite

- Source: <https://www.sqlite.org/2026/sqlite-amalgamation-3530000.zip>, version 3.53.0.
- `SQLITE_SOURCE_ID`: `2026-04-09 11:41:38 4525003a53a7fc63ca75c59b22c79608659ca12f0131f52c18637f829977f20b`.
- Unmodified `sqlite3.c` and `sqlite3.h`. Public domain dedication remains in source headers; see <https://www.sqlite.org/copyright.html>.
- Build enables FTS5, disables loadable extensions and compiler intrinsics for CJIT, and disables SQLite mutexes for sequential shared-core use. Do not reuse threading choice for concurrent callers without review.

## LibYAML

- Source: <https://github.com/yaml/libyaml>, commit `90a56d4500aa1a1798514c5cb55c3ad4cb095f94`, version 0.2.5.
- Retained `src/`, `include/` and MIT `License`. Build passes version macros explicitly. Parser reads only; no emission.
- Local patch in `src/yaml_private.h`: `IS_BREAK_AT` recognizes CR/LF only. NEL/LS/PS remain content rather than normalized line breaks, preserving UTF-8 values and source marks. No general YAML 1.2 equivalence claim; Attendant enforces flat subset.

## json-build

- Source: <https://github.com/lcsmuller/json-build>, commit `f4fdbf2e738a2c0dee5aeae3a89ac0e90f71b223`.
- Retained `json-build.h` and MIT `LICENSE`.
- Local changes: removed `_auto` allocation surface, realloc copy macros, dynamic escape helper and unused `<stdlib.h>`; guarded `JSONB_API` for export decoration. Fixed-buffer writer unchanged.
- Output capacity includes trailing NUL; wire length excludes it. Reject zero capacity before calling writer. Set `JSONB_MAX_DEPTH` consistently.
- Raw tokens must already be valid JSON. Caller supplies checked numeric tokens; upstream number writer does not establish Attendant's exact numeric contract.

## libpdjson5

- Source: <https://github.com/boris-kolpackov/libpdjson5>, version 1.0.1, commit `ed602821f2929930e5179cd9240d468abf210d60`.
- Unmodified `pdjson5.c` and `pdjson5.h`, flattened from upstream `libpdjson5/`; retained root `UNLICENSE`.
- Native reads use strict JSON, non-streaming mode and caller-arena allocation. Build sets `LIBPDJSON5_STACK_MAX=4`; query parameter values must be scalar/null. Raw number events permit exact conversion without floating-point intermediates.

## GUI stack

`bash tools/native-tool.sh prepare` acquires exact revisions only when explicitly run; `prepare --verify` checks existing pins/patches offline. Checkouts, licenses, assets and generated wrappers stay under `build/deps/`:

- cimgui: `2a6a93fccd10f275ee9206167dc9698f6f00fac1`, <https://github.com/cimgui/cimgui>, MIT.
- Dear ImGui: `c1aaac65e896a45f81f47a4d68f9959e0f86c859`, <https://github.com/ocornut/imgui>, MIT.
- RichMd: `609039721f838108251520079fff79011cf46659`, <https://github.com/pthom/imgui_rich_md>, MIT; includes separate third-party notices/assets.
- MD4C: `481fbfbdf72daab2912380d62bb5f2187d438408`, <https://github.com/mity/md4c>, MIT.
- LuaJIT, generation only: `a4c064539ea931274d407e1249b4bb575424a0f4`, <https://github.com/LuaJIT/LuaJIT>, MIT.
- GLFW, macOS only: `7b6aead9fb88b3623e3b3725ebb42670cbe4c579` (3.4), <https://github.com/glfw/glfw>, zlib/libpng license.

macOS uses GLFW/OpenGL3 following upstream RichMd examples. Windows uses Win32/D3D11. Retain upstream licenses/notices with redistributed libraries/assets, including font and embedded third-party notices in pinned checkouts. Core license files remain beside vendored sources: `libyaml/License`, `json-build/LICENSE` and `libpdjson5/UNLICENSE`; SQLite dedication remains in source headers.

### Maintained bridge and patches

`gui/richmd_c.cpp`, `gui/richmd_c.h`, `gui/preview-selection.cpp.inc` and `gui/find-text.h` are maintained source, not generated output. Native generation/patch helpers live under `tools/native/`; strict ordered ImGui/RichMd patch data lives in `tools/native/patches/`. Preview-method include remains authoritative, not copied into patch data.

1. Guard ImGui `IM_NODEBUGSTEP` for generator compatibility without layout changes. Add opt-in inactive input selection visibility for source find; no ImGui struct layout change.
2. Remove known duplicate generated C typedefs; name three C-view anonymous unions for C99. GUI startup checks C/C++ sizes and union offsets.
3. Reconstruct four RichMd file patches from pinned originals: genuine/synthetic run provenance, strict source proof, selectable preview/context, cached visible-text find/provenance, edge scrolling and invalidation/ownership. Reject unknown changes or missing expected patch.

Build verifies revisions/allowed tracked edits offline and regenerates wrappers. Stack manifest hashes maintained inputs, dependencies, generated files, generator and native library; records host/backend. GUI manifests bind application binaries to stack. Use one cimgui/ImGui implementation; never mix wrapper revisions.

`imgui_demo.cpp` remains because generated cimgui exports reference its entry points.

## Source attribution

`../src/include/test.h` originated in `proto-framework/include/test.h` at checkout commit `1721f1cb6da5689eb3e4998c9dda6a2f81ec0109`. Initial unchanged CRLF input SHA-256: `cd64e0180d8d814fff29d64cfdef15a87276e4d78d8d17b7d239d01674c865e5` (not current local header hash). Approved local extensions add suite/case lifecycle, owned fixture-directory helpers, bounded synchronous/caller-polled live test-command supervision/assertions, file/suffix assertions and teardown-safe generated runner; local file now uses LF. `base.h` adds ordinary application operations: exclusive creation/comparison, bounded tree operations, UTF-8 file/environment access, read-only setting and byte/text suffix comparison. Neither header depends on Attendant or adds test-only process supervision to `base.h`. Original source checkout remains unchanged. See [test interfaces/limits](../tests/README.md).

Shared `src/include/base.h`, local json-build modifications and decision fixture originated in `proto-framework` at commit `f0b848e2541903ac99696f1f8a71d93e72ef4c5b`. Bridge and shared support were retained in accepted Attendant snapshot `330136285cb1470ccecc00ca244894c9de2122a3`. Git preserves import history; no original local paths are required to build.

Decision fixture is test data, not project task authority. Checkout may change fixture line endings; checks compare actual copied bytes. Other fixtures are synthetic.
