# Native query / similarity validation checkpoint

Implementation base: `da9e810`. Automated rebuild/test pass completed before human visual review. No push, source-record changes, experiment reruns or OS input automation.

## Results

- Rebuilt vendored graph UI DLL with installed Zig 0.17.0 and prepared raylib 5.5 SDK at `C:/tools/raylib`. No downloads or vendor edits.
- CJIT 1.2.0: rebuilt headless, plain-view, graph-view and explorer-test executables. Native 127/127 and CLI checks pass. Plain/graph bounded screenshot suites pass.
- Zig C99 `-O2`: native 127/127 and CLI checks pass. Private adapter removes CJIT-only `-q` and forwards shared `build.ps1` source list; local/global caches stay under ignored build output.
- Node 24.16.0: 58 passed, 0 failed, 1 skipped, against each freshly built CJIT/Zig headless executable. Public Attendant and migration integrations enabled. Skipped test requires a desktop-authored draft report; no historical report presented as fresh evidence.
- Public Attendant `validate --strict`: crate 67 records; plumber 225 records. Both return zero diagnostics/corrections. Read schemas/usage first; no managed Markdown changes.
- Explorer owned-window suite passes on private copies of both examples. Eight production modes plus Grid captured per example. Native source and all original/private example hashes unchanged; zero remaining counted allocations.
- Extended window harness renders all nine MDS scopes, including finite Actors, plus outcomes, consistency and full object-binding inspection. Exercises pending-candidate failure, allocation failure, reload and reset. Ordinary pose/camera preserved during MDS rendering. Explicit test camera; no synthetic mouse/key events or claim about interactive Fit/picking.
- Independent numerical, adapter, pair/source and 75 rule/binding helper checks remain supporting evidence. Bounded windows do not replace real-pointer/manual usefulness review.

## Corrections found during full pass

- Restored required `Inspection topics:` label in Resource guide without changing corrected semantics.
- Updated legacy window assertion from `view.root` to actual shared `skills.selector` pre-window validation.
- Updated documented alternate-compiler source list for named/similarity modules.
- Catalogue now distinguishes completed bounded window/render checks from deferred real-pointer/manual usefulness.

## Evidence

Base: `skills/game-design-workbench/native/build/` (ignored local artifacts).

- `full-ui-rebuild.log`
- `full-cjit-plain-tests.log`, `full-cjit-view-tests.log`
- `full-zig-tests.log`, `full-node-tests.log`, `full-zig-node-tests.log`
- `full-crate-schema.json`, `full-plumber-schema.json`
- `full-crate-validation.json`, `full-plumber-validation.json`
- `full-explorer-features.log`
- `explorer-checks-6df8644a359d43618a809ca8b343bbe2/`: both examples, source hashes, state reports, screenshots; `similarity-0.png` through `similarity-8.png`, `rule-outcomes.png`, `rule-consistency.png`, `object-bindings.png`.

Rebuilt review executable: `skills/game-design-workbench/native/build/full-validation/gd-view.exe`. Adjacent rebuilt `gd-ui.dll` and prepared `raylib.dll` included.

## Human review gate

1. Typed related-path comparison: explicit endpoints, kind union/direction, original source witnesses, unfinished scope.
2. MDS: choose typed features/weights, inspect rows/pairs/missing entries, pan/zoom/Fit and return to ordinary canvas. Coincident points are not separated artificially; check overlapped labels and sidebar clipping/readability.
3. Resources: Object bindings / rule checks modes, exact filters, source navigation, applied/pending distinction and canonical case outlines. Conditional proposals are not gameplay execution or Resource flow.
4. Actors/Effects: inspect distinct local identities, finite hypotheses, proposal source and filters without treating declarations as applicability.
5. Switch/reset/refresh/Undo and independent pane/camera behavior with actual pointer. Preserve source records and drafts.

Resource layout remains unaccepted and unchanged. All-14 original-question usefulness, pre-UX capability assessment, fresh offline-history reports and IPC remain separate work; no blanket conformance or usability verdict.
