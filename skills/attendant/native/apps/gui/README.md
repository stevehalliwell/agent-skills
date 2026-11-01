# Markdown workspace GUI

C99 Win32 application using shared source API and pinned ImGui/RichMd DLL. Build/run: [build reference](../../../references/build.md). Readiness and resource limits: [native README](../../README.md).

## Human workflow

Collection/root-file navigation, reusable tabs, source editing/undo, Markdown body preview and typed/local links. Unresolved references remain visible. Save/reload/close guard dirty work; external changes invalidate saved selections and block stale replacement.

One **Comment on selection** action freezes latest source or preview selection independently of editor focus. Composer separates selected excerpt from human comment. Dirty Send requires explicit **Save & Send**. Save failure sends nothing; queue failure reports **saved-but-not-queued**, without rolling back successful save.

Preview selection crosses formatting, paragraphs, code and table cells. Capture rendered sections plus local Unicode highlights. Genuine ordered source runs may prove enclosing source span; otherwise retain context only. Never invent coordinates, search matching text or fall back to older source selection. Workspace owns snapshot; composer freezes independent copy.

Feedback binds saved target SHA-256 and zero-based/end-exclusive Unicode-code-point range, including BOM/front matter. Context-only feedback has zero bounds and empty selected source. Human body stays verbatim before separate rendered-context section. Reading never resolves feedback; explicit update changes status. Pending/resolved inbox follows saved anchors only after hash/text checks.

Undo remains ImGui-owned, isolated by literal full file-path bytes plus slot/generation. No custom persistent per-file history; same-path reopen lifetime is not promised. Workspace issue count warns of omitted/invalid records/collections and directs to read-only CLI `doctor`; warning never repairs source or prepares projection.

## Agent controls

```sh
# From native root. Existing responsive GUI required; neither command opens one.
build/gui/attendant-gui-zig.exe --project /path/to/workspace --reveal records/tasks/first.md --line 12
build/gui/attendant-gui-zig.exe --project /path/to/workspace --inspect
```

Reveal delivers separate agent hint. Receipt changes no tab, caret/selection, scroll or focus. Only human **Go** navigates/highlights. Dirty/stale targets receive no anchor. Ordinary local fixed/removable/RAM paths only; reject UNC, remote/extended namespace and reparse ancestry. Checks are not an atomic filesystem sandbox.

Inspection reports current document/dirty state and latest live selection, including preview context. It does not read/save disk, consume feedback, change selection/focus/scroll or auto-start GUI. Context selection exposes null coordinates, never invented source bounds. Rendered-context property is a JSON string. Revision is a string.

Controls emit one JSON object to stdout; errors also explain on stderr. Exit 0 means successful delivery/inspection, 2 invalid invocation, 1 source/recipient/transport failure. Missing/ambiguous GUI is an error. Same-user desktop IPC is not authentication. Inspection v2 carries bounded selected text/context, up to 4 MiB each; no hard real-time deadline guarantee.

## Rendering boundary and checks

GUI requires DLL beside executable and explicit `--assets` path. C++ exceptions stay behind C ABI. Borrowed bridge strings expire on next render/shutdown; copy before retaining them. Single UI thread/context. Terminal render failure abandons frame and exits; unsaved draft preservation remains unresolved and explicitly deferred.

`tests/gui/check.mjs` retains native model checks plus representative real rendering/ABI, references, diagnostics, editing/undo, dirty-close/save/queue, multi-section/context-only selection, inspection and resolution cases. No JS oracle or historical screenshot inventory. Model covers reveal validation and malformed inspection packets; reduced suite does not repeat former full reveal/selection permutation matrix. Current results/screenshots stay under ignored `build/`.
