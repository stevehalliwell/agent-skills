---
id: 01a16f86-7beb-76da-8a95-4ee7055c7a43
name: walk-into-pipe
created_at: 2026-10-24T20:07:26.955Z
desc: A solid pipe side stops horizontal travel.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/hero
verb: verbs/walk
context_nouns:
  - nouns/pipe
mechanics:
  - mechanics/solid-collision
context_axes:
  - axes/Contact
---
## Conditions

Horizontal travel meets a pipe's solid side.

## Consequences and feedback

Prevent penetration; hero remains adjacent. No damage, transfer, reward or automatic climb. A defined blocked action is not unknown coverage.

```gd-rule
case solid-side:
    context: Hero sideContacts Pipe
    unknown_conditions: Horizontal travel meets solid Pipe side.
    unknown_effects: Prevent penetration; remain adjacent. No damage, transfer, reward or automatic climb; exact separation unknown.
    Hero:
        noun: nouns/hero
    Pipe:
        noun: nouns/pipe
```
