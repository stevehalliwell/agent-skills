---
id: 01a16f86-69b6-7041-9787-0bceb2a13c70
name: pipe-transfer
created_at: 2026-10-24T20:07:22.294Z
desc: Declared entrance connects overworld to cellar and one-way return.
tags: []
status: accepted
nouns:
  - nouns/pipe
  - nouns/exit-pipe
  - nouns/hero
  - nouns/camera
verbs:
  - verbs/enter-pipe
resources:
  - resources/course-time
---
## Rule

Aligned down input on pipe-steps' declared entrance transfers to coin-cellar. Its exit transfers to bonus-return near shell-lane; no reverse transfer. Preserve lives, bank, score, power and remaining clock; clear transient shots and reset camera anchor. Ordinary pipes stay solid.

## Boundary

Challenge next links show route alternatives, not the alignment predicate or transfer state. No portal coordinates are stored.

```gd-rule
case entrance:
    unknown_conditions: Aligned Down input on pipe-steps declared entrance; instance/alignment predicate unencoded.
    unknown_effects: Transfer to coin-cellar, clear transient shots, reset Camera anchor. Transition time not active play. Other reset axes unspecified.
    Hero:
        noun: nouns/hero
        preserve: Form
        set:
            LiveSparks: 0[spark]
    Connector:
        noun: nouns/pipe
    Camera:
        noun: nouns/camera
    Lives:
        noun: nouns/lives
        preserve: Amount
    Bank:
        noun: nouns/coin-bank
        preserve: CoinAmount
    Score:
        noun: nouns/score
        preserve: ScoreAmount
    Clock:
        noun: nouns/clock
        preserve: TimeRemaining

case one-way-return:
    unknown_conditions: Use coin-cellar exit connector; no reverse transfer. Instance/coordinates unencoded.
    unknown_effects: Transfer to bonus-return near shell-lane; clear shots and reset Camera anchor. No recovery of skipped course opportunities through reverse transfer. Other reset dimensions unspecified.
    Hero:
        noun: nouns/hero
        preserve: Form
        set:
            LiveSparks: 0[spark]
    Connector:
        noun: nouns/exit-pipe
    Camera:
        noun: nouns/camera
    Lives:
        noun: nouns/lives
        preserve: Amount
    Bank:
        noun: nouns/coin-bank
        preserve: CoinAmount
    Score:
        noun: nouns/score
        preserve: ScoreAmount
    Clock:
        noun: nouns/clock
        preserve: TimeRemaining

case ordinary-solid-pipe:
    unknown_conditions: Pipe lacks authored entrance connection.
    unknown_effects: Ordinary Pipe remains solid; no transfer or invented destination.
    Hero:
        noun: nouns/hero
    Pipe:
        noun: nouns/pipe
```
