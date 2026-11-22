# Build and check native tools

Run commands from repository root. Build the headless CLI for repeatable analysis. Build the viewer only for desktop use. See [dependency notices](THIRD-PARTY.md).

## Build

All views use ImGui/cimgui **1.93.0**: Win32/D3D11 on Windows, GLFW/OpenGL3 on macOS. Application code is C99. Zig builds the C++ graphics dependencies; headless commands need no graphics library. See [graphics dependency pins](THIRD-PARTY.md#cimgui--dear-imgui).

### Windows

Run from repository root with installed CJIT and Zig. `-View` and `-Graph` build the same full viewer. Use fresh output paths; never replace running binaries.

```powershell
$native = 'skills/game-design-workbench/native'
& "$native/build.ps1"  # headless CLI
$ui = & "$native/build-native-ui.ps1" -OutputDirectory "$native/build/native-ui" | ConvertFrom-Json
$view = & "$native/build.ps1" -View -UiLibrary $ui.library -OutputDirectory "$native/build/native-view"
```

Build scripts never launch windows, download dependencies or write source data. Run returned `$view` directly or set `GD_VIEW_EXECUTABLE` for `gd.ps1`. CJIT 1.2.0 requires ASCII build paths; runtime source and screenshot paths support Unicode. DLL must match pinned headers and build profile.

### macOS

Needs installed CJIT, Zig and Apple SDK. Prepare upstream GLFW 3.4 source separately from [release tag `3.4`](https://github.com/glfw/glfw/tree/3.4). Scripts never download or modify dependency checkout. Apple Silicon build/runtime checks pass; see [Mac qualification](#mac-qualification).

```sh
native=skills/game-design-workbench/native
bash "$native/build.sh"                    # headless CJIT CLI
ui=$(bash "$native/build-native-ui.sh" --glfw-directory /path/to/glfw-3.4)
bash "$native/build.sh" --view --ui-library "$ui" --output-directory "$native/build/mac-native-view"
bash "$native/build.sh" --explorer-test --ui-library "$ui" --output-directory "$native/build/mac-native-adapter"
```

`CJIT`, `ZIG`, `SDKROOT` override tools/SDK. Native `libgd-native-ui.dylib` loads relative to executable; prepared GLFW header lives beside dependency. `build-sources.txt` owns shared application source lists. Native macOS experiment supervision still fails explicitly; no model launch.

#### Mac qualification

Checked macOS 14.5 arm64, Apple SDK 14.5, CJIT `164cf0a`, Zig `0.17.0`, GLFW 3.4.0 and pinned ImGui/cimgui 1.93.0:

- CJIT and Zig: headless CLI, native core, viewer transport fixture, full viewer and CPU explorer adapter build and run. Core: 150/150 each. Node native suite: 94 passed, 5 skipped each; optional pointer/deep/layout checks not run.
- All 10 Explorer modes render and save PNGs with both compilers against two example models: 40/40 bounded smoke checks. Source hashes unchanged. These checks do not establish configured-query or mouse/keyboard acceptance.
- Live read-only IPC: `viewer list`, `inspect`, `nodes`, `controls`, and `actions` answered through the Unix socket. Viewer exited cleanly with no draft changes.

Reproduce headless qualification after preparing `$ui` above. Use fresh adapter directories on rerun. Repeat with `compiler=zig`:

```sh
compiler=cjit
cli=$(bash "$native/build.sh" --compiler "$compiler")
core=$(bash "$native/build.sh" --compiler "$compiler" --test)
transport=$(bash "$native/build.sh" --compiler "$compiler" --viewer-test)
adapter=$(bash "$native/build.sh" --compiler "$compiler" --explorer-test \
  --ui-library "$ui" --output-directory "$native/build/mac-check-adapter-$compiler")
"$core" "$native/build/mac-check-core-$compiler"
GD_NATIVE_EXE="$cli" GD_VIEWER_TEST_EXE="$transport" GD_VIEWER_ADAPTER_EXE="$adapter" \
  node --test skills/game-design-workbench/test/native-*.test.mjs
```

## Windows checks

```powershell
& "$native/test.ps1"
& "$native/test.ps1" -View -UiLibrary $ui.library
```

The second command includes windowed checks. Run it only when a test window is appropriate. Generated binaries and test output stay under ignored `native/build/`. Core and capture checks do not establish real-pointer interaction, layout readability, or workflow usefulness.
