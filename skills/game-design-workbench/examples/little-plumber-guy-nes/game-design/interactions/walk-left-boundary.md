---
id: 01a16f86-7bec-79b9-aa9c-a612ef9c8989
name: walk-left-boundary
created_at: 2026-10-24T20:07:26.956Z
desc: Camera's left boundary prevents further backtracking.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/hero
verb: verbs/walk
context_nouns:
  - nouns/camera
mechanics:
  - mechanics/forward-camera
---
## Conditions

Hero requests leftward motion beyond the current camera boundary.

## Consequences and feedback

Stop at boundary without moving camera backward. No damage or course reset. Pipe transfer and retry can set a different anchor through their own rules.

```gd-rule
case left-limit:
    unknown_conditions: Hero requests leftward movement beyond current Camera boundary; coordinate unencoded.
    unknown_effects: Stop at boundary without backward Camera scroll; no damage/reset. Transfer/retry can set another anchor through their own rules.
    Hero:
        noun: nouns/hero
    Camera:
        noun: nouns/camera
```
