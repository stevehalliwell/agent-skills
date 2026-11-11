# Build native Attendant

- [Required tools](#required-tools)
- [CLI](#cli)
- [GUI dependencies and application](#gui-dependencies-and-application)
- [Focused development checks](#focused-development-checks)
- [Windows footprint sanity probe](#windows-footprint-sanity-probe)
- [macOS bring-up checkpoint](#macos-bring-up-checkpoint)
- [Missing build or failed setup](#missing-build-or-failed-setup)

Read when native executable is missing, user requests build/evaluation, or GUI dependencies need setup. Native CLI/GUI is active Attendant implementation; no JS fallback. Build with installed tools, then return to original target project and requested operation. Building does not authorize unrelated source edits.

Resolve `<skill-dir>` from `SKILL.md`; native root is `<skill-dir>/native`. Run commands below from native root. CLI/API/GUI builds support Windows Git Bash and macOS arm64 Bash. Windows commands use `.exe`; omit suffix on macOS. Platform details are below. Paths may contain spaces; quote them.

## Required tools

- Windows x64 or macOS arm64. GUI needs local desktop; macOS builds need installed Apple command-line tools/SDK.
- CLI: installed CJIT or Zig. Paths are independent; Zig does not require CJIT.
- GUI library: installed Zig, Git, Bash and Node.js; macOS also needs make. GUI C99 app can then build with CJIT or Zig.
- Native core/source tests: Bash and selected compiler only; no Node.js.
- Node.js 24.10.0+ for external-process/GUI integration checks and GUI build tools only. No Node runtime in native CLI/GUI.

No script installs compilers/packages. `CJIT` and `ZIG` override compilation executables. cimgui generator still invokes `zig cc` through PATH, even with `ZIG` override. Build caches live under native `build/`, never remove user's global cache.

## CLI

C dependencies are vendored; builds do not download.

```sh
bash build.sh cjit
bash build.sh zig
build/attendant-zig.exe --help
```

Choose compiler actually installed; both commands are needed only for dual-compiler verification. macOS outputs omit `.exe`, for example `build/attendant-zig --help`. `SDKROOT` overrides discovered Apple SDK; CJIT outputs receive local ad-hoc signatures. Zig uses `-O3`, no fast-math or explicit assertion removal.

## GUI dependencies and application

One explicit acquisition step requires network. Do not run it implicitly during ordinary record operations:

```sh
node tools/prepare-gui.mjs
```

For offline setup, supply already acquired pinned `build/deps/` checkouts, including Git metadata. `node tools/prepare-gui.mjs --verify` checks pins/patches offline; no network fallback. Sources/assets remain under native root.

```sh
bash tools/build-gui-stack.sh
bash build-gui.sh cjit
bash build-gui.sh zig
```

Library build verifies dependencies and regenerates matching wrappers. GUI build fails with build-reference guidance when library is absent; it never acquires dependencies. Runtime library is copied beside each application executable. Font/assets path is explicit:

```sh
# Windows: append .exe; macOS command below works with zig or cjit output.
build/gui/attendant-gui-zig --project /path/to/owned/workspace \
  --assets "$PWD/build/deps/richmd/assets" --file README.md
```

macOS Zig embeds executable-relative rpath. CJIT links against a private dylib copy with executable-relative install identity; runtime copy stays byte-identical to stack library. Link copy is build-manifest hashed. This avoids patching CJIT's Mach-O output, which installed `install_name_tool` rejects. Both GUI outputs run without `DYLD_LIBRARY_PATH`.

Never overwrite mapped application/DLL or restart a manual viewer to make build succeed. Set `ATTENDANT_GUI_VARIANT=review2` to use fresh `build/gui-review2/`; same variable must be used for related builds/checks. Allowed variant: lowercase letters, digits and hyphens, starting with letter/digit.

## Focused development checks

Direct C tests use locally extended `test.h` with shared `base.h` filesystem operations. No Node.js, GUI, downloads or prebuilt CLI needed:

```sh
bash test.sh cjit
bash test.sh zig
```

Each command runs 35 C cases under UTC and Pacific/Auckland plus 17 isolated framework scenarios. Stdout is retained run directory; TSV, diagnostics, generated runners and build logs stay in `build/native-tests/run-*/`. Assertions stay active at `-O3`/`NDEBUG`. Case failures exit 1; suite/infrastructure or main generation/build failures exit 2. Hooks clean successful owned case folders and retain/report failures; Attendant session/content setup stays test-local. See [test interfaces, memory/path limits and lifecycle contract](../native/tests/README.md).

External-process/GUI checks remain separate:

```sh
bash tools/build-api-check.sh cjit
bash tools/build-api-check.sh zig
bash build-gui.sh cjit --check
bash build-gui.sh zig --check
node tests/core/cli-check.mjs
node tests/core/api-check.mjs
node tests/core/workflow-check.mjs
node tests/core/install-check.mjs
node tests/gui/check.mjs
# macOS only:
node tests/gui/control-macos-check.mjs
node tests/gui/bridge-check.mjs
node tools/benchmark.mjs 5
```

Matching CLI/API-probe builds are prerequisites for external core integration; production/test GUI builds for GUI checks. `api-check.mjs` now has 16 API/CLI interoperability groups. Direct source/date/fault assertions moved to C suite; historical API totals below describe pre-migration suite. GUI check builds also produce headless model executables. Checks use unique owned fixture copies under `build/`, not live projects. Both compiler builds are required for combined check scripts. Generated evidence is not imported historical evidence. `workflow-check.mjs` exercises current init-project templates and task/decision flows with both native CLIs; run from repository checkout containing sibling `skills/init-project/templates/`. That test-only dependency is not needed to build or use Attendant. `install-check.mjs` copies skill without builds into owned temporary installation with spaces/Unicode, proves missing-executable failure, builds each CLI and returns to original owned workspace. It needs both compilers/Bash, acquires nothing and leaves logs plus installation path in `build/install-check-results.json`.

### Windows footprint sanity probe

After production builds, from native root with PowerShell 7:

```powershell
# Match variant used for production GUI builds; omit for build/gui.
$env:ATTENDANT_GUI_VARIANT = 'windows-regression'
pwsh -NoProfile -File tools/footprint-windows.ps1 -Seconds 10
```

Opens one owned viewer at a time for each compiler. Copies test collections and current product docs into unique `build/footprint/run-*/workspace/`; checks source inventory unchanged and closes only its own processes. JSON stdout and `results.json` record startup-to-inspection, idle CPU/working-set/private memory and inspection/reveal round trips. CPU percentage uses one logical core. Startup is not first paint; reveal response acknowledges receipt, not completed Go. No GPU/human-input latency measurement or numeric acceptance budget. Small-workspace sanity check only; macOS needs separate measurement.

## macOS bring-up checkpoint

Historical checkpoint before `test.h` migration: native CLI/API and interactive GLFW/OpenGL3 workspace build/run with Zig and CJIT. Combined acceptance passed 20 CLI groups, 94 API cases, 20 GUI/model groups (194 macOS model assertions per compiler) and 16 local-control groups on owned Unicode-path workspaces. API checks include timezone-independent UTC timestamps from 1601 through 9999, epoch boundaries and invalid-calendar rejection. Post-port Windows x64 qualification also passes: fresh CJIT/Zig CLI/API/production/test GUI/model builds, rebuilt Win32/D3D11 dependency DLL, 20 CLI groups, 94 API cases and 20 GUI/model groups with 193 model assertions per compiler. Pinned dependencies verified offline; checks use owned fixtures and fresh `ATTENDANT_GUI_VARIANT=windows-regression`.

```sh
bash build.sh zig
bash build.sh cjit
bash test.sh zig
bash test.sh cjit
bash tools/build-api-check.sh zig
bash tools/build-api-check.sh cjit
node tests/core/cli-check.mjs
node tests/core/api-check.mjs
build/attendant-zig --help
```

Source publication uses staged flushes and atomic rename/create-exclusive publication. On macOS, failed collection/feedback bootstrap retains already-published files and reports written paths; no check-and-unlink rollback or custom recovery system. Staging files and empty owned directories are cleaned. Inspect reported files before retrying. Windows keeps handle-based rollback. API failure checks cover retained seed/comment files, external edits, config conflicts and retry without overwrite.

GUI: native arm64 dependency library, C99 application and bridge consumers with GLFW 3.4/OpenGL3, matching pinned upstream RichMd examples. Windows retains Win32/D3D11. Requires installed Zig, make, Apple command-line tools/SDK, Node and CJIT for dual-compiler bridge checks. No script installs tools.

Explicit acquisition and checks:

```sh
node tools/prepare-gui.mjs
bash tools/build-gui-stack.sh
node tests/gui/bridge-check.mjs
```

Acquisition includes pinned GLFW only on macOS. Native LuaJIT recipe builds generation tool; no Windows x64 generator reuse. `SDKROOT` overrides Apple SDK; `MACOSX_DEPLOYMENT_TARGET` defaults to 11.0 for arm64 bring-up. This default is not tested minimum-OS support. SDK frameworks compile as system headers; Zig framework-link flags apply only at link time. No project diagnostic downgrade. MD4C alone uses upstream's `OFF_MAX` macro-warning compatibility flag.

Output: `build/bin/libcimgui_richmd.dylib`, `build/bin/luajit`, `build/bridge-check-results.json` and per-compiler P6 screenshots. Bridge checks create one bounded hidden window each; verify C99/C++ ABI, selectable Markdown rendering, image texture lifetime, Retina screenshot content and terminal render-failure cleanup. CJIT consumers use SDK includes and local ad-hoc signing; test process alone receives explicit library lookup path. No release signing/notarization claim.

macOS controls use existing bounded same-UID Unix message pipes. One request at a time; GUI pumps between frames, with frozen selection snapshot, per-connection deadline and bounded work. Discovery rejects ambiguous/unresponsive/malformed/unsafe candidates and never removes stale endpoints. Scope spans same UID across login sessions, not desktop isolation or application authentication. Same-UID processes can inspect selections or impersonate endpoints.

Human workflow and distribution/license review accepted as complete by user. Windows small-workspace footprint sanity check completed; macOS footprint measurement waived, with no macOS footprint claim. Native skill cutover approved. Known platform/failure limits stay documented, not an exhaustive rollout gate. Build/check success does not authorize committing or publishing.

## Missing build or failed setup

1. Read this reference, build requested target with installed compiler, then run its `--help` or focused check.
2. Return to original task and original target project; build working directory is not record workspace.
3. If tool/dependency/build is unavailable, report exact blocker and reference path. Do not auto-install, silently fall back to JS, or substitute old spike binaries.
4. Continue only original authorized operation; setup does not grant broader editing authority. Known limitations: [native README](../native/README.md#remaining-limits-and-history).
