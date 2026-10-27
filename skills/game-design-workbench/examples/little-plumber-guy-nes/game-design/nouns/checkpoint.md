---
id: 01a16f86-62e3-7d1f-9961-caa882e14c4a
name: checkpoint
created_at: 2026-10-24T20:07:20.547Z
desc: Invisible progress trigger setting the next retry anchor.
tags: []
status: accepted
axes: null
---
## Definition

Invisible trigger in checkpoint-strip. Stores a safe retry anchor without restoring lives, clock or power on crossing. A subsequent retry restores an attempt clock through death-and-retry. Persistent save games are outside scope.
