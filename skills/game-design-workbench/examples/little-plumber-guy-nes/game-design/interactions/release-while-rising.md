---
id: 01a16f86-7bec-7c9c-8ab2-9c6a048fe70b
name: release-while-rising
created_at: 2026-10-24T20:07:26.956Z
desc: Releasing jump during ascent cuts the arc.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/hero
verb: verbs/release-jump
mechanics:
  - mechanics/jump-cut
---
## Conditions

Jump is released while hero still ascends.

## Consequences and feedback

Reduce remaining rise and produce a visibly lower arc. Exact cutoff is open in jump-cut; do not invent a numeric trajectory or new ground support.

```gd-rule
case cut-rise:
    unknown_conditions: Jump released during ascent.
    unknown_effects: Reduce remaining upward travel, produce visibly lower arc. Exact cutoff law/magnitude unknown; do not invent trajectory or ground support.
    Hero:
        noun: nouns/hero
        requires: rising
```
