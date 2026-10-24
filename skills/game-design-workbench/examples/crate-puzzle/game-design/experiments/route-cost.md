---
{ "id": "01a15175-3c00-7e68-8f3b-b760984a9a2d", "name": "route-cost", "created_at": "2026-10-19T00:00:00.000Z", "desc": "Prepared seven-action route timing experiment; not executed", "tags": [], "status": "proposed", "stage": "proposed", "experiment_kind": "simulation", "control-vars": [ "control-vars/tile-distance", "control-vars/walk-speed", "control-vars/walk-duration", "control-vars/push-duration", "control-vars/initial-energy", "control-vars/energy-capacity", "control-vars/push-cost" ], "mechanics": [ "mechanics/grid-walking", "mechanics/crate-pushing" ], "interactions": [], "dynamics": [], "aesthetics": [], "program": experiments/route-cost/route_timing.c, "inputs": experiments/route-cost/frozen-input.json, "results": null }
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

## Prepared test

One baseline configuration, not a speed sweep: 1,000 trials with seed `0x000000000000002a`. Use resolved walk duration 0.5 s and push duration 1 s at walk speed 2 m/s. Initial/capacity energy is 3; each successful push spends 1. Program reads frozen values, not live design files or a second formula evaluator. Insufficient starting energy is unsupported; no exhaustion/recovery policy is invented.

Artifacts below are relative to example root; Markdown links resolve from this record:

- Program: [route_timing.c](../../experiments/route-cost/route_timing.c).
- Input: [frozen-input.json](../../experiments/route-cost/frozen-input.json), including typed controls, measures, assumptions, criterion, seed and limits.
- Reproduction: [recipe.md](../../experiments/route-cost/recipe.md), with build/run instructions and dependencies.
- Execution bounds: 1,000,000 steps per trial, 30 s wall time, 64 MiB output. No canonical source writes.
- Output: raw per-trial NDJSON measurements plus completion/provenance; stderr retained separately. Actual result artifact does not exist yet, so `results` remains null.

At source checkpoint, program has compiled under CJIT and Zig but no route trial has run. User authorized a narrow local checkpoint and one bounded baseline run from isolated checkout `D:/Repos/agent-skills-route-baseline`. Actual outputs and findings will be recorded after that run.

After an authorized run, retain raw output and record requested/attempted/completed/invalid counts, count within 8 s and denominator, measured fraction, supplied 90% criterion comparison, actual checkpoint/build/toolchain, and limitations here. A missing final record or failed run is incomplete evidence, not a passing experiment. Do not fill these values from analytical reference.

## Observations
None recorded.

## Results
Not executed. Analytical reference above is not simulation output.

## Interpretation
No execution or design verdict.

## Snapshot and provenance
Criterion migrated from expectations/route-time, id 01a15175-3c00-7003-8c7e-5f8225a0df35.

Embedded former measure identities: elapsed-time 01a15175-3c00-7ddf-805e-cf1b29a827c3; completion-time 01a15175-3c00-76dc-8930-00df128f5f6b; route-complete 01a15175-3c00-7868-8653-33fd950f27bd; remaining-energy 01a15175-3c00-7549-84c3-67f07be036b4.

## Limitations
Model and frozen inputs exist. At source checkpoint, baseline execution is approved but not yet performed. This tests timing under assumed delays and a legal prescribed route, not human decisions, planning, fun or physical puzzle solvability. Continuous analytical prediction and discrete seeded simulation are different evidence. Historical runs must retain their exact code/inputs rather than follow later control changes.
