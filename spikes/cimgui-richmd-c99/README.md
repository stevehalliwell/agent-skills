# cimgui + RichMd C99 spike

Branch: `spike/cimgui-richmd-c99`.

**Result: viable Windows x64 build/link/render path.** C99 application runs through CJIT against one Zig-built DLL containing matching cimgui, Dear ImGui 1.93.0, RichMd 1.0.0, and Win32/D3D11 backends. No ImGui Bundle or HelloImGui runtime.

Exact original-Markdown selection ranges remain unimplemented. See [RESULTS.md](RESULTS.md).

## Run

Requires Windows x64 desktop, Git, Bash, Node, Zig, and CJIT already installed. Scripts never install tools. `prepare.mjs` explicitly downloads pinned source; build/run stay offline.

```bash
cd spikes/cimgui-richmd-c99
node prepare.mjs
bash build.sh
node check.mjs
node --test tools.test.mjs

# Interactive C99 application, compiled in memory by CJIT:
bash run-cjit.sh

# Bounded run with screenshot:
bash run-cjit.sh --frames 30 --shot build/manual.ppm
```

Close window to exit interactive mode. Source editor changes memory only. Link clicks call C callback; they do not launch browser. Normal selection/copy uses Windows clipboard. Automated `--exercise` uses an in-process clipboard instead.

Zig-built C99 host uses same DLL:

```bash
build/bin/host-zig.exe --assets "$(cygpath -m "$PWD/build/deps/richmd/assets")"
```

`CJIT` overrides CJIT executable for runner/check. Build requires `zig` on PATH; `ZIG` changes compilation executable only, not generator's `zig cc` lookup. Node and locally built LuaJIT serve acquisition/generation/tests only. Neither is required by DLL or GUI at runtime.

## Boundary

- `host.c`: C99 application, Win32 window/message loop, cimgui source editor, layout, link/clipboard callbacks, smoke input.
- `richmd_c.h`: C99 ABI, opaque window, borrowed UTF-8 inputs, scalar/pointer outputs. No C++ types or ownership crosses boundary.
- `richmd_c.cpp`: RichMd bridge plus private Win32/D3D11 renderer state. C++17 dependencies stay inside DLL.
- `build/bin/cimgui_richmd.dll`: exports generated cimgui and bridge together. One ImGui implementation/context universe.
- `build/bin/host-zig.exe`: independent native C99 control host. Zig compilation includes `-std=c99 -pedantic-errors -O3 -Wall -Wextra -Werror`.

Single UI thread/context. Initialize ImGui before bridge; destroy bridge before ImGui. C++ exceptions caught at bridge boundary; upstream assertions, OS faults, invalid callback lifetimes, and allocation aborts are not converted into recoverable errors. This is not production feedback core.

## Dependencies and patches

`prepare.mjs` pins source commits. All checkouts and generated wrappers stay under ignored `build/deps/`. No installed repository's vendor tree changed.

- cimgui generator: `2a6a93fccd10f275ee9206167dc9698f6f00fac1`.
- Dear ImGui v1.93.0: `c1aaac65e896a45f81f47a4d68f9959e0f86c859`.
- RichMd v1.0.0: `609039721f838108251520079fff79011cf46659`.
- MD4C: `481fbfbdf72daab2912380d62bb5f2187d438408`, RichMd's submodule revision.
- LuaJIT generation tool: `a4c064539ea931274d407e1249b4bb575424a0f4`; built locally with Zig, not installed.

Regenerate cimgui against pinned 1.93 header; do not mix shipped 1.92.9b wrapper layouts with 1.93 DLL.

Required narrow compatibility changes:

