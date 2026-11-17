# Native ImGui migration

## Selected direction

Remove raylib. Use Win32/D3D11 on Windows and GLFW/OpenGL3 on macOS, following Attendant's platform split. User selected ImGui **1.93** and approved dependency-only Zig/C++ builds. Application remains C99/CJIT. Full compiler/platform matrix stays at Step 6.

No-raylib backend is **not yet connected to Workbench viewer**. Do not rename `gd-native-ui.dll` to `gd-ui.dll`; interfaces differ. Existing raylib viewer remains usable during migration and uses rebuilt matching 1.93 transitional library. Running old viewers keep their own binaries/DLLs.

## Current slice

- [x] Upgrade pinned ImGui/cimgui to 1.93.0; retain licenses and upstream native backend sources.
- [x] Build isolated Win32/D3D11 library with C ABI, no raylib or RichMd dependency.
- [x] Verify C99/CJIT ABI, named-union offsets, context access, pre-init failure handling and absence of loaded raylib:21/21 headless checks.
- [x] Rebuild transitional dependency, production viewer and CPU adapter against 1.93. Crate426/Mario400 IPC checks pass, plus3330 controls/25 feedback/36 sidebar/123 configuration each. These remain raylib-backed application checks.
- [x] User-authorized hidden native GPU lifecycle check:48/48 passes. Three frames with resize, draw-data check, duplicate-init refusal and shutdown/reinitialization; no raylib loaded or visible window. Human graphical acceptance remains open.
- [ ] Replace C99 window/input/timing/cursor/screenshot plumbing and canvas rendering; preserve guarded IPC, independent panes, drafts, pending/applied inputs and last-good results.
- [ ] Implement macOS GLFW/OpenGL3 build/host and native qualification.
- [ ] Remove transitional raylib/rlImGui source/build dependencies after consumers migrate. No compatibility shim recreating whole raylib API.

## Boundary

`gd_native_ui.h` is C ABI. `gd_native_ui.cpp` owns device/swap chain, upstream ImGui platform/renderer adapters, resize and frame submission. C99 caller owns native window, ImGui context, message pump and all application/model state. One UI thread/context per adapter. Shutdown releases graphics resources, not caller window/context.

Native frame API: `begin_frame` returns `1` ready, `0` minimized, `-1` error. Other mutating operations return `1` success/`0` error. Error text is borrowed until next error. Failed frame is terminal for adapter; shut down graphics and destroy ImGui context, never retry that frame. Application retains model/drafts independently; adapter never saves sources, closes window or exits process.

Initialization attempts hardware then WARP. Neither path creates/focuses window. Resize handles back-buffer ownership explicitly. Render/present errors remain failures, not success or implicit query execution.

[Dependency pins, C99 header adaptation and required build profile](vendor/cimgui/PROVENANCE.md) own version/layout contract. Initial ABI failure traced to missing `IMGUI_DISABLE_OBSOLETE_FUNCTIONS` in C++ build, not a reason to bypass size checks. Corrected shared profile; removed unsuccessful regeneration experiment. No changes to other projects' caches or sources.

## Windows commands

Run from native directory. Use fresh output directories. Existing dependency DLL and nonempty check output are refused. No scripts download or install dependencies.

```powershell
$env:ZIG_GLOBAL_CACHE_DIR = "$PWD/build/native-ui-zig-cache"
./build-native-ui.ps1 -OutputDirectory ./build/native-ui-review
./test-native-ui.ps1 -UiLibrary ./build/native-ui-review/gd-native-ui.dll
```

Build emits one JSON result; compiler output goes to stderr. Check emits one JSON result, exit0 success/2 failure. Missing library refuses before creating check output. Test output defaults to fresh GUID directory; explicit nonempty output is refused.

Only after explicit window-check approval:

```powershell
./test-native-ui.ps1 -UiLibrary ./build/native-ui-review/gd-native-ui.dll -Gpu
```

`-Gpu` creates one owned hidden non-activating window, renders three frames with resize, checks drawing data, shuts down/reinitializes backend and destroys owned window. Never posts OS input or manipulates existing viewers. This is device-lifecycle validation, not human graphical acceptance.

## Evidence and limits

Current dependency/check: `build/native-ui-193-final/gd-native-ui.dll`, `build/native-ui-193-gpu-check-v2/test-native-ui.exe`. Headless21 and hidden GPU48 checks pass. Transitional viewer: `build/imgui-193-transition-view/gd-view.exe`; compiled, not launched. Verification receipts also sit beside dependency DLL; logs stay in session temp directory.

Actors noun-browser fix has independent core150/150 and two-example CPU checks. Full historical shared-UX helper still fails empty-State-charts history assumption in prior checkpoint binary too; no unrelated driver repair or weakened assertion retained. Native backend draws into owned hidden window; actual Workbench native rendering, device-loss recovery, screenshot capture, final platform checks and all remaining walkthrough feedback stay open.
