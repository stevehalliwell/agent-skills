---
id: 01a16f86-6879-7e4c-bfa4-a13b9baae177
name: run-score
created_at: 2026-10-24T20:07:21.977Z
desc: Uncapped score retained through retries.
tags: []
status: accepted
noun: nouns/score
initial_control: control-vars/starting-score
---
## Ownership, scope, and lifetime

Current game owns nonnegative score across retries and room transfers. Initial value is configured; capacity is intentionally absent, not zero. No spending or score-to-life conversion.

## Canonical behavior links

mechanics/score-awards defines fixed additions. Future implementation must bound machine storage separately without silently changing design semantics.
