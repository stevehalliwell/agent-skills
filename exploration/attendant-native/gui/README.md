# Shared Markdown workspace

Windows C99 host. Zig-built DLL owns cimgui/ImGui/RichMd/Win32/D3D11. Native CLI remains GUI/C++-independent.

Current UI: collection/root-file navigation, reusable document tabs, editable source/undo, body preview, typed/local links, visible unresolved targets, save/reload/close safeguards and one **Comment on selection** action above source/preview. Pending/resolved inbox views open comments and exact saved target ranges. Dirty Send requires explicit Save & Send; failed queue reports saved-but-not-queued. Reading never resolves feedback. Preview Send accepts proven contiguous original-source spans only; unsupported selection stays disabled. External agent hints use separate source-line highlight, never human caret/selection. Git context deferred.

## Build and run

Requires existing CJIT or Zig and explicitly prepared/built [GUI stack](../../../spikes/cimgui-richmd-c99/README.md). No automatic installs/downloads. Build and acceptance use Node; GUI runtime does not.

From repo root:

```bash
bash exploration/attendant-native/build-gui.sh cjit
bash exploration/attendant-native/build-gui.sh zig
exploration/attendant-native/build/gui/attendant-gui-zig.exe \
  --project "$PWD/exploration/attendant-native/fixtures/project" \
  --assets "$PWD/spikes/cimgui-richmd-c99/build/deps/richmd/assets" \
  --file records/tasks/first.md
```

Keep copied `cimgui_richmd.dll` beside executable. Runtime assets remain external. Launcher/installed skill unchanged.

`--help` lists arguments. `--frames 1..6000 --shot FILE.ppm` runs bounded real GUI and records D3D buffer readback; screenshot needs frame bound. Missing project/assets and invalid arguments fail without writes.

## Agent location hint

With workspace already running:

```bash
exploration/attendant-native/build/gui/attendant-gui-zig.exe \
  --project "$workspace" --reveal records/tasks/first.md --line 12
# stdout: {"ok":true,"hint_updated":true,"persistent":false,"focus_changed":false}
```

- Latest transient hint wins. Fixed-height bar shows target; no automatic tab open, activation, scroll, source API request from message callback, foreground focus or project mailbox. Display update waits for pointer/navigation release and an already-queued human Go.
- **Go to agent hint** explicitly opens/reuses file and scrolls source to amber line outline/gutter mark. Does not move source caret/selection, including original tab when opening another file. Preview ownership follows normal displayed-document changes. Agent marker is not a source/preview selection and grants no edit/Send permission. Dismiss is local, not feedback resolution.
- `--line` defaults to 1; counts full-source LF/CRLF lines including front matter/BOM, not rendered body rows. Bare-CR mapping unsupported. Sender reads exact saved UTF-8/hash without preparation. Dirty, changed-hash or unavailable targets retain readable hint but receive no guessed highlight/scroll. Go never reloads dirty work.
- Relative file resolves from project. Allow project descendants or registered/open external documents. Agent reveal requires accessible ordinary fixed/removable/RAM-drive paths without reparse ancestry. Sender preflights before source read; UI checks outside callback and again before Go. Junction/symlink, UNC, mapped-remote and extended-namespace reveal paths unsupported; ordinary editor/feedback behavior unchanged. Preflight is not an atomic filesystem sandbox against concurrent reparse swaps.
- Windows local-desktop `WM_COPYDATA`, versioned fixed-width payload and owned copies. Discovery checks all visits: 4096 total windows, 16 matching-class candidates, 2000-ms budget; individual delivery capped at 1000 ms. Filesystem I/O has no hard latency guarantee. Signal is not caller authentication/edit authority; Windows message permissions apply. Use same desktop/integrity level.
- Sender creates no GUI/renderer; assets not needed. Automation should inherit console or set `windowsHide: true` to avoid sender-console activation. No auto-start. Missing/ambiguous/unavailable GUI returns error; ambiguous workspace instances are not arbitrarily chosen.
- Control stdout always one JSON object; failures also explain on stderr. Exit 0: hint delivered, not persistent/necessarily displayed yet. Exit 2: invalid invocation. Exit 1: source/recipient/transport failure. `reveal-delivery-unknown` means hint may have arrived; retry is safe. `--help` prints text. Control cannot combine startup/test flags. Native CLI unchanged.

