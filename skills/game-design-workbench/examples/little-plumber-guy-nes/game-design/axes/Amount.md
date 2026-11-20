---
id: 01a17bd2-16bf-7cb3-bb94-671720d3190d
name: Amount
created_at: 2026-10-27T05:25:28.383Z
desc: Integer retry quantity on the existing lives noun.
tags: []
status: proposed
value_type: integer
allowed_values: []
unit: life
closed: false
---
## Meaning

Integer retry quantity on canonical [lives](../nouns/lives.md), measured in `life` units. [Retry-budget](../resources/retry-budget.md) retains initial/capacity configuration; [death-and-retry](../mechanics/death-and-retry.md) and [earned-life](../mechanics/earned-life.md) own changes. No new starting quantity, bound or overflow policy inferred. Lives are not hero form or health-like power.
