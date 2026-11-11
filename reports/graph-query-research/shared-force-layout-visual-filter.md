# Render-only One Euro filter

Status: implemented and CPU-checked. Unified explorer Free graphs default to filtered rendering. Workspace width and height increased **20% each** after user feedback. Spring lengths unchanged; shared degree/footprint-aware spring rule is next experiment recorded in root `todo.md`. User accepted reviewed bounds/filter presentation and authorized checkpoint, then shared spring-length experiment. No solver-convergence or every-view acceptance implied.

## Behavior

- Shared GUI checkbox **Visual position filter (One Euro)**; IPC presentation boolean `visual_filter`, default true. Immediate toggle, no Run, source writes or changes to selection/pause/query inputs.
- Apply only to prepared Free graphs, including Actor interaction overview. Keep Native constrained layouts, Grid/columns, fixed Usage cloud, similarity canvases and distance-focus animations unchanged. Legacy standalone windows retain existing unsmoothed behavior.
- Solver positions and velocities remain authoritative. One Euro reads them into separate presentation layout. No smoothed value enters force integration, springs, contacts or workspace containment. IPC node positions remain raw solver positions.
- Cards/nodes, links, labels, witness/provenance overlays, selection marks and picking share displayed pose. Renderer measures physical text/badge footprints at **raw integer anchors**, preventing filtered sub-pixel rounding from changing collision geometry.
- Compound variant stacks share anchor translation, not independent filters. Preserve relative offsets. Held node/compound retains visible pickup offset and follows pointer deltas without smoothing lag. Clicking does not teleport solver node or change velocity. Release resumes filtering toward raw pose.
- Pausing stops solver; visual pose can finish catching up. After release outside workspace, raw containment occurs on next active solver step; filtered return may briefly lag outside border. Filtering does not guarantee collision-free display or solver convergence.

## Filter and lifetime

Independent One Euro X/Y channels in layout units; elapsed seconds from frame time:

```text
minimum cutoff = 1 Hz
derivative cutoff = 1 Hz
beta = 0.005
alpha(cutoff, dt) = 1 / (1 + 1 / (2*pi*cutoff*dt))
derivative = low_pass((raw - previous_raw) / dt, derivative_cutoff)
cutoff = minimum_cutoff + beta * abs(derivative)
shown = low_pass(raw, cutoff)
```

Trial defaults, not tuned usability claims. [Algorithm and tuning guidance](https://gery.casiez.net/1euro/), Casiez, Roussel and Vogel, CHI 2012. No external library or copied implementation.

- One viewer-owned bounded scratch allocation: **2,261,152 bytes**. Copy active geometry/routing plus fixed layer table; do not copy collision scratch. No per-frame allocation or solver cloning in production.
- Reset on pane/geometry identity replacement, key/label change, tick rollback, explicit Fit/reframing, filter disable/re-enable, unsupported presentation and invalid or longer-than-250-ms frame interval. Clamp positive sub-millisecond intervals to 1 ms.
- Input uses previous displayed pose. Step raw solver, then advance filter and draw. Never temporarily overwrite raw coordinates for drawing. Failed pose preparation falls back to raw drawing with message.

## Measurements

Production-adapter scenes, fixed 60 Hz filter sampling, 4096 solver steps, final 256-step window. Adapter-only footprints, not GPU/font/annotation rasterization. Both examples retain 190 topology/motion JSON records total; motion records now include `visual_rms` and `visual_max_step`.

Mario Effects, raw versus displayed RMS in world units/step:

- Unbounded Native: **2.452279 versus 0.462785**. Maximum step **22.421452 versus 1.923667**. Raw overlaps remain **4**.
- Unbounded Neighborhood: **0.692194 versus 0.175581**. Maximum step **3.810456 versus 0.682755**. Raw overlaps remain **0**.
- Enlarged-bounds Native: **2.655957 versus 0.325746**. Maximum step **23.290053 versus 2.565699**. Raw overlaps **4**.
- Enlarged-bounds Neighborhood: **1.529184 versus 0.157472**. Maximum step **12.800049 versus 1.315864**. Raw overlaps **0**.

Unbounded raw traces retain previous measurements. Enlarged bounds intentionally change bounded solver traces; do not attribute that change to filtering. Lower displayed RMS is not proof of perceptual quality or convergence. Crate Effects remains near rest; filter may retain tiny catch-up motion.

## Verification

- CJIT CLI, core, explorer-adapter and graph builds pass without compiler warnings.
- Core **144/144**. Filtered/unfiltered solver twins remain byte-identical for **600 steps**, including positions, velocities and collision metadata.
- Synthetic alternating input: squared displayed motion **1.256525**, raw **480** over final 120 steps. Ramp lag bounded; compound offsets retained; held anchor bypass, resets, invalid anchors and alias rejection checked.
- Shared footprint tests compare filtered-anchor measurement against ordinary raw-anchor measurement byte-for-byte at scales **1 and 3**.
- Real query fixtures verify supplied-pose hit ordering, compound anchor identity and compound dragging. Native restoration and exact typed cards/edges/source ownership remain covered.
- Focused CLI/both-example production-adapter tests **5/5**. Presentation toggles retain geometry, pause and selection; bounded profiles retain containment; original noun regressions pass. Source hashes unchanged and owned allocations released.
- Artifacts: ignored `skills/game-design-workbench/native/build/visual-filter-work/`: `build-final.log`, `core.log`, `adapter.log`, `mario.{json,log}`, `crate.{json,log}`, `measurements.jsonl`, new `gd.exe` and `gd-view.exe`.

## Next

Review enlarged bounds and filter on/off before changing spring lengths. Then test shared endpoint-footprint/connectedness rest-length rule recorded in root tracker. Keep raw motion, displayed jitter, overlap and drag response separate.

## Comments

User authorized separate updated review viewer: **PID 32508**, session **`32508-51eddb3f34ccc797`**. Confirmed Effects / Free / Neighborhood, enlarged workspace enabled, `visual_filter=true`, unpaused and Fit applied. User reviewed this viewer, found presentation "actually pretty good now", and chose to keep it. No existing window replaced or closed, source records edited, commit made or push performed. Final compiler matrix and broader visual acceptance remain open. Original bounds measurements belong to [first workspace trial](shared-force-layout-workspace.md), before 20% enlargement.
