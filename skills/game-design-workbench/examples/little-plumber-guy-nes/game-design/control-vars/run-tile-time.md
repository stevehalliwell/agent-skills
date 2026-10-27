---
id: 01a16f86-66a8-7d6b-9de4-fabe156df493
name: run-tile-time
created_at: 2026-10-24T20:07:21.512Z
desc: Derived time to cover one tile at target run speed.
tags: []
status: accepted
value_type: number
expression: "{control-vars/tile-size} / {control-vars/run-speed}"
unit: "s"
allowed_values: []
---
## Meaning

Derived ideal time for one unobstructed tile at target run speed. Change its upstream literals rather than overwriting the formula result in an imported configuration.
