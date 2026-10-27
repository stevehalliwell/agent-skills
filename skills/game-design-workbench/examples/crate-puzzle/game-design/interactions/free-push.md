---
{
  "id": "01a15175-3c00-72f0-8497-ad9ed07f327a",
  "name": "free-push",
  "created_at": "2026-10-19T00:00:00.000Z",
  "desc": "Energy-sufficient free push",
  "tags": [],
  "status": "accepted",
  "nouns": [
    "nouns/crate"
  ],
  "verb": "verbs/push",
  "context_nouns": [
    "nouns/player"
  ],
  "mechanics": [
    "mechanics/crate-pushing"
  ],
  "control-vars": [],
  "resources": [],
  "coverage": "defined"
}
---
# Free push

## Conditions
Destination is free; energy satisfies the shared pushing rule.

## Consequences and feedback
Apply [crate pushing](../mechanics/crate-pushing.md). Crate movement makes displacement visible.

```gd-rule
case free-push:
    unknown_conditions: Push request into free destination; spatial placement, direction and destination binding unencoded.
    unknown_effects: Apply crate-pushing: move Crate one cell into free destination; visible displacement. Player-follow movement unspecified.
    Player:
        noun: nouns/player
    Crate:
        noun: nouns/crate
    Energy:
        noun: nouns/energy
        requires: Amount >= {control-vars/push-cost}
        set:
            Amount: this - {control-vars/push-cost}
```
