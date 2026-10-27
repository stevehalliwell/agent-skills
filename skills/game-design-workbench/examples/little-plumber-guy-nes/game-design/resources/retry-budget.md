---
id: 01a16f86-6878-786b-b544-2806c9b993b4
name: retry-budget
created_at: 2026-10-24T20:07:21.976Z
desc: Lives pool; loss and earning rules live in mechanics.
tags: []
status: accepted
noun: nouns/lives
initial_control: control-vars/starting-lives
capacity_control: control-vars/life-capacity
---
## Ownership, scope, and lifetime

Current game owns lives pool across attempts and room transfers. New game uses starting-lives; capacity uses life-capacity. Lethal event spends one; exhaustion ends game.

## Canonical behavior links

mechanics/death-and-retry spends; mechanics/earned-life adds with clamping. Power-state damage absorption is separate from this pool.
