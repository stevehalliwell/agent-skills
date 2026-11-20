---
id: 01a16f86-6c74-7ed5-ad87-11a7e05e60e4
name: locomotion
created_at: 2026-10-24T20:07:22.996Z
desc: Movement, jump control, geometry and viewport constraints.
tags: []
status: accepted
mechanics:
  - mechanics/horizontal-movement
  - mechanics/jump-launch
  - mechanics/jump-cut
  - mechanics/crouching
  - mechanics/solid-collision
  - mechanics/forward-camera
---
## Arrangement

Horizontal input, grounded launch, early release, crouch and solid collision compose movement inside a forward viewport. The arrangement lets similar inputs address flat space, overhead constraints and raised supports. Links identify rule composition; they do not supply trajectories or show that players understand movement.
