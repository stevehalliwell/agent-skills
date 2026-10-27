---
id: 01a17bd2-1570-76a3-8ebc-6a116189eaa9
name: Occupancy
created_at: 2026-10-27T05:25:28.048Z
desc: Known crate occupancy states of the switch.
tags: []
status: proposed
value_type: enum
allowed_values:
  - empty
  - occupied
unit: ""
closed: false
---
## Meaning

Known crate occupancy of [switch](../nouns/switch.md): `empty` or `occupied`. Here occupied means occupied by a crate, not any arbitrary actor. [Switch-exit](../mechanics/switch-exit.md) owns the exit response. Open vocabulary; no general collision or player-activation behavior added.
