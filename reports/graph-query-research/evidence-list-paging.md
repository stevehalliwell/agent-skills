# Evidence-list paging consistency

Checkpoints: first slice `6f298e2`; named-inspector/similarity-navigation follow-up `395a3e7`. Shared compact graph-layer evidence lists now paged; user authorized checkpoint commit. Kind-catalogue bounds, legacy Resource sidebar and broader visual acceptance remain open. No unperformed visual acceptance inferred.

## Outcome

Make retained evidence reachable through bounded pages. Distinguish rows outside current GUI page from evidence omitted by native producer. Preserve exact source/case/local identities and explicit source navigation.

## Before this slice

- Rules/bindings: 64-row pages use zero-based sliders. Declared binding axes use 16-row pages.
- Noun-pair and Actor-map case lists: scan 64 native case slots per page, then filter. Valid pages can contain no matches; page count covers whole rule model, not matching evidence.
- Effects: selected contributor paths display first64, then direct user to native CLI. Remaining retained paths cannot be paged in that inspector.
- Structural lists mix Previous/Next, zero-based sliders and first64 caps. Some capped lists disclose further rows; finding rows have no page control.
- Similarity entries use 32 rows/page; members use64. Similarity navigation remains separate task.
- IPC evidence uses `total_slots`, native source-slot `offset`, opaque row keys, selection token and `next_offset`. Filtered pages may be empty. Retained result JSON uses separate byte pagination. Interface is unreleased and repository-local. User permits IPC changes; update callers/tests with explicit new semantics rather than keeping compatibility aliases.
- Presentation metadata still limits `rules_page`/`rules_axis_page` to Resources although rule workspace now exists in other panes. Include this mismatch in paging work.

## First-slice contract

- Shared bounded pager/range labels for current rule/binding workflow: binding rows, rule proposals/comparisons, declared binding axes, Noun-pair/Actor-map case evidence and selected Effect contributor paths.
- Use Previous/Next plus human-readable range and page position. Default64 rows; retain smaller16-axis page where nested detail makes it useful. No unbounded append or automatic loading.
- GUI/IPC convention: page matching retained rows, not empty native-slot ranges. Example, not measured fixture: 200 case slots with seven matching cases becomes `Cases 1–7 of 7`; no three mostly empty GUI pages.
- Keep canonical/native source slots as selection identities. Page ordinal is not a selector. Build bounded read-only matching index where needed; do not Run, re-evaluate guards or change solver geometry.
- Page changes preserve selected evidence, primary canvas selection, camera, gold brush, pending/applied inputs, drafts/history and other panes. Do not select or navigate first row automatically.
- Separate `Retained: N`, `Showing: A–B`, and producer omissions/unknown completeness. GUI pagination never claims complete source evidence when native producer is limited. Zero rows use explicit empty state, not invalid range.
- Change IPC evidence `offset` to matching-row offset. Return matching retained total, returned count and next matching offset. Keep each row's native source slot and opaque selection key distinct from its page position. Update repository callers/tests together; no legacy slot-paging alias.
- Preserve stale-token guards and full-result JSON byte pagination. Align scope/omission wording and expose page controls in every eligible pane.
- Structural/similarity-list migration remains follow-up under parent item. Do not mark all evidence-list consistency complete after this slice.

## IPC shape

Evidence pages use matching retained-row offsets. Illustrative first page requested with `--limit 1`, not measured fixture:

```json
{
  "offset": 0,
  "offset_units": "matching-retained-rows",
  "source_slots": 200,
  "total_rows": 7,
  "returned_rows": 1,
  "next_offset": 1,
  "producer_omissions": null,
  "rows": [{"source_slot": 153, "key": "opaque-native-selection-key"}]
}
```

Example illustrates envelope fields; `rows` retains each family's existing exact payload. `source_slot` is native snapshot position, not page offset or cross-reload identity. Use advertised guarded selection keys/references, never ordinal. Not every evidence row has direct selection action. Empty result uses total/returned0 and next offset null. Offset beyond retained total returns empty page, not unrelated rows. Native omissions/limits remain separate and unknown completeness stays unknown.

