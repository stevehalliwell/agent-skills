# Similarity navigation

Confirmed: explicit source opening shows ordinary source canvas with Return to similarity. Source opening stays in current pane; no forced projection jump. Paging work continues under [separate contract](evidence-list-paging.md).

## Before migration

- Similarity row/comparison and camera live in question workspace. Ordinary canvas selection/camera live in pane. Selecting analytical row is not source navigation.
- GUI **Open linked source definition** calls `gd_explorer_structure_source_node()`. It updates ordinary selection/navigation but does not explicitly leave similarity mode. Skills/Challenges ordinary navigation can also change active projection for opposite-role source.
- IPC explicit ordinary `select` clears similarity mode. Rule-row inspection previously did too; paging follow-up fixes rule/check inspection to preserve mode. Explicit ordinary source policy remains unchanged.
- **Show separate similarity canvas** toggles mode. No named return action records return destination. Selection history rejects entries whose canvas mode differs; it does not restore that mode.
- **Fit similarity** targets map even when ordinary canvas is shown. Source opening, current canvas and Fit target therefore need explicit labels/behavior.

## Confirmed direction

Explicit source opening shows ordinary source canvas in current pane. Provide **Return to similarity**; retain exact analytical row, comparison peer and map camera. Preserve ordinary camera; Fit/focus remain explicit. Select exact unique ordinary representation when available, otherwise inspect linked source with missing/ambiguous representation disclosure. Never jump Skills/Challenges automatically. Row inspection stays on current canvas. No query Run or input replacement.

Example:

1. Select analytical entry and comparison peer.
2. Open linked source explicitly; inspect source on ordinary canvas.
3. Return to same retained similarity context without recomputing map or losing comparison.

## Preserved boundaries

- Preserve independent cameras, pending/applied inputs, retained results, drafts/history and other panes.
- Retain exact typed entry identity when showing source definition. Definition identity does not imply exact local/case binding.
- Missing/unplaced/ambiguous source stays disclosed; no nearest-entry or first-case substitution. Never Run to repair navigation.
- Similarity remains analytical distance, not topology. No map layout/solver change or authored-data change.

## Delivery shape

- Shared GUI/IPC actions open selected similarity source and return to retained map. Invalid/missing source rejects without partial mutation. No allocation or implicit preparation/Run.
- Show current canvas and return destination. Fit control names current target; do not silently fit hidden map from source mode.
- Keep existing guarded selection history. Back does not silently switch canvas mode or Run; Return is explicit mode transition.
- Verify both unchanged examples: exact source identity/current pane, retained row/peer/map camera, independent ordinary camera, no pending/applied/result/draft/history replacement, missing source and no-result rejection. Rendered review remains separate.

## Implementation and checks

- Shared `view.similarity.source` / `view.similarity.return` actions serve GUI and guarded IPC. Source opening validates retained typed identity before switching mode. Invalid record/entry rejects without partial selection/mode change.
- Source lookup scans bounded ordinary representations. Select exact unique representation; missing/multiple representations use linked source inspection with disclosure. No first alias, projection jump, camera change, new allocation or reader Run. Source reveal clears stale queued ordinary fit so hidden fit cannot move camera on next frame. Canonical-context and rule-source buttons use same route while similarity result is retained.
- GUI shows current canvas and pane. Return retains row/peer/map camera. Fit labels target visible canvas; comparison-peer button names its role. Dedicated readout says Similarity pair; global `view.compare` availability/dispatch follows visible canvas rather than hidden retained map.
- Windows CJIT focused suite9/9 on unchanged examples. Production adapter tests actual MDA map source/return actions, exact source record, retained row/peer/result/inputs, independent cameras, active pane and zero allocation. Invalid entry/source and no-result return reject. Skills/Challenge source test stays in current pane; synthetic missing/duplicate representation metadata restores before rendering.
- User authorized separate Mario rendered-review launch, including setup/analytical Runs confined to new session. PID8604/session`8604-1556b702799e3e8a` prepared successfully. Existing viewer list was empty. No commit made.
- Review ready: MDA similarity uses exact discovered `retained-partners` and `source-loaded-role` features; entry `mechanics/enemy-contact`, peer `mechanics/death-and-retry`. Actors direct-connections inventory retains576 source-backed nodes, page8 (GUI page9), ready for last-page review. Physics paused. Setup used exact session/control tokens; no timeout or mutation retry.
- `review-ready.json` reports draft modified-records0/Undo0/Redo0, MDA active, selected typed entry8/peer6 and retained map camera. Setup is not pointer/readability acceptance. No restart/standalone acceptance or final compiler/platform matrix. Review artifacts share `native/build/evidence-paging-followup/`.

## Review status

Reviewed source/return transition accepted: user confirms changing between views was fine. This does not accept similarity-map readability or unperformed every-view/restart/platform behavior. Initial review lacked context; user requested explanation, then explicitly requested IPC demonstration. Restaged exact MDA comparison in authorized session and demonstrated map, source opening, then return, with source-view pause. `demo-restored.json` verifies exact analytical row8/peer6/map camera restored; current pane stays MDA and draft modified-records/Undo/Redo remain0. Opening source retained ordinary camera. Ordinary camera differed after source-view pause; no immediately-before-return snapshot exists, so whole-cycle ordinary-camera invariance is not established by this demo. Live IPC demonstration is not human pointer/readability acceptance. Paging review remains separate.

## Readability follow-up

Status: needs-refinement. User explicitly requests recording that current similarity view needs work because it cannot be read. Navigation acceptance stays separate.

- Read-only native result:44 records at12 distinct coordinates. Enemy contact shares position with4 other records; Death and retry shares with11 others. Overlapping records need clear reveal/selection.
- Above32 entries, renderer labels only selected/peer row numbers. Record names are not legible/discoverable enough.
- Explain chosen measurements and weights with explicit legend. Current retained-neighbour-count/record-type demo does not explain gameplay meaning.
- Explain that axes have no independent meaning; proximity is approximate similarity on chosen features, not execution order, causal influence or importance. Identical feature values can hide distinct records at same point.
- Captured directions: record names, measurement legend and overlap reveal/selection. Exact design and acceptance TBD. No readability redesign authorized or implemented.

## Comments

Grounding: `gd_explorer_similarity_view.c`, `gd_explorer_structure_view.c`, ordinary navigation dispatch in `gd_explorer_view.c`, `gd_explorer_viewer.h`, `gd_explorer_history.h`. Compact graph-layer paging caps remain separate open work; see paging report.
