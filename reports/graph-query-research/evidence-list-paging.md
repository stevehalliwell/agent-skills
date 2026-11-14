# Evidence-list paging consistency

First slice accepted for checkpoint by user without further visual review. User authorized commit. Matching-row paging covers GUI and IPC after contextual tools checkpoint `62ffa02`. Structural/similarity migration remains follow-up. No unperformed visual acceptance inferred.

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

## Implementation

- `gd_evidence_page.h`: allocation-free range/clamp helper. Shared GUI pager uses Previous/Next, 1-based page slider and retained range; empty list reports 0 rows. Presentation page values remain 0-based.
- Rule/binding rows, proposal/comparison rows, declared axes and Effect paths page retained native indices. Row inspection stays separate from page position.
- Noun cases use pair's retained count and matching ordinal. Actor cases use bounded native membership scan and per-edge cursor. Query case-page array costs 32,768 bytes per prepared Actor/Effect owner; compile-time bound protects byte page indices.
- Noun overview source lists keep separate, pair-keyed cursors within existing 64 visible-source cap. Filter/source-order changes cannot reuse another pair's page; brush changes reset these cursors. Primary inspector page stays independent. Source-group cap itself remains follow-up.
- Effect path pager removes first64 display cap. Retained paths remain native declarations, not execution trace. Producer limits, incomplete source, unexamined links and unresolved targets stay separate from visible page count.
- Scoped presentation maximums cover rule rows/axes, Noun cases, Actor cases and Effect paths. Rules page fields work outside Resources. `actor_case_page` targets primary selected map association; `contributor_page` targets retained Effect selection.
- IPC evidence performs bounded count/fill scans over matching retained rows. Row keys retain native source indices; no page-relative selectors or allocations. Added `actor-cases` for selected retained map association. Removed `total_slots`/row `slot`; use `total_rows`/`source_slot`, with `returned_rows`, matching `next_offset` and `source_slots` candidate count. No compatibility aliases.
- Unknown producer omission count is null, not fabricated 0. Numeric diagnostic omissions cover native diagnostic scope, not inferred matching omissions for reference filter. Native limit/incomplete flags remain authoritative. Full-result byte pages unchanged.
- Paging verification exposed refresh shortcut that skipped rule-owner remap in Actor/Effect panes. Refresh now rebuilds those clean applied workspaces and restores caller's context pane after candidate walk. Pending forms still block refresh; failed candidate retains old project/workspaces. No pending inputs run implicitly.

## Verification

Artifacts: `skills/game-design-workbench/native/build/evidence-paging-work/`.

- Warning-free Windows CJIT Graph, ExplorerTest, CLI and core builds.
- Core 148/148. Focused two-example Find/shared-UX/production-IPC suite 9/9. Authored source hashes unchanged.
- Paging checks: Crate 5,418; Mario 6,103. Walk every matching Noun case with `--limit 1`, preserve exact native source slots/keys, count sparse known rows, read Actor-case totals/first source, handle empty/out-of-range pages.
- Range tests cover 0, 1, 16, 17, 63, 64, 65, 128, 129 and `INT_MAX`, including clamped last page. Rules page bounds outside Resources preserve row inspection, native result, canvas query inputs, raw geometry, camera, primary selection, brush and draft/history hash without allocating.
- Noun Free source-cursor test changes visible source order and brush; cursors stay separate from primary inspector and reset on pair/scope change. Successful Actor scene replacement clears case-page cache; injected allocation failure preserves nonzero cache/camera/selection in Mario. Existing rule-context failure/rollback checks still pass.
- Synthetic 129 Effect cardinality tests zero-based page2 bounds without inventing authored contributor paths. Count restored before row serialization. Beyond16-axis behavior uses pure range coverage; no claim that examples contain that many declared axes. Real beyond64 Effect-path visual review remains unperformed.
- Clean-refresh regression retains Actor bindings owner/count/applied inputs and caller pane. Harness explicitly undoes its own synthetic draft edits and discards harness Redo before this check; no live viewer buffers involved.
- Existing Mario review viewer not changed or closed. User authorized separate paging review launch. PID10864/session`10864-68897605425c2e1a` appeared, then exited before setup; controls/inspect reads timed out. No mutation, staging or Run sent. Exit report showed no draft/source changes. User chose leave closed; no relaunch or automatic retry. GPU/font/pointer acceptance remains pending. Deep/research checks and final compiler/platform matrix not run; existing Mario deep drift blocker unchanged.

## Comments

Grounded in `gd_explorer_rules_view.c`, `gd_explorer_query_view.c`, `gd_noun_grid_view.h`, `gd_explorer_structure_view.c`, `gd_explorer_similarity_view.c`, `gd_explorer_presentation.h` and `gd_explorer_viewer_evidence.h`. No general paging framework or public protocol redesign assumed.
