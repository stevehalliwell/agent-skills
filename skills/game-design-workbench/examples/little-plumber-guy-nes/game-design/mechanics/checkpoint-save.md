---
id: 01a16f86-69b5-7c69-a970-2739a77e1e4d
name: checkpoint-save
created_at: 2026-10-24T20:07:22.293Z
desc: Crossing the progress trigger changes the retry anchor for this game.
tags: []
status: accepted
nouns:
  - nouns/checkpoint
  - nouns/hero
verbs:
  - verbs/touch
---
## Rule

Crossing checkpoint-strip changes the current game's retry anchor. Crossing alone grants no life, clock refill or power. Future death uses that anchor and retry resets. New game clears anchor; there is no persistent save file.

## Feedback

Trigger is invisible in the world. Whether to add a HUD acknowledgement is open, so the data does not promise that players notice it.
