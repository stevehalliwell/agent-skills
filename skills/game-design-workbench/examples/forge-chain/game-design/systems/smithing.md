---
"id": "01a1e639-9342-7d53-b9b6-03ed8875d3f7"
"name": "smithing"
"created_at": "2026-11-16T21:18:15.362Z"
"desc": "Two recipes share ingot and fuel pools without implied scheduling."
"tags": []
"status": "accepted"
"mechanics":
  - "mechanics/smelt"
  - "mechanics/forge"
---
## Arrangement

[Smelting](../mechanics/smelt.md) credits the explicit ingot-stock pool used by [forging](../mechanics/forge.md). Both recipes spend fuel-stock and require their own station ready. Shared pools describe identity; system membership supplies no invocation or automatic scheduling.

## Open questions

Starting inventory, readiness, storage ownership, capacity, timing, physical access and intended player experience remain unspecified. No conservation, profitability, reachability or final-outcome proof.
