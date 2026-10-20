---
{
  "id": "01a15175-3c00-79b6-87a8-f49d3e41416f",
  "name": "wall-blocked-push",
  "created_at": "2026-10-19T00:00:00.000Z",
  "desc": "Wall-blocked push",
  "tags": [],
  "status": "accepted",
  "nouns": [
    "nouns/crate"
  ],
  "verb": "verbs/push",
  "context_nouns": [
    "nouns/player",
    "nouns/wall"
  ],
  "mechanics": [
    "mechanics/crate-pushing"
  ],
  "control-vars": [],
  "resources": [],
  "coverage": "defined"
}
---
# Wall-blocked push

## Conditions
Wall occupies destination.

## Consequences and feedback
Apply shared blocked-push rule: no movement or spending. Additional blocked-action feedback remains unspecified.
