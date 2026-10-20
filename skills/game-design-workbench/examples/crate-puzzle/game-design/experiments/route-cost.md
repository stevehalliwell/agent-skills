---
id: 01a15175-3c00-7e68-8f3b-b760984a9a2d
name: route-cost
created_at: 2026-10-19T00:00:00.000Z
desc: Proposed route timing investigation
tags: []
status: proposed
experiment_kind: simulation
controls:
  - controls/tile-distance
  - controls/walk-speed
  - controls/walk-duration
  - controls/push-duration
  - controls/initial-energy
  - controls/energy-capacity
  - controls/push-cost
measures:
  - measures/elapsed-time
  - measures/completion-time
  - measures/route-complete
  - measures/remaining-energy
expectations:
  - expectations/route-time
mechanics:
  - mechanics/grid-walking
  - mechanics/crate-pushing
interactions: []
program: null
inputs: null
results: null
---
# Question

What is route completion time under assumed delays?

## Proposed method and local assumptions
Four walks then three successful pushes. Assume route legality and correct decisions, not human puzzle solving. Independent uniform 0–1 second delay per action is an experiment-local assumption, not measured player behavior.

Analytical reference: base time 5 s, continuous time range 5–12 s, probability of completion by 8 s is 1312/5040 ≈ 0.2603. This is mathematics, NOT executed simulation output.

## Availability
Model has not been generated or run. This release captures question/data only. Later runtime must prepare inputs, checkpoint with approval, and resume frozen recipe. Historical models never follow current controls automatically.
