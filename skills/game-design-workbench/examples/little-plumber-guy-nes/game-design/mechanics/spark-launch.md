---
id: 01a16f86-69b5-7ee6-a6ea-a2b6ad77ae67
name: spark-launch
created_at: 2026-10-24T20:07:22.293Z
desc: Spark form fires while below the live-projectile cap.
tags: []
status: accepted
nouns:
  - nouns/hero
  - nouns/spark
  - nouns/power-state
verbs:
  - verbs/shoot
control-vars:
  - control-vars/spark-capacity
---
## Rule

A shoot press in spark form creates one hero-owned projectile in facing direction if live count is below spark-capacity. Other forms or a full cap yield no shot; requests are not queued. Existing sparks are removed on retry or room transfer.

## Open design

Shot speed, bounce arc and maximum range need implementation-level tuning, not guessed NES constants.

```gd-rule
case fire:
    unknown_conditions: Fresh shoot press; input event/facing/spawn geometry unencoded.
    unknown_effects: Create one Hero-owned Spark facing forward, firing feedback. Concurrent count, not persistent ammo; speed/bounce arc/range unconfigured.
    Hero:
        noun: nouns/hero
        requires: spark AND LiveSparks < {control-vars/spark-capacity}
        set:
            LiveSparks: this + 1[spark]

case at-cap:
    unknown_conditions: Shoot press while at live cap; event timing unencoded.
    unknown_effects: No shot/no queued request; later slot release does not fire. Zero cap deliberately disables creation.
    Hero:
        noun: nouns/hero
        requires: spark AND LiveSparks == {control-vars/spark-capacity}

case other-form:
    unknown_conditions: Shoot press.
    unknown_effects: No projectile; no queued request or resource cost.
    Hero:
        noun: nouns/hero
        requires: small OR tall
        preserve: Form

case retry-clears:
    unknown_conditions: Later retry occurrence, not same lethal transition.
    unknown_effects: Remove existing transient Sparks; other reset details belong to death-and-retry.
    Hero:
        noun: nouns/hero
        set:
            LiveSparks: 0[spark]

case transfer-clears:
    unknown_conditions: Authored room-transfer occurrence.
    unknown_effects: Remove existing Sparks; no queued request retroactively fires.
    Hero:
        noun: nouns/hero
        set:
            LiveSparks: 0[spark]
```
