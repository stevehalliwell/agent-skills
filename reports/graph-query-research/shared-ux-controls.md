# Shared UX controls

## Agreed contract

- List search and canvas filters have independent state. Literal ASCII-case-insensitive substring matching; other UTF-8 bytes exact. Find retains current shown/placed eligibility and live selection behavior.
- Shared section: Canvas Display. Distinguish list search, canvas visibility and analytical inputs. Disclose shown-graph analysis scope. Name One Euro control Smooth displayed motion.
- Use Run for analytical evaluation. Keep immediate MDA/neighborhood exploration. Show pending/applied status beside Run. Failed Run preserves pending inputs and last-good result.
- Reset view restores current pane camera, layout and general canvas-display defaults. Preserve selection, analytical inputs/results, list-search text and typed context: Actor ghost/primary/compare, contributor context, similarity canvas, relationship neighborhood and MDA traversal root. Context controls remain explicit; do not remove a selected variant to achieve a presentation default.
- Reset to defaults restores analytical inputs as pending changes; retain display settings and result until Run. Neither reset modifies drafts, draft history or other panes. No extra Revert/Clear buttons in this pass; preserve compatible native action keys where possible.
- Reset all, added after interactive review, sits beside Redo. Clear selections, analytical results/inputs, canvas/list filters, Find, camera/layout state, navigation history and shared folds across all panes. Keep active projection, loaded source snapshot, unsaved draft edits and Undo/Redo. User explicitly chose draft preservation. No source reload/write or Run of prior pending inputs. Pending rank edit/drag blocks reset. Prepare default active canvas before releasing any last-good pane; failure retains all panes.
- Back/Forward: one cross-view typed-selection history. Restore view/selection and readable Focus when visible/represented. Do not restore filters, analytical inputs/results or drafts. Never Run or reveal hidden items. Report unavailable target without substitution. Include mouse Back/Forward.
- Undo/Redo: draft edits only; independent of selection history. Do not undo camera, filters, resets or analytical Runs. Refresh blocks while Redo remains. GUI requires explicit discard confirmation; IPC requires presentation-token-guarded `draft.redo.discard` before refresh. Never silently discard Redo.
- Show relevant unavailable actions disabled with reasons. GUI and IPC share eligibility rules.

## Delivery

1. Independent search/filter state, common matching and wording.
2. Run/status presentation and reset contracts.
3. Cross-view selection history and draft Redo.
4. Shared action-availability reasons.

Preserve typed identities, independent pane state, atomic last-good failures, explicit Run and source-write guards. CPU/CJIT checks first. No unsolicited GUI launch, replacement or OS input. Final compiler/platform and pointer review remain separate.

## Acceptance

- Changing list text cannot change canvas visibility or analytical inputs. Canvas filtering cannot change list text. Matching agrees with Find across ASCII case and UTF-8 examples.
- Pending inputs and failed Run leave prior result intact. Reset contracts preserve draft/history/other panes and do not Run.
- Back/Forward restores exact typed targets across views, handles hidden/unavailable targets and does not alter query state. Selection history remains distinct from draft Undo/Redo.
- Undo/Redo round-trips draft edits and invalidates redo branch on new edit. Relevant disabled actions expose reasons through GUI and IPC.
- Record measured checks and limits without inferring rendered/pointer acceptance.

## Implementation

- Shared matcher `gd_text_match.h` retains Find's bounded linear literal matching. Independent `list_search` uses actual current-canvas labels, source names/references and visibility, including similarity entries. Resource/MDA canvas filters remain separate. Legacy `filter` aliases list search outside Resources; Resource keeps its canvas semantics.
- Run status beside each analytical Run; IPC `input_status` reports `not-run`, `applied` or `pending-changes`. Presentation tokens now bind pending as well as applied analytical forms, guarding multi-form default reset.
- Public Reset view uses existing atomic presentation rearrangement, not a fresh pane/query. Reset to defaults stages analytical forms but preserves five query-display controls. Legacy Clear IPC actions remain compatible; GUI no longer shows separate analytical Clear buttons.
- Shell-owned `gd_explorer_history.h`: 128 bounded entries, copied exact IDs/references, no borrowed result pointers or snapshot-local identity fallback. Node, pair/case, source, unique-edge and empty-selection targets; empty views remain navigable. New selection replaces forward branch; oldest entries evicted at capacity. Live text changes coalesce until release. Missing/ambiguous/unprepared/different-canvas targets fail without changing selection/camera or running analysis. Identity capacity/work failures disclose missing history entry; no truncated IDs stored.
- History keeps current filters/results/context. Readable Focus uses matching render pose when available; hidden selections retain camera. No history persistence. Toolbar/mouse and IPC `view.back`/`view.forward` share handlers.
- Draft Redo swaps inverse values in existing 4096-entry history storage. No extra per-edit allocation. Undo restores exact baseline edge declarations at sync point. Rank bounds, prerequisite capacity, duplicate declarations, invalid/no-op edits and branch replacement retain existing safeguards.
- Toolbar Undo/Redo plus Ctrl+Z / Ctrl+Shift+Z / Ctrl+Y execute after inspector readers. Pending rank edits and active drag guard actions. Redo-only refresh confirmation has Cancel; IPC requires fresh presentation token for `draft.redo.discard`. Inspection exposes `redo_steps`; Undo/Redo disclose in-memory draft mutation, not disk writes.
- Shared disabled reasons drive GUI tooltip and IPC `disabled_reason`. Relevant actions remain visible. Private window-test setup now uses explicitly named fresh-pane fixture helper; retired whole-pane reset is not confused with public presentation reset.

