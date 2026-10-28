# Recent GUI dependency check

Checked 2026-10-28. User requires very recently released ImGui/Markdown capabilities, not blind reuse of on-disk vendor stack. No dependency update, download-to-repository or build performed yet.

## Findings

- [Dear ImGui latest-release API](https://api.github.com/repos/ocornut/imgui/releases/latest): `v1.93.0`, published `2026-10-14T16:37:04Z`. Core public header and changelog retrieved; no Markdown API found there. [Release](https://github.com/ocornut/imgui/releases/tag/v1.93.0).
- [Dear ImGui Bundle v1.93.0 changelog](https://github.com/pthom/imgui_bundle/blob/main/CHANGELOG.md): “RichMd: the markdown renderer becomes a standalone library, with documents, selectable text, Mermaid diagrams and narrative programming.” [Release overview](https://imgui-bundle.pages.dev/releases/1.93/). Distinguish Bundle's included libraries from stock Dear ImGui core.
- [RichMd README](https://github.com/pthom/imgui_rich_md): “It works with a stock Dear ImGui 1.92+ (C++17).” Public API uses C++ strings/callbacks; not directly callable from our C99 application. [Latest-release API](https://api.github.com/repos/pthom/imgui_rich_md/releases/latest): `v1.0.0`, published `2026-10-21T17:44:02Z`. Candidate pinned release: [v1.0.0](https://github.com/pthom/imgui_rich_md/releases/tag/v1.0.0).
- RichMd supports selectable/copyable text, Markdown links/wikilinks, document search and headings. Actual source-range export still unverified: inspected main public header has selectable-text option, but no `GetSelection`/`GetSelectedText` API found. Do not infer source mapping from clipboard support.
- Both `D:/Repos/c-ink/vendor/cimgui/imgui/imgui.h` and `D:/Repos/proto-framework/vendor/cimgui/imgui/imgui.h` report `IMGUI_VERSION "1.92.9b"`. Do not modify those projects while building Attendant.
- [Current cimgui README](https://github.com/cimgui/cimgui/blob/master/README.md): wrapper based on `1.92.9b`; wraps functions/structs/enums in `imgui.h` and optionally `imgui_internal.h`. It does not establish RichMd C bindings. Its generator supports selecting another ImGui revision, requiring LuaJIT and C++ preprocessing.
- Queried [cimgui branches API](https://api.github.com/repos/cimgui/cimgui/branches?per_page=100): `master` at `c4461a7eadbd17b6a6a8124a8abbf499aba6318d`; `docking_inter` at `2a6a93fccd10f275ee9206167dc9698f6f00fac1`. Retrieved branch READMEs target 1.92.9b. Master submodule points to ImGui commit `6d32174508fb8c518e7a832cc1d571fadd5cba76`. No verified matching 1.93.0 cimgui build found in this bounded check; not a claim none exists anywhere.
- cimgui GitHub `releases/latest` returns an old 2018 tag, not current generated-wrapper compatibility. Do not use that endpoint alone to choose wrapper version.

## Implementation gate

1. Inspect pinned ImGui 1.93.0 + RichMd 1.0.0 sources/licenses/dependencies. Check current fixes before pinning final revisions.
2. Establish cimgui generation against chosen ImGui revision; never link 1.92-generated structs blindly against 1.93 internals.
3. Use or add narrow `extern "C"` adapter for needed RichMd operations if no suitable wrapper exists. C++ limited to dependency bridge; application/core remains C99. Avoid importing whole Bundle/Python/browser runtime merely for Markdown.
4. Disable unnecessary optional features/downloads for initial slim build. RichMd README lists optional LaTeX, URL-image downloads and code-editor integration; no automatic package fetch/install. Verify raylib texture/font/backend integration separately.
5. Prove same dependency artifact can be linked by CJIT and Zig C99 hosts, with explicit ABI/runtime ownership. Document compiler/dependency requirements rather than quietly making CJIT path depend on an undeclared Zig installation.
6. Demonstrate Markdown render, link callback and selection extraction. Determine how original source positions are obtained before committing to preview feedback implementation. Source editor remains independently testable.
7. Record GUI runtime/visual results, not just compile success. Existing CLI stays independent.

Feedback collection/reminder work can proceed without this GUI stack; step 0 of [GUI-PLAN.md](GUI-PLAN.md) remains incomplete until compile/runtime/source-range checks pass.
