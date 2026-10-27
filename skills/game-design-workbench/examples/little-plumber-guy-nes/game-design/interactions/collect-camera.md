---
id: 01a16f86-7bed-7908-b39e-e3839bf3d411
name: collect-camera
created_at: 2026-10-24T20:07:26.957Z
desc: Camera is not a collectible or world pickup.
tags: []
status: accepted
coverage: not_applicable
nouns:
  - nouns/camera
verb: verbs/touch
---
## Not applicable

Camera is viewport state, not a world pickup, so hero overlap does not form a meaningful collection interaction. This example distinguishes inapplicable semantics from an unknown response or an intentionally excluded feature.

```gd-rule
case not-applicable:
    unknown_conditions: Coverage not_applicable: Camera is viewport state, not a world pickup; collection overlap not meaningful. This is not a gameplay applicability case.
    unknown_effects: No collectible outcome declared; not unknown response, exclusion or a defined no-effect pickup.
    Camera:
        noun: nouns/camera
```
