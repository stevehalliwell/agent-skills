---
id: 01a16f86-7bec-75fa-b43f-df69114a0eb2
name: jump-in-air
created_at: 2026-10-24T20:07:26.956Z
desc: A second jump input while unsupported has no effect.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/hero
verb: verbs/jump
mechanics:
  - mechanics/jump-launch
---
## Conditions

Hero is unsupported when another jump press occurs.

## Consequences and feedback

No second launch. Preserve current motion apart from other active rules. This is explicitly defined no-effect behavior, not an absent jump case.

```gd-rule
case no-second-launch:
    unknown_conditions: Repeat jump press while unsupported.
    unknown_effects: No second launch; preserve current motion against this action only. Other active gravity/collision/movement continue; exact continuing quantities not configured.
    Hero:
        noun: nouns/hero
        requires: unsupported
        preserve: Motion
```
