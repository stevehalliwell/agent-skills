---
id: 01a16f86-7bec-7fb4-9883-af987ac6ef81
name: stomp-moving-shell
created_at: 2026-10-24T20:07:26.956Z
desc: Descending top contact stops a moving shell.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/shell
verb: verbs/stomp
context_nouns:
  - nouns/hero
mechanics:
  - mechanics/shell-travel
context_axes:
  - axes/Contact
---
## Conditions

Descending hero meets top of a sliding shell.

## Consequences and feedback

Stop shell and rebound hero. Do not grant another original-enemy award. Stopped shell can later be kicked; this is a state transition, not destruction.

```gd-rule
case descending-stop:
    context: Hero topContacts Shell
    unknown_effects: Stop persistent Shell and rebound Hero; exact rebound magnitude/trajectory unconfigured. No repeated original-enemy award; Shell can be kicked later, not destroyed.
    Hero:
        noun: nouns/hero
        requires: falling
        becomes: rising
    Shell:
        noun: nouns/shellback
        requires: sliding
        becomes: resting
```
