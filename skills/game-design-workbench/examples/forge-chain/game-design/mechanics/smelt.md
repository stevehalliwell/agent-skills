---
"id": "01a1e639-9301-7c34-8b17-76fe117f4dd9"
"name": "smelt"
"created_at": "2026-11-16T21:18:15.297Z"
"desc": "Ready furnace converts ore and fuel into ingots."
"tags": []
"status": "accepted"
"nouns":
  - "nouns/ore"
  - "nouns/fuel"
  - "nouns/ingot"
  - "nouns/furnace"
"context_axes": []
"control-vars":
  - "control-vars/ore-per-smelt"
  - "control-vars/fuel-per-smelt"
  - "control-vars/ingots-per-smelt"
"resources":
  - "resources/ore-stock"
  - "resources/fuel-stock"
  - "resources/ingot-stock"
---
## Rule

Ready furnace enables one recipe occurrence. Spend ore and fuel; credit ingots in shared ingot-stock. Readiness is preserved, not spent. All quantities read the same starting facts; no call invokes forging.

```gd-rule
case smelt:
    target: Ingot
    context: ANY
    Ore:
        noun: nouns/ore
        pools:
            OreAmount: resources/ore-stock
        requires: OreAmount >= {control-vars/ore-per-smelt}
        set:
            OreAmount: this - {control-vars/ore-per-smelt}
    Fuel:
        noun: nouns/fuel
        pools:
            FuelAmount: resources/fuel-stock
        requires: FuelAmount >= {control-vars/fuel-per-smelt}
        set:
            FuelAmount: this - {control-vars/fuel-per-smelt}
    Ingot:
        noun: nouns/ingot
        pools:
            IngotAmount: resources/ingot-stock
        requires: ANY
        set:
            IngotAmount: this + {control-vars/ingots-per-smelt}
    Furnace:
        noun: nouns/furnace
        requires: ready
        preserve: Readiness
```

## Scope and unknowns

No current stocks, initial readiness, capacity, timing or physical placement supplied. Named controls define demo amounts, not measured balance. Conditional structural witness cannot establish that this occurrence happens or enables a later occurrence.
