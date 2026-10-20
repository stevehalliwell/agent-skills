---
{
  "id": "01a15175-3c00-796c-8a54-fee40b0c84bd",
  "name": "player-energy",
  "created_at": "2026-10-19T00:00:00.000Z",
  "desc": "Player energy pool",
  "tags": [],
  "status": "accepted",
  "noun": "nouns/energy",
  "initial_control": "control-vars/initial-energy",
  "capacity_control": "control-vars/energy-capacity"
}
---
# Player energy

Owned per player per challenge. Initialize from linked control on challenge start. State-changing behavior belongs to [crate pushing](../mechanics/crate-pushing.md), not a second rule here. No regeneration/conversion defined.