Use bounded read-only scans or matching-index scratch; no allocation proportional to unbounded external data. Failed candidate preserves old workspace. New retained result or changed evidence scope resets/clamps that list's page; ordinary page changes retain selected evidence even when its row leaves visible page. Page state stays per-pane/session-only.

## Acceptance shape

- Sparse matching cases remain reachable without empty GUI pages under matching-row convention.
- Reach all retained rows in each first-slice list, including beyond64 contributors and beyond16 axes. Source identity does not change with page position.
- Counts distinguish retained rows, visible page and native omissions/unknowns; no inferred completeness.
- Page changes never Run, retarget queries, move camera or replace primary selection/results/pending edits. Last-good state survives allocation failure; pane state remains independent.
- IPC evidence offsets count matching rows; native source slots and row keys remain exact. Full-result byte offsets remain bytes. Rules page controls work outside Resources. Test empty, exact-boundary, final-page, sparse, stale-scope and failure cases.
- Focused Windows CJIT/CPU verification on both unchanged examples; separate authorized visual review. Existing review viewer remains untouched.

## First-slice implementation

- `gd_evidence_page.h`: allocation-free range/clamp helper. Shared GUI pager uses Previous/Next, 1-based page slider and retained range; empty list reports 0 rows. Presentation page values remain 0-based.
- Rule/binding rows, proposal/comparison rows, declared axes and Effect paths page retained native indices. Row inspection stays separate from page position.
- Noun cases use pair's retained count and matching ordinal. Actor cases use bounded native membership scan and per-edge cursor. Query case-page array costs 32,768 bytes per prepared Actor/Effect owner; compile-time bound protects byte page indices.
- Noun overview source lists keep separate, pair-keyed cursors within existing 64 visible-source cap. Filter/source-order changes cannot reuse another pair's page; brush changes reset these cursors. Primary inspector page stays independent. Source-group cap itself remains follow-up.
- Effect path pager removes first64 display cap. Retained paths remain native declarations, not execution trace. Producer limits, incomplete source, unexamined links and unresolved targets stay separate from visible page count.
- Scoped presentation maximums cover rule rows/axes, Noun cases, Actor cases and Effect paths. Rules page fields work outside Resources. `actor_case_page` targets primary selected map association; `contributor_page` targets retained Effect selection.
- IPC evidence performs bounded count/fill scans over matching retained rows. Row keys retain native source indices; no page-relative selectors or allocations. Added `actor-cases` for selected retained map association. Removed `total_slots`/row `slot`; use `total_rows`/`source_slot`, with `returned_rows`, matching `next_offset` and `source_slots` candidate count. No compatibility aliases.
- Unknown producer omission count is null, not fabricated 0. Numeric diagnostic omissions cover native diagnostic scope, not inferred matching omissions for reference filter. Native limit/incomplete flags remain authoritative. Full-result byte pages unchanged.
- Paging verification exposed refresh shortcut that skipped rule-owner remap in Actor/Effect panes. Refresh now rebuilds those clean applied workspaces and restores caller's context pane after candidate walk. Pending forms still block refresh; failed candidate retains old project/workspaces. No pending inputs run implicitly.

## First-slice verification

Artifacts: `skills/game-design-workbench/native/build/evidence-paging-work/`.

