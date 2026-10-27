---
id: 01a16f86-69b6-7fc4-917f-09fab5c850e0
name: earned-life
created_at: 2026-10-24T20:07:22.294Z
desc: Coin threshold or life-cap grants a life up to capacity.
tags: []
status: accepted
nouns:
  - nouns/coin-bank
  - nouns/life-cap
  - nouns/lives
  - nouns/hero
verbs:
  - verbs/touch
control-vars:
  - control-vars/coins-for-life
  - control-vars/life-capacity
resources:
  - resources/coin-progress
  - resources/retry-budget
context_axes:
  - axes/Contact
---
## Rule

On coin threshold, subtract coins-for-life and add a life, clamped at life-capacity. Consume life-cap pickup for the same capped award without changing coin bank. At capacity, award is lost but token conversion/pickup consumption still occurs. No score-based life rule.

## Feedback

Show life count change when one occurs; capacity must not wrap to zero.

```gd-rule
case coin-threshold:
    unknown_conditions: Threshold-conversion occurrence, not repeated application to same original event. Multiple thresholds from unstable bank state unspecified.
    unknown_effects: Convert coins even at life cap; capped award lost there. Show life count change when any; no score-based life rule or wrapping lives.
    Bank:
        noun: nouns/coin-bank
        requires: CoinAmount >= {control-vars/coins-for-life}
        set:
            CoinAmount: this - {control-vars/coins-for-life}
    Lives:
        noun: nouns/lives
        set:
            Amount: min(this + 1[life], {control-vars/life-capacity})

case life-cap:
    context: Hero overlaps Cap
    unknown_conditions: Original Life-cap collection once; instance identity unencoded.
    unknown_effects: Consume even at cap; award lost there without bank change. No score-based life or wrap; show count change when any.
    Hero:
        noun: nouns/hero
    Cap:
        noun: nouns/life-cap
        becomes: consumed
    Bank:
        noun: nouns/coin-bank
        preserve: CoinAmount
    Lives:
        noun: nouns/lives
        set:
            Amount: min(this + 1[life], {control-vars/life-capacity})
```
