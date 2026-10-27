---
id: 01a16f86-6ff6-7383-b466-3b5755f5c97f
name: checkpoint-strip
created_at: 2026-10-24T20:07:23.894Z
desc: Safe strip anchors retries before the last spatial sequence.
tags: []
status: accepted
entry: false
next:
  - challenges/stair-fork
introduces:
  - skill-atoms/manage-clock
mechanics:
  - mechanics/checkpoint-save
  - mechanics/death-and-retry
nouns:
  - nouns/checkpoint
---
## Goal and context

Safe strip contains retry-anchor trigger before the ending sequence.

## Assistance and prior knowledge

Opportunity to notice remaining time and prepare a final approach. Crossing checkpoint is not a refill or proof of clock awareness.

## Route

Continue to stair-fork. Death can return here through stateful retry, but those retry edges are not encoded as ordinary next links.
