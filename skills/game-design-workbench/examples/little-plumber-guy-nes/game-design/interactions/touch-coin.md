---
id: 01a16f86-7bec-7b8a-ace9-96b89bf1bcf5
name: touch-coin
created_at: 2026-10-24T20:07:26.956Z
desc: Token disappears and is credited once.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/coin
verb: verbs/touch
context_nouns:
  - nouns/hero
mechanics:
  - mechanics/coin-collection
context_axes:
  - axes/Contact
---
## Conditions

Alive hero overlaps an uncollected world coin.

## Consequences and feedback

Remove token, credit bank and score, then process threshold conversion. Update HUD. Enemy and projectile contacts do not collect coins for hero.

```gd-rule
case below-threshold:
    context: Hero overlaps Coin
    unknown_conditions: Collect original world Coin once; event identity/deduplication unencoded. Enemy/projectile contacts do not collect for Hero.
    unknown_effects: Remove token, credit bank/score and update HUD; no clock restoration, growth or protection.
    Hero:
        noun: nouns/hero
        requires: alive
    Coin:
        noun: nouns/coin
        requires: available
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

case threshold:
    context: Hero overlaps Coin
    unknown_conditions: Original world Coin credited once; event identity unencoded. Enemy/projectile contacts cannot collect.
    unknown_effects: Remove token, update HUD; immediate threshold conversion even at life cap. This final result is not two conflicting writes or repeated collection; multiple thresholds from unstable initial banks unspecified.
    Hero:
        noun: nouns/hero
        requires: alive
    Coin:
        noun: nouns/coin
        requires: available
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
```
