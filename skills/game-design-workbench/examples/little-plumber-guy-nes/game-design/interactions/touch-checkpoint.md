---
id: 01a16f86-7bed-7b43-84e9-5b271fc101ee
name: touch-checkpoint
created_at: 2026-10-24T20:07:26.957Z
desc: Crossing trigger saves a retry anchor but grants no power or time.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/checkpoint
verb: verbs/touch
context_nouns:
  - nouns/hero
mechanics:
  - mechanics/checkpoint-save
---
## Conditions

Alive hero crosses checkpoint-strip's progress trigger.

## Consequences and feedback

Store retry anchor for current game. No immediate time, power or life refill. Trigger visibility/acknowledgement is unresolved; storing state does not prove player noticed it.
