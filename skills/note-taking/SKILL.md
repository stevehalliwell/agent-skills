---
name: note-taking
description: "Take notes, capture meeting notes, jot this down, summarize these notes, or record observations in the current session. Preserve supplied meaning in a concise structured note, separate observations, decisions, and follow-ups, and flag possible durable work without writing files, task records, handoffs, or decisions."
---

# Note taking

Capture faithful session-only notes without turning them into work.

## Workflow

1. Enter note-taking mode. State: `Note-taking start. Current work resumes after note capture.` Identify the supplied material to capture; ask only when no material is available. Done when the capture target is clear.
2. Preserve meaning. Extract observations, confirmed decisions, follow-ups, attribution, qualifiers, and uncertainty only when supplied. Do not infer commitment, priority, ownership, or scope. Done when the note distinguishes stated facts from open items.
3. Flag, do not record. Label a potential task or decision as a follow-up signal. Do not write files, task records, handoffs, or decisions; invite an explicit workflow switch when durable capture is wanted. Done when no persistent state was created.
4. Return the note and exit. State: `Notes captured: <concise summary>. Next: resume prior work or explicitly switch workflows.` Done when the user has a usable session note and a clear continuation.

## Output shape

```text
Notes:
- <observation or context>

Decisions:
- <confirmed decision, or None stated>

Follow-ups:
- <open item or potential task/decision signal, or None stated>
```

## Rules

- Keep notes concise and faithful to supplied material.
- Mark uncertain statements and attribution instead of resolving them.
- Never implement, refine, prioritize, or persist captured content.
- Use backlog capture or another explicit workflow only after the user requests that switch.
