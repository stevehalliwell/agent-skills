---
id: 01a16f86-7bec-7afe-b2ca-c9ce45e3088c
name: bump-coin-supply
created_at: 2026-10-24T20:07:26.956Z
desc: Coin supply directly credits one coin and becomes spent.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/supply-block
verb: verbs/bump
context_nouns:
  - nouns/coin
mechanics:
  - mechanics/supply-release
  - mechanics/coin-collection
context_axes:
  - axes/Contact
---
## Conditions

Hero hits an unspent coin supply from below.

## Consequences and feedback

Credit one coin through coin-collection and show payout; turn block spent. Visual coin emergence is feedback, not a second world coin that can pay again.

```gd-rule
case direct-below-threshold:
    context: Hero undersideContacts Block
    unknown_conditions: Valid unspent coin-supply underside hit; credit original payout once. Rising-hit qualifier supplied by supply-release, not inferred from noun.
    unknown_effects: Consume direct payout and show coin feedback/HUD; no second collectible world Coin.
    Hero:
        noun: nouns/hero
    Block:
        noun: nouns/supply-block
        requires: unspent AND SupplyContent.coin
        becomes: spent
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
    context: Hero undersideContacts Block
    unknown_conditions: Valid unspent coin-supply underside hit; original payout once. Rising-hit qualifier from supply-release.
    unknown_effects: Direct payout consumed, HUD feedback; immediate threshold conversion even at life cap. Not another world Coin or sequential double assignment. Multiple thresholds from unstable initial banks unspecified.
    Hero:
        noun: nouns/hero
    Block:
        noun: nouns/supply-block
        requires: unspent AND SupplyContent.coin
        becomes: spent
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