- Warning-free Windows CJIT Graph, ExplorerTest, CLI and core builds.
- Core 148/148. Focused two-example Find/shared-UX/production-IPC suite 9/9. Authored source hashes unchanged.
- Paging checks: Crate 5,418; Mario 6,103. Walk every matching Noun case with `--limit 1`, preserve exact native source slots/keys, count sparse known rows, read Actor-case totals/first source, handle empty/out-of-range pages.
- Range tests cover 0, 1, 16, 17, 63, 64, 65, 128, 129 and `INT_MAX`, including clamped last page. Rules page bounds outside Resources preserve row inspection, native result, canvas query inputs, raw geometry, camera, primary selection, brush and draft/history hash without allocating.
- Noun Free source-cursor test changes visible source order and brush; cursors stay separate from primary inspector and reset on pair/scope change. Successful Actor scene replacement clears case-page cache; injected allocation failure preserves nonzero cache/camera/selection in Mario. Existing rule-context failure/rollback checks still pass.
- Synthetic 129 Effect cardinality tests zero-based page2 bounds without inventing authored contributor paths. Count restored before row serialization. Beyond16-axis behavior uses pure range coverage; no claim that examples contain that many declared axes. Real beyond64 Effect-path visual review remains unperformed.
- Clean-refresh regression retains Actor bindings owner/count/applied inputs and caller pane. Harness explicitly undoes its own synthetic draft edits and discards harness Redo before this check; no live viewer buffers involved.
- Existing Mario review viewer not changed or closed. User authorized separate paging review launch. PID10864/session`10864-68897605425c2e1a` appeared, then exited before setup; controls/inspect reads timed out. No mutation, staging or Run sent. Exit report showed no draft/source changes. User chose leave closed; no relaunch or automatic retry. GPU/font/pointer acceptance remains pending. Deep/research checks and final compiler/platform matrix not run; existing Mario deep drift blocker unchanged.

## Follow-up implementation

- Named structural inspectors share 64-row paging: node counts, declaration gaps, findings, finite hypotheses, marked nodes/links, source-backed nodes, checked peers, reached members, roots/targets, matrix loaded cases, related pairs and nonempty adjacency cells.
- Support witnesses retain up to `GD_MAX_RECORDS` parent arcs, not first64. Page in existing root-to-target display order. These are graph support paths, not execution traces.
- Similarity entries now use64 rows instead of32. Feature definitions and selected feature members use same pager. `structure_page` bounds follow node/gap table; feature catalogue uses independent cursor from optional canonical-context node table; `similarity_page` follows retained entries. State-coverage hypotheses use shared pager and scoped `coverage_page` maximum.
- Main explorer overview/foundation/reachability source declarations and current-view Find representations page matching rows. Noun source-group cache keeps64 visible slots, keyed by native pair; page-relative cache position never becomes source identity. Groups beyond first64 remain reachable. Primary Noun-case page stays separate.
- Rule and Actor/Effect case participant details page independently. Participant cursors live in heap-owned rule/query workspaces, not stack-copied `GD_ExplorerPane`; no stack-limit increase. Each owner adds8,192 bytes. Structural nested cursor arrays add90,112 bytes per activated question owner, plus scalar cursors and16,384-byte heap witness scratch. Expanded support paths do not add16KiB renderer stack frames. New retained result clears cursors; failed candidate preserves previous cursors/result.
- Shared matching-row admission helper advances only after native membership test. Empty and last-page handling use existing bounded range helper. No new evaluator, solver, allocation per page or authored-data change.
- IPC rule-row inspection exposed unconditional similarity-mode clearing. `select --form rule` and `rule.check` now retain current canvas, both cameras and ordinary selection. Explicit source/link/cell navigation retains existing mode policy; broader source-navigation direction remains separate.
- Named-question inspectors remain separate from shared compact graph layers. Compact-layer migration follows below. Keep parent paging item open for residual bounds and acceptance.

## Follow-up verification

Artifacts: `skills/game-design-workbench/native/build/evidence-paging-followup/`.

