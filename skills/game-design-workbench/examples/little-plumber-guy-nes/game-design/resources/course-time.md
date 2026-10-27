---
id: 01a16f86-6879-7be6-805b-21529640b483
name: course-time
created_at: 2026-10-24T20:07:21.977Z
desc: Active attempt time shared by overworld and cellar.
tags: []
status: accepted
noun: nouns/clock
initial_control: control-vars/opening-clock
capacity_control: control-vars/opening-clock
---
## Ownership, scope, and lifetime

Current attempt owns remaining active seconds. Overworld and cellar share it. Fresh attempt or retry initializes from opening-clock; transfer preserves remaining value.

## Canonical behavior links

mechanics/active-clock spends; mechanics/course-finish stops; mechanics/death-and-retry initializes next attempt. Expiry is lethal regardless of power.
