---
id: 01a16f86-7bed-734b-9b6c-73ef92f74490
name: wait-on-ground
created_at: 2026-10-24T20:07:26.957Z
desc: Waiting on safe floor still consumes active course time.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/hero
verb: verbs/wait
context_nouns:
  - nouns/ground
  - nouns/clock
mechanics:
  - mechanics/active-clock
---
## Conditions

Hero remains on safe floor while course is active.

## Consequences and feedback

No displacement from input; clock and enemies continue. Clock expiry can still end an attempt. Waiting is not equivalent to pausing or completing a room.
