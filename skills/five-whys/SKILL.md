---
name: five-whys
description: "Use for Five Whys, root-cause analysis, cause diagnosis, purpose discovery, or when someone asks why a product, design, game, narrative, or technical outcome exists or failed. Guide one evidence-aware, non-blaming question at a time toward actionable insight; do not use for quick fact lookup or to assign fault."
---

# Five Whys

Guide cause or purpose inquiry until next action becomes clear.

## Trigger clarification

Use when user wants to understand why something happened, why it matters, or what purpose it serves. If request only asks for known facts or demands blame, state mismatch and use appropriate workflow instead.

## Workflow

1. Frame inquiry. If interrupting existing work, anchor its pause point and state that it resumes after inquiry closes. State observed outcome or intended purpose, scope, stakes, and what user wants to decide. Separate facts from assumptions. Done when question is narrow enough to probe.
2. Establish evidence. Record available observations, sources, constraints, and unknowns. Separate observation, reported experience, inference, and hypothesis; distinguish proximate causes, contributing conditions, and systemic context. Look for evidence that would disconfirm the leading explanation. Done when first question can target highest-value gap.
3. Ask one question. Ask one open, non-leading “why” or purpose question. Wait for user response; do not supply a chain of questions. Done when user can answer or correct premise.
4. Map response. Add response to causal/purpose map, label evidence versus hypothesis, identify branches, then ask next highest-value question. Continue only while it adds understanding. Done when chain, branch, or evidence gap is clear.
5. Stop safely. Stop before or after five questions when insight is actionable, evidence is insufficient, user reaches a limit, causes become speculative, or further probing has diminishing value. Done when no justified next question remains.
6. Synthesize and exit. Check the map against supplied evidence; correct unsupported causal certainty and recheck, or retain a named evidence gap. Return the map, strongest hypotheses, and one next action or open question without assigning blame. Close inquiry explicitly and resume the anchored work only if its blocking question was resolved; otherwise name the blocker. Done when the inquiry's outcome and continuation are explicit.

## Output shape

```text
Five Whys: <outcome or purpose>

Current map:
- <observation> → <response/hypothesis>

Evidence and gaps:
- <evidence or unknown>

Next question:
- <one open question>
```

At completion:

```text
Five Whys synthesis: <outcome or purpose>

Map:
- <chain and branches>

Hypotheses:
- <supported / tentative>

Evidence gaps:
- <what would test or disprove>

Next action or open question:
- <one useful move>
```

## Rules

- “Five” is adaptive; use fewer or more questions when evidence warrants it.
- Support cause diagnosis and purpose/intent discovery across product, design, games, narrative, and technical work.
- Keep each question focused, open, and non-coercive.
- Preserve user control: user may stop, redirect, answer partially, or choose depth.
- Keep inquiry evidence-aware, non-blaming, and focused on the next highest-value question.
