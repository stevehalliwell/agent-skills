# cimgui and Dear ImGui provenance

- cimgui upstream: https://github.com/cimgui/cimgui
- Exact cimgui commit: `125f397e0982fe9466eb0a162de547be5efec849`.
- Dear ImGui upstream: https://github.com/ocornut/imgui
- Exact ImGui submodule commit supplied by cimgui: `b48d1afbe8ee8b238e2961dc363a949dd7304e23`.
- ImGui version in these sources: **1.92.9b**, version number **19291**.
- Licenses: cimgui `LICENSE` and ImGui `imgui/LICENSE.txt` (MIT).

Vendored contents are generated `cimgui.h`, `cimgui.cpp`, `cimconfig.h`, licenses, and the ImGui core headers/sources, including the demo translation unit required by generated wrapper exports. Sources are unchanged from these revisions. The docking-capable generated binding exposes upstream APIs, but the showcase does not enable docking or multiple OS windows.

Generator scripts, binding metadata, upstream build systems, examples, other backends, binaries, and repository metadata are omitted. Normal builds compile the supplied generated bindings; LuaJIT or a binding generator is not required. This subset was paired with the revision in `../rlImGui/PROVENANCE.md` and raylib 6.0 in its source prototype project. Workbench copies the same sources unchanged but explicitly recompiles against its raylib 5.5 headers/import library; native graph captures check that matching 5.5 runtime. See `../../THIRD-PARTY.md`.
