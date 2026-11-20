---
id: 01a16f86-66a8-78b9-b154-80c1a8a7c29f
name: walk-speed
created_at: 2026-10-24T20:07:21.512Z
desc: Toy design's target walking speed, not measured physics.
tags: []
status: accepted
value_type: number
expression: ""
unit: "tile/s"
allowed_values: []
value: 2
minimum: 0.1
---
## Meaning

Target speed in ordinary grounded travel. Acceleration and braking profiles remain unspecified. Changing this literal recomputes walk-tile-time; it does not by itself prove jump reachability.
