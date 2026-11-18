# cimgui and Dear ImGui provenance

- cimgui upstream: https://github.com/cimgui/cimgui
- cimgui pin: `2a6a93fccd10f275ee9206167dc9698f6f00fac1`.
- Dear ImGui upstream: https://github.com/ocornut/imgui
- ImGui pin: `c1aaac65e896a45f81f47a4d68f9959e0f86c859`.
- ImGui version: **1.93.0**, version number **19300**.
- Licenses: `LICENSE` and `imgui/LICENSE.txt` (MIT).

Imported from existing local pinned Git objects; no downloads. Pins match native Attendant's dependency declarations, not cimgui's default submodule revision. Retain generated bindings, core/demo sources, licenses and upstream Win32/D3D11 and GLFW/OpenGL3 backends. No upstream examples, generator, build system, repository metadata or binaries retained. Line endings and thirteen imported trailing-whitespace locations normalized; no executable changes.

## Local header adaptation

`cimgui.h` uses Attendant's C99/CJIT layout adaptation: remove duplicate generated aliases and name three anonymous unions `value`, `backup` and `event`. Header marks alteration. No RichMd-specific selection flag or ImGui behavior patch imported. Other sources match pinned content after documented whitespace normalization.

Compile **all** C++ translation units with `IMGUI_DISABLE_OBSOLETE_FUNCTIONS`, matching generated binding profile. Without this definition, ImGuiIO/ImGuiStyle sizes differ across C99/C++ boundary. `test-native-ui.ps1` verifies public sizes, named-union offsets and context access against built DLL; do not bypass failed ABI checks.

## Migration boundary

`build-native-ui.ps1` selects Win32/D3D11; `build-native-ui.sh` selects GLFW/OpenGL3. Application remains C99/CJIT. Raylib/rlImGui are removed; running old binaries retain their own dependencies. Windows build checked; macOS runtime and human graphical acceptance pending. See `../../THIRD-PARTY.md`.
