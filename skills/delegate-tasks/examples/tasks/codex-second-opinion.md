# Codex delegated second opinion

## Goal

Independently assess the named local implementation and identify the most important correctness or simplicity risks.

## Inputs

- Read the files named by the parent task before making conclusions.
- This is a bounded, ephemeral one-shot task. It receives this prompt, not the parent conversation.

## Constraints

- Do not modify files.
- Use current web information only when the task's `web_search` setting is `true` and the goal requires it.
- Do not wait for approvals; work within the configured sandbox.
- Separate evidence from recommendation.

## Deliverable

Return a short second opinion: key findings, relevant file references, and a recommended next action. State explicitly when no material issue is found.
