---
id: 01a16f86-69b5-7bc9-874d-d5b7dcb0e4bb
name: jump-cut
created_at: 2026-10-24T20:07:22.293Z
desc: Releasing jump during ascent shortens its remaining rise.
tags: []
status: accepted
nouns:
  - nouns/hero
  - nouns/ceiling
verbs:
  - verbs/release-jump
---
## Rule

Releasing jump during ascent reduces remaining upward travel. Releasing during descent leaves descent unchanged. This permits a lower arc under a ceiling without creating a separate jump type.

## Feedback and limits

Arc change should be visible. Exact cutoff law and attainable heights remain open; no native graph analysis can derive them from these links.

```gd-rule
case ascending-release:
    unknown_conditions: Jump released during ascent.
    unknown_effects: Reduce remaining upward travel, visibly lower arc; not new jump type. Exact cutoff law/heights unspecified.
    Hero:
        noun: nouns/hero
        requires: rising

case descending-release:
    unknown_conditions: Jump released during descent.
    unknown_effects: Jump-cut leaves descent unchanged against release; gravity/collision continue, not frozen motion.
    Hero:
        noun: nouns/hero
        requires: falling
        preserve: Motion
```
