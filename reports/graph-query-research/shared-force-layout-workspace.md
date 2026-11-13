# Workspace-bounds trial

Status: opt-in shared containment implemented and CPU-checked; default remains off. First trial contains Effects but increases motion/overlap. User finds bounds promising and requests slightly more room. Follow-on pass increases width/height 20% each and adds [render-only One Euro filtering](shared-force-layout-visual-filter.md). Neither is a convergence claim or blanket visual acceptance.

## Contract

- User requested bounds first, then position filtering. Possible filter: per-node One Euro. Filtering must never modify solver positions or velocities.
- Shared GUI/IPC presentation boolean: `workspace_bounds`. Checkbox: **Workspace bounds (trial)**. Available on prepared Free graphs, including Actor interaction overview; unavailable in Native constrained layouts, Grid/columns, fixed Usage cloud and similarity canvases.
- Toggle changes workspace metadata and requests Fit. It does not move nodes immediately, unpause, apply pending inputs, Run a query, change evidence or change selection.
- Keep preference when changing layout; apply only in Free mode. Explicit reseed, geometry replacement or re-enable recomputes workspace. Selection, pan, zoom and viewport resize do not.
- Draw fixed workspace border. Updated framing policy: `F` fits selections, otherwise shown node footprints; workspace remains solver container and is excluded. [Selection framing follow-up](selection-framing.md).
- All nodes remain movable. Pointer owns held node, even outside workspace. Release returns it inside on next active solver step; paused settling defers correction. A compound variant stack moves and clips as one spatial unit.

## Workspace sizing

Use current spatial-unit footprints, not record importance or runtime frequency:

```text
area = sum(footprint_width * footprint_height)
spring_density = 2 * active_nonself_springs / node_count
slack = 4 + min(4, spring_density)
height = max(
    sqrt(area * slack / aspect),
    maximum_footprint_height + 16,
    (maximum_footprint_width + 16) / aspect,
    initial_footprint_bounds_height + 16,
    (initial_footprint_bounds_width + 16) / aspect
)
height = height * 1.2  # Follow-on review adjustment; first trial omitted this.
width = height * aspect
```

Center rectangle on prepared footprint bounds. Including current pose avoids activation teleportation; poorly spread Native seeds can therefore produce large workspaces. Aspect uses viewport, with 1:1 fallback. No per-frame resizing or recentering.

After existing integration and contact separation, project each unheld footprint inside rectangle. Remove only velocity pointing out of a contacted wall. Do not pin nodes or replace solver/group forces. Boundary correction is separate from contact movement budget; release from a distant drag can snap back. Invalid setup retains prior workspace. A future footprint larger than rectangle is centered but cannot fit fully.

Setup costs O(nodes + edges); containment costs O(nodes). No per-frame allocation. Disabled workspace preserves previous solver behavior.

## First-trial findings, before 20% enlargement

Matched reference protocol: both examples, Native/Neighborhood, samples 0/120/600/4096, final 256-step motion window, aspect 1.4, adapter-only footprints. Existing Resource startup and unweighted Effects controls retained. Expanded diagnostic emits 19 topology + 76 motion records per example: 190 total.

Mario Effects, unbounded versus bounded:

- Native RMS: **2.452279 versus 4.952074** world units/step. Maximum step: **22.421452 versus 24.997364**. Centroid drift: **85.536084 versus 80.442627** world units. Final overlaps: **4 versus 9**. Two nodes within 1 world unit of boundary.
- Neighborhood RMS: **0.692194 versus 1.139660**. Maximum step: **3.810456 versus 10.440859**. Centroid drift: **27.339965 versus 9.069275**. Final overlaps: **0 versus 2**. Three nodes within 1 world unit of boundary.
- All bounded samples have zero outside nodes and zero measured overflow. Outside-node counts ignore overflow at most 0.01 world units for float roundoff.
- Lower Neighborhood centroid drift does not mean lower internal motion. Contact correction followed by clipping can create overlaps; walls can crowd nodes.

Other references:

- Mario Verb usage Native RMS increases **0.203590 to 0.712840**; three boundary nodes, no final overlaps. Neighborhood unchanged: **0.004444**.
- Mario Resources nearly unchanged: Native **0.099441 versus 0.099434**, Neighborhood **0.007337**. No final overlaps.
- Mario Noun interactions unchanged: Native **0.015804**, Neighborhood **0.001575**, no final overlaps. Existing unbounded noun regressions remain enforced.
- Crate bounded tails match corresponding unbounded controls; no final overlaps or boundary contacts.

### Rejected force-only stage

Initial soft restoring forces did not contain Effects. Native maximum overflow reached **295.833984** world units; Neighborhood **1053.233154**. Collision correction can outrun integrated restoring force. Removed soft-force hook; retained explicit post-contact containment for optional review. Do not infer that higher attraction would solve overlap or jitter.

## Checks and evidence

- CJIT CLI, core, explorer-adapter and graph builds pass.
- Core **143/143**; focused CLI/both-example adapter checks **5/5**.
- Added API checks: area/aspect, unchanged activation geometry, invalid setup retaining last-good workspace, post-contact containment, held-node exemption, outward velocity removal, fixed workspace during stepping, disable and unsupported modes.
- GUI/IPC checks preserve pause, selection and node geometry while toggling; query preparation/reseeding preserves enabled preference, compounds and typed source/evidence. Native restore clears active workspace. Failed geometry candidates retain prior scene.
- Matched probes check source checksums, finite geometry, fixed workspace, contact work bound, ownership and default noun-motion regressions. Node checks validate profile identities and bounded sample overflow.
- Artifacts: ignored `skills/game-design-workbench/native/build/workspace-bounds-work/`: `build.log`, `core.log`, `adapter.log`, `mario.{json,log}`, `crate.{json,log}`, `measurements.jsonl`, and `*-soft.log`.
- CPU checks opened no viewer. After user chose live review, opened separate Mario viewer **PID 7300**, session **`7300-3a70463101b454c4`**: Effects / Free / Neighborhood, `workspace_bounds=true`, unpaused, Fit applied. GUI/IPC state confirmed; user acceptance pending. Existing windows untouched; no source record edited, commit made or push performed.

## Next

1. Review enlarged bounds and implemented [visual filter](shared-force-layout-visual-filter.md); keep bounds off by default. Do not claim Effects solver repair.
2. Next experiment: shared degree/footprint-aware spring lengths, recorded in root tracker. No spring change in filter pass.
3. Compare raw motion, visible jitter, responsiveness and overlap independently. A quiet rendering is not solver convergence. Keep dense-hub contact/pass/movement diagnosis open.

## Comments

CPU footprints omit final renderer/font/annotation unions. RMS measures total motion, not perceptual jitter alone. Results do not establish visual usability, GPU behavior or a durable bounds/default policy. Baseline comparison and earlier hypotheses remain in [reference-layout review](shared-force-layout-reference-review.md).
