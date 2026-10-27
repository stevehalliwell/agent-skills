---
{
  "id": "01a15175-3c00-7a58-803c-f7e2cfabbd6f",
  "name": "grid-walking",
  "created_at": "2026-10-19T00:00:00.000Z",
  "desc": "Grid walking rule",
  "tags": [],
  "status": "accepted",
  "nouns": [
    "nouns/player",
    "nouns/wall"
  ],
  "verbs": [
    "verbs/walk"
  ],
  "control-vars": [
    "control-vars/tile-distance",
    "control-vars/walk-speed",
    "control-vars/walk-duration"
  ],
  "resources": []
}
---
# Grid walking

Player walks into free adjacent cells; walls block movement. Walking remains possible at zero energy. Walk duration references canonical control variables.

```gd-rule
case free-adjacent-cell:
    unknown_conditions: Walk request into free adjacent cell; zero energy does not disable walking. Geometry/direction unencoded.
    unknown_effects: Move Player into cell; duration references tile-distance, walk-speed and walk-duration. No current/initial coordinates supplied.
    Player:
        noun: nouns/player

case wall-blocked:
    unknown_conditions: Wall blocks requested walking.
    unknown_effects: Prevent this movement; unrelated results unspecified.
    Player:
        noun: nouns/player
    Wall:
        noun: nouns/wall
```