## Inspect current document selection

```bash
exploration/attendant-native/build/gui/attendant-gui-zig.exe --project "$workspace" --inspect
```

```json
{"ok":true,"document":{"path":"D:\\workspace\\records\\tasks\\first.md","dirty":true,"buffer_revision":"7"},"selection":{"view":"source","status":"exact","text":"café Ω","range_start":20,"range_end":26,"range_unit":"unicode-codepoint"}}
```

- Inspect active document and latest human document selection, not OS keyboard focus or comment-box text. Selection remains queryable after switching focus to chat. Document switch/reload clears it; source edits invalidate old revisions. Agent marker is never a human selection.
- Source and exact raw-preview spans share ownership state. `view`: `none`, `source`, `preview`. `status`: `none`, `exact`, `unsupported`. Empty/unmapped selection returns null text/range with exit 0, never older source fallback. Child/custom-widget interaction reports `unsupported`, including code children whose text is not selectable. Renderer ownership serial includes context-menu/keyboard actions; host does not infer ownership from mouse position. No open document returns `document:null`.
- Exact range counts full live-buffer Unicode code points, including BOM/front matter and CR/LF separately. Unsaved text is allowed; `dirty:true` and decimal-string `buffer_revision` distinguish live state. This is not a saved-file feedback anchor or permission to edit.
- No preparation, disk read/write, feedback consumption, tab/focus/selection/scroll change or auto-start. Requires one responsive inspection-capable GUI; legacy GUI returns `inspection-unavailable`. Same stdout/exit conventions and matching-window discovery as reveal. Startup/reveal flags cannot combine with `--inspect`.
- Local desktop request/reply uses invisible message-only client window and bounded synchronous messages. Receiver snapshots stable C-owned buffers, never source API/ImGui from callback; busy rendering retries up to 16 attempts with 2000-ms budget checked before each request, each request at most 1000 ms and nested reply 250 ms. No hard real-time guarantee. No endpoint files, second transport or clipboard inference.
- At most 4 MiB selected UTF-8, plus fixed path/header metadata. Only selected bytes copied; full-buffer prefix traversed on request for code-point range. JSON capacity covers worst-case escaping. Inspection response allocated lazily; no per-frame source copy. Same desktop/integrity and non-authenticated same-user signal limit as reveal.

## Acceptance

```bash
bash exploration/attendant-native/build-gui.sh cjit --check
bash exploration/attendant-native/build-gui.sh zig --check
node --test exploration/attendant-native/gui/tools.test.mjs
node exploration/attendant-native/gui/check.mjs
```

- `check.mjs`: unique generated copies only; 32 real window/D3D runs across both compilers. Preview pixels, Unicode/space paths, real reference/tab input, invalid-record recovery, source edit/undo/redo, dirty-window-close Cancel, comment input, Save & Send, pending-comment navigation and exact source selection. Normal browsing does not prepare DB or change source.
- Actual GUI Send → native/`.mjs` query → explicit resolution passes both compilers; complete source UTF-8/supplementary bytes, CRLF, 71-code-point selection and human body preserved. Reading does not consume. Separate actual GUI failure case saves source, preserves conflicting files/config, shows saved-but-not-queued and cancels composer.
- Preview GUI edit → exact body selection → Save & Send → native/`.mjs` query → explicit resolution passes. BOM/CRLF/supplementary bytes remain exact; full-file code-point range is `[35,62)`. Source editor selection stays collapsed and unchanged. No projection or unrelated source created by GUI.
- Test-only input driver absent from production build. Mutable exercises/model require generated ownership marker. Both compilers run 168 always-on model checks, including stale/dirty target anchors, alternate absolute-directory spelling, per-file failures, injected read/metadata gap and surviving-write recovery evidence after refresh.
- Cross-compiler external-process inspection checks exact full selected UTF-8/code-point range for clean/dirty buffers, repeats without consumption, preserves actual caret/selection/geometry/scroll/focus and rejects absent/ambiguous GUI and invalid control combinations. Actual preview keyboard-menu Select All replaces older source selection for exact/plain and unsupported/markup spans; child-code interaction also reports unsupported. Unified button opens only exact latest selection; unsupported context cannot use older source range. Inspection remains unchanged when comment input takes focus. Model covers empty document/selection, preview ownership, UTF-8 boundaries, revision/document invalidation, JSON capacity, busy/malformed requests and malformed/unaligned response packets.
- Cross-compiler external-process reveal checks preserve actual ImGui caret/selection/active ID, document, editor geometry, scroll and Win32 focus on receipt. Explicit Go preserves caret/selection in same/new tab. Dirty/stale source gets no marker/scroll. Amber pixel checks require source-only clipping. Reject absent/ambiguous GUI, malformed arguments/payload, out-of-scope paths and junctions; recheck when ordinary directory becomes outside junction after delivery. Production omits test driver/hash override.
- Build manifests bind C/build inputs, generated stack headers/dependency manifest, actual DLL and native executables. Guards reject stale source/substituted DLL before GUI execution. Acceptance replaces stale success with partial failure report.
- Local evidence: `build/gui-check-results.json`, owned runs under `build/gui-check/`, source/toolchain manifests under `build/gui/`. Retained checkpoint: [evidence/windows-x64.json](evidence/windows-x64.json).
- Real window/D3D validation requires interactive Windows desktop. Fixed 1440×1000 requested window; hardware/WARP choice not recorded. No long-running/device-loss/minimize, wider glyph/path, performance or release qualification yet. Test emoji renders fallback glyph with current fonts/ImGui configuration; exact underlying supplementary bytes and offsets are preserved, not full glyph coverage.

