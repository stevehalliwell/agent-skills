# Noun-interaction link count encoding

## Contract

- Improve count distinction in Noun interactions Free graph. Existing count coloring/thickness is present but subtle; replace rendering, not count extraction.
- User chose relative scale from all retained positive noun-pair case counts. Camera, zoom, selection and clipping never change range. Same count may look different in another graph; legend discloses full-graph range.
- Use existing linear positive-count fraction: minimum 0, maximum 1; equal positive bounds 0.5. Width `2 + 6 * fraction` screen pixels, converted through zoom for drawing. Count<=0 has no populated-link style.
- Brighter blue-to-teal strokes use same fraction. Keep grid/cell palette and existing dark count-label backgrounds for white text contrast. Shared normalization does not require identical fill/stroke RGB.
- Matching link legend shows actual-count line samples at minimum, integer middle and maximum, omitting duplicates. Equal counts show one sample; empty graph says no populated links. Counts remain distinct declared cases, not records or runtime frequency.
- Preserve numeric labels and selected-link gold halo. Normal gray underlay must not wash out count hue. Keep self-loops, exact typed selection/evidence and independent panes.
- No change to spring weights, geometry, node styles, grids, Actor overview record-count links, Verb usage, filters or analytical Run. No new preference, dependency or IPC schema.

## Example and acceptance

- Retained counts1..4: count 1 is muted/thin 2 pixels, count 4 bright/thick 8 pixels, count 2 width 4 and count 3 width 6. Legend and strokes share helper; labels retain exact integers.
- All counts 4: all links share midpoint color/5-pixel width; legend says all links 4.
- Removing item from camera view or changing selection leaves every count's style unchanged. Replacing retained graph may change range.
- Verify empty/equal/range/large counts, bounded monotonic color/width, selection halo and explicit metric. Focused source-read-only checks on both examples; new-build human comparison required. No convergence or platform acceptance implied.

## Checks

- CJIT production graph and CPU adapter builds: no errors/warnings. Focused Node 6/6: shared UX and production IPC on both examples, with unchanged source hashes.
- New shared-UX checks cover empty/zero styles, equal midpoint, distinct legend samples, range 1..4 widths/colors, monotonic brightness, large integer bounds and unchanged cell-fill endpoints. Legend and routed strokes call same style helper; count range still comes from full retained pair array before camera drawing.
- Artifacts under `native/build/link-counts-work/`: `build-graph.log`, `build-adapter.log`, `checks.log`. Transport reuses checkpoint CLI in `noun-labels-work/`.
- Core, deep layout research, compiler matrix, GPU readability and pointer acceptance not rerun. User authorized separate Mario Noun interactions Free review viewer: PID47096 / session `47096-15919711364f94ad`, executable `native/build/link-counts-work/gd-view.exe`. No existing viewer closed or modified. User reports reviewed presentation looks great and authorizes checkpoint. Acceptance covers reviewed Mario Free link styling, not unperformed every-view, pointer or platform checks.
