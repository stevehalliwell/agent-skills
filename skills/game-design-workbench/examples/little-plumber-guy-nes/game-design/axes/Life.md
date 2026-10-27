---
id: 01a17bd2-16be-7b75-af9e-3b857458c42b
name: Life
created_at: 2026-10-27T05:25:28.382Z
desc: Alive/dead dimension, scoped to the affected actor.
tags: []
status: proposed
value_type: enum
allowed_values:
  - alive
  - dead
unit: ""
closed: false
---
## Meaning

Known `alive`/`dead` states used by the [hero](../nouns/hero.md) and [walker](../nouns/walker.md). [Enemy-contact](../mechanics/enemy-contact.md), [walker-stomp](../mechanics/walker-stomp.md) and [death-and-retry](../mechanics/death-and-retry.md) own different transitions. Shared axis meaning does not merge affected actors: player death and enemy defeat retain their participant identity. Vocabulary stays open; no reachability or resurrection rule added.
