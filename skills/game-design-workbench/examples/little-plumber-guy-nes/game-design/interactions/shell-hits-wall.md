---
id: 01a16f86-7bec-76ff-9d31-dc4850df535a
name: shell-hits-wall
created_at: 2026-10-24T20:07:26.956Z
desc: Sliding shell reverses at a solid wall or pipe.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/shell
verb: verbs/touch
context_nouns:
  - nouns/pipe
  - nouns/step
mechanics:
  - mechanics/shell-travel
  - mechanics/solid-collision
---
## Conditions

Sliding shell reaches solid side of pipe or step.

## Consequences and feedback

Reverse horizontal direction and remain sliding. Returning shell can threaten its kicker. No reward, pipe transport or wall destruction occurs.
