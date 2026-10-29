# Shared Markdown GUI implementation plan

Status: implementation authorized. Start with feedback discovery, then owned collection creation and GUI. Existing native CLI and copied-real-data acceptance remain baseline. Installed skill/launcher switch stays last.

## Result

Small Windows-first shared Markdown workspace: collection/file navigation, top document tabs, source editing/preview, clickable front-matter references and selected-text feedback that agents retrieve through ordinary Attendant queries.

Application/core is C99, compilable with CJIT and `zig cc`. Third-party native dependencies may contain C++. Use very recent Dear ImGui/Markdown stack; do not assume existing on-disk versions meet requirement. Ink player's cimgui/raylib pattern is candidate, not proof of CJIT GUI compatibility: its current checked GUI recipe uses Zig. Verify upstream release, Markdown component, matching cimgui generation and dependency ABI/build paths before adopting. Research: [GUI-DEPENDENCIES.md](GUI-DEPENDENCIES.md). A narrow C ABI bridge for unwrapped third-party C++ APIs is allowed; application/core remains C99. No automatic dependency installs/downloads. Preserve GUI-independent CLI build.

## Document and navigation contract

- Markdown files authoritative; SQLite disposable. Save files normally; preparation refreshes projection on next interaction. No DB-mediated save or collaborative merge service.
- Open workspace with narrow collection/file navigation and top document tabs. Reopen same canonical file by focusing existing tab. Opening reference does not replace original tab.
- Read front-matter reference declarations through existing collection/schema rules. Each `depends_on` item opens target document; missing targets show unresolved, not guessed paths.
- Edit source and show rendered Markdown preview. Preserve unrelated source/body bytes where core operates on metadata; human full-document edits save intended text. Dirty-tab save/discard/cancel is explicit. Reload clean buffers after external changes; never silently overwrite dirty work. No locking/CRDT project.
- Source editor selection is first interaction. Rendered-preview selection must map to original Markdown before offering range-based feedback; never use rendered character offsets as source positions. Finish/test mapping before calling preview feedback complete.
- Agent reveal/navigation must not steal current selection/focus. Selecting text alone grants no edit permission.

## Feedback data contract

One managed collection per workspace: alias `_meta_comments`, default directory `records/_meta_comments`. Create/register on first Send only; browsing/querying must not create it. Existing incompatible alias/directory/schema is an explicit conflict, never overwritten. Compatible configured collection may use another directory; resolve through config.

Proposed v1 schema, finalized here for implementation:

```yaml
---
status: [pending, resolved]
target_path: ""
target_hash: ""
range_start: 0
range_end: 0
range_unit: [unicode-codepoint]
selected_text: ""
---
```

Use normal generated `id`, `name`, `created_at`, `desc`, `tags` metadata. Comment goes in Markdown body, not DB-only storage. `target_path` identifies selected saved file using core's absolute path convention; UI may display workspace-relative path. Built-in `source_path`/`source_hash` continue to identify/hash the comment Markdown itself; target fields must not collide with either. No automatic path rewriting after relocation.

- Ranges: zero-based Unicode code-point offsets, end-exclusive, in complete saved Markdown including front matter. CR/LF each count as code points. Not UTF-8 byte or UTF-16 code-unit offsets. UI adapters translate explicitly; tests include non-ASCII/supplementary characters and CRLF.
- `target_hash`: lowercase SHA-256 of exact saved UTF-8 file bytes. Verify selected text equals specified source range at submission. Validate integer range bounds and UTF-8 before publishing comment.
- Dirty document offers explicit **Save & Send** or Cancel. Failed save sends nothing. After successful save, create feedback against saved text/hash. Failed comment creation leaves source saved and reports failure; never imply comment was queued.
- Reading does not consume feedback. Agent queries pending records, considers/responds in task context, then explicitly resolves through ordinary `update`. Resolution is not automatic source deletion or blanket approval for unrelated work.
- Feedback can outlive source changes: stored selection remains readable; changed hash marks range stale. Do not silently apply old range to new content.
- Local collection is inbox; no direct Pi/session injection or separate messaging daemon. Reuse core source publication/validation where it fits; do not expose new CLI commands merely for GUI internals. Keep operational failures recoverable in GUI rather than using process-fatal CLI handling.

Example agent loop, against native executable:

```sh
native query -p "$workspace" \
  -s 'SELECT name,source_path,target_path,target_hash,range_start,range_end,range_unit,selected_text FROM _meta_comments WHERE status=:status ORDER BY created_at,name' \
  -P '{"status":"pending"}'
# Open returned comment Markdown to read body, inspect source, respond/act.
native update -p "$workspace" -c _meta_comments \
  -i '[{"name":"COMMENT_FILENAME_STEM","fields":{"status":"resolved"}}]'
```

