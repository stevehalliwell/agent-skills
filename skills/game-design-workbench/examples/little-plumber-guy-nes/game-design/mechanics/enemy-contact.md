---
id: 01a16f86-69b5-76f4-96bd-5c0566ed7395
name: enemy-contact
created_at: 2026-10-24T20:07:22.293Z
desc: Unprotected side contact costs power or a life; protection blocks repeated enemy damage.
tags: []
status: accepted
nouns:
  - nouns/hero
  - nouns/walker
  - nouns/shellback
  - nouns/shell
  - nouns/power-state
verbs:
  - verbs/touch
control-vars:
  - control-vars/safety-window
resources:
  - resources/retry-budget
---
## Rule

Unprotected harmful side contact with walker, shellback or sliding shell kills small form. Tall or spark instead becomes small and gains safety-window enemy protection. Protected contacts cause no further enemy damage. Gap death and clock expiry ignore this protection.

## Feedback

Show power loss and protected state distinctly. Descending top contacts use their own stomp cases, not this side-contact rule.
