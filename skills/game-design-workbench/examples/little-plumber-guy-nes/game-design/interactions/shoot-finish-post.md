---
id: 01a16f86-7bed-7386-be96-4c99532d47e7
name: shoot-finish-post
created_at: 2026-10-24T20:07:26.957Z
desc: Projectile activation of the finish is outside this design.
tags: []
status: accepted
coverage: excluded
nouns:
  - nouns/finish-post
verb: verbs/shoot
mechanics:
  - mechanics/course-finish
---
## Scope exclusion

Remote projectile activation of finish-post is deliberately outside this design. Hero must reach the marker. Coverage excluded expresses chosen scope, not a missing rule to invent or a general claim that every object ignores projectiles.

```gd-rule
case excluded-remote-activation:
    unknown_conditions: Coverage excluded: remote projectile activation deliberately outside design, not gameplay applicability.
    unknown_effects: Hero must reach marker; no universal projectile-ignore collision behavior declared.
    Finish:
        noun: nouns/finish-post
```
