# rlImGui provenance

- Upstream: https://github.com/raylib-extras/rlImGui
- Exact commit: `1550009359ad975927f7f0e4a3f47e3f27123ea9`.
- License: `LICENSE` (zlib).
- Included: `rlImGui.cpp`, `rlImGui.h`, `imgui_impl_raylib.h`, and license.
- Source prototype paired these files with raylib 6.0 and cimgui/ImGui pins in `../cimgui/PROVENANCE.md`; its Windows/browser checks are origin evidence only. Workbench explicitly rebuilds these sources against matching raylib 5.5 headers/import library and checks native graph captures with that 5.5 runtime. No workbench browser-build claim; arbitrary versions are not interchangeable.

Optional Font Awesome support is disabled with `NO_FONT_AWESOME`; its font data, icon headers, assets, upstream tools, build systems, examples, and binaries are not included.

## Local changes

`rlImGui.cpp` is an altered source, marked near its includes:

- Clear `Renderer_RenderState` after freeing backend state, allowing safe repeated initialization/shutdown.
- Guard texture bookkeeping allocation and invalid GPU texture creation. Leave the upload request outstanding on failure so the project integration can report an explicit error rather than dereference NULL or render texture zero.

No other backend behavior is intentionally changed. Source prototype used low-level API from its UI module. Workbench graph uses upstream high-level C API directly; no prototype UI module/adapter is copied.
