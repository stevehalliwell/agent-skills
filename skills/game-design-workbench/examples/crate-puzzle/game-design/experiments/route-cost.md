---
{ "id": "01a15175-3c00-7e68-8f3b-b760984a9a2d", "name": "route-cost", "created_at": "2026-10-19T00:00:00.000Z", "desc": "Executed 1000-trial route timing baseline; 245 finished within 8 s", "tags": [], "status": "proposed", "stage": "completed", "experiment_kind": "simulation", "control-vars": [ "control-vars/tile-distance", "control-vars/walk-speed", "control-vars/walk-duration", "control-vars/push-duration", "control-vars/initial-energy", "control-vars/energy-capacity", "control-vars/push-cost" ], "mechanics": [ "mechanics/grid-walking", "mechanics/crate-pushing" ], "interactions": [], "dynamics": [], "aesthetics": [], "program": experiments/route-cost/route_timing.c, "inputs": experiments/route-cost/frozen-input.json, "results": game-design-results/route-cost/20261024T193020Z-baseline-speed-2 }
---
# Question

What is route completion time under assumed delays?

## Hypotheses
No player-behavior hypothesis supplied for this timing model. Energy/planning investigation is separate.

## Measures
These names are local to this experiment; values belong in results.

- elapsed-time: value_type number; unit s; allowed_values []. Elapsed attempted route time, even if route stops early.
- completion-time: value_type number; unit s; allowed_values []. Time to finish declared route; undefined with reason if route is not completed.
- route-complete: value_type boolean; unit empty string; allowed_values []. Whether all seven prescribed actions complete.
- remaining-energy: value_type integer; unit energy; allowed_values []. Energy at trial termination.

Clock starts before first delay, includes all seven delays and action durations, and stops when final successful push completes. This is an experiment-local assumption, not an adopted game rule. Interrupted routes retain elapsed time and energy; completion-time is undefined with a reason.

## Expectations
For local completion-time: statistic per_run; operator le; target 8 s; minimum_pass_rate 0.9; upper_target, target_control, and upper_target_control remain null. In separately requested analysis, compare fraction completing within 8 seconds with 90% criterion. Simulation itself reports measurements only.

## Method and assumptions
Four walks then three successful pushes. Assume route legality and correct decisions, not human puzzle solving. Independent uniform 0–1 second delay per action is an experiment-local assumption, not measured player behavior.

Analytical reference: base time 5 s, continuous time range 5–12 s, probability of completion by 8 s is 1312/5040 ≈ 0.2603. This is mathematics, NOT executed simulation output.

## Executed test

One baseline configuration, not a speed sweep: 1,000 trials with seed `0x000000000000002a`. Use resolved walk duration 0.5 s and push duration 1 s at walk speed 2 m/s. Initial/capacity energy is 3; each successful push spends 1. Program reads frozen values, not live design files or a second formula evaluator. Insufficient starting energy is unsupported; no exhaustion/recovery policy is invented.

Artifacts below are relative to example root; Markdown links resolve from this record:

- Program: [route_timing.c](../../experiments/route-cost/route_timing.c).
- Input: [frozen-input.json](../../experiments/route-cost/frozen-input.json), including typed controls, measures, assumptions, criterion, seed and limits.
- Reproduction: [recipe.md](../../experiments/route-cost/recipe.md), with build/run instructions and dependencies.
- Execution bounds: 1,000,000 steps per trial, 30 s wall time, 64 MiB output. No canonical source writes.
- Output: raw per-trial NDJSON measurements plus completion/provenance; stderr retained separately. Retained evidence is linked below.

User authorized a narrow local checkpoint and one bounded baseline run. Executed exact committed source in isolated checkout `D:/Repos/agent-skills-route-baseline`, using Zig 0.17.0. No sweep or second seed was run.

## Observations
No playtest participants or observed player behavior. Results below are measurements from an actual execution of the stated simulation, not human observations.

## Results
Executed 2026-10-24 UTC; run identity `20261024T193020Z-baseline-speed-2`.

- Requested / attempted / completed: 1,000 / 1,000 / 1,000 trials. Invalid trials: 0. Undefined completion times: 0. No exclusions.
- Finished within 8 s: **245 / 1,000 = 24.5%**. Supplied criterion requires at least 90%; this run does **not** satisfy it.
- Mean simulated completion time: 8.510268915928435 s. Minimum: 6.472825908102095 s. Maximum: 10.635222096694633 s.
- Every route completed all seven actions and ended with 0 energy. This is distinct from meeting the time criterion.
- Process exit 0; complete validated protocol, 1,002 records, 234,847 stdout bytes, empty stderr. No timeout or output limit reached.
- Capture wall time: 1.0038945999986026 s, not simulated route duration.

Evidence: [analysis and hashes](../../game-design-results/route-cost/20261024T193020Z-baseline-speed-2/summary.json), [raw trial output](../../game-design-results/route-cost/20261024T193020Z-baseline-speed-2/stdout.ndjson), [supervision and validation](../../game-design-results/route-cost/20261024T193020Z-baseline-speed-2/supervision.json), [recorded input](../../game-design-results/route-cost/20261024T193020Z-baseline-speed-2/input.json), [stderr](../../game-design-results/route-cost/20261024T193020Z-baseline-speed-2/stderr.log).

## Interpretation
Under this prescribed route and assumed delay model, baseline did not meet the 8 s / 90% criterion. Recorded 24.5% is a finite seeded sample; continuous analytical reference is approximately 26.03%, not an additional measured run or proof of distribution correctness. No conclusion about human planning, fun, puzzle solvability or game quality follows. No control changes adopted.

## Snapshot and provenance
Source checkpoint: `014af8bd477524a145d6ad077abf3ca2ad60c47a`. Relevant source was clean in isolated checkout before execution. Frozen configuration `baseline-speed-2`; master seed `0x000000000000002a`; one 1,000-trial run. Exact compiler executable, argument array and build working directory: [build.json](../../game-design-results/route-cost/20261024T193020Z-baseline-speed-2/build.json). Native supervisor checked the stream; separate read-only native validation also passed. Input copy equals committed input byte-for-byte. Analysis counted all validated trial times <= 8 s and divided by all 1,000 attempted trials; range and arithmetic mean use the same 1,000 times.

Criterion migrated from expectations/route-time, id 01a15175-3c00-7003-8c7e-5f8225a0df35.

Embedded former measure identities: elapsed-time 01a15175-3c00-7ddf-805e-cf1b29a827c3; completion-time 01a15175-3c00-76dc-8930-00df128f5f6b; route-complete 01a15175-3c00-7868-8653-33fd950f27bd; remaining-energy 01a15175-3c00-7549-84c3-67f07be036b4.

## Limitations
One Windows/Zig baseline execution is recorded. No second seed, candidate comparison, playtest or cross-platform execution was performed. This tests timing under assumed delays and a legal prescribed route, not human decisions, planning, fun or physical puzzle solvability. Continuous analytical prediction and discrete seeded simulation are different evidence. Historical runs must retain their exact code/inputs rather than follow later control changes.
