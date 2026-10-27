---
id: 01a16f86-69b6-76be-8c8d-5b86c755b9bb
name: forward-camera
created_at: 2026-10-24T20:07:22.294Z
desc: Camera reveals forward space and constrains backtracking.
tags: []
status: accepted
nouns:
  - nouns/camera
  - nouns/hero
verbs:
  - verbs/walk
---
## Rule

Follow forward progress without scrolling backward. Left viewport boundary prevents hero retreat beyond current camera window. Room transfer sets a new anchor; retry restores its safe anchor. Do not treat ordinary challenge arrows as camera coordinates.

## Open design

Scroll lead, offscreen enemy activation and pickup persistence remain unspecified.

```gd-rule
case forward-follow:
    unknown_conditions: Forward progress; follow threshold/coordinates unencoded.
    unknown_effects: Follow without backward scrolling; left viewport boundary constrains retreat. Scroll lead, offscreen enemy activation and pickup persistence unspecified.
    Hero:
        noun: nouns/hero
    Camera:
        noun: nouns/camera

case room-transfer-anchor:
    unknown_conditions: Authored room-transfer occurrence.
    unknown_effects: Set new Camera anchor; coordinates unspecified.
    Camera:
        noun: nouns/camera

case retry-anchor:
    unknown_conditions: Later retry occurrence, not ordinary challenge next arrow.
    unknown_effects: Restore safe Camera anchor; coordinates unspecified.
    Camera:
        noun: nouns/camera
```
