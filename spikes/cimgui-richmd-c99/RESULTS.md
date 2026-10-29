# Spike result

## Verdict

**Proceed with this stack for further GUI exploration.** Windows x64 CJIT can run C99 application against Zig-built DLL combining freshly regenerated cimgui, Dear ImGui 1.93.0, RichMd 1.0.0, and native Win32/D3D11 backends.

This proves actual build, link, shared context, rendering, and C callbacks—not just C declarations. Follow-up now proves strict contiguous original-input selection export. Additive selectable-preview API now supports integrated contextual feedback. This does not complete Attendant GUI or general transformed-source mapping.

## Original checkpoint: 2026-10-28

Following six-run evidence belongs original build, retained separately from source-selection follow-up below.

Environment: Windows x64, Zig 0.17.0, CJIT 1.2.0, Node 24.16.0. Native application compilation uses strict C99 with `-pedantic-errors`; dependency/bridge compilation uses C++17. DLL uses `-O3`, no fast-math/assertion removal.

Checks:

- Six real window/render runs: Zig and CJIT × blank preview, Markdown preview, interaction exercise. Each completes 30 frames.
- Blank preview: 2740 vertices, zero bright preview pixels.
- Markdown preview: 4706 vertices, 7666 bright preview pixels.
- Zig/CJIT produce byte-identical blank and rendered-preview PPM images, 1264×901 pixels.
- Interaction: source edit reflected in live preview; one `attendant://tasks/example` click reaches C callback; preview drag and Ctrl+C produce 54 bytes through C clipboard callback. Test clipboard stays in process.
- Principal ImGui sizes and three patched union sizes/offsets match across C99/C++ boundary.
- Owned image count returns to zero during teardown. Success emitted after teardown.
- Invalid frame bound exits 2 through both hosts.
- Two guard tests pass: reject tracked dependency edits; reject stale build inputs and replace previous successful report with explicit failure.
- Direct PE imports contain Windows/D3D/UCRT libraries, no external C++, LuaJIT, Node, Python, GCC, or pthread runtime DLL.
- Visually inspected actual CJIT screenshot: source editor, formatted preview, table, code block, admonition, UTF-8 text, local image.
- Runner smoke also passes independently through `bash run-cjit.sh --frames 2`.

Machine-readable checkpoint: [evidence/windows-x64.json](evidence/windows-x64.json). Full current build-input manifest and screenshots remain under ignored `build/`.

## Necessary integration fixes

1. **Regenerate cimgui.** Local/shipped 1.92.9b wrappers cannot establish ABI match with ImGui 1.93. Pin generator and regenerate against chosen core.
2. **Suppress generator debug attributes correctly.** ImGui 1.93 unconditionally defines `IM_NODEBUGSTEP`, defeating generator's empty macro. Guard definition; otherwise generated declarations contain `artificial]]`.
3. **Make generated C view strict C99.** Remove seven duplicate typedefs and name three anonymous unions. Sizes/offsets unchanged; named field access differs from unmodified cimgui.
4. **Link available Windows libraries.** Zig uses `d3dcompiler_47`; CJIT requires explicit System32 library search path for application `user32` symbols.
5. **Own image teardown.** RichMd default uploader queues CPU texture deletion for later renderer sweep. Private D3D11 image uploader instead frees owned views with Markdown cache lifetime. Fonts remain managed by ImGui renderer.

No changes to installed vendor trees or tool installation. Integrated workspace lives under [Attendant GUI](../../exploration/attendant-native/gui/README.md); launcher unchanged.

## Source-range finding

Independent source inspection found useful internal mapping, not a public getter:

- `internal/rich_md_renderer.h`: `TextRun` retains fragment byte intervals, screen rectangles, and replacement text. Selection anchor/focus are fragment byte offsets. Fields/extraction methods private.
- `internal/rich_md_renderer.cpp`: `selected_text()` extracts visible text; `visible_text(..., sourceOffsets)` can produce per-output-byte provenance. Generated separators/replacements need distinct handling.
- `selected_markdown()` expands to source lines. It is not exact selected endpoint slice.
- `rich_md.cpp`: ordinary `Render()` unindents and resolves transclusions before `RenderRaw()`. Internal offsets therefore need not address original file.
- Code widgets, formulas, synthetic breaks, hidden content, and transclusions complicate exact anchors. Searching extracted text back into source is ambiguous.

