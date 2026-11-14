---
name: attendant
description: "Set up Attendant, create an empty collection/table, create or update records, query or search local records, validate records, diagnose projection health, read human feedback, or launch/control the native Markdown GUI through reveal, find and selection inspection. Use for `.pi/attendant.tables`, collection schemas/usage, Attendant-managed Markdown or explicit Attendant workspace requests. Route setup and empty collections through guarded workflows. Skip ordinary Markdown editing without a GUI request and non-Attendant data; migration/import is not supported."
---

# Attendant

Use native CLI for metadata and queries; native GUI for human Markdown editing and feedback. Markdown is source of truth; `.attendant/` is disposable SQLite projection state. No Node.js runtime required.

## Trigger clarification

Use for Attendant operations, managed records or explicit native workspace interaction. Skip ordinary Markdown editing without a GUI request and non-Attendant data. On explicit invocation, inspect target project's `.pi/attendant.tables` and requested operation; ask one focused question only if operation remains unknown.

- `Find open tasks` with configured Attendant: inspect schema/usage, then query declared fields.
- `Read human comments`: inspect feedback schema/usage, then query pending comments.
- `Show this record in Attendant`: use GUI reveal; human chooses **Go**.
- `Edit README`: skip Attendant unless native workspace interaction was requested.
- `Migrate existing Markdown`: report unsupported operation; preserve files and config. Do not invent a migration command.

## Workflow

