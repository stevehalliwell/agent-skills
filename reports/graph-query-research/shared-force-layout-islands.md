# Connected-island separation

## Contract

- Add **Separate connected islands**, off by default, to all unified and standalone Free/force viewers. GUI/IPC share boolean `island_separation`.
- Compute undirected connected components from retained active non-self edges, not semantic adapter groups, visible subset or inferred relationships. Preserve edge identity, counts, filters, analytical inputs and results. Stacked variant cards remain compound spatial units.
- Recompute axis-aligned island bounds from current node footprints/positions. Draw outlines around shown members using displayed pose; no rendered-coordinate feedback into solver.
- Higher-level force acts between overlapping island bounds. Translate every member by same bounded island displacement; retain existing internal spring/contact solver. Move island attraction anchor offsets with displacement so old anchors do not pull islands back together.
- One component receives no inter-island force. Already-separated components receive no overlap force; ordinary Skills/Verb usage/MDA may therefore change little. Do not assume those graphs contain only one component.
- Use16-world-unit outline padding per side and8-unit clearance between padded bounds (40 units between footprint envelopes). Repel along box-centre direction, with magnitude from overlap/clearance depth; minimum-axis pushes can cancel in crowded rows. Island velocity uses0.8 damping and0.1 force gain. Cap force-layer displacement at6 units/island/step and pair visits at65536. No per-frame allocation. Held island stays fixed by this layer; other islands may move away.
- Existing workspace remains fixed. Clamp whole-island displacement to available workspace room; disclose work/space limits. Soft force does not guarantee non-overlap in every frame or infeasible workspace.
- Enabling resumes settling, keeps camera/selection and does not Run analysis. Pausing stops forces, not outline updates. Disabling stops layer/outlines without teleporting nodes. Reset view/all return toggle to off.

## Acceptance

- Off preserves current solver geometry/velocities. Single-component enabled twins preserve existing node motion.
- Synthetic disconnected overlapping islands separate over bounded steps; active/inactive/parallel/self edges and compound mapping retain exact component behavior.
- Island displacement preserves within-island relative geometry for force-layer application; held island and workspace constraints remain intact.
- Rendering follows displayed pose/visibility without modifying raw geometry. All Free views expose shared toggle; fixed/Grid/similarity views do not apply it.
- Verify both examples and scoped core tests. Human review of Resource islands completed; broader every-view checks remain open. Deep checks/research stay opt-in.

## Implementation and checks

- Shared kernel: `gd_layout_islands.h`, owner-held <=256 KiB bounded state/scratch per layout. Active-edge union/find, current footprint envelopes, bounded sweep candidates and damped box-centre repulsion. No per-frame allocation. Existing node contact budgets unchanged; island layer has its own65536 pair-visit cap.
- Ordinary and compound query layouts use same layer. Query scatter maps each typed card back to spatial-unit component, retaining declared edges/counts/identities. Standalone graph/MDA candidate swaps retain preference and last-good failure behavior; Resource standalone has clickable header toggle.
- Shared renderer unions displayed shown-member positions with raw-local footprints. Uses64 KiB stack scratch; cannot change raw solver state. Deep CPU test translates displayed pose by(+100,-40), hides members, verifies outline envelopes and byte-identical raw layout.
- Core148/148 pass, including new force, single-component/workspace and capacity-budget tests. First-step enabled/off twins establish uniform whole-island displacement; held island remains fixed. Single-component and already-separated twins preserve exact node geometry/velocities. Compound tests run enabled and preserve stack offsets and exact Native restoration.
- Shared-UX checks Crate552 / Mario1793. Every available Free pane exposes toggle, starts off, resumes settling on enable and retains camera/selection/query options/result ownership. Fixed canvases reject editing; Reset view/all disable layer.
- Focused Node13/13 pass, both example source hashes unchanged. Deliberately enabled deep checks for new layer and geometry boundary. Prior Resource/card tests, thresholds and600-step windows retained. New island trial requires zero padded-outline and card overlaps after600 steps. Comparative research not rerun.
- Resource trial starts from existing120-step startup pose with default footprints and workspace enabled. Crate17 islands:22 padded-envelope overlaps initially,0 after600 steps; maximum penetration31.0167 to0.0000 units. Mario168 islands:333 to0; maximum169.1040 to0.0000 units. Both finish island-limited=false and card overlaps0. This does not prove perpetual convergence, pixel readability or all-view behavior.
- Minimum-axis-only repulsion left Mario262 overlapping envelopes /119.9148 maximum penetration after600 steps. Centre-line repulsion with initial weak gain reduced depth but left266 /63.0989. Final0.1 gain /0.8 damping /8-unit outline clearance removes observed overlaps without raising6-unit displacement or65536 pair caps. Intermediate trials not retained as product options.
- Default-off diagnostic timing, alternating same-machine Mario runs: prior build1.782/1.743 seconds; new build1.798/1.791 seconds including process startup. About2% difference in this small sample; no latency guarantee. Existing sub1-second routine budget remains open.
- Artifacts: `native/build/island-separation-work/` contains build logs, `core-final.log`, `final-checks.log`, both shared-UX JSON files and both deep JSON/log files. No authored example edits.
- Previous review viewer PID43208 exited before review preparation; viewer list empty. No closure or replacement performed for this change. User authorized new launch. Opened updated Mario Resources PID21672 / session `21672-d53900cbd5a2a161`, fitted whole graph, left island toggle off for comparison. User reports result is great and authorizes checkpoint commit. Reviewed Mario Resources presentation accepted; no unperformed every-view, standalone-pointer or restart acceptance inferred. Final compiler/platform matrix remains separate.
