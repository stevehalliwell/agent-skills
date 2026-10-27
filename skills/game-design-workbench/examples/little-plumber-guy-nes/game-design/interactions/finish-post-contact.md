---
id: 01a16f86-7bed-7a11-bab1-562baa00cdf4
name: finish-post-contact
created_at: 2026-10-24T20:07:26.957Z
desc: Contact commits completion and disables remaining hazards.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/finish-post
verb: verbs/cross-finish
context_nouns:
  - nouns/hero
mechanics:
  - mechanics/course-finish
context_axes:
  - axes/Contact
---
## Conditions

Alive course-active hero contacts finish-post beyond final-gap.

## Consequences and feedback

Commit course completion, stop clock and disable later hazards. Show clear end feedback. Same-frame death/finish priority remains an explicit unresolved ordering question.

```gd-rule
case finish:
    context: Hero touches Finish
    unknown_conditions: Course active; marker beyond final-gap. Same-frame death/finish priority unresolved.
    unknown_effects: Commit completion, stop Clock spending and disable later hazards. Clear end acknowledgement; exact animation and simultaneous ordering unspecified.
    Hero:
        noun: nouns/hero
        requires: alive
    Finish:
        noun: nouns/finish-post
    Clock:
        noun: nouns/clock
```
