---
id: 01a16f86-7bec-76ff-9d31-dc4850df535a
name: shell-hits-wall
created_at: 2026-10-24T20:07:26.956Z
desc: Sliding shell reverses at a solid wall or pipe.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/shell
verb: verbs/touch
context_nouns:
  - nouns/pipe
  - nouns/step
mechanics:
  - mechanics/shell-travel
  - mechanics/solid-collision
context_axes:
  - axes/Contact
---
## Conditions

Sliding shell reaches solid side of pipe or step.

## Consequences and feedback

Reverse horizontal direction and remain sliding. Returning shell can threaten its kicker. No reward, pipe transport or wall destruction occurs.

```gd-rule
case pipe-side:
    context: Shell sideContacts Pipe
    unknown_effects: Reverse horizontal direction, remain sliding. No reward, transport or destruction; returning Shell can threaten kicker. No speed or separation supplied.
    Shell:
        noun: nouns/shellback
        requires: sliding
        preserve: ShellState
    Pipe:
        noun: nouns/pipe

case step-side:
    context: Shell sideContacts Step
    unknown_effects: Reverse horizontal direction, remain sliding; no reward, transport or destruction. Speed/separation unspecified.
    Shell:
        noun: nouns/shellback
        requires: sliding
        preserve: ShellState
    Step:
        noun: nouns/step
```
