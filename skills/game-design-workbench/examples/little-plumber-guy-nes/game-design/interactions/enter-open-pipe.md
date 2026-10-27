---
id: 01a16f86-7bed-7187-8cdc-f906ac8b35c9
name: enter-open-pipe
created_at: 2026-10-24T20:07:26.957Z
desc: Aligned downward input on declared entrance transfers rooms.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/pipe
verb: verbs/enter-pipe
context_nouns:
  - nouns/hero
mechanics:
  - mechanics/pipe-transfer
---
## Conditions

Hero is aligned and supported on pipe-steps' declared entrance and presses down.

## Consequences and feedback

Transfer to coin-cellar with carried state and remaining clock. Camera anchor changes; active shots clear. Transport-transition time itself is not active course time.

```gd-rule
case entrance-transfer:
    unknown_conditions: Hero aligned/supported on pipe-steps declared entrance and presses Down. Instance identity/alignment geometry unencoded.
    unknown_effects: Transfer to coin-cellar, reanchor Camera, clear active shots. Transition time not active Clock time. Carry values explicitly preserved by pipe-transfer; other reset axes unspecified.
    Hero:
        noun: nouns/hero
        requires: supported
        preserve: Form
        set:
            LiveSparks: 0[spark]
    Pipe:
        noun: nouns/pipe
    Camera:
        noun: nouns/camera
    Clock:
        noun: nouns/clock
        preserve: TimeRemaining
    Lives:
        noun: nouns/lives
        preserve: Amount
    Bank:
        noun: nouns/coin-bank
        preserve: CoinAmount
    Score:
        noun: nouns/score
        preserve: ScoreAmount
```
