# Pi delegated review

## Goal

Review the named local implementation for correctness, maintainability, and test coverage.

## Inputs

- Read the files named by the parent task before making conclusions.
- This is an isolated Pi session: it receives this prompt, not the parent conversation.
- The batch allowlists `technical-review`. Use only resources explicitly available to this child.

## Constraints

- Do not modify files.
- Do not use web/network access unless a separately allowlisted resource explicitly provides it.
- Distinguish observed facts from recommendations.

## Deliverable

Write a concise review with findings ordered by severity, file references, and suggested verification. If there are no findings, say so explicitly.
