---
id: 01a16f86-6ff6-707d-87d4-6420ace35548
name: first-walker
created_at: 2026-10-24T20:07:23.894Z
desc: First moving threat invites jump or retreat.
tags: []
status: accepted
entry: false
next:
  - challenges/first-supply
introduces:
  - skill-atoms/jump
  - skill-atoms/stomp-walker
  - skill-atoms/land-on-target
  - skill-atoms/jump-stomp
requires:
  - skill-atoms/move-right
mechanics:
  - mechanics/walker-stomp
  - mechanics/enemy-contact
nouns:
  - nouns/walker
---
## Goal and context

A lone walker approaches across flat floor. Get past it by jumping over or attempting a stomp; taking damage is possible, not required.

## Assistance and prior knowledge

Space behind hero allows retreat. Jumping, target landing, above-impact understanding and the proposed jump-stomp combination are introduced together; whether that first threat teaches them legibly is untested. This does not claim an opportunity to practise the fall-stomp method.

## Decision and assessment

Contextual assessment uses [shared vocabulary](../../../../references/decision-structure.md). Task: get past Walker alive. Jumping over, attempting a stomp and retreat are available; defeating Walker is not required.

Survival test combines contact understanding and execution. In [touch-walker-small](../interactions/touch-walker-small.md), case `lethal-side`, small, unprotected Hero dies from harmful side contact. That response fails this survival task rather than expressing a different reward preference. Protected/powered contact has different conditions; do not generalize lethal result to every contact.

Player may know correct response but mistime it. No knowledge-only quiz, acquired mastery or observed mistake is established. Deliberation, reaction and input-precision windows are unspecified; execution alone does not establish a particular reaction or dexterity demand.

Intent check: should first encounter primarily teach contact distinction, test execution, or combine both? Multiple new skills arrive together; current source does not establish whether that demand fits intended introduction.

## Route

Continue to first-supply. Next does not assert that enemy was defeated.
