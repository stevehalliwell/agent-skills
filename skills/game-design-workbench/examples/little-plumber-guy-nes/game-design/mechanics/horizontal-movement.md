---
id: 01a16f86-69b4-73ea-93f7-65ca948dd6ca
name: horizontal-movement
created_at: 2026-10-24T20:07:22.292Z
desc: Walking and running set different horizontal target speeds.
tags: []
status: accepted
nouns:
  - nouns/hero
  - nouns/ground
verbs:
  - verbs/walk
control-vars:
  - control-vars/walk-speed
  - control-vars/run-speed
  - control-vars/tile-size
---
## Rule

Supported hero moves toward facing/input direction at walk-speed, or run-speed while run is held. Walls and camera boundary constrain displacement; stepping beyond support begins falling. Opposite inputs cancel. Air steering is allowed, but acceleration, braking and air-control strength remain unspecified.

## Feedback

Movement and facing should follow input visibly. No inference that target speed is reached instantly in a future implementation.

```gd-rule
case walk-target:
    unknown_conditions: Supported directional input toward facing/input direction with Run not held; walls/Camera constrain travel.
    unknown_effects: Move toward target and visibly follow input/facing; actual displacement, acceleration/braking and instant target attainment not specified.
    Hero:
        noun: nouns/hero
        requires: supported
        set:
            TargetSpeed: {control-vars/walk-speed}
    Ground:
        noun: nouns/ground

case run-target:
    unknown_conditions: Supported directional input with Run held; walls/Camera constrain displacement.
    unknown_effects: Move toward input/facing target; actual velocity/acceleration unconfigured, not instant attainment.
    Hero:
        noun: nouns/hero
        requires: supported
        set:
            TargetSpeed: {control-vars/run-speed}
    Ground:
        noun: nouns/ground

case opposite-inputs:
    unknown_conditions: Opposite horizontal inputs held together.
    unknown_effects: Inputs cancel; not invented immediate stop of actual velocity.
    Hero:
        noun: nouns/hero

case leave-support:
    unknown_conditions: Step beyond support without another authored jump launch.
    unknown_effects: Begin fall; no automatic hop or premature death; geometry unencoded.
    Hero:
        noun: nouns/hero
        becomes: unsupported, falling

case air-steering:
    unknown_conditions: Horizontal input while airborne.
    unknown_effects: Air steering allowed; acceleration, braking and strength unspecified.
    Hero:
        noun: nouns/hero
        requires: unsupported
```