1. Read instructions for requested operation:
   - Setup or empty collection: [Setup and collection workflows](references/collections.md).
   - Create/update, validate/sync, schema/query/search, diagnosis or human feedback: [Commands and feedback](references/commands.md).
   - Start/use GUI, reveal, find or inspect live selection: [GUI workspace](#gui-workspace) below.
   - Missing build, explicit build or GUI dependency setup: [Build reference](references/build.md).
   - Source types, write failures, projection or resource limits: [Data contract](references/data-contract.md).
   - Renderer/transport implementation or GUI check work: [GUI internals](native/apps/gui/README.md).
   - Done: preconditions, approval gates and operation's completion condition are known.
2. Resolve `<skill-dir>` as directory containing this file. For CLI operations, resolve `<attendant>` to quoted absolute path of `native/build/attendant-zig.exe` on Windows or `native/build/attendant-zig` on macOS. Use built CJIT counterpart when Zig output is absent. Build one installed compiler's target if neither exists, following build reference; return to original target project afterward. Do not install tools, acquire GUI dependencies implicitly or reuse spike binaries. Use only native executables built from this skill.

   ```sh
   # Run from target project; -p also accepts explicit project root.
   <attendant> schema -p /path/to/project
   ```

   CLI supports Windows x64 and macOS arm64. GUI is separate and not required for CLI work; GUI-only requests use resolution below, without building CLI. Runtime, build and check tools need no Node.js; build prerequisites live in build reference.
   - Done: executable and target project are unambiguous; missing build is resolved or exact blocker reported.
3. Follow selected workflow, including inspection and user-confirmation gates. Commands emit one JSON value on stdout; failures also explain on stderr. Check exit status and diagnostics, not stdout presence alone. Before retrying mutation, inspect actual source and reported `written`/`failed` paths; completed writes may remain. Correct only supported in-scope defects and rerun failed action. Report unresolved blockers, source paths and remaining diagnostics; include raw JSON only when requested.
   - Done: source changes are within approved scope and result is understood.

## Collection contract

Each collection keeps three separate source files:

- `.schema.md`: flat YAML front matter declaring fields, types, defaults and allowed values.
- `.usage.md`: purpose, field meanings, lifecycle, operating rules and approval requirements. Not record-body copy or additional schema.
- `.template.md`: optional body copy for new records.

Run `schema` to discover source paths, declared fields and full `.usage.md` text. Read usage before creating, updating, selecting or resuming records. Empty file supplies no guidance; missing file reports `usage: null`. Do not infer rules from collection name or another project's defaults. Resolve material ambiguity with user. Schema declarations remain authoritative; report conflicting guidance rather than choosing silently.

Author usage from confirmed purpose and rules. Do not invent policy or copy external skill instructions without request. Usage is not projected as record or enforced by schema validation.

## GUI workspace

Use GUI for human Markdown editing, feedback and explicit live selection/navigation requests. GUI reads do not correct metadata or prepare SQLite. Inspecting a selection or receiving feedback grants no editing authority.

### Resolve and launch

- Resolve `<gui>` to quoted absolute `<skill-dir>/native/build/gui/attendant-gui-zig.exe` on Windows, without `.exe` on macOS. Use built CJIT counterpart if Zig output is absent. For a named build variant, use matching `build/gui-<variant>/` executable. Do not substitute spike or stale incompatible builds.
- Resolve `<assets>` to quoted absolute `<skill-dir>/native/build/deps/richmd/assets`. Native library must remain beside executable; assets are required for startup, not controls. Both macOS compiler outputs run without `DYLD_LIBRARY_PATH`.
- If missing, follow [build reference](references/build.md#gui-dependencies-and-application). Dependency acquisition is explicit, not authorized by an ordinary query/control request. Never install tools or download GUI dependencies implicitly. Use fresh `ATTENDANT_GUI_VARIANT` instead of overwriting mapped binaries/DLLs or closing a user viewer.
- Start a viewer only when requested. Prefer existing responsive viewer for same project; reveal/find/inspect never start one. Launch as a persistent desktop process, wait for successful inspection before sending controls, and leave human viewer running. Report missing/ambiguous/unresponsive recipients; do not kill them or open a duplicate to bypass discovery failure.

```sh
# Placeholders denote quoted absolute paths. --file is optional.
<gui> --project /path/to/project --assets <assets> --file README.md
<gui> --project /path/to/project --inspect
```

Startup accepts `--frames 1..6000 --shot FILE.ppm` for bounded captures; `--shot` requires `--frames`. Use owned workspace copies for screenshots/tests, not an existing human viewer. Controls cannot combine startup/capture flags.

### Human editing and navigation

- Sidebar groups configured collection records and root Markdown. Automatic open/Refresh files discovers only non-dot root `.md` files plus configured collection files, not other root folders or descendants. Skip links/junctions/reparse points and special entries. Explicit **Open Markdown** remains available; do not promise recursive discovery.
- Navigation deduplicates managed records, compacts genuine single-child paths and preserves current-window expansion on refresh. Folders precede files; ASCII case-insensitive sort uses UTF-8 byte tie-break. Capacity is 4096 documents. Incomplete discovery keeps partial results and warning; hard refresh failure keeps previous navigation and dirty tabs. No Git filtering or watcher.
- Tabs show raw source and rendered body. Local links and typed record references navigate; unresolved references stay visible. **Save** or Ctrl+S writes source. Reload/close guard dirty work; external changes invalidate saved selections and block stale replacement. Far-right tab-bar **X** closes all; Save/Discard/Cancel applies per dirty tab. Cancel stops remaining closes without reopening earlier tabs.
- **Wrap** defaults on across raw editors in window. Toggle/reflow does not change bytes, dirty state, selection or undo. Drag vertical dividers to resize sidebar and source/preview; widths and wrap/find settings do not persist after restart.
- Source undo is isolated between files and find fields. No persistent undo promise across close/reopen. Issue count warns of omitted/invalid data; use read-only CLI `doctor`, not automatic source repair.
- Known renderer failure exits without preserving unsaved tabs/comment drafts. Report this risk; never treat unsaved selection as saved source.

### Find

- Ctrl+F targets last-focused sidebar/source/preview pane; fallback is active source or sidebar without document. Dialog/composer text entry retains its own shortcuts.
- Sidebar filters names and project-relative paths, not contents; feedback-status filters remain active. Matching ancestors expand temporarily. Clear/Escape clears, blurs and hides sidebar filter, restoring expansion.
- Source find includes syntax and unsaved edits. Preview find searches visible rendered text across inline formatting, not separate blocks; excludes hidden syntax, URLs and image metadata. Soft wraps do not split matches.
- Queries are literal and nonoverlapping: ASCII case-insensitive by default, other Unicode exact. **Match case** is shared across contexts; query text is independent. No regex, whole-word mode, Unicode folding or project-wide content search.
- Changed document query selects first hit. Enter/F3 moves next; Shift+Enter/Shift+F3 moves previous, wrapping at ends. Escape closes document find and retains selection. Empty query has no hits/jump; zero hits shows `0/0`.
- Query limit is 1023 UTF-8 bytes plus terminator. Invalid UTF-8, multiline/oversized queries and preview-capacity failures are refused, not silently truncated. Find changes no source bytes, dirty state or undo.

### Selection and feedback

- Human selects source or preview text, then **Comment on selection** freezes independent excerpt/context snapshot. Preview selection can cross formatting, paragraphs, code and table cells. Proven source runs may map an enclosing range; otherwise selection stays context-only. Never invent coordinates or reuse older source selection.
- Composer separates excerpt from human comment. Dirty source requires explicit **Save & Send**. Save failure queues nothing. Queue failure after successful save reports **saved-but-not-queued**; saved source is not rolled back. First Send can create/register managed `_meta_comments` collection; merely reading feedback must not bootstrap it.
- Saved feedback binds target SHA-256 and zero-based/end-exclusive Unicode-code-point range, including BOM/front matter. Context-only stored feedback uses zero bounds and empty selected source, with rendered context separately. Human body remains verbatim. Live inspection instead uses null bounds for context-only selection.
- Pending/resolved feedback follows saved anchors only after hash/text checks. Reading, inspecting or opening feedback never resolves it. Follow [commands and feedback](references/commands.md#human-feedback) for schema/usage, hash checks, authorized work and explicit resolution.

### IPC commands

Use GUI executable as client; do not simulate clicks or construct binary packets for ordinary operations. Always pass explicit project root. Controls require one matching responsive viewer; no automatic startup, disk writes or OS focus stealing.

```sh
# Passive hint; human chooses Go. Optional line is positive and 1-based.
<gui> --project /path/to/project --reveal records/tasks/first.md --line 12
# Current document and live source/preview selection, including unsaved buffer:
<gui> --project /path/to/project --inspect
# Explicit immediate search; optional --file applies only to source/preview:
<gui> --project /path/to/project --find "the plan" --view preview --file README.md
<gui> --project /path/to/project --find "TODO" --view source --match-case
<gui> --project /path/to/project --find "tasks" --view sidebar
```

**Reveal:** receipt updates a separate, nonpersistent agent hint; does not switch tab, move caret/selection, scroll or focus. Only human **Go** navigates/highlights. Latest hint wins. Dirty/stale target receives no anchor; hash/line checks protect saved location. Receipt is not proof human followed it:

```json
{"ok":true,"hint_updated":true,"persistent":false,"focus_changed":false}
```

**Find:** `--view sidebar|source|preview` is required. Optional `--file PATH` opens a permitted document or targets active unsaved tab when omitted; sidebar forbids `--file`. `--match-case` sets shared case option. Queue applies next frame, opens find field and selects/scrolls first document hit. Reject modal/composer/close state, rendering reentry or already-queued search. Empty/zero-match queries succeed without jumping. Receipt confirms queued delivery, not match count or completed rendering; later open failure stays visible in GUI:

```json
{"ok":true,"delivered":true,"persistent":false,"os_focus_changed":false}
```

**Inspect:** read current document/dirty flag and latest live selection without changing focus, selection, scroll or feedback status. Does not read/save disk. `document` is null if no tab; otherwise contains `path`, `dirty` and string `buffer_revision`. Selection contains:

- `view`: `none`, `source` or `preview`.
- `status`: `none`, `exact`, `mapped`, `context` or `unsupported`.
- `text`: selected text for exact/mapped/context; otherwise null.
- `range_start`, `range_end`: zero-based/end-exclusive Unicode-code-point offsets for exact/mapped; otherwise null. `range_unit` is `unicode-codepoint`, not bytes or UTF-16 units.
- `rendered_context`: optional JSON string for preview context, not nested JSON. Mapped preview text can differ from raw source substring; context-only selection has no exact source anchor.

```json
{"ok":true,"document":{"path":"/project/README.md","dirty":false,"buffer_revision":"1"},"selection":{"view":"source","status":"exact","text":"Hello","range_start":0,"range_end":5,"range_unit":"unicode-codepoint"}}
```

Treat dirty selections as unsaved buffer state; revision is not disk hash. Inspection v2 bounds selected text and rendered context to 4 MiB each. Check status before treating range as an edit anchor.

### IPC failures, paths and transport

- Controls emit one JSON object on stdout; errors also explain on stderr. Exit 0 means delivered hint/queued find/successful inspection, 2 invalid invocation, 1 source/recipient/transport failure. Error shape is `{"ok":false,"code":"...","message":"..."}`. Check exit status and fields; never treat missing viewer as empty selection.
- Use exactly one control mode per command. `--line` belongs only to reveal; `--view`/`--match-case` only to find. No `--assets`, `--shot` or `--frames` in control commands. `--file` is allowed only for source/preview find.
- Paths must be ordinary local paths without application-link ancestry. Windows rejects UNC, remote/extended namespace and reparse paths. macOS permits standard `/tmp`, `/var`, `/etc` OS aliases, rejects other symlinks and nonlocal volumes. Checks are not an atomic filesystem sandbox.
- Windows discovers matching `AttendantMarkdownWorkspace` windows on current desktop and uses bounded `WM_COPYDATA` requests/replies. No network listener. Busy/rendering viewers can refuse inspection; do not force focus or close viewers to recover.
- macOS uses private same-UID Unix sockets through bounded message pipes, one request at a time, pumped between UI frames. Same UID spans login sessions; this is not same-desktop isolation or application authentication. Other same-user processes can inspect selections or impersonate endpoints. Discovery rejects ambiguous, unsafe or unresponsive endpoints and never deletes stale ones. Report stale-endpoint blockers rather than deleting endpoints automatically.
- Reveal v1 and inspection v2 retain separate layouts; find uses its own bounded message. Prefer matching current client/viewer builds; old viewers can refuse newer controls. Transport has deadlines and capacity checks, not hard real-time guarantees. Do not widen timeouts or retry indefinitely to hide failures.

## Rules

- `@path` reads UTF-8 option input; `-` reads stdin once. Create/update items are arrays of `{name, fields}`; fields and query params are objects.
- Before collection-specific lookup or writes, read schema/usage. Use declared fields and documented built-ins only.
- `schema`, `query` and `search` may apply reported mechanical metadata corrections before preparing projection. `doctor`, `validate --no-correct` and `validate --strict` preserve source; strict validation fails for diagnostics or pending corrections.
- Use `search` for text lookup. `query` accepts one read-only, authorizer-contained SQL statement. Normal actions prepare projection; do not add routine validation/sync churn. Use `doctor` for health/projection problems.
- Preserve Markdown; never edit `.attendant/` directly. Edit source bodies as Markdown; metadata writes use CLI.
- Reading feedback or inspecting selection grants no editing authority and never resolves feedback. Resolve only by explicit status update after requested work/disposition.
- Keep schemas flat. Record references use `collection/name`. Migration/import is unavailable.
