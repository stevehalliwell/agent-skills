---
{
  "id": "01a15175-3c00-7945-8043-a72bff731a7f",
  "name": "closed-exit-walk",
  "created_at": "2026-10-19T00:00:00.000Z",
  "desc": "Closed exit walking unresolved",
  "tags": [],
  "status": "accepted",
  "nouns": [
    "nouns/exit"
  ],
  "verb": "verbs/walk",
  "context_nouns": [
    "nouns/player",
    "nouns/switch"
  ],
  "mechanics": [
    "mechanics/switch-exit"
  ],
  "control-vars": [],
  "resources": [],
  "coverage": "unknown"
}
---
# Unresolved case

What occurs when player attempts to enter a closed exit? Leave consequence unknown.

```gd-rule
case closed-entry:
    unknown_conditions: Player attempts entry into closed Exit; input and geometry unencoded.
    unknown_effects: No consequence selected for closed-exit walking.
    Player:
        noun: nouns/player
    Exit:
        noun: nouns/exit
        requires: closed
    Switch:
        noun: nouns/switch
```
