---
id: 01a17c6d-284b-7741-b0f1-fc237a3aaa1c
name: Contact
created_at: 2026-10-27T08:14:50.955Z
desc: Ordered contact vocabulary used by the powered walker-contact rule; not
  actor state or observed geometry.
tags: []
status: proposed
value_type: enum
allowed_values:
  - sideContacts
unit: ""
minimum: null
maximum: null
closed: false
---
## Meaning

Ordered contact vocabulary used by [powered walker contact](../interactions/touch-walker-powered.md). `sideContacts` retains that rule's authored `Player sideContacts Walker` relation. Operands refer to its local participants; swapping them does not establish the same condition.

Rule `context_axes` declares this vocabulary. No noun implicitly gains a Contact state. Open vocabulary; this record does not enumerate other directions, define collision geometry, measure a scene, supply initial state or prove that any contact occurs. Other contact cases still need faithful rule migration. [Model limits](../../model-limits.md) remain.
