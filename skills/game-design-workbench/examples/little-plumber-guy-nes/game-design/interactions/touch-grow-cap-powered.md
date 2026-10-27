---
id: 01a16f86-7bec-7f5f-b196-1ed8a4468619
name: touch-grow-cap-powered
created_at: 2026-10-24T20:07:26.956Z
desc: Tall or spark hero consumes cap without changing form.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/grow-cap
verb: verbs/touch
context_nouns:
  - nouns/hero
  - nouns/power-state
mechanics:
  - mechanics/growth
context_axes:
  - axes/Contact
---
## Conditions

Tall or spark hero overlaps grow-cap.

## Consequences and feedback

Consume pickup while preserving form. No extra life, damage protection or invented score award. This is a defined consumed-without-upgrade outcome.

```gd-rule
case collect-powered:
    context: Hero overlaps Cap
    unknown_effects: Consume pickup; no extra life, protection or invented score.
    Hero:
        noun: nouns/hero
        requires: tall OR spark
        preserve: Form
    Cap:
        noun: nouns/grow-cap
        becomes: consumed
```
