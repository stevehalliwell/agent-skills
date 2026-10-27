---
id: 01a17bd2-16bf-75ec-99f3-928b35dc2d42
name: ProtectionTime
created_at: 2026-10-27T05:25:28.383Z
desc: Numeric duration of temporary enemy-contact protection.
tags: []
status: proposed
value_type: number
allowed_values: []
unit: s
closed: false
---
## Meaning

Numeric duration for temporary enemy-contact protection, measured in seconds. [Powered walker contact](../interactions/touch-walker-powered.md) assigns [safety-window](../control-vars/safety-window.md) to this hero quantity. Axis does not own the configured duration or imply a timer tick, clamp, decay implementation or initial protection state. Numeric bounds remain unspecified.
