# GUI dependencies and maintained bridge

Imported from accepted runtime spike `330136285cb1470ccecc00ca244894c9de2122a3`, paths `spikes/cimgui-richmd-c99/{richmd_c.cpp,richmd_c.h,preview-selection.cpp.inc}`. Bridge and selection implementation are maintained source, not generated output. Generation/patch scripts now live under `tools/`.

`tools/prepare-gui.mjs` acquires exact upstream revisions only when explicitly run. Checkouts, licenses, assets and generated wrappers stay under ignored `build/deps/`:

- cimgui: `2a6a93fccd10f275ee9206167dc9698f6f00fac1`, <https://github.com/cimgui/cimgui>, MIT.
- Dear ImGui: `c1aaac65e896a45f81f47a4d68f9959e0f86c859`, <https://github.com/ocornut/imgui>, MIT.
- RichMd: `609039721f838108251520079fff79011cf46659`, <https://github.com/pthom/imgui_rich_md>, MIT; includes separate third-party notices/assets.
- MD4C: `481fbfbdf72daab2912380d62bb5f2187d438408`, <https://github.com/mity/md4c>, MIT.
- LuaJIT, generation only: `a4c064539ea931274d407e1249b4bb575424a0f4`, <https://github.com/LuaJIT/LuaJIT>, MIT.

Retain upstream license/notices with redistributed DLL/assets; font and embedded third-party notices remain in pinned checkouts. No claim of completed distribution/license audit.

Required deterministic patches:

1. Guard ImGui `IM_NODEBUGSTEP` definition for generator compatibility, without layout changes.
2. Remove known duplicate generated C typedefs; name three C-view anonymous unions for C99. GUI startup verifies sizes/union offsets against C++ DLL.
3. Reconstruct four RichMd file patches from pinned originals: genuine/synthetic run provenance, strict source proof, selectable preview/context, edge scrolling and invalidation/ownership. Refuse unknown changes or missing expected patch.

Build verifies revisions/allowed tracked edits offline and regenerates wrappers. Stack manifest hashes maintained inputs, dependency/generated files, generator and DLL; GUI manifests bind application binaries to stack. Use one cimgui/ImGui implementation in DLL; never mix wrapper revisions.

`imgui_demo.cpp` remains because full generated cimgui exports reference its entry points. Separate spike host and historical comparison harnesses are not imported.
