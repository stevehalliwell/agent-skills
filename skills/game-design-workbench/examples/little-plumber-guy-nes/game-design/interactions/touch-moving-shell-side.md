---
id: 01a16f86-7bec-7918-9352-2837e58f27e1
name: touch-moving-shell-side
created_at: 2026-10-24T20:07:26.956Z
desc: Sliding shell's side is an enemy-contact hazard.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/shell
verb: verbs/touch
context_nouns:
  - nouns/hero
mechanics:
  - mechanics/shell-travel
  - mechanics/enemy-contact
---
## Conditions

Hero side contacts a sliding shell outside a valid stomp.

## Consequences and feedback

Apply enemy-contact according to form and protection. A shell launched by hero is not friendly afterward. Original ownership does not suppress its hazard.
