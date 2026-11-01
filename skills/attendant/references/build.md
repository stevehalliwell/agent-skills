# Build native Attendant

Use when explicitly evaluating/building native Attendant, or when a future native-selected invocation reports missing build. Existing JS runner remains default until rollout approval. Building native does not switch launcher or authorize editing live workspaces.

Resolve `<skill-dir>` from `SKILL.md`; native root is `<skill-dir>/native`. Run commands below from native root in Windows Git Bash. Paths may contain spaces; quote them.

## Required tools

- Windows x64 desktop for GUI; Windows x64 for CLI.
- CLI: installed CJIT or Zig. Paths are independent; Zig does not require CJIT.
- GUI DLL: installed Zig, Git, Bash and Node.js. GUI C99 app can then build with CJIT or Zig.
- Node.js 24.10.0+ for development checks/tools only. No Node runtime in native CLI/GUI.

No script installs compilers/packages. `CJIT` and `ZIG` override compilation executables. cimgui generator still invokes `zig cc` through PATH, even with `ZIG` override. Build caches live under native `build/`, never remove user's global cache.

## CLI

C dependencies are vendored; builds do not download.

```sh
bash build.sh cjit
bash build.sh zig
build/attendant-zig.exe --help
```

Choose compiler actually installed; both commands are needed only for dual-compiler verification. Zig uses `-O3`, no fast-math or explicit assertion removal.

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

DLL build verifies dependencies and regenerates matching wrappers. GUI build fails with build-reference guidance when DLL is absent; it never acquires dependencies. DLL is copied beside each application executable. Font/assets path is explicit:

```sh
build/gui/attendant-gui-zig.exe --project /path/to/owned/workspace \
  --assets "$PWD/build/deps/richmd/assets" --file README.md
```

Never overwrite mapped application/DLL or restart a manual viewer to make build succeed. Set `ATTENDANT_GUI_VARIANT=review2` to use fresh `build/gui-review2/`; same variable must be used for related builds/checks. Allowed variant: lowercase letters, digits and hyphens, starting with letter/digit.

## Focused development checks

```sh
bash tools/build-api-check.sh cjit
bash tools/build-api-check.sh zig
bash build-gui.sh cjit --check
bash build-gui.sh zig --check
node tests/core/cli-check.mjs
node tests/core/api-check.mjs
node tests/gui/check.mjs
node tools/benchmark.mjs 5
```

CLI and production GUI builds above are prerequisites. GUI check builds also produce headless model executables. Checks use unique owned fixture copies under `build/`, not live projects. Both compiler builds are required for combined check scripts. Generated evidence is not imported historical evidence.

## Missing build or failed setup

1. Read this reference, build requested target with installed compiler, then run its `--help` or focused check.
2. Return to original task and original target project; build working directory is not record workspace.
3. If tool/dependency/build is unavailable, report exact blocker and reference path. Do not auto-install, silently fall back to JS, or substitute old spike binaries.
4. Build/check success does not grant native rollout approval. Known limitations: [native README](../native/README.md#remaining-limits-and-history).
