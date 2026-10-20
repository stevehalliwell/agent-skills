---
{
  "id": "01a15175-3c00-7e68-8f3b-b760984a9a2d",
  "name": "route-cost",
  "created_at": "2026-10-19T00:00:00.000Z",
  "desc": "Proposed route timing investigation",
  "tags": [],
  "status": "proposed",
  "stage": "proposed",
  "experiment_kind": "simulation",
  "controls": [
    "controls/tile-distance",
    "controls/walk-speed",
    "controls/walk-duration",
    "controls/push-duration",
    "controls/initial-energy",
    "controls/energy-capacity",
    "controls/push-cost"
  ],
  "measures": [
    "measures/elapsed-time",
    "measures/completion-time",
    "measures/route-complete",
    "measures/remaining-energy"
  ],
  "mechanics": [
    "mechanics/grid-walking",
    "mechanics/crate-pushing"
  ],
  "interactions": [],
  "dynamics": [],
  "aesthetics": [],
  "program": null,
  "inputs": null,
  "results": null
}
---
# Question

What is route completion time under assumed delays?

## Hypotheses
No player-behavior hypothesis supplied for this timing model. Energy/planning investigation is separate.

## Expectations
For measures/completion-time: statistic per_run; operator le; target 8 s; minimum_pass_rate 0.9; upper_target, target_control, and upper_target_control remain null. In separately requested analysis, compare fraction completing within 8 seconds with 90% criterion. Simulation itself reports measurements only.

## Method and assumptions
Four walks then three successful pushes. Assume route legality and correct decisions, not human puzzle solving. Independent uniform 0–1 second delay per action is an experiment-local assumption, not measured player behavior.

Analytical reference: base time 5 s, continuous time range 5–12 s, probability of completion by 8 s is 1312/5040 ≈ 0.2603. This is mathematics, NOT executed simulation output.

## Observations
None recorded.

## Results
Not executed. Analytical reference above is not simulation output.

## Interpretation
No execution or design verdict.

## Snapshot and provenance
Criterion migrated from expectations/route-time, id 01a15175-3c00-7003-8c7e-5f8225a0df35.

## Limitations
Model has not been generated or run. This release captures question/data only. Later runtime must prepare inputs, checkpoint with approval, and resume frozen recipe. Historical models never follow current controls automatically.
