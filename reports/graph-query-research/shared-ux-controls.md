# Shared UX controls

## Agreed contract

- List search and canvas filters have independent state. Literal ASCII-case-insensitive substring matching; other UTF-8 bytes exact. Find retains current shown/placed eligibility and live selection behavior.
- Shared section: Canvas Display. Distinguish list search, canvas visibility and analytical inputs. Disclose shown-graph analysis scope. Name One Euro control Smooth displayed motion.
- Use Run for analytical evaluation. Keep immediate MDA/neighborhood exploration. Show pending/applied status beside Run. Failed Run preserves pending inputs and last-good result.
- Reset view restores current pane camera, layout and canvas-display defaults. Preserve selection, analytical inputs and retained results.
- Reset to defaults restores analytical inputs as pending changes; retain result until Run. Neither reset modifies drafts, draft history or other panes. No extra Revert/Clear buttons in this pass; preserve compatible native action keys where possible.
- Back/Forward: one cross-view typed-selection history. Restore view/selection and readable Focus when visible/represented. Do not restore filters, analytical inputs/results or drafts. Never Run or reveal hidden items. Report unavailable target without substitution. Include mouse Back/Forward.
- Undo/Redo: draft edits only; independent of selection history. Do not undo camera, filters, resets or analytical Runs.
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

## Status

Contract checkpoint requested before implementation. Code unchanged at contract capture; implementation follows checkpoint.
