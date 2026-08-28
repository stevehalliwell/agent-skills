---
name: brainstorming
description: "Brainstorm ideas, generate options, explore possibilities, think of alternatives, riff on concepts, or unblock creative ideation. Use for routine exploratory ideation that should associate, expand, and refine options without choosing a material direction, implementing work, or treating ideas as evidence; use Six Thinking Hats instead when perspectives need deliberate separation."
---

# Brainstorming

Explore possibilities, then return selection to the user.

## Trigger clarification

Use when the user wants generative ideation rather than evidence-first research, deliberate multi-perspective analysis, or a material decision. If the request needs sources, use research; if it needs separated facts, feelings, benefits, risks, and alternatives, use Six Thinking Hats.

## Workflow

1. Enter brainstorming mode. State: `Brainstorming start. Current work resumes after ideation.` Identify the topic and desired outcome before optimizing possible solutions. Ask for goal, constraints, audience/context, or selection criteria only when their absence would materially distort ideas. Done when there is enough context to generate useful options.
2. Associate. Surface varied starting associations, analogies, tensions, and adjacent approaches. Keep distinct directions visible; label assumptions rather than presenting them as evidence. Done when the option space extends beyond the first obvious answer.
3. Expand. Build on promising associations through combinations, variants, reversals, and low-commitment experiments. Prefer breadth before judgment. Done when each direction has enough detail to compare.
4. Contract. Group related ideas, state practical trade-offs against supplied criteria, and identify a small set of next-step candidates. Do not select a material direction or turn ideas into implementation. Done when the user can choose, test, research, or refine a candidate.
5. Return and exit. State: `Brainstorming complete: <concise result>. Next: choose a candidate, research, refine, or resume prior work.` Done when user ownership of the next step is explicit.

## Output shape

```text
Brainstorming: <topic>

Associations:
- <distinct starting direction>

Expanded ideas:
- <idea> — <brief value or variation>

Trade-offs:
- <candidate/group> — <upside and constraint>

Next-step candidates:
- <user-owned option>
```

## Rules

- Keep generated ideas separate from evidence and confirmed decisions.
- Make material direction selection and implementation explicit follow-on work.
- Preserve user control to redirect, constrain, pause, or stop ideation.
- Do not replace research, Six Thinking Hats, or another narrower workflow when it fits better.
