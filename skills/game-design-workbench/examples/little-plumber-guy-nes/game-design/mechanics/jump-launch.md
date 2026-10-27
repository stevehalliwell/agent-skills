---
id: 01a16f86-69b5-795d-a3fe-ff67ceca89eb
name: jump-launch
created_at: 2026-10-24T20:07:22.293Z
desc: Ground support permits a jump; there is no double jump.
tags: []
status: accepted
nouns:
  - nouns/hero
  - nouns/ground
  - nouns/step
verbs:
  - verbs/jump
---
## Rule

A new jump press while supported launches hero upward; gravity later returns hero to support. Unsupported repeat presses do nothing. Launch direction can combine with horizontal input. No double jump, wall jump or automatic edge hop.

## Open design

Jump impulse, gravity, collision tolerances, coyote time and input buffering are not defined. Challenge routes express intended opportunities, not measured reachability.

```gd-rule
case supported-new-press:
    unknown_conditions: New jump press with support; horizontal input can combine; timing/geometry unencoded.
    unknown_effects: Launch upward; gravity later returns Hero to support as later occurrence, not simultaneous landing. Impulse, gravity, tolerances, coyote time and buffering undefined.
    Hero:
        noun: nouns/hero
        requires: supported
        becomes: rising

case unsupported-repeat:
    unknown_conditions: Unsupported repeat jump press.
    unknown_effects: No second launch; no double jump, wall jump or automatic edge hop. Other active motion continues.
    Hero:
        noun: nouns/hero
        requires: unsupported
        preserve: Motion

case later-return-to-support:
    unknown_conditions: Later landing on support after jump, not same launch occurrence; contact geometry/time unknown.
    unknown_effects: Gravity eventually returns Hero to support; no trajectory/tick or tolerance supplied.
    Hero:
        noun: nouns/hero
        becomes: supported
```
