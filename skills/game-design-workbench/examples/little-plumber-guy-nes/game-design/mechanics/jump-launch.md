---
id: 01a16f86-69b5-795d-a3fe-ff67ceca89eb
name: jump-launch
created_at: 2026-10-24T20:07:22.293Z
desc: Ground support permits a jump; there is no double jump.
tags: []
status: accepted
nouns:
  - nouns/hero
  - nouns/ground
  - nouns/step
verbs:
  - verbs/jump
---
## Rule

A new jump press while supported launches hero upward; gravity later returns hero to support. Unsupported repeat presses do nothing. Launch direction can combine with horizontal input. No double jump, wall jump or automatic edge hop.

## Open design

Jump impulse, gravity, collision tolerances, coyote time and input buffering are not defined. Challenge routes express intended opportunities, not measured reachability.
