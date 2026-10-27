---
id: 01a16f86-7beb-77a1-8fbe-1631525b58ca
name: walk-off-edge
created_at: 2026-10-24T20:07:26.955Z
desc: Leaving support begins a fall rather than an automatic jump.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/hero
verb: verbs/walk
context_nouns:
  - nouns/gap
mechanics:
  - mechanics/solid-collision
  - mechanics/death-and-retry
---
## Conditions

Hero's horizontal movement passes beyond floor support without an active jump.

## Consequences and feedback

Begin unsupported fall. Death occurs only at lower boundary, not at the instant support ends. No automatic rescue jump is implied.