- Windows CJIT Graph/ExplorerTest builds; core148/148; focused two-example Find/shared-UX/production-IPC suite9/9. No deep/research run or compiler/platform matrix. Later user-authorized separate Mario review prepared; session/setup recorded in [navigation report](similarity-navigation.md). Pointer acceptance remains pending.
- Extended paging:22 checks/example. Covers sparse native slots67/153/998 and `INT_MAX` ordinal increment, actual structural counts (Crate63/Mario576), scoped valid/invalid page writes, no allocation/input/result/geometry/camera/selection changes, failed-reader cursor retention and successful-reader cursor clearing.
- Synthetic similarity metadata uses129 entries; restore before serialization. Source-group metadata exercises slots70/129 and restores graph before rendering. Synthetic130-node chain retains129 witness arcs and reaches last page without changing native analysis work. No synthetic authored examples or runtime claims.
- Production adapter reproduces rule-row inspection clearing active MDA similarity mode on both examples before fix. Regression verifies retained mode, row inspection, both cameras, ordinary MDA selection, active pane and allocation count after fix.
- Existing paging checks remain Crate5,418/Mario6,103. Authored source hashes remain unchanged. CPU checks do not establish pointer/readability/restart/standalone acceptance.

## Compact graph-layer slice

- Reuse same pager in shared cimgui graph layers: retained shared-neighbor matches, starts, mandatory ancestors, support candidates, foundation peers/members, reachability peers/targets, full retained closure-parent witnesses, repeated families, signatures, family members and overview-bin identities.
- Standalone graph/MDA marked-declaration lists also page matching retained rows. Deduplicated Skills links still expose first retained source declaration per projected link; paging does not recover producer omissions.
- Keep 80-byte `GD_GraphEvidencePages` separate from analytical controls/cache keys. Own one per unified pane and standalone graph context. Reset view clears these cursors. No heap allocation per page or enlarged witness stack buffer.
- `gd_graph_closure_path_slice()` reads existing retained parent arcs with bounded64-arc scratch. Offset counts backward from target internally; GUI pages remain root-to-target. Full length, native arc/source identity and cached analysis work stay unchanged. Existing full/prefix reader remains in use.
- Candidate/target previews retain exact chosen native identity outside visible page. Target picker no longer replaces selection with first row during rendering. Invalid scope clears target to explicit choice, not inferred first target.
- Native mandatory-support/bypass producers still retain last64 arcs. Labels retain full-length/retained-count distinction; this slice does not enlarge native results or claim complete source semantics.

### Checks

Artifacts: `skills/game-design-workbench/native/build/compact-sidebar-paging/` and `build/compact-sidebar-core/`.

- Warning-free Windows CJIT Graph/ExplorerTest/core builds. Core148/148; focused two-example Find/shared-UX/production-IPC suite9/9. Example source hashes unchanged.
- Closure oracle checks sliced native arc identities, untouched output guards and beyond-end offsets across existing64 randomized graphs/directions. Native work unchanged.
- Synthetic130-node chain checks all129 arcs across three pages. CPU ImGui frames exercise reached-target/witness final pages, first-page return and mandatory-ancestor page3; exact peer/target/controls and native work retained, no project allocator calls.
- Synthetic repeated representations exercise family membership beyond64 without first-alias navigation. Synthetic130-node star exercises128 retained shared-neighbor matches while retaining peer outside visible page. Fixtures never change authored models.
- No live viewer launched or mutated. No GPU/pointer, restart, standalone launch, deep/research or compiler/platform acceptance inferred.

### Residual bounds

- Shared relation-kind picker and overview kind histogram retain64 distinct tags. Not migrated in this slice.
- Legacy standalone raylib Resource sidebar retains its own compact member/path/source summaries. Shared cimgui migration does not change that renderer.
- Parent paging task stays open. Practical pointer review and broader every-view/restart/standalone checks remain unperformed. Similarity readability stays deferred under separate task.

## Comments

Grounded in `gd_explorer_rules_view.c`, `gd_explorer_query_view.c`, `gd_noun_grid_view.h`, `gd_explorer_structure_view.c`, `gd_explorer_similarity_view.c`, `gd_explorer_presentation.h` and `gd_explorer_viewer_evidence.h`. No general paging framework or public protocol redesign assumed.
