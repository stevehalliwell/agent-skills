---
id: 01a16f86-69b5-7fd5-b37b-5620365c2137
name: death-and-retry
created_at: 2026-10-24T20:07:22.293Z
desc: Lethal hazards spend a life and restart at the latest anchor.
tags: []
status: accepted
nouns:
  - nouns/hero
  - nouns/gap
  - nouns/lives
  - nouns/clock
  - nouns/checkpoint
verbs:
  - verbs/touch
control-vars:
  - control-vars/initial-power
  - control-vars/opening-clock
resources:
  - resources/retry-budget
  - resources/course-time
---
## Rule

Lethal contact, falling below a gap or clock expiry spends one life. If lives remain, restart at latest checkpoint or course start, restore opening-clock and initial-power, and clear transient enemies/projectiles to authored local state. Retain score and coin bank; course rewards reset, so retry collection is allowed. No lives ends game.

## Open design

Reset granularity and simultaneous lethal-event deduplication need implementation detail. Route graph does not encode retry state.

```gd-rule
case lethal:
    unknown_conditions: One lethal contact OR floor-gap lower-boundary crossing OR Clock-expiry occurrence. Trigger binding and simultaneous deduplication unresolved; observing dead state is not another lethal trigger.
    unknown_effects: One death spends one life. Later retry/game-end are separate occurrences; do not revive or apply damage recursively in this transition.
    Hero:
        noun: nouns/hero
        becomes: dead
    Lives:
        noun: nouns/lives
        set:
            Amount: this - 1[life]

case later-retry:
    unknown_conditions: Later retry after completed lethal life spending, not same occurrence; reset/lifecycle staging unresolved.
    unknown_effects: Restart at latest checkpoint or course start. Restore authored local transient enemies/projectiles and course rewards, allowing retry collection. Exact reset granularity/anchors and reset values for Protection, Support, Motion, Stance and other axes unspecified. No fabricated initial scene.
    Hero:
        noun: nouns/hero
        requires: dead
        becomes: alive, {control-vars/initial-power}
        set:
            LiveSparks: 0[spark]
    Lives:
        noun: nouns/lives
        requires: Amount > 0[life]
    Clock:
        noun: nouns/clock
        set:
            TimeRemaining: {control-vars/opening-clock}
    Score:
        noun: nouns/score
        preserve: ScoreAmount
    Bank:
        noun: nouns/coin-bank
        preserve: CoinAmount
    Checkpoint:
        noun: nouns/checkpoint

case later-game-end:
    unknown_conditions: Later exhaustion handling after lethal spending; not another charge.
    unknown_effects: End game, no retry. Negative/invalid life state has no invented repair policy.
    Hero:
        noun: nouns/hero
        requires: dead
    Lives:
        noun: nouns/lives
        requires: Amount == 0[life]
```
