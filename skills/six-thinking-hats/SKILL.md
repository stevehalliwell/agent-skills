---
name: six-thinking-hats
description: "Use for deliberate Six Thinking Hats, multi-perspective exploration, creative direction, design exploration, or decision framing when user wants facts, feelings, benefits, risks, and alternatives kept distinct before choosing. Use before creative, design, or material decision conclusion; not for routine brainstorming, fact research, or adversarial critique."
---

# Six Thinking Hats

Explore one question through selected lenses, then return choice to user.

## Trigger clarification

Use only when deliberate separation of perspectives will improve a creative, design, or decision exploration. For routine ideation, factual research, or adversarial critique, use narrower workflow instead.

## Workflow

1. Frame exploration. State question, decision horizon, constraints, available evidence, and what remains user-owned. Recommend `pragmatic-collaborator + advisory + structured`; do not select or mutate facets. Done when question is narrow enough to examine.
2. Select lenses. Explain hats and ask user to select relevant ones and optional order:
   - **Facts** — evidence, data, constraints, unknowns.
   - **Feelings** — intuition, reactions, hopes, concerns; not evidence claims.
   - **Benefits** — value, opportunities, upside, success signals.
   - **Risks** — cautions, failure modes, costs, constraints.
   - **Alternatives** — new options, combinations, experiments.
   - **Process/synthesis** — framing, order, convergence, and close; always active.
   Done when selected lenses and scope are known.
3. Sequence lenses. Use user order when given. Otherwise use facts → feelings → benefits → risks → alternatives → synthesis. Keep every lens distinct. Done when exploration order is explicit.
4. Explore each lens. For each selected lens, state observations, assumptions, and unknowns separately. Ask targeted follow-up only when needed to make that lens useful. Do not treat intuition as fact or material recommendation as a decision. Done when each lens has a bounded contribution.
5. Synthesize. Compare convergences and tensions; form options or small experiments, not unilateral material decision. End by asking user for chosen next step. Done when user can decide, investigate, or iterate.

## Output shape

```text
Six Thinking Hats: <question>

Framing:
- <outcome, constraints, evidence limit>

<Selected hat>:
- Observations: <...>
- Assumptions: <...>
- Unknowns: <...>

Convergences:
- <...>

Tensions:
- <...>

Options or experiments:
- <...>

Next step:
- <explicit question for user>
```

## Rules

- Process/synthesis frames and closes every run, even when user does not select it.
- Keep lens outputs separate; surface disagreement instead of blending it away.
- Label evidence, assumptions, and unknowns in every substantive lens.
- User retains material decisions and may add, remove, reorder, or stop lenses.
- Do not add persistent hat facets, a preset, or a prompt template.