## Checks

- Warning-free CJIT CLI, production graph viewer and explorer adapter builds in `native/build/shared-ux-work/`.
- Core145/145 pass, including added Redo rank/prerequisite round-trips, duplicate-edge baseline restoration, invalid/no-op preservation, branch replacement and 4096-step history capacity.
- New `--ux-helpers ROOT`: Crate288 / Mario304 checks. Covers shared matching, independent filters/list text, retained result pointers/pending edits, reset invariants/failure, cross-view navigation, hidden/missing/empty selections, pair/case identity, branch/capacity, allocation-free navigation, draft history and refresh/discard safeguards.
- Final Node13/13 pass: both Find examples, both shared-UX examples, flag parsing, both normal IPC examples and both explicit deep examples. Source hashes unchanged. Default Crate268.2342ms / Mario874.8698ms including hashes; combined default group1143.8788ms. No whole-suite-under1-second claim.
- Explicit deep checks retain existing Resource/noun/layout/selection regression thresholds and pass: Crate511.4843ms / Mario3443.4919ms. Comparative research not rerun.
- Final logs: `final-checks.log`, `build-adapter-final.log`, `build-graph-final.log`, prior CLI/core logs and `crate-ux.json` / `mario-ux.json` under build directory.
- No GUI launch/replacement, OS input or authored source writes. Initial CPU checks do not establish visual toolbar/status placement, real mouse Back/Forward, keyboard interactions, confirmation dialog, full window-based regressions or final compiler/platform behavior. Subsequent human review recorded below.

## Reset all follow-up

- Authorized Mario review viewer PID11940 / session `11940-db8859b3d4951114` opened from `shared-ux-work/`, MDA brick-response framed. User reports Back/Forward works. Undo/Redo disabled with no draft steps; expected. User found no clear-selection affordance and requested global reset beside Redo.
- Shared `view.reset.all` action replaces all pane exploration state, clears selection/analysis and resets MDA traversal. Draft object remains byte-identical, including modified records and both history directions. Other panes return to lazy default preparation; active projection stays open. Default Actor/Effect canvas preparation follows normal startup, not pending analytical Run.
- Toolbar handler executes after frame readers. Native IPC requires current presentation token. Both paths clear pending hints and cached evidence tokens. Find closes/clears in every pane; default fold flags persist using existing UI-only settings path. Source-staleness warnings remain truthful; reset is not source refresh.
- Allocation failure at replacement-array or active-layout allocation leaves old layouts/results, selection, navigation, Find, folds and MDA query intact. No draft discard confirmation needed because draft edits/history are preserved.
- Warning-free CJIT adapter and production viewer builds: `native/build/shared-ux-reset-work/`. Implementation checkpoint authorized after interactive review.
- Focused Node10/10 pass with unchanged hashes for both examples: shared UX, Find, flags and normal IPC. Extended deep checks/research deliberately not rerun for this follow-up. Current shared-UX checks: Crate344 / Mario360, including global reset, failure rollback, drag guard, draft byte identity, working preserved Undo/Redo, missing-token rejection and successful guarded IPC reset.
- User authorized closing prior viewer and launching new build. Verified prior viewer had no draft edits/history/pending rank text; closed PID11940 through normal window-close request, without forced termination. Opened Mario MDA PID3884 / session `3884-c734fb8b8e31108d` from `shared-ux-reset-work/`.
- User reports Reset all worked and authorizes checkpoint commit. Back/Forward also reviewed as working. Acceptance covers these reviewed interactions, not unperformed draft-edit/refresh, every-view or platform checks.

## Status

Contract checkpoint `e0458db` committed before implementation, not pushed. Implementation complete for reviewed contract and subsequent clarifications. User authorized implementation checkpoint after Back/Forward and Reset all review. Broader presentation, draft-edit/refresh and every-view acceptance remain open.
