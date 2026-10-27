---
id: 01a17bd2-156f-7143-9827-8fc6e196cc0b
name: ExitState
created_at: 2026-10-27T05:25:28.047Z
desc: Known open and closed exit states from switch-exit.
tags: []
status: proposed
value_type: enum
allowed_values:
  - closed
  - open
unit: ""
closed: false
---
## Meaning

Known `closed`/`open` states of [exit](../nouns/exit.md), from [switch-exit](../mechanics/switch-exit.md). A crate occupying the switch opens the exit; removing it closes the exit. This definition does not fill the unresolved closed-exit walking case. Vocabulary stays open; no extra states or reachable combinations inferred.
