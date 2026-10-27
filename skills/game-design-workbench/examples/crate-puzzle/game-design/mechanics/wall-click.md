---
{
  "id": "01a15175-3c00-772c-8e58-9a629e231696",
  "name": "wall-click",
  "created_at": "2026-10-19T00:00:00.000Z",
  "desc": "Wall contact click proposal",
  "tags": [],
  "status": "proposed",
  "nouns": [],
  "verbs": [],
  "control-vars": [],
  "resources": []
}
---
# Unlinked feedback rule

A blocked wall contact could emit a short click. No interaction/challenge currently uses this proposed rule. It is not accepted gameplay.

```gd-rule
case proposal-only:
    unknown_conditions: Proposed blocked wall contact; no linked interaction/challenge or chosen event binding.
    unknown_effects: A short click could be emitted; not adopted gameplay; sound parameters unspecified.
```
