# Selection framing and clear

## Contract

- Collapsing overview hides panel only. Retain selected bin, exact contributing-node/link highlights and evidence. Gold dot in header indicates active brush, including while collapsed. Clear brush explicitly, not by minimizing.
- `F`, Frame button and presentation action `view.fit` frame current selection. Union primary selection with active overview contributing endpoints; do not choose one at expense of another. Use visible node footprints and existing padding, not workspace or island containers. Keep existing zoom limits; no minimum-readable-zoom clipping for this exact fit.
- With no selection, frame all shown node footprints. Exclude workspace, island bounds and route-only extents. Workspace remains solver container, not information to frame. Initial all-node fits follow same rule.
- Support selected node, case/pair endpoints, represented source records, MDA node/link, overview brush and similarity entry pair. Hidden/unrepresented/empty-brush selection does not reveal data or silently frame unrelated graph. Grids frame selected cell/header; similarity retains separate camera and no topology interpretation.
- Existing Focus selection button remains readable selection/direct-context action. `F` instead fits whole selected set, even when spread widely. Neither action Runs pending analytical inputs or writes solver geometry.
- Standard clear: Escape, **Clear selection (Esc)** button, presentation action `view.selection.clear`. Active pane only. Clear primary/source/pair selection, overview brush, MDA inspection/path marks and similarity row/peer inspection. Clear selection-associated source Find reference, not Find search text or filters.
- Clear preserves camera, layout, overview expansion, analytical inputs/results, query-selected values, draft edits and Undo/Redo. Hold distance-focus pose until next selection rather than retargeting layout on clear. Typed selection history follows existing empty-selection behavior; no new overview-brush history format.
- Escape dismisses active Find/text/popup before clearing selection. Pending rank edits keep existing application-action guard. Standalone graph/MDA/Resources also use Escape to clear rather than exit, and F frames primary plus brush before falling back to node bounds. Normal window close remains available.

## Examples

- Select overview cell with several contributing nodes, collapse panel: graph highlights remain, header dot stays gold. F fits contributing nodes. Select another graph node: F fits union. Reopen preserves cell.
- Clear selection: highlights/brush disappear without moving camera. F now fits shown nodes. Enlarging workspace alone does not change fit.
- Clearing Resources selection does not clear another pane or run staged analytical forms. Escape while typing or dismissing popup does not also clear canvas selection.

## Checks

- Warning-free CJIT production graph, CPU adapter and core builds. Core 148/148; focused Node 9/9 on both examples, unchanged source hashes. Deep research/compiler matrix not rerun.
- Core regression retains brush endpoints/counts with panel collapsed. New shared-UX checks exercise collapsed brush plus extra primary node, exact union framing, shared standalone framing helper parity, no geometry writes, camera-preserving clear, held distance-focus pose, no-selection node fit despite enormous workspace and IPC `view.selection.clear` dispatch.
- Existing Focus empty-selection fixture now explicitly clears overview brush too; no old zoom/readability thresholds weakened.
- Artifacts: `native/build/selection-framing-work/{build-graph.log,build-adapter.log,build-core.log,core.log,checks.log}`. Existing transport CLI remains `noun-labels-work/gd.exe`.
- User accepts reviewed Mario Resources interaction behavior: collapse retains highlights, F frames selected information, Escape clears selection and F then frames nodes rather than workspace. No whole-app restart, standalone or every-view acceptance inferred; existing review viewer untouched. User authorized separate Mario Resources review window: PID36028 / session `36028-bdd4b14098cf4bf6`, executable `native/build/selection-framing-work/gd-view.exe`. No existing viewer closed or modified. User reports updated behavior is great and authorizes checkpoint.
