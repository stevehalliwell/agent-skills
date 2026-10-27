---
id: 01a16f86-69b6-73fb-aed4-6db9b5204968
name: score-awards
created_at: 2026-10-24T20:07:22.294Z
desc: Fixed awards reward events; no hidden chain multiplier or spending.
tags: []
status: accepted
nouns:
  - nouns/score
  - nouns/coin
  - nouns/brick
  - nouns/walker
  - nouns/shellback
control-vars:
  - control-vars/coin-points
  - control-vars/brick-points
  - control-vars/walker-points
  - control-vars/shellback-points
resources:
  - resources/run-score
---
## Rule

Apply named fixed point awards for coin credit, brick destruction and original enemy defeat/retraction. Credit each event once. Shell stops and kicks do not farm the original enemy award. Score survives retries but cannot be spent and grants no lives.

## Boundary

This is an authored feedback economy, not a player-skill or enjoyment metric.

```gd-rule
case coin-credit:
    unknown_conditions: Original coin-credit occurrence credited once; event identity unencoded. Not a second award atop same collection declaration.
    Score:
        noun: nouns/score
        set:
            ScoreAmount: this + {control-vars/coin-points}

case brick-destruction:
    unknown_conditions: Original Brick destruction credited once; event identity unencoded. Not another independent award for same event.
    Score:
        noun: nouns/score
        set:
            ScoreAmount: this + {control-vars/brick-points}

case walker-defeat:
    unknown_conditions: Original Walker defeat credited once; event identity unencoded; no chain multiplier.
    Score:
        noun: nouns/score
        set:
            ScoreAmount: this + {control-vars/walker-points}

case original-shellback-defeat-or-retraction:
    unknown_conditions: Original Shellback defeat OR retraction credited once, not repeated Shell kick/stop; event identity unencoded.
    Score:
        noun: nouns/score
        set:
            ScoreAmount: this + {control-vars/shellback-points}

case shell-kick-or-stop:
    unknown_conditions: Shell kick/stop without new original-enemy award.
    unknown_effects: No repeat award; no hidden chain multiplier.
    Score:
        noun: nouns/score
        preserve: ScoreAmount

case retry-lifetime:
    unknown_conditions: Later retry occurrence.
    unknown_effects: Score survives retry, cannot be spent and grants no lives; not player-skill/enjoyment metric.
    Score:
        noun: nouns/score
        preserve: ScoreAmount
```
