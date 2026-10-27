---
id: 01a16f86-66a8-7110-8450-4f81eabcdf31
name: walk-tile-time
created_at: 2026-10-24T20:07:21.512Z
desc: Derived time to cover one tile at target walk speed.
tags: []
status: accepted
value_type: number
expression: "{control-vars/tile-size} / {control-vars/walk-speed}"
unit: "s"
allowed_values: []
---
## Meaning

Derived ideal time for one unobstructed tile at target walk speed. Shared resolver owns formula evaluation. Does not include acceleration, jumping, stopping, enemies or player hesitation.
