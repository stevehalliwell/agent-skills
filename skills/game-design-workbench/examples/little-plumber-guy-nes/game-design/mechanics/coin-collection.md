---
id: 01a16f86-69b5-79c4-b2e0-8cd155e521bd
name: coin-collection
created_at: 2026-10-24T20:07:22.293Z
desc: Collecting a token updates coin bank and score exactly once.
tags: []
status: accepted
nouns:
  - nouns/coin
  - nouns/coin-bank
  - nouns/score
  - nouns/hero
verbs:
  - verbs/touch
control-vars:
  - control-vars/coin-points
resources:
  - resources/coin-progress
  - resources/run-score
context_axes:
  - axes/Contact
---
## Rule

Hero collection removes a world coin or consumes one direct block payout, increments coin bank and adds coin-points to score. Apply earned-life conversion immediately if threshold is reached. A consumed instance cannot pay twice.

## Feedback

HUD bank and score change. Collection does not restore clock, grow hero or protect against enemies.

```gd-rule
case world-below:
    context: Hero overlaps Coin
    unknown_conditions: Hero collection of original world Coin once; event identity unencoded.
    unknown_effects: Remove token/update HUD. No Clock restoration, growth or enemy protection.
    Hero:
        noun: nouns/hero
    Coin:
        noun: nouns/coin
        becomes: consumed
    Bank:
        noun: nouns/coin-bank
        requires: CoinAmount < {control-vars/coins-for-life} - 1[coin]
        set:
            CoinAmount: this + 1[coin]
    Score:
        noun: nouns/score
        set:
            ScoreAmount: this + {control-vars/coin-points}

case world-threshold:
    context: Hero overlaps Coin
    unknown_conditions: Original world Coin collection credited once; event identity unencoded.
    unknown_effects: Remove token/update HUD, immediate conversion even at life cap; no sequential second credit. Multiple thresholds from unstable initial bank unspecified. No Clock restoration/growth/protection.
    Hero:
        noun: nouns/hero
    Coin:
        noun: nouns/coin
        becomes: consumed
    Bank:
        noun: nouns/coin-bank
        requires: CoinAmount >= {control-vars/coins-for-life} - 1[coin]
        set:
            CoinAmount: this + 1[coin] - {control-vars/coins-for-life}
    Score:
        noun: nouns/score
        set:
            ScoreAmount: this + {control-vars/coin-points}
    Lives:
        noun: nouns/lives
        set:
            Amount: min(this + 1[life], {control-vars/life-capacity})

case direct-below:
    unknown_conditions: Hero collection consumes one direct block payout, not second world Coin; once-only event identity unencoded.
    unknown_effects: Consume payout/update HUD. No Clock restoration, growth or protection.
    Hero:
        noun: nouns/hero
    Bank:
        noun: nouns/coin-bank
        requires: CoinAmount < {control-vars/coins-for-life} - 1[coin]
        set:
            CoinAmount: this + 1[coin]
    Score:
        noun: nouns/score
        set:
            ScoreAmount: this + {control-vars/coin-points}

case direct-threshold:
    unknown_conditions: Hero consumes original direct block payout once, not second world Coin; event identity unencoded.
    unknown_effects: Consume payout/update HUD; immediate conversion even at life cap. Multiple thresholds from unstable bank unspecified; no restoration/growth/protection.
    Hero:
        noun: nouns/hero
    Bank:
        noun: nouns/coin-bank
        requires: CoinAmount >= {control-vars/coins-for-life} - 1[coin]
        set:
            CoinAmount: this + 1[coin] - {control-vars/coins-for-life}
    Score:
        noun: nouns/score
        set:
            ScoreAmount: this + {control-vars/coin-points}
    Lives:
        noun: nouns/lives
        set:
            Amount: min(this + 1[life], {control-vars/life-capacity})
```
