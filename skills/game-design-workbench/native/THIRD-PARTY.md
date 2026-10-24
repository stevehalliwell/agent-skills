# Native dependency notices

## LibYAML

- Upstream: https://github.com/yaml/libyaml
- Pin: `2c891fc7a770e8ba2fec34fc6b545c672beb37e6` (0.2.5).
- Retained unchanged: `include/yaml.h`, `src/api.c`, `src/parser.c`, `src/reader.c`, `src/scanner.c`, `src/yaml_private.h`, `License` under `vendor/libyaml/`.
- Local addition: `include/config.h` supplies version macros for direct compilation. No emitter/dumper, test suite, upstream build system, or binaries retained.
- License: MIT; see `vendor/libyaml/License`.
- Purpose: parse YAML events, not interpret Attendant schemas. LibYAML owns parser allocation; domain model uses explicit caller allocator.

## PCG minimal C

- Upstream: https://github.com/imneme/pcg-c-basic
- Pin: `bc39cd76ac3d541e618606bcc6e1e5ba5e5e6aa3`.
- License: Apache-2.0, copyright 2014 Melissa O'Neill. Unchanged source/header and full license retained; [provenance and hashes](vendor/pcg-basic/PROVENANCE.md).
- Purpose: experiment-local reproducible streams, not cryptography. Reentrant API only; shared helper bounds its own rejection sampling. Builds never fetch dependencies.

## Raylib

- Upstream: https://github.com/raysan5/raylib
- Version: 5.5; `raylib.h` from `5.5/src/raylib.h`, `rlgl.h` from matching SDK, license retained under `vendor/raylib/`.
- License: zlib; see `vendor/raylib/LICENSE`.
- Optional Windows SDK: https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_win64_mingw-w64.zip
- Tested archive SHA-256: `fe8feda1b92fc02826e2d19c631d648fd0563b93b144893fe18140e9fe4fb6fd`.
- SDK/DLL exists only in ignored local cache/build output. Build accepts explicit SDK path and never downloads it. Headless program does not link raylib.

## Proto headers

[Copied headers, hashes, owner permission, and MIT license](vendor/proto-headers/README.md). Retained unchanged from `D:/Repos/proto-framework/include/`. Tests use `base.h` and `test.h`; no game-framework module or generated runner imported.

## cimgui / Dear ImGui / rlImGui

- Copied vendored source subset from `D:/Repos/proto-framework/vendor/`, with licenses/provenance intact. No proprietary framework modules, adapter, or prebuilt prototype DLL copied.
- cimgui pin: `125f397e0982fe9466eb0a162de547be5efec849`; Dear ImGui pin: `b48d1afbe8ee8b238e2961dc363a949dd7304e23` (1.92.9b / 19291). MIT licenses: `vendor/cimgui/LICENSE`, `vendor/cimgui/imgui/LICENSE.txt`.
- rlImGui pin: `1550009359ad975927f7f0e4a3f47e3f27123ea9`; zlib license: `vendor/rlImGui/LICENSE`. Retained marked local safety patches for backend shutdown and failed texture bookkeeping/upload; source provenance details them.
- `build-ui.ps1` explicitly compiles source into ignored `gd-ui.dll` with installed Zig C++, exceptions/RTTI disabled, `NO_FONT_AWESOME`, and matching raylib **5.5** headers/import library. No font/icon assets, bindings generator, package install, or download required.
- Source copy was previously paired with raylib 6.0 in prototype project. Workbench graph tests use newly built UI DLL plus raylib 5.5 SDK/runtime; versions are not silently mixed. Checked native cimgui controls/details and bounded force/column graph capture.
- Application stays C99; C++ exists only in vendored dependencies. Headless computation and raylib-only table build do not link these libraries.
