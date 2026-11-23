# Markdown workspace GUI internals

C99 application using shared source API and pinned ImGui/RichMd native library. Windows uses Win32/D3D11; macOS uses GLFW/OpenGL3.

- Human workflow, launch, feedback, find and agent IPC: [GUI workspace](../../../SKILL.md#gui-workspace).
- Build setup and platform checks: [build reference](../../../references/build.md).
- Source publication and resource limits: [data contract](../../../references/data-contract.md).

## Implementation boundaries

- `main.c` owns UI/focus, tabs, navigation, wrapped source geometry, find routing and pending agent controls. `find.h`, `find-ui.h` and `navigation.h` keep application policy outside reusable filesystem helpers.
- `workspace.c` owns source sessions, document revisions and selection snapshots. Undo remains ImGui-owned, isolated by literal full file-path bytes plus slot/generation; no persistent per-file history or same-path reopen lifetime promise.
- `reveal.h` defines passive reveal v1 and the shared navigation packet for live `--find` and `--open`. `inspect.h` defines inspection v2. `ipc-macos.h` supplies framed same-UID transport; Windows uses bounded `WM_COPYDATA`. Use GUI CLI client for ordinary controls, not hand-built packets.
- Automatic discovery uses shared `base.h` search with native `*.md` enumeration on Windows and `root_only`; no recursive directory pass. Generic recursive search remains available but disabled for automatic workspace discovery.
- GUI requires native library beside executable and explicit assets path. Both macOS compiler outputs launch without loader-environment setup. C++ exceptions stay behind C ABI. Borrowed bridge strings expire on next render/shutdown; copy before retaining them. Single UI thread/context.
- Terminal render failure abandons frame and exits; unsaved tabs/composer drafts are not recovered.

## Checks

`bash tools/check-gui.sh` uses native coordinator and retains model checks plus representative real rendering/ABI, wrapped reveal geometry, context-aware find/filter/clipboard refusal, references, diagnostics, editing/undo, dirty-close/save/queue, multi-section/context-only selection, inspection and resolution cases. Model covers reveal validation and malformed inspection packets; reduced suite does not repeat former full reveal/selection permutation matrix.

`bash tools/check-control-macos.sh` adds real cross-compiler reveal/inspection, dirty/stale Go guards, new-tab navigation, ambiguous/unrelated viewers, malformed/fragmented transport, deadlines and endpoint-policy checks. Both GUI suites explicitly remove `DYLD_LIBRARY_PATH` from macOS child environments. `bash tools/check-bridge.sh` preserves independent bridge lifetime/ABI/P6 checks. Earlier C macOS coordinators passed native compilation/runtime validation; later usability, root-only discovery and cleanup changes still need macOS validation. Current results/screenshots stay under ignored `build/`.
