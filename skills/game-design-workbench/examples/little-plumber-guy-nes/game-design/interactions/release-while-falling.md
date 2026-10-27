---
id: 01a16f86-7bec-758b-9fda-f0ad6d491bbc
name: release-while-falling
created_at: 2026-10-24T20:07:26.956Z
desc: Releasing jump during descent does not change that descent.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/hero
verb: verbs/release-jump
mechanics:
  - mechanics/jump-cut
---
## Conditions

Jump is released after ascent has ended.

## Consequences and feedback

Do not alter descent through jump-cut. Gravity and collision continue. This alternative shares the same noun–verb cell as rising release while preserving a different condition and consequence.
