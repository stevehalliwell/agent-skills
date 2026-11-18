# Native ImGui migration

Raylib and rlImGui removed from source and build recipes. All Workbench entry points share ImGui/cimgui **1.93.0**: Win32/D3D11 on Windows and GLFW/OpenGL3 on macOS. Windows CJIT build and CPU integration checks pass. User accepted reviewed Windows Crate presentation and authorized checkpoint. macOS code and recipe are present; no Mac build or runtime result yet.

Checkpoint `530c8d7` saves Actors recovery and initial dependency. Native port checkpoint follows. Existing viewers keep their own binaries and draft state. Do not rename or reuse transitional `gd-ui.dll`.

## Ownership

- `gd_native_window*.c`: C99 owns window, ImGui context, event pump and close requests. Windows pumps at most 256 owned messages per frame. Native adapters supply keyboard, pointer, clipboard and timing. Window never writes source records or decides draft exit.
- `gd_native_ui*.cpp`: graphics dependency only. One owning thread/context. `begin_frame`: 1 ready, 0 minimized, -1 failed. No retry of failed graphics frame. Explorer still polls guarded IPC when minimized; retained draft can finish on explicit close after graphics failure.
- `gd_native_canvas.c` / `gd_scene_draw.c`: shared anti-aliased ImGui primitives, transform and nested screen clipping. Bundled vector font uses fixed world metrics and cache keys across zoom; no extra font assets. Canvas replaces drawing calls, not pane state, query, history, filters or selection contracts.
- PNG: re-submit completed draw data to owned offscreen target, never re-run UI/model or read another window. Bound to 16,777,216 pixels; explicit UTF-8 filename. Windows WIC; macOS ImageIO.
- ABI: matching pinned headers/library and `IMGUI_DISABLE_OBSOLETE_FUNCTIONS` for all dependency translation units. See [pins and C99 adapter profile](vendor/cimgui/PROVENANCE.md).

## Build and checks

[Build commands](README.md#build). Use fresh output directories. Windows `-View`/`-Graph` build same full viewer. `-ExplorerTest` compiles app-local checks; no window is launched by builds. `--scene-helpers`, `--ipc-helpers`, `--state-chart-helpers`, `--pointer-helpers` and `--find-helpers` run on explicit game-design fixture paths without native windows. Pointer helpers feed ImGui events inside test process only, not OS input.

- Windows build: `build/full-native-review-win/gd-view.exe`; PE imports have no raylib or `gd-ui.dll`. Core150/150, headless native ABI25/25. Crate/Mario IPC426/400; each has 3330 controls, 25 feedback, 37 sidebar, 123 configuration, 425 app-local pointer, 244 Find and 45 State charts checks. Each builds 15 CPU ImGui frames covering all unified projections, both Grid variants, Similarity and both applied state-chart layouts. Receipts in `build/full-native-review-adapter/`. Historical deep layout/UX gates remain deferred, not relaxed.
- Prior authorized hidden backend checks passed143/143, including resize, PNG text/channel readback, Unicode path and teardown. Authorized hidden standalone matrix checks passed31 Crate/59 Mario; sources/model unchanged. User separately reviewed running native Crate Explorer; startup IPC inspection saw clean draft and no stale flag. This does not establish every-view interaction acceptance.
- Headless commands do not link graphics. Native application still uses CJIT; dependency-only Zig/C++ exception remains. macOS compiler/runtime qualification, DPI, live IPC/restart, performance and visible all-view review remain open. Step 6 full matrix remains deferred.
