---
id: 01a16f86-69b5-791a-a411-4929abf1c104
name: walker-stomp
created_at: 2026-10-24T20:07:22.293Z
desc: A descending top contact defeats a walker and rebounds the hero.
tags: []
status: accepted
nouns:
  - nouns/hero
  - nouns/walker
verbs:
  - verbs/stomp
control-vars:
  - control-vars/walker-points
resources:
  - resources/run-score
---
## Rule

Descending top contact defeats walker, pays walker-points once and rebounds hero. Side contact remains harmful. Walker patrol reverses at walls but may fall off edges. No defeat-chain multiplier is assumed.

## Open design

Rebound height and simultaneous contact arbitration remain unspecified.
