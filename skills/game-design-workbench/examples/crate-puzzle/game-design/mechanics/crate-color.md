---
{
  "id": "01a15175-3c00-7181-8b33-f2edece007b0",
  "name": "crate-color",
  "created_at": "2026-10-19T00:00:00.000Z",
  "desc": "Unchosen crate color alternative",
  "tags": [],
  "status": "proposed",
  "nouns": [
    "nouns/crate"
  ],
  "verbs": [],
  "control-vars": [],
  "resources": []
}
---
# Proposal

Use crate color to communicate whether next push leaves a recovery option. Not adopted; rule and accessibility implications remain open.

## Migration provenance
Former decisions/crate-color; record_kind proposal; alternative_group crate-feedback; related_decisions []; subjects [nouns/crate].

```gd-rule
case proposal-only:
    unknown_conditions: Proposed, not adopted: whether next push leaves a recovery option; test undefined.
    unknown_effects: Crate color could communicate recovery availability; no color mapping or accessibility behavior selected.
    Crate:
        noun: nouns/crate
```
