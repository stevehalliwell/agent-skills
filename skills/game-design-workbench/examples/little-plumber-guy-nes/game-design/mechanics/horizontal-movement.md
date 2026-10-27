---
id: 01a16f86-69b4-73ea-93f7-65ca948dd6ca
name: horizontal-movement
created_at: 2026-10-24T20:07:22.292Z
desc: Walking and running set different horizontal target speeds.
tags: []
status: accepted
nouns:
  - nouns/hero
  - nouns/ground
verbs:
  - verbs/walk
control-vars:
  - control-vars/walk-speed
  - control-vars/run-speed
  - control-vars/tile-size
---
## Rule

Supported hero moves toward facing/input direction at walk-speed, or run-speed while run is held. Walls and camera boundary constrain displacement; stepping beyond support begins falling. Opposite inputs cancel. Air steering is allowed, but acceleration, braking and air-control strength remain unspecified.

## Feedback

Movement and facing should follow input visibly. No inference that target speed is reached instantly in a future implementation.
