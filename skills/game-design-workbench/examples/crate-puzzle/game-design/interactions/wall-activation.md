---
{
  "id": "01a15175-3c00-7490-8f6e-f763cd7846d6",
  "name": "wall-activation",
  "created_at": "2026-10-19T00:00:00.000Z",
  "desc": "Wall activation is not applicable",
  "tags": [],
  "status": "accepted",
  "nouns": [
    "nouns/wall"
  ],
  "verb": "verbs/activate",
  "context_nouns": [],
  "mechanics": [],
  "control-vars": [],
  "resources": [],
  "coverage": "not_applicable"
}
---
# Not applicable

Walls have no activation operation. This exclusion is not a defined no-effect activation rule.

```gd-rule
case not-applicable:
    unknown_conditions: Coverage not_applicable: Wall has no activation operation; this is a disposition declaration, not gameplay applicability.
    unknown_effects: No activation outcome is declared. Do not reinterpret as a defined no-effect activation rule.
    Wall:
        noun: nouns/wall
```
