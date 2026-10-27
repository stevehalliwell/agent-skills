---
{ "id": "01a15175-3c00-72cd-8510-700280faaab6", "name": "switch-exit", "created_at": "2026-10-19T00:00:00.000Z", "desc": "Switch and exit rule", "tags": [], "status": "accepted", "nouns": [ "nouns/crate", "nouns/switch", "nouns/exit" ], "verbs": [ "verbs/activate" ], "control-vars": [], "resources": [], context_axes: [ axes/Contact ] }
---
# Switch and exit

A crate occupying the switch opens the exit; removing it closes the exit. Closed-exit walking behavior is not yet specified.

```gd-rule
case occupied:
    context: Crate occupies Switch
    Crate:
        noun: nouns/crate
    Switch:
        noun: nouns/switch
        becomes: occupied
    Exit:
        noun: nouns/exit
        becomes: open

case crate-removed:
    unknown_conditions: Crate removed from this Switch; event and geometry unencoded.
    unknown_effects: Closed-exit walking remains unspecified.
    Crate:
        noun: nouns/crate
    Switch:
        noun: nouns/switch
        becomes: empty
    Exit:
        noun: nouns/exit
        becomes: closed
```
