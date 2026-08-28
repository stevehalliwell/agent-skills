---
name: note-taking
description: "Take notes, capture meeting notes, jot this down, summarize these notes, or record observations. Ask where notes belong, then append clear faithful notes to that document; when note-taking ends, offer a categorized rewrite in a new file. Do not use for task tracking, implementation, or decisions."
---

# Note taking

Capture supplied material in a user-chosen persistent notes file without turning it into work.

## Workflow

1. Enter note-taking mode. State: `Note-taking start. Current work resumes after note capture.` Ask where notes should go, including a path or existing document. Wait for destination before capturing. Done when a writable destination is clear.
2. Append each capture. Add concise, faithful notes to destination; preserve supplied observations, confirmed decisions, follow-ups, attribution, qualifiers, and uncertainty. Create destination if it does not exist. Never overwrite existing content. Done when new material is appended and stated facts remain distinct from open items.
3. Keep scope. Do not infer commitment, priority, ownership, or scope. Flag potential tasks or decisions only as follow-up signals. Done when no task record, handoff, implementation, prioritization, or decision was created.
4. On explicit completion of note-taking, ask whether to rewrite notes into a new file, more clearly categorized with default headings: `Observations`, `Decisions`, and `Follow-ups`. Wait for answer. If accepted, ask for new-file path only when none was supplied, create new file without changing source notes, then write categorized rewrite. Done when rewrite is declined or new file is written.
5. Exit. State: `Notes captured: <concise summary>. Next: resume prior work or explicitly switch workflows.` Done when user has notes in chosen destination and clear continuation.

## Rules

- Ask for destination once at mode entry; retain it until note-taking ends or user changes it.
- Append notes in clear Markdown when destination supports it; otherwise use clear plain text.
- Keep notes concise and faithful to supplied material.
- Mark uncertainty and attribution instead of resolving them.
- Use default rewrite headings even when category has no items; write `None stated` in empty sections.
- Use backlog capture or another explicit workflow only after user requests that switch.
