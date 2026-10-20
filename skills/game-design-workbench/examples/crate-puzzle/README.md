# Crate-and-switch workbench example

Open this folder as project. Delivered folder contains 68 source records across 19 collections, deliberately incomplete design, and local `tools/game-design-workbench/` snapshot. Requires Node.js `>=24.10.0`; installed `attendant` skill required; no service, compiler, or display. Load Attendant and set `GD_ATTENDANT_SKILL` to its absolute skill directory. Workbench does not bundle Attendant. Tutorial is not a complete game or executed simulation.

## Start

Set dependency path from loaded Attendant skill (not this example):

```sh
export GD_ATTENDANT_SKILL="/absolute/path/to/attendant"
```

PowerShell: `$env:GD_ATTENDANT_SKILL = 'C:/absolute/path/to/attendant'`.

Load `tools/game-design-workbench/SKILL.md` in your agent, or run from this folder:

```sh
node tools/game-design-workbench/scripts/workbench.mjs capabilities --json
node tools/game-design-workbench/scripts/workbench.mjs validate --json
node tools/game-design-workbench/scripts/workbench.mjs query skills --fields name,rank,prerequisites --json
node tools/game-design-workbench/scripts/workbench.mjs query matrix --json
node tools/game-design-workbench/scripts/workbench.mjs resolve --term "  BOX " --json
```

Canonical package also bundles tutorial source without recursively copying installed tools. If this example copy has no `tools/`, use `../../scripts/workbench.mjs` for the commands above, or request setup proposal/apply using that helper. Do not pretend absent tools are installed.

Actual source: `.pi/attendant.tables` and `game-design/<role>/`. Installed tool's own `examples/` is reference material, not this model's configured source. Make a folder copy before experiments if you want to retain the original tutorial unchanged.

## Walkthroughs

1. **Cases, not a single cell.** Inspect matrix and three crate/push case files: free push, wall-blocked push, low-energy push. Blocked actions have defined feedback/no movement. `wall-activation` is `not_applicable`; `closed-exit-walk` is `unknown`. Do not invent remaining combinations or repair unknowns during inspection.
2. **Partial authored order.** Query six skills. `move` rank 0; `push` 1; `plan-push` null; independent `read-switch` root 3; `route-switch` 3; `coordinate` 2 with AND prerequisites plan-push/route-switch. Rank is not graph depth or physical room. No automatic rank diagnostic is claimed.
3. **One requested edit.** Ask `Change coordinate rank to 3, preserve every other authored field/rank`. Use Attendant update or exact source edit from capture instructions. Query again; compare full source diff. Only selected rank should change semantically; null plan-push and both rank-3 switch skills stay unchanged. Restore selected field only if requested; never reset unrelated work.
4. **Declared routes.** Query routes. Entry move-demo leads to push-room; push-room can go to switch-practice or directly to combined-room. Ask whether bypassed practice has intentional support. This is a question about structural learning opportunities, not proof of impossible completion.
5. **Aliases and memory.** Resolve box → crate; shove → push. `shoves` stays unknown. Query decisions: pulling remains rejected with rationale; crate-color stays proposed. Asking for new options must surface relevant rejection and retain distinct new proposals without adoption.
6. **One selected lens.** Ask `Review combined-room using skill-atoms; do not change source`. Read only that pack plus operation/model source. Discuss rank/prerequisite intent and bypassed practice as agent questions. Do not run all packs, change ranks, or claim deterministic C99 checks. For Bartle, report limited applicability to this solo puzzle rather than inventing multiplayer.
7. **Evidence boundary.** Inspect controls, resources, experiment route-cost, expectation route-time, and empty observations. Energy identity is separate from initial/capacity tuning and spending rule. Formula text is not evaluated. Proposed timing model is not observed play or executed evidence.
8. **Save a dated review.** Capture snapshot before inspecting. Write findings input outside canonical collections, then save with selected lens and snapshot. Reopen report; `historical: true`, `freshly_run: false`. After an authorized rank edit, source match becomes false; old report remains unchanged.

Saved history retains original review and dependency-corrected review. `review-baseline.json` is original historical snapshot; `review-baseline-installed-attendant.json` captures required-skill packaging. Inspect dated `game-design-results/` directories to find saved runs. Original review stays stale after dependency correction; never rewrite it to match new tools. Capture a new uniquely named baseline before any new review.

For command/schema details, read local skill's `references/cli.md`, `references/review.md`, and `references/capture.md`. Example report finding uses coordinate stable ID `01a15175-3c00-7529-8fb6-09255d506737`, source `game-design/skills/coordinate.md`, field `rank`, origin `agent`, kind `unknown_unresolved`. Do not present this manual interpretation as native analysis.

## Timing reference, not execution

Fixed illustrative route: 4 walks, 3 successful pushes. Base time 5 s with authored durations. Assumed independent uniform delay 0–1 s per action gives range 5–12 s; analytical probability of at most 8 s is `1312/5040`, about `0.2603`. Delays model hypothetical player behavior, not randomness in puzzle rules. No trial counts, distributions, measured success rates, or participant observations have been generated.

Simulation, temporary control overrides, exports, native graphs/views, and automatic cycle/rank/route diagnostics belong to later slices. Opening or reviewing this folder never compiles/runs a simulation or creates a Git checkpoint.
