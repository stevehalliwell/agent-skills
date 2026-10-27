---
id: 01a16f86-69b5-7866-99dc-569deba690cc
name: shell-travel
created_at: 2026-10-24T20:07:22.293Z
desc: A kick starts a hazardous sliding shell; a stomp stops it.
tags: []
status: accepted
nouns:
  - nouns/shell
  - nouns/hero
  - nouns/walker
  - nouns/shellback
  - nouns/pipe
verbs:
  - verbs/kick
  - verbs/stomp
  - verbs/touch
---
## Rule

Hero side contact launches a resting shell away from hero. Sliding shell defeats a walker and continues; walls and pipes reverse it. Descending hero stomp stops it; harmful side contact uses enemy-contact. A resting shell never wakes automatically.

## Open design

Two-shell collision, shell contact with another walking shellback, and precise launch separation remain unresolved.