## Original-source selection follow-up

`patch-richmd.mjs` maintains exact changes to four pinned RichMd files. `prepare.mjs --verify` accepts only reproducible patched bytes, not arbitrary edits in allowlisted paths. `crmd_render_raw()` uses named exact input and copies POD snapshot before destruction. Full document key/revision/input comparison clears old ownership; ImGui's 32-bit hash is not identity.

Require current fragment, nonempty UTF-8 boundaries, consecutive genuine original-input runs, no replacement and visible bytes exactly equal source slice. Synthetic/foreign content, gaps/overlap and widget-local interaction stay unclaimed. Empty input clears selection. Child-widget interaction clears stale surrounding selection and reports unsupported, including non-selectable code children; context-menu Copy still works. Separate ownership serial includes context-menu/keyboard actions and child interaction, without changing C selection POD layout. No source searching or clipboard inference.

Current proof: **62 fresh real window/D3D runs**, both strict C99 hosts. Six original workflows rerun plus **56 selection cases**: forward/reverse drag, duplicate words, Unicode/supplementary/decomposed bytes, link-label interior, menu Copy, document/revision/input/empty changes, actual source/key hash collisions, child-code ownership and rejection across markup, escapes, omitted entities, synthetic breaks, tables, code/widgets, hidden content and wrapped text. Three guard tests pass, including exact patch reconstruction and allowlisted-file drift rejection. Invalid-context output clearing tested; no injected allocation-failure certification.

Current build/checker/DLL identity and results: [evidence/source-selection-windows-x64.json](evidence/source-selection-windows-x64.json). Integrated GUI adds known body offset, freezes composer range without changing source selection, and validates against full saved Unicode-code-point range/hash; BOM/CRLF/supplementary Save & Send and native/`.mjs` resolution pass both compilers.

## Selectable preview follow-up

`crmd_render_preview()` adds borrowed rendered text/section JSON and optional enclosing source span without changing legacy C selection POD. Code stays in selectable flow; edge drag auto-scrolls. Full render identity and owned C99 snapshots preserve latest human selection through composer focus. Generated/unproven pieces retain context, never guessed source bounds. See [workspace contract and acceptance](../../exploration/attendant-native/gui/README.md#preview-selection-contract). Strict legacy tests above remain separate proof; integrated tests qualify new path.

## Limits

- Rendering can use hardware D3D11 or WARP fallback. Driver choice not recorded; no hardware-only performance claim.
- Fresh-process tests, not long-running/repeated-context leak certification, device-loss recovery, arbitrary DPI, accessibility, or full input-routing qualification.
- Pixel checks establish visible output, not complete Markdown conformance. Named-union ABI probes cover changed unions, not every exported cimgui function.
- Direct import inspection is not transitive dependency certification.
- Upstream missing-font assertions/OS faults/allocation aborts remain possible. C++ exception containment is not universal failure recovery.
- Standalone host edits memory only; it has no save, workspace tabs, collection feedback, file navigation or Git context. Integrated workspace has separate acceptance. Exact-range proof covers conservative raw spans, not general Markdown/transclusion mappings.
- No speed, startup, memory, deployment footprint, or release-readiness benchmark. Recorded DLL size is not total distribution size; fonts/assets and notices also required.
- Compatibility/selection patches need upkeep or upstream equivalents. Legacy raw API still rejects replacements, wraps, hidden syntax and independent widgets. Integrated selectable preview instead retains rendered context when source proof is unavailable. No general transclusion mapping.

## Comments

Read-only helper `richmd-api-review` inspected selection internals and reviewed bridge/tooling. Review identified stale-success/provenance and image-teardown/error-reporting gaps; spike addresses those paths and retains remaining limits above. Helper did not independently run build/tests. Source inspection does not certify broader GUI correctness.
