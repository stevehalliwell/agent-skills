# Markdown workspace GUI

C99 application using shared source API and pinned ImGui/RichMd native library. Windows uses Win32/D3D11; macOS uses GLFW/OpenGL3. Build/run: [build reference](../../../references/build.md). Readiness and resource limits: [native README](../../README.md).

## Human workflow

Collection/recursive-file navigation, reusable tabs, source editing/undo, Markdown body preview and typed/local links. Unresolved references remain visible. Save/reload/close guard dirty work; external changes invalidate saved selections and block stale replacement. Far-right tab-bar **X** closes all tabs. Dirty files keep existing Save/Discard/Cancel prompts; Cancel stops remaining closes, without reopening tabs already closed.

One **Comment on selection** action freezes latest source or preview selection independently of editor focus. Composer separates selected excerpt from human comment. Dirty Send requires explicit **Save & Send**. Save failure sends nothing; queue failure reports **saved-but-not-queued**, without rolling back successful save.

Preview selection crosses formatting, paragraphs, code and table cells. Capture rendered sections plus local Unicode highlights. Genuine ordered source runs may prove enclosing source span; otherwise retain context only. Never invent coordinates, search matching text or fall back to older source selection. Workspace owns snapshot; composer freezes independent copy.

Feedback binds saved target SHA-256 and zero-based/end-exclusive Unicode-code-point range, including BOM/front matter. Context-only feedback has zero bounds and empty selected source. Human body stays verbatim before separate rendered-context section. Reading never resolves feedback; explicit update changes status. Pending/resolved inbox follows saved anchors only after hash/text checks.

Undo remains ImGui-owned, isolated by literal full file-path bytes plus slot/generation. No custom persistent per-file history; same-path reopen lifetime is not promised. Workspace issue count warns of omitted/invalid records/collections and directs to read-only CLI `doctor`; warning never repairs source or prepares projection.

## Navigation and find

- **Wrap** defaults on for raw editors. Window-wide toggle; restart resets it. Soft reflow changes no source bytes, dirty state, selection or undo. Agent logical-line hints stay logical lines; **Go** and markers use wrapped geometry.
- Temporary discovery scope on open and **Refresh files**: non-dot Markdown files directly in project root plus configured Attendant collection files. Do not enter other root folders, generated/dependency folders or deeper directories. Configured collections still load their declared files. Intentional scope exclusion is not incomplete discovery. Skip links/junctions/reparse points and special entries. Collection records remain in their groups, never duplicated in file tree. No Git lookup or filesystem watcher.
- Branching folders start collapsed. Compact straight single-child chains into one path label, including Markdown leaf when no branch remains; do not require opening intermediate folders. Filtering does not manufacture single-child chains from hidden siblings. **Refresh files** preserves current-window expansion. Folders precede files; sort ASCII case-insensitively with UTF-8 byte tie-break. Shared capacity remains 4096 documents. Root discovery uses shared `base.h` search with native `*.md` enumeration on Windows and `root_only`; no recursive directory pass. Reusable recursive search remains available but disabled for automatic workspace discovery. Incomplete discovery retains partial results and persistent warning with bounded path/limit details. Hard refresh failure retains previous navigation and dirty tabs.
- Sidebar filter stays hidden until Ctrl+F; Clear and Escape clear/blurs/hides it. Query field has no trailing label. Ctrl+F focuses last sidebar/source/preview context; fallback is active source or sidebar without document. Sidebar filters names/project-relative paths, not contents. Matching ancestors open temporarily; clearing restores expansion. Feedback-status filters remain active.
- Document find searches unsaved raw text or visible preview text. Preview crosses inline formatting, not blocks; excludes hidden syntax, link targets and image metadata. Literal, nonoverlapping matches; default ASCII case-insensitive, other Unicode exact. **Match case** applies across contexts. No regex, Unicode folding or project-wide content search.
- Independent query per context. Enter/Shift+Enter or F3/Shift+F3 cycle document hits with wraparound. Escape closes document find and retains selection; sidebar Escape clears/blurs filter. Empty query has no hits/jump; zero matches shows `0/0`. Queries allow 1023 UTF-8 bytes; invalid/oversized input and preview capacity failures are visible, never searched as truncated terms. No find state persists after restart.

