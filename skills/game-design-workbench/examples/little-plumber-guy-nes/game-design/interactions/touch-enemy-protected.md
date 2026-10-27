---
id: 01a16f86-7bec-77da-b09d-4bde1553b505
name: touch-enemy-protected
created_at: 2026-10-24T20:07:26.956Z
desc: Contact during protection causes no additional enemy damage.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/walker
  - nouns/shellback
  - nouns/shell
verb: verbs/touch
context_nouns:
  - nouns/hero
mechanics:
  - mechanics/enemy-contact
---
## Conditions

Hero is in enemy-contact safety window and contacts walker, shellback or sliding shell.

## Consequences and feedback

No additional enemy damage. Keep protection feedback visible until expiry. This does not disable gap death, clock spending or geometry.
