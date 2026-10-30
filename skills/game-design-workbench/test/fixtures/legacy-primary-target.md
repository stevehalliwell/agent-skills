---
{ "id": "01a15175-3c00-7b48-808f-34e6fe935d28", "name": "occupied-switch", "created_at": "2026-10-19T00:00:00.000Z", "desc": "Crate activates switch", "tags": [], "status": "accepted", "nouns": [ "nouns/switch" ], "verb": "verbs/activate", "context_nouns": [ "nouns/crate", "nouns/exit" ], "mechanics": [ "mechanics/switch-exit" ], "control-vars": [], "resources": [], "coverage": "defined", context_axes: [ axes/Contact ] }
---
# Occupied switch

## Conditions
Crate occupies switch.

## Consequences
Apply [switch and exit](../mechanics/switch-exit.md). Exit state is visible; sound feedback unspecified.

```gd-rule
case occupied:
    context: Crate occupies Switch
    unknown_effects: Apply switch-exit; make Exit state visible. Sound feedback unspecified.
    Crate:
        noun: nouns/crate
    Switch:
        noun: nouns/switch
        becomes: occupied
    Exit:
        noun: nouns/exit
        becomes: open
```