Drag vertical dividers to resize sidebar and source/preview widths. Initial widths stay unchanged; resize state lasts only for current window. Minimum widths shrink only when whole window cannot fit them.

## Agent controls

```sh
# From native root. Existing responsive GUI required; neither command opens one.
# macOS: omit .exe from both commands.
build/gui/attendant-gui-zig.exe --project /path/to/workspace --reveal records/tasks/first.md --line 12
build/gui/attendant-gui-zig.exe --project /path/to/workspace --inspect
# Explicitly show GUI search results; not CLI record/content search:
build/gui/attendant-gui-zig.exe --project /path/to/workspace --find "the plan" --view preview --file README.md
build/gui/attendant-gui-zig.exe --project /path/to/workspace --find "tasks" --view sidebar
```

Reveal delivers separate agent hint. Receipt changes no tab, caret/selection, scroll or focus. Only human **Go** navigates/highlights. Dirty/stale targets receive no anchor. Paths must be local and have no application-link ancestry. Windows rejects UNC, remote/extended namespace and reparse paths. macOS permits standard `/tmp`, `/var` and `/etc` OS aliases, rejects other symlinks and nonlocal volumes. Checks are not an atomic filesystem sandbox.

`--find QUERY --view sidebar|source|preview [--file PATH] [--match-case]` queues an immediate GUI find on next frame. Explicit file may open a permitted Markdown tab; otherwise document find uses active tab, including unsaved text. Sidebar cannot combine with `--file`. Same literal/query limits as human find. Request updates find field/case option and selects/scrolls first document hit, without OS focus stealing, source writes or automatic viewer startup. Reject modal dialogs, rendering reentry and already-queued search. Zero matches succeeds; empty query has no hits/jump. JSON receipt is `{"ok":true,"delivered":true,"persistent":false,"os_focus_changed":false}`: delivery is not a match count or proof of completed rendering. Later file/open failure remains visible in GUI. Discovering no/ambiguous viewer fails; old viewers may reject unsupported search. Use matching new build. Existing reveal/inspection packet layouts stay unchanged; find uses separate bounded message.

Inspection reports current document/dirty state and latest live selection, including preview context. It does not read/save disk, consume feedback, change selection/focus/scroll or auto-start GUI. Context selection exposes null coordinates, never invented source bounds. Rendered-context property is a JSON string. Revision is a string.

Controls emit one JSON object to stdout; errors also explain on stderr. Exit 0 means successful delivery/inspection, 2 invalid invocation, 1 source/recipient/transport failure. Missing/ambiguous GUI is an error. Windows discovery stays on current desktop. macOS uses private same-UID Unix sockets across login sessions, not desktop isolation. Same-UID processes can inspect selections or impersonate endpoints; transport is not application authentication. Inspection v2 carries bounded selected text/context, up to 4 MiB each; no hard real-time deadline guarantee.

## Rendering boundary and checks

GUI requires native library beside executable and explicit `--assets` path. Both macOS compiler outputs launch directly without loader-environment setup. C++ exceptions stay behind C ABI. Borrowed bridge strings expire on next render/shutdown; copy before retaining them. Single UI thread/context. Terminal render failure abandons frame and exits; unsaved draft preservation remains unresolved and explicitly deferred.

`bash tools/check-gui.sh` uses native coordinator and retains model checks plus representative real rendering/ABI, wrapped reveal geometry, context-aware find/filter/clipboard refusal, references, diagnostics, editing/undo, dirty-close/save/queue, multi-section/context-only selection, inspection and resolution cases. No JS oracle or historical screenshot inventory. Model covers reveal validation and malformed inspection packets; reduced suite does not repeat former full reveal/selection permutation matrix. `bash tools/check-control-macos.sh` adds real cross-compiler reveal/inspection, dirty/stale Go guards, new-tab navigation, ambiguous/unrelated viewers, malformed/fragmented transport, deadlines and endpoint-policy checks. Both GUI suites explicitly remove `DYLD_LIBRARY_PATH` from macOS child environments. `bash tools/check-bridge.sh` preserves independent bridge lifetime/ABI/P6 checks. C macOS coordinators pass native compilation/runtime validation; [project checks](../../../../../TODO.md#checks) retain current evidence. Current results/screenshots stay under ignored `build/`.
