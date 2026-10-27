---
id: 01a16f86-69b5-732a-9e3c-1e5d7a294561
name: supply-release
created_at: 2026-10-24T20:07:22.293Z
desc: A rising head hit releases one authored reward and spends the block.
tags: []
status: accepted
nouns:
  - nouns/supply-block
  - nouns/spent-block
  - nouns/grow-cap
  - nouns/spark-flower
  - nouns/life-cap
  - nouns/coin
verbs:
  - verbs/bump
context_axes:
  - axes/Contact
---
## Rule

An unspent block pays once on a rising underside hit. Authored coin content credits coin-collection directly. Growth content releases grow-cap for small form or spark-flower otherwise. Life content releases life-cap. Block becomes spent and solid. Concealed supply appears on its valid hit.

## Feedback

Show release and spent state clearly. No randomized drop table or repeated coin payout.

```gd-rule
case coin-below:
    context: Hero undersideContacts Block
    unknown_conditions: Valid original payout hit once; event identity/geometry unencoded.
    unknown_effects: Direct coin-collection payout, not second world Coin; show payout/spent state and retain solid Block. No randomized/repeated payout.
    Hero:
        noun: nouns/hero
        requires: rising
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

case coin-threshold:
    context: Hero undersideContacts Block
    unknown_conditions: Valid original payout hit once; event identity/geometry unencoded.
    unknown_effects: Direct credit and immediate conversion even at life cap; show spent/payout/HUD, keep solid Block. No second Coin or repeated payout. Multiple thresholds from unstable initial bank unspecified.
    Hero:
        noun: nouns/hero
        requires: rising
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

case growth-small:
    context: Hero undersideContacts Block
    unknown_conditions: Valid authored single-use hit.
    unknown_effects: Release one moving Grow-cap above Block, keep spent solid geometry. Not collection or random drop; spawn geometry/motion unconfigured.
    Hero:
        noun: nouns/hero
        requires: rising AND small
    Block:
        noun: nouns/supply-block
        requires: unspent AND SupplyContent.growth
        becomes: spent

case growth-powered:
    context: Hero undersideContacts Block
    unknown_conditions: Valid authored single-use hit.
    unknown_effects: Release one stationary Spark-flower above Block, keep spent solid geometry. No random drop or immediate form upgrade; exact placement unknown.
    Hero:
        noun: nouns/hero
        requires: rising AND (tall OR spark)
    Block:
        noun: nouns/supply-block
        requires: unspent AND SupplyContent.growth
        becomes: spent

case life-release:
    context: Hero undersideContacts Block
    unknown_conditions: Valid authored single-use hit.
    unknown_effects: Release Life-cap, not immediate life award; keep spent Block solid. Exact spawn geometry unknown.
    Hero:
        noun: nouns/hero
        requires: rising
    Block:
        noun: nouns/supply-block
        requires: unspent AND SupplyContent.life
        becomes: spent

case concealed-reveal:
    context: Hero undersideContacts Block
    unknown_conditions: Same valid single-use rising payout hit on concealed authored supply; not independent second payout.
    unknown_effects: Make concealed supply visible; selected content case supplies same-occurrence payout/spending. Partial compatible declaration, no implicit invocation or duplicate coins.
    Hero:
        noun: nouns/hero
        requires: rising
    Block:
        noun: nouns/supply-block
        requires: unspent AND concealed
        becomes: revealed

case spent-no-payout:
    unknown_conditions: Later hit after original payout.
    unknown_effects: Remain solid and dispense nothing; no Brick-rule break. Show release/spent state clearly.
    Block:
        noun: nouns/supply-block
        requires: spent
        preserve: SupplyState
```
