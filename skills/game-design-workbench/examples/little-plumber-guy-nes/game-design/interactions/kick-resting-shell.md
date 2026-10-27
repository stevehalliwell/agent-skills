---
id: 01a16f86-7bec-76df-9696-6df57e689e1f
name: kick-resting-shell
created_at: 2026-10-24T20:07:26.956Z
desc: Side contact launches a resting shell away from hero.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/shell
verb: verbs/kick
context_nouns:
  - nouns/hero
mechanics:
  - mechanics/shell-travel
context_axes:
  - axes/Contact
---
## Conditions

Hero side contacts a shell in resting state.

## Consequences and feedback

Launch shell away from hero. No extra kick-button input or repeated shellback award. Exact separation before the shell becomes hazardous remains an implementation detail to resolve.

```gd-rule
case kick:
    context: Hero sideContacts Shell
    unknown_effects: Launch Shell away from Hero; no extra kick-button or repeated original-enemy award. Precise separation before hazard and launch speed unresolved.
    Hero:
        noun: nouns/hero
    Shell:
        noun: nouns/shellback
        requires: resting
        becomes: sliding
```
