---
"id": "01a1e639-9226-794c-9c11-8e1e76c5bc29"
"name": "ore-per-smelt"
"created_at": "2026-11-16T21:18:15.078Z"
"desc": "Recipe amount for ore-per-smelt."
"tags": []
"status": "accepted"
"value_type": "integer"
"value": 2
"expression": ""
"unit": "ore"
"allowed_values": []
"minimum": 1
---
## Meaning

Ore required and spent per smelt occurrence. mechanics/smelt uses this value in both stock guard and debit.

## Constraints and rationale

Positive integer demo amount in ore units. Not starting inventory, capacity or observed balance. No automatic tuning or adoption from test output.
