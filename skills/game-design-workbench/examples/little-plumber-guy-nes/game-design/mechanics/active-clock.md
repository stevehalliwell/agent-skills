---
id: 01a16f86-69b5-7699-9d7e-5d55b1337d5a
name: active-clock
created_at: 2026-10-24T20:07:22.293Z
desc: Course time drains during active play, including the cellar.
tags: []
status: accepted
nouns:
  - nouns/clock
  - nouns/hero
verbs:
  - verbs/wait
control-vars:
  - control-vars/clock-runs-in-cellar
resources:
  - resources/course-time
  - resources/retry-budget
---
## Rule

Spend course time during active movement, waiting and cellar exploration when clock-runs-in-cellar is true. Pause/menu and room-transfer transition time are not active play. Expiry is lethal even in protected or powered state. Finish stops spending.

## Open design

Same-frame expiry versus finish priority is unresolved. Time units here are authored seconds, not emulated hardware ticks.
