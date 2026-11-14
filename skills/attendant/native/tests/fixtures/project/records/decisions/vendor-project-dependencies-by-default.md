---
id: 01a1094f-6054-7326-9479-1113deab9d7d
name: vendor-project-dependencies-by-default
created_at: 2026-10-04T23:45:59.380Z
desc: Vendor project dependencies into the repository by default.
tags: []
status: accepted
revisit_triggers: []
---
## Context

During refinement of `tasks/bring-over-performance-faker-starting-point`, the user requested vendoring dependencies for the manual raylib check and making this the expected project default in working guidance.

## Decision

Vendor project dependencies into this repository by default. Record this default in the project-specific agent guidelines task. This decision does not select all future dependencies or require vendoring the installed compiler.

## Options considered

- Use reference dependency paths in place: proposed for a smaller initial import, but the user selected vendoring instead.
- Vendor dependencies locally: selected by the user.

## Trade-offs / consequences

Local dependency copies avoid coupling builds to the reference repository's path. They add repository content and responsibility for provenance, licenses, and future updates. Exact vendor contents and Git tracking policy are not settled by this decision.

## Affected areas

- `tasks/bring-over-performance-faker-starting-point`: vendor raylib for the manual Windows check.
- `tasks/develop-project-specific-agent-guidelines`: document the expected default.
- Future dependency setup tasks.

## Guardrails

Do not modify the reference repository. Retain applicable license and provenance information. Resolve dependency-specific contents and tracking before implementation; do not infer that prebuilt binaries or unrelated dependencies are required.

## Revisit trigger

No specific revisit trigger has been identified. Changes to this default require user agreement.
