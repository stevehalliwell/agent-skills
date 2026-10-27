---
id: 01a16f86-7803-70a9-93df-78688493c485
name: travel-budget
created_at: 2026-10-24T20:07:25.955Z
desc: Proposed calculation of idealized travel budget, not physical reachability.
tags: []
status: proposed
stage: proposed
experiment_kind: simulation
control-vars:
  - control-vars/tile-size
  - control-vars/walk-speed
  - control-vars/run-speed
  - control-vars/walk-tile-time
  - control-vars/run-tile-time
  - control-vars/opening-clock
mechanics:
  - mechanics/horizontal-movement
  - mechanics/active-clock
dynamics:
  - dynamics/clock-tradeoff
---
## Question

Once authored path lengths exist, how much active clock would ideal unobstructed walking or running consume on each selected route?

## Local measures

- ideal-travel-time: number, s, sum of resolved per-tile travel durations for an explicit path length/mode sequence.
- time-margin: number, s, opening-clock minus ideal travel time; negative values retained.
- route-defined: boolean, unitless, whether all required path lengths and movement modes are supplied.

## Method and expectations

Use common resolver for durations. Prefer direct calculation; generate a small model only if later state transitions require it. Freeze explicit path lengths and assumptions rather than infer geometry from challenge links. No player-delay distribution, speedrun policy or success threshold supplied.

## Results and execution state

Not executed; required path lengths are absent. Program, inputs and results are unset. No checkpoint or run requested by this example.

## Limits

Excludes acceleration, jumps, enemies, hesitation and room-transition implementation. A positive margin would not prove reachability or player success. This proposal demonstrates an honest blocked investigation, not another runtime feature.
