---
id: 01a15175-3c00-7b48-808f-34e6fe935d28
name: occupied-switch
created_at: 2026-10-19T00:00:00.000Z
desc: Crate activates switch
tags: []
status: accepted
nouns:
  - nouns/switch
verb: verbs/activate
context_nouns:
  - nouns/crate
  - nouns/exit
mechanics:
  - mechanics/switch-exit
controls: []
resources: []
coverage: defined
---
# Occupied switch

## Conditions
Crate occupies switch.

## Consequences
Apply [switch and exit](../mechanics/switch-exit.md). Exit state is visible; sound feedback unspecified.
