---
"id": "01a1e639-92ba-7bcc-ab27-d57a98fc3dc1"
"name": "fuel-stock"
"created_at": "2026-11-16T21:18:15.226Z"
"desc": "Explicit shared fuel pool."
"tags": []
"status": "accepted"
"noun": "nouns/fuel"
---
## Ownership, scope, and lifetime

One explicit fuel pool shared by both recipes on nouns/fuel and axes/FuelAmount. Physical storage, owner, starting stock, capacity and persistence remain unspecified. Shared identity does not provide a sufficient inventory budget.

## Canonical behavior links

mechanics/smelt and mechanics/forge each declare their own guarded debit. No replenishment rule authored.
