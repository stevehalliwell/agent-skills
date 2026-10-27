---
id: 01a16f86-7bed-7778-9507-d9a0c2a6c882
name: wait-in-cellar
created_at: 2026-10-24T20:07:26.957Z
desc: Coin room exploration shares the active course clock.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/hero
verb: verbs/wait
context_nouns:
  - nouns/clock
  - nouns/exit-pipe
mechanics:
  - mechanics/active-clock
  - mechanics/pipe-transfer
control-vars:
  - control-vars/clock-runs-in-cellar
---
## Conditions

Hero explores or waits in coin-cellar with current clock-runs-in-cellar policy true.

## Consequences and feedback

Spend the shared active clock. Coins are not free of time cost. If policy is changed to false, spending stops in that room; changing policy requires explicit design adoption.
