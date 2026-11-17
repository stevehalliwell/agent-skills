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
- Version: **6.0**, commit `dbc56a87da87d973a9c5baa4e7438a9d20121d28`. Unmodified `raylib.h`, `rlgl.h`, license and [hashes/provenance](vendor/raylib/PROVENANCE.md) retained under `vendor/raylib/`.
- License: zlib; see `vendor/raylib/LICENSE`.
- Explicit dependency preparation: [Build](README.md#build). Sources/SDK/binaries remain in ignored build/cache or supplied external path. Builds never download.
- Mac `build-ui.sh` builds raylib and existing UI sources into `libgd-ui.dylib`. Recipe follows c-ink's Mac story-player build and native Attendant's SDK system-header flags. No c-ink audio/window extensions or prebuilt libraries imported.
- Windows scripts require matching 6.0 SDK headers/libraries; rebuild old 5.5 dependencies. Windows 6.0 runtime remains unverified. Headless program does not link raylib.

## Proto headers

[Copied headers, hashes, owner permission, and MIT license](vendor/proto-headers/README.md). Retained unchanged from `D:/Repos/proto-framework/include/`. Tests use `base.h` and `test.h`; no game-framework module or generated runner imported.

## cimgui / Dear ImGui / rlImGui

- cimgui/ImGui updated from existing local pinned Git objects; no downloads or prebuilt Attendant DLL imported. [Pins, C99 header adaptation and ABI profile](vendor/cimgui/PROVENANCE.md).
- cimgui pin: `2a6a93fccd10f275ee9206167dc9698f6f00fac1`; Dear ImGui pin: `c1aaac65e896a45f81f47a4d68f9959e0f86c859` (1.93.0 / 19300). MIT licenses: `vendor/cimgui/LICENSE`, `vendor/cimgui/imgui/LICENSE.txt`. Retained upstream Win32/D3D11 and GLFW/OpenGL3 backend sources share ImGui license.
- rlImGui pin: `1550009359ad975927f7f0e4a3f47e3f27123ea9`; zlib license: `vendor/rlImGui/LICENSE`. Retained marked local safety patches for backend shutdown and failed texture bookkeeping/upload; source provenance details them.
- `build-ui.ps1` / `build-ui.sh` explicitly compile existing UI sources with installed Zig C++, exceptions/RTTI disabled, `NO_FONT_AWESOME`, and matching raylib **6.0** headers/library. No font/icon assets, bindings generator, package install, or implicit download required.
- Existing Windows visual evidence used 5.5. Mac 6.0 compile/link and CPU adapter checks do not establish rendered acceptance.
- Native migration: `build-native-ui.ps1` compiles ImGui and narrow `gd_native_ui.cpp` C ABI adapter into no-raylib Win32/D3D11 DLL. `test-native-ui.ps1` compiles headless ABI check with CJIT. Application/window owner stays C99; C++ is limited to GUI dependency and its C ABI adapter. Headless computation does not link graphics. Existing raylib application remains until host/canvas migration; macOS native backend remains pending. Windows plain `-View` remains raylib-only; Mac `--view` shares prepared UI dylib to avoid a second dependency build.

## Local message pipes

- [Copied source, permission, license and hashes](vendor/local-pipe/PROVENANCE.md). Reused unchanged from c-ink's extracted proto-framework backend; Mac adapter retains workbench JSON protocol/session policy.
- Same-user Unix sockets, bounded framed I/O, cancellation and owned-inode cleanup. No automatic stale-endpoint removal or runtime dependency on sibling checkout.
