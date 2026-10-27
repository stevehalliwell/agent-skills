---
id: 01a16f86-69b5-7fd5-b37b-5620365c2137
name: death-and-retry
created_at: 2026-10-24T20:07:22.293Z
desc: Lethal hazards spend a life and restart at the latest anchor.
tags: []
status: accepted
nouns:
  - nouns/hero
  - nouns/gap
  - nouns/lives
  - nouns/clock
  - nouns/checkpoint
verbs:
  - verbs/touch
control-vars:
  - control-vars/initial-power
  - control-vars/opening-clock
resources:
  - resources/retry-budget
  - resources/course-time
---
## Rule

Lethal contact, falling below a gap or clock expiry spends one life. If lives remain, restart at latest checkpoint or course start, restore opening-clock and initial-power, and clear transient enemies/projectiles to authored local state. Retain score and coin bank; course rewards reset, so retry collection is allowed. No lives ends game.

## Open design

Reset granularity and simultaneous lethal-event deduplication need implementation detail. Route graph does not encode retry state.