## Pending reminder contract

Native `query` and `search` already prepare projection. On those commands only, when `_meta_comments` exists, append small sibling field without changing existing result fields:

```json
{
  "columns": ["name"], "rows": [], "returned": 0, "truncated": false,
  "meta_comments": {"pending": 3}
}
```

Also emit `3 _meta_comments are waiting for you.` on stderr when count > 0. Use singular grammar for 1. Include `{ "pending": 0 }` for configured compatible collection with no pending rows; no notice at zero. Omit field entirely when collection absent. Never dump comment bodies as sidecar.

Read count from same prepared SQLite read snapshot as command. Count visible valid projected pending rows, consistent with inbox query. Invalid/blocked records remain exposed through existing diagnostics. If alias exists but status declaration is not exactly `[pending, resolved]`, append `{ "pending": null, "error": "incompatible-schema" }` with concise warning rather than claiming zero. This check recognizes discovery/lifecycle only; step 2 validates complete managed schema before creating feedback.

No inbox creation, writes, FTS construction or extra preparation just to count. No new notices on doctor/validate/help/add-table/create/update/sync in first version. Existing non-writing commands retain behavior. Reading inbox itself does not clear count. Installed `.mjs` need not gain sidecar before native rollout; its ordinary query/update can operate on same collection.

## Git and boundaries

GUI may show files changed from HEAD and staged/unstaged/untracked state. Read-only context only; no auto-commit, discard or Git-based recovery. Existing native CLI stays Git-independent. Missing Git/non-repository workspace does not prevent editing or feedback.

Do not expand Unicode search or long-path/UNC qualification now: recorded corpus audit has no affected Unicode classification mismatch, and user deferred Windows edge coverage. No viewer-induced launcher rollout, nested-YAML expansion, full collaboration protocol or new task lifecycle policy.

## Ordered steps and observable acceptance

- [ ] **0. Recent GUI dependency compatibility check.** Build/link/render portion passed in [C99 GUI spike](../../spikes/cimgui-richmd-c99/RESULTS.md): pinned ImGui 1.93.0/RichMd 1.0.0, regenerated cimgui, Zig-built DLL, strict C99 Zig host and CJIT host. Actual editing, selection/copy and C link callbacks passed. Remaining gate: export and verify original-Markdown selection positions, not clipboard text alone. This does not block feedback core or source-selection GUI work; it blocks claiming preview feedback complete.
- [x] **1. Native pending-feedback discovery.** Add query/search sidecar and concise notice; compatible/absent/zero/incompatible cases. Ordinary querying must never create collection. Both compilers and relative/absolute configs; unchanged source/hash checks, reading does not consume, resolving clears count, body remains intact. Existing read/CLI/search-mode/real-data suites remain green.
- [x] **2. Managed collection and feedback core.** Canonical schema/usage/template seeds; narrowly allow `_meta_comments` alias without relaxing normal record-name/path validation. First-Send bootstrap uses staged publication and rollback; no conflicting-file overwrite. Create one comment with generated identity, range/text/hash verification and Markdown body. Recoverable API for GUI, tested headlessly with both compilers. Failure tests cover save failure, schema/path conflict, invalid offsets, source change and comment publication after successful save. Ordinary `.mjs` and native query/update read/resolve same records.
- [x] **3. GUI build and read-only workspace.** Prove CJIT/Zig application compile/link with chosen dependency packaging; separate outputs, no CLI dependency on GUI/C++. Open owned fixture workspace, browse records and show document tabs/preview. Source reference clicks open/focus correct tabs; unresolved links visible. Check clean shutdown and bad-file errors without losing workspace.
- [x] **4. Editing and source-selection feedback.** Editable source, undo, save/discard/cancel, source selection and comment composer. Save & Send uses step 2; feedback pending badge/list opens source. End-to-end GUI send → CLI query → explicit resolution. Test UTF-8 offsets/CRLF and stale source markers. Preview-selection mapping follows as part of completing feedback surface, not guessed offsets.
- [ ] **5. Complete preview navigation and feedback review.** Source-mapped preview selection, Markdown links and front-matter links, multiple tabs, resolved/pending views and agent reveal without focus theft. Show read-only Git badges/diff where available; missing Git graceful. No silent external-buffer overwrite.
- [ ] **6. Guidance and integrated acceptance.** Explain sidecar/inbox workflow in CLI help/reference and skill guidance without switching launcher. Test normal queries, comment body inspection, resolution, malformed feedback and absent inbox. Use disposable real-project copies for GUI/CLI loop, source preservation, reference navigation and external edits. Record actual visual/interaction checks separately from headless tests.
- [ ] **7. Rollout gate, then launcher last.** Review correctness coverage, useful additional capabilities and representative performance against existing runner; measure GUI startup/idle/interaction footprint rather than infer. Only after gate, implement/discover/cache compiler build and switch skill launcher. No automatic installs. Final user acceptance remains required before treating GUI as release-ready.

