---
id: 01a16f86-66a8-7b59-8ff3-286ce0c965a9
name: spark-capacity
created_at: 2026-10-24T20:07:21.512Z
desc: Maximum live player projectiles at once.
tags: []
status: accepted
value_type: integer
expression: ""
unit: "spark"
allowed_values: []
value: 2
minimum: 0
maximum: 8
---
## Meaning

Concurrent live shots allowed for this hero. A zero value deliberately disables shot creation without changing power state. Unfulfilled shot requests are not queued.
