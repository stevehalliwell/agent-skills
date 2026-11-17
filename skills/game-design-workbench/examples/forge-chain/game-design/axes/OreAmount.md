---
"id": "01a1e639-91e1-7cbd-992a-79de05b2787c"
"name": "OreAmount"
"created_at": "2026-11-16T21:18:15.009Z"
"desc": "Nonnegative integer ore quantity."
"tags": []
"status": "accepted"
"value_type": "integer"
"allowed_values": []
"unit": "ore"
"minimum": 0
"closed": false
---
## Meaning

Nonnegative integer quantity in ore units. nouns/ore declares applicability; resources/ore-stock supplies explicit pool identity; mechanics/smelt owns spending. Minimum is a domain bound, not a current value or automatic clamp. Initial quantity and capacity remain unknown.
