---
id: 01a16f86-69b6-7fbf-9ca7-a8d748e6e1fd
name: course-finish
created_at: 2026-10-24T20:07:22.294Z
desc: Reaching finish commits success and stops active clock.
tags: []
status: accepted
nouns:
  - nouns/finish-post
  - nouns/hero
  - nouns/clock
verbs:
  - verbs/cross-finish
resources:
  - resources/course-time
context_axes:
  - axes/Contact
---
## Rule

Alive hero contacting finish-post during active course commits completion, stops clock and disables further hazard processing. Shot contact does not finish. No post-height award, remaining-time conversion or next-level implementation is specified.

## Feedback

Clear end-state acknowledgement is required; exact animation is not part of this dataset.

```gd-rule
case hero-finish:
    context: Hero touches Finish
    unknown_conditions: Active course; same-frame lethal/expiry ordering unresolved.
    unknown_effects: Commit completion, stop Clock spending and disable further hazards. Clear end acknowledgement required; exact animation, post-height award, remaining-time conversion and next-level implementation unspecified.
    Hero:
        noun: nouns/hero
        requires: alive
    Finish:
        noun: nouns/finish-post
    Clock:
        noun: nouns/clock

case shot-not-finish:
    context: Spark touches Finish
    unknown_effects: Shot contact does not finish course. Other projectile collision response unspecified, not automatic ignore/pass-through.
    Spark:
        noun: nouns/spark
    Finish:
        noun: nouns/finish-post
```
