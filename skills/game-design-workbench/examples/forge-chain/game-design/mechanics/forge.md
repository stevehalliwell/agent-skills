---
"id": "01a1e639-9301-7b8e-9812-0eaf94cf4a31"
"name": "forge"
"created_at": "2026-11-16T21:18:15.297Z"
"desc": "Ready forge converts ingots and fuel into keys."
"tags": []
"status": "accepted"
"nouns":
  - "nouns/ingot"
  - "nouns/fuel"
  - "nouns/key"
  - "nouns/forge"
"context_axes": []
"control-vars":
  - "control-vars/ingots-per-key"
  - "control-vars/fuel-per-key"
  - "control-vars/keys-per-forge"
"resources":
  - "resources/ingot-stock"
  - "resources/fuel-stock"
  - "resources/key-stock"
---
## Rule

Ready forge enables one recipe occurrence. Spend ingots from the same explicit pool credited by smelting; spend fuel from shared fuel-stock; credit keys. Readiness is preserved, not spent. No implicit smelting invocation or execution order.

```gd-rule
case forge:
    target: Key
    context: ANY
    Ingot:
        noun: nouns/ingot
        pools:
            IngotAmount: resources/ingot-stock
        requires: IngotAmount >= {control-vars/ingots-per-key}
        set:
            IngotAmount: this - {control-vars/ingots-per-key}
    Fuel:
        noun: nouns/fuel
        pools:
            FuelAmount: resources/fuel-stock
        requires: FuelAmount >= {control-vars/fuel-per-key}
        set:
            FuelAmount: this - {control-vars/fuel-per-key}
    Key:
        noun: nouns/key
        pools:
            KeyAmount: resources/key-stock
        requires: ANY
        set:
            KeyAmount: this + {control-vars/keys-per-forge}
    Forge:
        noun: nouns/forge
        requires: ready
        preserve: Readiness
```

## Scope and unknowns

Starting stocks, readiness, capacity, timing and physical placement remain unspecified. Fuel costs in both recipes remain separate side declarations; route inspection does not sum an inventory budget or prove a feasible schedule.
