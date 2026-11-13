# Overview visibility

## Contract

- Open adjacency/incidence overview by default on existing supported graph views: Skills, Challenges, Relationships, Verb usage graph layouts, MDA and Resources. Existing standalone graph/MDA/Resources windows also start open.
- Float in upper-right canvas. Reserve no graph space; add 24-pixel header above existing content. No dragging/resizing feature added. Keep at most 288-pixel matrix side and 32 identity bins per axis.
- Click header or trailing `[-]` to collapse; compact `Overview [+]` header remains to reopen. Both states keep same top/right anchor. Expanded panel consumes its full pointer region; collapsed state consumes only header. Tiny canvas retains header where space permits without erasing expanded preference.
- Reuse existing per-pane `overview.enabled` and public `overview_enabled` control. Bool now controls expansion; sidebar checkbox says Expand. No new IPC schema, preference file or stored investigation state.
- Each pane remembers expansion during session. New pane/session starts open; Reset view and Reset all restore open. Failed Reset view retains previous expansion and last-good presentation.
- Grid and similarity modes hide overview without overwriting graph expansion preference. Selecting another node never reopens collapsed overview.
- Header toggle changes only expansion: no camera/viewport changes, reseed, source edits, selection replacement, history edits or Run of pending analytical inputs. Expanded overview passively derives existing shown-projection bins/counts; this is not an analytical query Run.
- Collapse retains selected-cell graph highlights/evidence and gold header indicator; expansion is not brush enablement. F frames selected endpoints plus primary selection; Escape / Clear selection clears brush explicitly. [Framing and clear contract](selection-framing.md).
- Preserve exact projected link-row counts, linear cell-fill scale, cell selection and evidence. No new case/record aggregation or added view coverage. [Selected-node bin cue](overview-node-cue.md) is separate render-only follow-up. Overview is topology, not spatial minimap or similarity distance.

## Checks

- CJIT production graph and CPU explorer adapter builds: no errors/warnings. Focused Node 9/9: shared UX, current-view Find and production IPC on both examples; source hashes unchanged.
- New shared-UX checks cover fresh expanded defaults; header collapse/reopen and input bounds; stable top/right anchor; short/tiny canvas; independent pane state; repeated pane preparation; selection without reopening; unchanged raw layout/camera/query inputs/fit on toggle; Verb usage Grid round trip; Reset view/all; injected failed Reset view retention.
- Artifact directory: `native/build/overview-visibility-work/`, including `build-graph.log`, `build-adapter.log`, `checks.log`. Transport reuses checkpoint CLI in `noun-labels-work/`.
- Core/deep research/compiler matrix not rerun for UI expansion change. Physical pointer, whole-app restart and standalone visual acceptance remain open. User authorized separate Mario Resources review window: PID32248 / session `32248-92dff957c52f8b00`, executable `native/build/overview-visibility-work/gd-view.exe`. No existing viewer closed or modified. Initial review required selection/framing corrections. User accepts corrected Mario Resources behavior in PID36028 and authorizes checkpoint; see selection-framing follow-up. No every-view, restart or standalone acceptance inferred.
