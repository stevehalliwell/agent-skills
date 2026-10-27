---
id: 01a16f86-7bec-79e1-b4bb-e4c79722f1e9
name: touch-life-cap
created_at: 2026-10-24T20:07:26.956Z
desc: Life-cap is consumed; carried lives increase only below capacity.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/life-cap
verb: verbs/touch
context_nouns:
  - nouns/hero
  - nouns/lives
mechanics:
  - mechanics/earned-life
---
## Conditions

Alive hero overlaps life-cap.

## Consequences and feedback

Consume item and award one life up to configured capacity. At capacity, consume without increasing lives. Preserve coin progress and power state; no enemy protection is granted.
