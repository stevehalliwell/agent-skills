# Shared footprint/fan-out spring spacing

## Result

Shared Free rule implemented after accepted bounds/filter checkpoint `3bd4645`. User reviewed Effects presentation, chose to keep it and authorized checkpoint. Acceptance covers reviewed presentation, not solver convergence or every view.

Mario Effects final sampled overlaps fall from 4 to 0 for Native seed. Enlarged-bounds Neighborhood raw/displayed RMS roughly halve. Motion persists; contact-pass limits remain active. Verb usage motion is not uniformly better.

## Rule

Use physical spatial-unit footprint and distinct active neighbors, not authored classification, record frequency or semantic connectivity metadata.

```text
neighbors = distinct active non-self neighboring spatial units
fan_out = clamp(neighbors, 1, 64)
radius = 0.5 * hypot(footprint_width, footprint_height)
         * sqrt(1 + (fan_out - 1) / 8)
ideal_length = max(previous_directional_footprint_target,
                   radius_a + radius_b + 8)
```

- Each extra neighbor adds one eighth of base exclusion area. Radius grows with square root; factor caps below 3 at 64 neighbors. Constants remain trial choices.
- Larger footprint increases radial demand. More neighbors increase demand until cap. Previous 110-unit floor and adapter `spring_gap` remain directional minima; do not add custom gap a second time to radial demand.
- Count unordered endpoint pairs once if any parallel/reverse edge is active. Ignore self-loops. Recompute counts each active Free step so active-mask changes apply without reseeding.
- Compound variants remain one physical unit. Edges inside same compound do not create fan-out. Typed records, evidence and declared-count semantics remain unchanged.
- Preserve weighted spring strength, repulsion, damping, speed/contact/work limits, centre attraction, workspace and render-only filter. No pinning, freeze, extra warmup or budget increase.
- Apply through shared solver, including standalone Free graphs. Directed/column layouts and fixed Usage cloud do not use spring rule. Native/Neighborhood here identify Free seeds, not constrained Native layouts.

`gd_layout_springs.h` reuses prepared routing's sorted unordered pairs. Added work: O(nodes + edges), no sorting or allocation per frame. Fixed derived count scratch: 16,384 bytes per `GD_Layout`. Render pose now 2,277,536 bytes; count scratch is not copied into presentation geometry. Authored `GD_LayoutNode.degree` remains unchanged.

## Matched evidence

Compare accepted bounds/filter build `native/build/visual-filter-work` with candidate `native/build/degree-spacing-work`. Same two private example inputs, seeds, prepared footprints, aspect 1.4, workspace policy, 4096 steps and final 256-step window. Visual samples use fixed 60 Hz; positions/velocities remain raw. Retained 190 topology/motion records in candidate `measurements.jsonl`.

Mario Effects, enlarged bounds:

- Native raw RMS: 2.655957 to 1.036995. Displayed RMS: 0.325746 to 0.118934. Raw max step: 23.290053 to 12.396742. Final overlaps: 4 to 0.
- Neighborhood raw RMS: 1.529184 to 0.783144. Displayed RMS: 0.157472 to 0.079408. Raw max step: 12.800049 to 7.100082. Final overlaps stay 0.
- Both still reach contact-pass limit in all 256 tail steps. Containment checks pass. No convergence claim.

Mario Effects, unbounded:

- Native raw RMS: 2.452279 to 1.036995; final overlaps 4 to 0.
- Neighborhood raw RMS: 0.692194 to 0.521877. Raw max step worsens from 3.810456 to 6.172188. Lower average motion is not uniform improvement.

Other references, unbounded:

- Mario Resources cold Native raw RMS: 0.099441 to 0.005485; Neighborhood: 0.007337 to 0.002774. Final overlaps stay 0.
- Mario Verb usage Native raw RMS worsens from 0.203590 to 0.219190; Neighborhood from 0.004444 to 0.013970. Neighborhood centroid drift worsens from 0.856520 to 2.587090. Final overlaps stay 0. Include Verb usage in visual review.
- Mario noun Native: min linked centres 171.989, RMS 0.010134, max step 0.031230, drift 0.436512. Neighborhood: 192.359 / 0.000528 / 0.001348 / 0.026744. No final overlaps. Existing gates remain unchanged: centres at least 128, RMS at most 0.05, max step at most 0.2, drift at most 1.
- Crate noun measurements unchanged: Native RMS/drift 0; Neighborhood RMS 0.000081, max 0.000195, drift 0.001206. Existing gap absorbs radial target at this fan-out. Other Crate references remain low-motion without final overlaps.

## Verification and review boundary

- CJIT CLI, core, explorer adapter and graph builds: no compiler warnings.
- Core: 145/145. New checks cover distinct-neighbor counting, active-mask changes, reverse/parallel links, loops/isolates, footprint growth, symmetry, degree cap, maximum admitted geometry, preserved authored degree and actual outward force for vertically arranged wide cards.
- Updated old single-step custom-gap assertion to exact new expected displacement. Default circumscribed footprint target already exceeds old 110-unit target, so old greater-than-4 delta no longer describes rule. Retain custom-gap settled spacing check and every noun convergence gate.
- Filter isolation twins still byte-identical for 600 steps. Raw-anchor footprint and compound/typed picking checks pass.
- Focused production/unified integration: 5/5; both example inputs/source hashes unchanged. Both direct real-example adapters pass and retain stdout JSON contract.
- User authorized separate review viewer: PID `16372`, session `16372-a18f22842d6838ad`, executable `native/build/degree-spacing-work/gd-view.exe`. Confirmed Effects / Free / Neighborhood, workspace bounds and visual filter on, unpaused, Fit applied. Guarded presentation-only IPC; no analytical Run. No previous session discovered before launch; no windows replaced or closed. Logs/configuration retained beside binary.
- User accepted reviewed Effects presentation: "its fine, lets keep it", and authorized checkpoint. Full compiler matrix, Verb usage regression review and broader every-view acceptance remain pending.

Next: finish remaining Actor overview work, including requested Usage cloud Force option. Retain accepted Effects presentation; do not reopen solver tuning without new usability evidence. Include Verb usage in later regression review. Do not treat lower CPU RMS as convergence proof.

## Comments

Stronger full `sqrt(neighbors)` radial demand failed Crate noun RMS or drift gates. Removed, not hidden by relaxing gates. Final conservative area-growth rule retains those gates; it is not proof of optimal parameters or Effects root cause.
