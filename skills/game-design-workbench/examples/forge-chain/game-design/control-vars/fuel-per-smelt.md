---
"id": "01a1e639-9226-7134-953b-3a094830f2e3"
"name": "fuel-per-smelt"
"created_at": "2026-11-16T21:18:15.078Z"
"desc": "Recipe amount for fuel-per-smelt."
"tags": []
"status": "accepted"
"value_type": "integer"
"value": 1
"expression": ""
"unit": "fuel"
"allowed_values": []
"minimum": 1
---
## Meaning

Fuel required and spent per smelt occurrence. Separate from fuel-per-key even though both debit the same explicit fuel pool.

## Constraints and rationale

Positive integer demo amount in fuel units. No starting stock, recipe scheduling or aggregate budget implied.
