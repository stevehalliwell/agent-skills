---
id: 01a16f86-7bed-795b-9949-972e6409d0ef
name: enter-solid-pipe
created_at: 2026-10-24T20:07:26.957Z
desc: Ordinary obstacle pipe does not transfer rooms.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/pipe
verb: verbs/enter-pipe
context_nouns:
  - nouns/hero
mechanics:
  - mechanics/pipe-transfer
---
## Conditions

Hero presses down on a pipe without an authored entrance connection.

## Consequences and feedback

No transfer; pipe remains solid support. Correct alignment does not create a missing connection. This is defined blocked entry rather than unknown destination data.

```gd-rule
case no-connection:
    unknown_conditions: Down pressed on Pipe without authored entrance; correct alignment cannot create connection.
    unknown_effects: No transfer; Pipe remains solid support. No invented destination.
    Hero:
        noun: nouns/hero
    Pipe:
        noun: nouns/pipe
```