## Model and ownership

`workspace.c` owns tab buffers and navigation snapshots; core API results remain borrowed. Refresh never changes active selection or dirty source. Explicit navigation changes active tab; duplicate canonical paths reuse existing buffer. Clean external changes reload; dirty changes stay in memory and require explicit save/discard/cancel. Save verifies exact previously read hash even when metadata polling misses a change.

Bounds: 16 tabs, 4 MiB UTF-8 per document, 4096 navigation files, 16 MiB navigation arena. Project/source API uses existing native capacities. Preview hides recognized leading front matter; full source remains unchanged. Source selection tracks UTF-8 byte endpoints; model converts to saved full-file Unicode code-point offsets for core verification. Source API returns metadata associated with read/staged bytes; model never samples newer metadata after reading an older buffer. One bad collection record stays listed as invalid rather than blocking healthy files; CLI retains strict policy. Recovery path copies survive later requests/refresh.

## Preview selection contract

- `RenderRaw()` receives exact body bytes. No unindent or transclusion expansion. Body framing recognizes optional UTF-8 BOM and CR/LF/CRLF front matter; full source remains authoritative.
- Full document key, workspace-wide text revision and exact input bytes invalidate prior selection. Tab reuse, reload, edits, identical documents and actual ImGui hash collisions cannot reuse old ownership.
- Snapshot runs before named render input dies. Require current fragment, UTF-8 boundaries, consecutive genuine source runs, no replacement, and visible bytes equal exact source interval. Copy POD endpoints; never reverse-search clipboard text.
- Shared **Comment on selection** action enables only for latest `EXACT` selection. Unsupported latest preview context disables it instead of using older source selection. Add known body byte offset and freeze composer range without changing source caret. Existing model converts full-file bytes to saved Unicode code points/hash; dirty source still requires Save & Send.
- Plain/direct text and link-label interiors can pass. Crossing omitted markup, escaped syntax, generated breaks/wrap spaces, hidden content or code widgets fails closed. Rendered replacements/foreign-buffer content cannot pass. Use source editor for unsupported spans. LaTeX/transclusions are not enabled, so literal syntax is not transformed output.

Stack proof: 56 actual source-selection cases across both C99 compilers, including forward/reverse drag, duplicate words, Unicode/decomposed text, link labels, menu Copy, child-widget ownership and exact source/key hash collisions. [Stack results](../../../spikes/cimgui-richmd-c99/RESULTS.md) retain pinned patch and separate compatibility evidence.

## Comments

Test conditions never rely on C `assert`: current Zig `-O3` implicitly defines `NDEBUG`. No explicit assertion-removal or fast-math build option is added. Historical spike/performance evidence remains tied to its recorded binaries; no new performance claim here.
