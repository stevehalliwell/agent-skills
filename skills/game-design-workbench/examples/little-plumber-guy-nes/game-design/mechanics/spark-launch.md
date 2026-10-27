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
