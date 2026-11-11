# Selection continuity across matrix layouts

## Result

Unified Verb usage and Noun interactions now keep selected context visibly framed through Grid, columns and Free changes. Case/source inspection remains retained. User accepted reviewed presentation and authorized checkpoint commit. Acceptance does not cover unperformed pointer checks or every-view usability.

Reported brick × bump problem was not just selection storage: existing code retained cell/case IDs, but switched to whole-graph framing and used a thin world-space selected line. Endpoint nodes had no pair-selection emphasis. CPU checks of IDs alone missed visible continuity.

## Contract

- Preserve exact cell/pair, chosen case, inspected source, noun case page and endpoint references across layout, sort and seed changes. Clear held drag only, as before. Prepare candidate before replacing last-good layout.
- When retained node or endpoint pair has graph representation, frame that selection instead of entire graph/workspace after geometry change. Use node footprint union and shown selected route. This changes camera only, not geometry, velocity, filters or query scope. `F` remains full-canvas Fit.
- Outline both selected endpoints in gold at constant screen thickness. Selected Verb usage line uses 4 screen pixels. Drawing consumes same endpoint resolution as framing; no new source or solver data.
- Restore Grid at readable zoom centred on retained cell, blank cell or selected header. Keep cell outline when its source is inspected separately.
- Blank cells retain endpoint context without inventing edge/cases. Inactive links stay hidden unless existing overlay is enabled; endpoints remain outlined and notice explains hidden link. No automatic filter change.
- Existing `view.cell.fit` now labels **Frame selected pair**, available for selected pairs in Grid, columns and Free in both matrix projections. GUI and IPC share operation. Node-only selection disables this pair action; general Focus selection remains separate work.
- Ordinary projection switching keeps independent pane selection, camera, filters and inputs. Existing explicit **Find selected in…** carries qualified source reference, highlights source-associated representations and retains destination selection/filters. These representations are not canonical equivalence. No automatic cross-view selection overwrite or analytical Run.

## Implementation

- `gd_explorer_pair_selection.h`: bounded endpoint resolution and shared presentation bounds/outline. Maximum two endpoints; one for self-pair. No allocations, model enlargement or solver writes. Draw-time lookup at most node count; route scan only when framing.
- `gd_explorer_presentation.h`: retain existing atomic remap and request selection framing after successful matrix geometry replacement. Shared pair-frame action supports graphs without reinterpreting source identity.
- Grid renderers: readable return framing and retained context outline.
- `gd_graph_view.c`: selected edge width in screen units; no change to edge visibility/counts.
- Explicit cross-view Find policy already implemented in `enter_view()` / `matches()`. Retain and test rather than add competing global selection system. Matches prior specification's independent-pane and explicit-follow rules.

## Verification

- CJIT CLI, explorer adapter and graph builds pass without warnings. Core 145/145; focused production/unified/standalone CLI integration 6/6.
- Both examples retain source hashes and release owned memory. Existing matched force/reference and noun convergence checks pass; no solver changes.
- New production-adapter checks use same endpoint resolver and bounds consumed by drawing/framing. Cover Grid/Free/columns, selected source/case and page retention, viewport bounds, node/header remap, blank endpoints where available, and ordinary/explicit cross-view memory.
- Camera framing leaves full raw layout bytes and selection unchanged. Blank context does not enable overlay. Existing allocation-failure checks retain last-good layout and selected context.
- Preserve adapter's final retained-layer JSON after added checks; no stdout contract change.
- CPU checks do not prove pixels, real-pointer behavior or every-view usability. General Focus selection/MDA framing, broad live cross-view review and final compiler matrix remain open.

## Live IPC review setup

Separate Mario viewer opened with authorization: PID17456, session `17456-73b165f9c444c791`, executable/artifacts `native/build/selection-follow-work/`. Discovery found no running viewer before launch; none closed or replaced.

- Selected `nouns/brick` × `verbs/bump`, second case `interactions/bump-brick-tall` (offset1).
- Applied Grid → Free → columns → Grid → Free. Entire selection DTO identical after every change. Source DTO identical before/after sequence and final framing.
- Left Free/Native seed, bounds and One Euro on, settling paused for selection review. **Frame selected pair** applied after pause. Space resumes settling.
- Artifacts: `review-before.json`, `review-after.json`, `review-cases.json`, guarded `review-configure.jsonl`, stdout/stderr logs.

User accepted reviewed presentation and requested checkpoint. IPC state equality does not replace unperformed pixel/pointer checks. Next priority: general Focus selection GUI/CLI action, including readable MDA framing; broader cross-view live review remains open.

## Comments

Review log includes one rejected inspect request: inspect does not accept projection. Rejection occurred before mutation; corrected read, then continued from verified state. No unknown/timeout mutation retried.
