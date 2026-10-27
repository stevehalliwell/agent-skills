---
id: 01a16f86-66a8-7751-918a-9503dbe8df4f
name: coin-bank-capacity
created_at: 2026-10-24T20:07:21.512Z
desc: Largest stable coin-bank value before threshold conversion.
tags: []
status: accepted
value_type: integer
expression: "${control-vars/coins-for-life} - 1[coin]"
unit: "coin"
allowed_values: []
minimum: 0
---
## Meaning

Derived highest stable bank value between conversion events. Coin-unit literal in the expression exercises typed arithmetic; this is not a separate tuning knob.
