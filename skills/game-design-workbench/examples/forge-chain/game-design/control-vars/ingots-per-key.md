---
"id": "01a1e639-9226-7c83-a318-0fe12fc45890"
"name": "ingots-per-key"
"created_at": "2026-11-16T21:18:15.078Z"
"desc": "Recipe amount for ingots-per-key."
"tags": []
"status": "accepted"
"value_type": "integer"
"value": 1
"expression": ""
"unit": "ingot"
"allowed_values": []
"minimum": 1
---
## Meaning

Ingots required and spent per forge occurrence. mechanics/forge uses this value in stock guard and debit.

## Constraints and rationale

Positive integer demo amount in ingot units. Same noun/type does not establish source pool identity; explicit rule bindings do.
