---
{
  "id": "01a15175-3c00-7115-8cde-8cb37cfe1ff1",
  "name": "crate-pushing",
  "created_at": "2026-10-19T00:00:00.000Z",
  "desc": "Crate pushing rule",
  "tags": [],
  "status": "accepted",
  "nouns": [
    "nouns/player",
    "nouns/crate",
    "nouns/wall"
  ],
  "verbs": [
    "verbs/push"
  ],
  "control-vars": [
    "control-vars/push-duration",
    "control-vars/push-cost"
  ],
  "resources": [
    "resources/player-energy"
  ]
}
---
# Crate pushing

A successful push requires a free destination and energy at least push-cost. Move crate one cell, then spend push-cost. Wall-blocked or insufficient-energy pushes move nothing and cost nothing. Pulling is unavailable.

```gd-rule
case successful-push:
    unknown_conditions: Push request with free destination; geometry, direction and input bindings unencoded.
    unknown_effects: Move Crate one cell into free destination; canonical push-duration describes duration. No Player-follow result supplied. Movement/spending are one occurrence, not sequential expression evaluation.
    Player:
        noun: nouns/player
    Crate:
        noun: nouns/crate
    Energy:
        noun: nouns/energy
        requires: Amount >= {control-vars/push-cost}
        set:
            Amount: this - {control-vars/push-cost}

case wall-blocked:
    unknown_conditions: Wall occupies attempted push destination.
    unknown_effects: Move nothing; spend nothing for this push.
    Player:
        noun: nouns/player
    Crate:
        noun: nouns/crate
    Wall:
        noun: nouns/wall
    Energy:
        noun: nouns/energy
        preserve: Amount

case insufficient-energy:
    unknown_conditions: Push attempted.
    unknown_effects: Move nothing; spend nothing. Pulling unavailable, not another invented no-effect rule.
    Player:
        noun: nouns/player
    Crate:
        noun: nouns/crate
    Energy:
        noun: nouns/energy
        requires: Amount < {control-vars/push-cost}
        preserve: Amount
```
