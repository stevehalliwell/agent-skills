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
