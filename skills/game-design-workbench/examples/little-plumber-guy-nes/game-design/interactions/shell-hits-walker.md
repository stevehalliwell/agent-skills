---
id: 01a16f86-7bec-771d-999c-a46aa9fc6bd7
name: shell-hits-walker
created_at: 2026-10-24T20:07:26.956Z
desc: Sliding shell removes a walker and continues.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/walker
verb: verbs/touch
context_nouns:
  - nouns/shell
mechanics:
  - mechanics/shell-travel
  - mechanics/score-awards
context_axes:
  - axes/Contact
---
## Conditions

Sliding shell contacts a living walker.

## Consequences and feedback

Remove walker, award its configured points once and let shell continue. No chain multiplier or shell ownership-based safety is implied.

```gd-rule
case sliding-defeat:
    context: Shell touches Walker
    unknown_conditions: Credit original defeat once; event identity/deduplication unencoded.
    unknown_effects: Remove Walker; Shell continues sliding. No chain multiplier or ownership safety; continued motion magnitude unknown.
    Shell:
        noun: nouns/shellback
        requires: sliding
        preserve: ShellState
    Walker:
        noun: nouns/walker
        requires: alive
        becomes: dead
    Score:
        noun: nouns/score
        set:
            ScoreAmount: this + {control-vars/walker-points}
```
