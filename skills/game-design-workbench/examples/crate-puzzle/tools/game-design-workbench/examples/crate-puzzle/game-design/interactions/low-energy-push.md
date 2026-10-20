---
{
  "id": "01a15175-3c00-71db-87ca-676558bbea15",
  "name": "low-energy-push",
  "created_at": "2026-10-19T00:00:00.000Z",
  "desc": "Insufficient-energy push",
  "tags": [],
  "status": "accepted",
  "nouns": [
    "nouns/crate"
  ],
  "verb": "verbs/push",
  "context_nouns": [
    "nouns/player",
    "nouns/energy"
  ],
  "mechanics": [
    "mechanics/crate-pushing"
  ],
  "control-vars": [],
  "resources": [],
  "coverage": "defined"
}
---
# Insufficient energy

## Conditions
Energy is below push-cost.

## Consequences
Shared pushing rule prevents displacement and spending. Walking remains governed by grid walking.