1. Guard ImGui `IM_NODEBUGSTEP` definition so generator's existing empty override suppresses `[[gnu::artificial]]`. Otherwise parser emits invalid `artificial]]` declarations. Application compilation retains original attribute behavior.
2. Remove seven identical duplicate typedef declarations from generated C header. Duplicates require C11 and fail strict C99.
3. Give three C-view anonymous unions names: `ImGuiStoragePair.value`, `ImGuiStyleMod.backup`, `ImGuiInputEvent.event`. C99 lacks anonymous unions. C++ definitions remain untouched. Runtime probe compares sizes and union offsets between C99 consumer and C++ DLL.

Build rechecks revisions and tracked edits offline, then regenerates wrappers. Build manifest binds source-content hashes, generated headers, generator binary, Zig version, DLL, and native host. Check rejects changed inputs/binaries. Patch anchors fail closed on unexpected upstream content. Generated header's three named-union access paths are spike-specific; do not treat this as drop-in unmodified cimgui distribution.

DLL/core build uses `-O3`; generation tools use `-O2`. No fast-math or explicit assertion removal. Keep `imgui_demo.cpp` because full generated cimgui exports reference demo entry points.

## Enabled surface

Headings, lists, formatted/selectable text, tables, code blocks, admonitions, local bundled images, links, dynamic fonts/textures. Default asset reader loads fonts/images from RichMd assets folder. Private D3D11 image uploader owns shader-resource views through `shared_ptr`, capped at 4096×4096 pixels. This avoids RichMd default uploader's final-frame deferred CPU-texture cleanup gap. ImGui backend handles font textures. Host checks zero owned images after shutdown; success prints only after teardown.

Not enabled: Mermaid, LaTeX/MicroTeX, syntax-highlighting editor, FreeType, URL image downloads, embedded assets. No GLFW/raylib dependency. No runtime network dependency for tested document.

Windows links `d3dcompiler_47`, not generic `d3dcompiler` unavailable in tested Zig import-library set. CJIT 1.2 needs explicit `-L <Windows>/System32 -luser32` for application Win32 symbols; runner supplies it.

## Checks and artifacts

`check.mjs` runs fresh serial GUI processes, not headless mocks:

- Zig/CJIT: blank preview, rendered preview, scripted interaction; 30 frames each.
- Native layout check for ImGui IO/style/vector/draw types and patched union views.
- Actual D3D11 buffer readback; extra preview geometry and bright pixels.
- Byte-identical Zig/CJIT blank and rendered-preview images.
- Edit source, click link through C callback, drag preview selection, copy through C callback. No global mouse/keyboard injection or test overwrite of Windows clipboard.
- Reject invalid frame bound with exit 2.
- Guard tests reject tracked dependency drift and changed build inputs, replacing stale smoke success with failure.
- Inspect PE import table for external language/compiler runtime DLLs.

`build/results.json` retains current result, verified build identity, dependency pins, tool versions, DLL hash/imports, screenshot hashes. `build/build-manifest.json` retains individual build-input hashes. Failure replaces stale success with partial failure report. `build/*.{ppm,bmp,txt}` retain visual/log evidence. [evidence/windows-x64.json](evidence/windows-x64.json) records verified checkpoint.

Input exercise uses fixed test-window coordinates/default scale. It does not qualify arbitrary DPI, accessibility, real physical input routing, device loss, minimized-window loops, long paths, or production editing/save behavior. No performance benchmark claimed.

## Licenses

Downloaded licenses remain with pinned sources. Before distributing DLL/assets, package applicable notices from:

- `build/deps/cimgui/LICENSE` and `build/deps/cimgui/imgui/LICENSE.txt`.
- `build/deps/richmd/LICENSE`, `THIRD_PARTY_LICENSES.md`, `imgui_rich_md/LICENSE.imgui_md`.
- `build/deps/richmd/external/md4c/LICENSE.md` and stb notices in source.
- Font licenses under `build/deps/richmd/assets/fonts/` and image notices covered by upstream assets documentation.
- `build/deps/luajit/COPYRIGHT` if distributing generation tool.

No release/distribution packaging implemented.
