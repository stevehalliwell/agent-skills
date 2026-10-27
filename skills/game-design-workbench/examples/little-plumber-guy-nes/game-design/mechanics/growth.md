---
id: 01a16f86-69b5-775e-91cb-5e1b27962ed2
name: growth
created_at: 2026-10-24T20:07:22.293Z
desc: Grow-cap makes small form tall; otherwise it preserves current form.
tags: []
status: accepted
nouns:
  - nouns/grow-cap
  - nouns/hero
  - nouns/power-state
verbs:
  - verbs/touch
context_axes:
  - axes/Contact
---
## Rule

Touching grow-cap consumes pickup. Small becomes tall; tall and spark preserve current form. Grants no life or protection window. Grow-cap travels on support and reverses at walls; it can be lost over an edge.

## Open design

Growth while standing under a low ceiling is unresolved; choose clearance behavior before implementing a collision model.

```gd-rule
case small-collection:
    context: Hero overlaps Cap
    unknown_effects: Consume pickup; no life/protection window. Low-ceiling growth handling unresolved, no automatic correction.
    Hero:
        noun: nouns/hero
        requires: small
        becomes: tall
    Cap:
        noun: nouns/grow-cap
        becomes: consumed

case powered-collection:
    context: Hero overlaps Cap
    unknown_effects: Consume pickup; no life/protection window or invented score.
    Hero:
        noun: nouns/hero
        requires: tall OR spark
        preserve: Form
    Cap:
        noun: nouns/grow-cap
        becomes: consumed

case pickup-travel:
    unknown_conditions: Moving Grow-cap on support; wall/edge geometry and active instance unencoded.
    unknown_effects: Travel on support, reverse at walls, can fall/loss over edge. No speed, trajectory or initial position supplied.
    Cap:
        noun: nouns/grow-cap
```
