---
"id": "01a1e639-92ba-7b85-8858-1d4b9a5031a2"
"name": "ore-stock"
"created_at": "2026-11-16T21:18:15.226Z"
"desc": "Explicit shared ore pool."
"tags": []
"status": "accepted"
"noun": "nouns/ore"
---
## Ownership, scope, and lifetime

Explicit ore pool for this example. Extends nouns/ore on axes/OreAmount; does not create another ore entity. Physical storage, owner, starting stock, capacity and persistence remain unspecified.

## Canonical behavior links

mechanics/smelt explicitly binds this pool and debits the named recipe amount. No replenishment rule authored.
