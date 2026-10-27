---
id: 01a16f86-6ff6-7779-9867-33e619d7d301
name: bonus-entry
created_at: 2026-10-24T20:07:23.894Z
desc: Optional downward pipe transfers into a coin cellar.
tags: []
status: accepted
entry: false
next:
  - challenges/coin-cellar
requires:
  - skill-atoms/use-pipe
mechanics:
  - mechanics/pipe-transfer
nouns:
  - nouns/pipe
---
## Goal and context

Commit the optional down-entry action on the designated pipe.

## Assistance and prior knowledge

Requires use-pipe, introduced in pipe-steps. Whether players discover that affordance is unknown.

## Route

Transfer to coin-cellar preserving carried state. This node represents route commitment, not a second physical room or an automatic transfer from every pipe.
