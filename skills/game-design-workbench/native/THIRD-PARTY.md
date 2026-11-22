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

## GLFW (macOS only)

- Upstream: [GLFW 3.4](https://github.com/glfw/glfw/tree/3.4), zlib/libpng license. Prepare sources explicitly; not vendored. Build copies upstream license and matching public header beside library.
- `build-native-ui.sh` compiles Cocoa GLFW and ImGui OpenGL3. Capture uses Apple CoreGraphics/ImageIO. Windows uses Win32/D3D11 and Windows Imaging Component; no GLFW dependency there.

## Proto headers

[Copied headers, hashes, owner permission, and MIT license](vendor/proto-headers/README.md). Retained unchanged from `D:/Repos/proto-framework/include/`. Tests use `base.h` and `test.h`; no game-framework module or generated runner imported.

## cimgui / Dear ImGui

- cimgui/ImGui updated from existing local pinned Git objects; no downloads or prebuilt Attendant DLL imported. [Pins, C99 header adaptation and ABI profile](vendor/cimgui/PROVENANCE.md).
- cimgui pin: `2a6a93fccd10f275ee9206167dc9698f6f00fac1`; Dear ImGui pin: `c1aaac65e896a45f81f47a4d68f9959e0f86c859` (1.93.0 / 19300). MIT licenses: `vendor/cimgui/LICENSE`, `vendor/cimgui/imgui/LICENSE.txt`. Retained upstream Win32/D3D11 and GLFW/OpenGL3 backend sources share ImGui license.
- `build-native-ui.ps1` and `build-native-ui.sh` build matching dependency libraries with Zig/C++; application/window owner stays C99. `IMGUI_DISABLE_OBSOLETE_FUNCTIONS` is mandatory across dependency translation units.
- Bundled scalable ImGui font; no external font/icon assets. Headless computation never links graphics.

## Local message pipes

- [Copied source, permission, license and hashes](vendor/local-pipe/PROVENANCE.md). Reused unchanged from c-ink's extracted proto-framework backend; Mac adapter retains workbench JSON protocol/session policy.
- Same-user Unix sockets, bounded framed I/O, cancellation and owned-inode cleanup. No automatic stale-endpoint removal or runtime dependency on sibling checkout.
