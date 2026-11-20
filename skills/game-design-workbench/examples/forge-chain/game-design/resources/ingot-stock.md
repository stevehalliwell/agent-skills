---
"id": "01a1e639-92ba-7def-8a34-d76aebcff791"
"name": "ingot-stock"
"created_at": "2026-11-16T21:18:15.226Z"
"desc": "Explicit shared ingot pool."
"tags": []
"status": "accepted"
"noun": "nouns/ingot"
---
## Ownership, scope, and lifetime

Explicit intermediate pool on nouns/ingot and axes/IngotAmount. Both recipes deliberately share this pool identity. Physical ownership, storage, initial stock, capacity and persistence remain unspecified.

## Canonical behavior links

mechanics/smelt credits; mechanics/forge debits. This join supplies a conditional structural witness, not automatic transfer, occurrence ordering or feasible execution.