## Checks

- Planning/research checkpoint committed as `26b7894` before implementation. Subsequent `spike/cimgui-richmd-c99` verifies dependency build/link/render through both compilers. Step 0 remains partial only because original-source selection-range export is not implemented. Spike documents compatibility patches and teardown/provenance checks; no further broad dependency research needed before step 2.
- Step 1 implemented in `read.h`: query/search sidecar and notice, same read snapshot, no inbox bootstrap or FTS work solely for count. Existing output unchanged when collection absent. Lifecycle-only schema recognition; complete managed-schema/bootstrap validation belongs to step 2.
- `meta-comments-check.mjs`: 11 cases × CJIT/Zig × relative/absolute paths = 44 passing cases. Covers absent/empty/pending/resolved/incompatible, inbox reads, external status edits, hidden/malformed records, non-notifying commands, legacy reuse, source preservation and cold query without FTS.
- Both native builds pass existing read, CLI and 13-case search-mode suites in both config forms; 66 copied-real-data workflow cases and 21 harness/existing-runner tests pass. Evidence: `build/review/meta-comments-results.json`, `build/meta-comments-{results,read-results,cli-results,mode-results}.jsonl`, `build/meta-comments-real-data-results.json`.
- Corrected managed selected-file hash field to `target_hash` during fixture validation: built-in `source_hash` is reserved for comment Markdown, just as built-in `source_path` is. No existing source schema or behavior changed.
- Feedback discovery and GUI spike checkpointed as `f9de9c7` and `340098c`. Active implementation branch: `feat/shared-markdown-gui`. Launcher/release gate unchanged.
- Step 2 implemented through `native-api.h`/`native-api.c`: recoverable single-thread request boundary; source read/optimistic atomic save; full managed-schema validation; Unicode code-point range/text/hash checks; first-Send seeds/comment with config published last. Missing config is created on first Send only. Existing compatible alternate directory reused; no normal filename validation relaxed. No unrelated source correction or projection preparation.
- `api-check.mjs`: 68 cases across CJIT/Zig and relative/absolute configs. Includes stage/seed/comment rollback, external target/config changes after staging, failed save, stale hash, empty comment, supplementary Unicode/CRLF, schema/directory conflicts, ordinary native/`.mjs` read/resolve, body preservation, and saved-but-not-queued behavior. Evidence: `build/api-check-results.json`; owned fixtures: `build/api-check/run-gAGkGQ`. Existing read/CLI/source suites pass both compilers. Independent core review found rollback path ownership could change after publication; cleanup now compares expected bytes under a non-write/delete-shared handle and deletes through that handle. Changed files survive, appear in `written`, and force explicit conflict on retry; focused regression passes.
- Step 3: integrated C99 GUI and source-only workspace discovery implemented under `gui/`. CJIT/Zig production and test hosts compile/link against same verified DLL. Actual D3D previews, Unicode/space project paths, typed-reference button input, tab reuse, visible unresolved targets and unchanged source/no projection passed. Both compilers pass 42 always-on headless document-state checks; two build-provenance guard tests pass. Evidence: `build/gui-check/run-VbtQvZ`, `build/gui-check-results.json`, retained `gui/evidence/windows-x64.json`; details in `gui/README.md`.
- Step 4: source editor/undo/redo, save/reload/close Save/Discard/Cancel, source-selection composer, explicit Save & Send, saved-but-not-queued, pending/resolved views and saved-target selection implemented. `attendant_read_comment()` validates managed anchor without consumption; fresh target hash/text/range must match before selection, and dirty targets stay untouched.
- Independent workspace review found three issues, now fixed/tested: unreadable record blocked whole GUI; post-read stat could accept newer metadata for old bytes; failed Send lost surviving-path evidence. Browse policy now records per-file diagnostics while CLI stays strict; read/save return associated metadata; model-owned failed/surviving paths survive refresh.
- Current GUI acceptance: 10 fresh real windows/D3D runs, both compilers, real edit/undo/close-Cancel/Send/target-button input, 71-code-point UTF-8/supplementary/CRLF source selection, native and `.mjs` read/resolve/body preservation, failed-queue UI and no other source/config change. Both compilers pass 81 always-on model checks; two provenance guards pass. Evidence: `build/gui-check/run-JNHl9H`, `gui/evidence/windows-x64.json`; 68 core API cases and existing read/CLI/source suites remain green.
- Continue step 5 preview-range export, Git context and non-focusing agent reveal. Copied-real-project integrated loop, GUI footprint evidence and launcher change remain incomplete.
