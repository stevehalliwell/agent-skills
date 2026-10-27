---
id: 01a16f86-69b5-7699-9d7e-5d55b1337d5a
name: active-clock
created_at: 2026-10-24T20:07:22.293Z
desc: Course time drains during active play, including the cellar.
tags: []
status: accepted
nouns:
  - nouns/clock
  - nouns/hero
verbs:
  - verbs/wait
control-vars:
  - control-vars/clock-runs-in-cellar
resources:
  - resources/course-time
  - resources/retry-budget
---
## Rule

Spend course time during active movement, waiting and cellar exploration when clock-runs-in-cellar is true. Pause/menu and room-transfer transition time are not active play. Expiry is lethal even in protected or powered state. Finish stops spending.

## Open design

Same-frame expiry versus finish priority is unresolved. Time units here are authored seconds, not emulated hardware ticks.

```gd-rule
case active-overworld:
    unknown_conditions: Active movement/waiting outside cellar, not pause/menu/room-transfer transition. Elapsed active interval unencoded.
    unknown_effects: Spend remaining course seconds for elapsed active play; no fixed tick or implicit clamp.
    Hero:
        noun: nouns/hero
    Clock:
        noun: nouns/clock

case active-cellar:
    unknown_conditions: Active cellar exploration/waiting, not pause/menu/transfer; room identity and elapsed interval unencoded.
    unknown_effects: Spend same shared Clock, not second room clock. False cellar policy would stop room spending.
    Hero:
        noun: nouns/hero
        requires: {control-vars/clock-runs-in-cellar}
    Clock:
        noun: nouns/clock

case expiry:
    unknown_conditions: Clock-expiry occurrence, not every observation of expired value. Same-frame finish priority and lethal deduplication unresolved. Applies across forms/protection.
    unknown_effects: One lethal occurrence spends one life; later retry separate, no simultaneous resurrection.
    Hero:
        noun: nouns/hero
        becomes: dead
    Clock:
        noun: nouns/clock
        requires: TimeRemaining <= 0[s]
    Lives:
        noun: nouns/lives
        set:
            Amount: this - 1[life]

case finish-stops-spending:
    unknown_conditions: Course finish has committed; simultaneous expiry order unresolved.
    unknown_effects: Stop spending after finish. Pause/menu/transfer are not active spending occurrences; no interval fabricated.
    Clock:
        noun: nouns/clock
```
