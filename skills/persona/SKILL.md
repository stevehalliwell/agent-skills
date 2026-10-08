---
name: persona
description: "Start a bounded role-based discussion or work session with /skill:persona. Choose agent and user roles, frame shared work, and resume prior work when the session ends. Roles supply perspectives, not decision authority or implementation approval."
disable-model-invocation: true
---

# Persona

Establish complementary roles for a bounded discussion or work session, then return to prior work when the session ends.

## Role cards

Read only selected cards, after roles are known. Each card supplies a perspective, not authority, workflow, or communication style.

Available cards:

- [Art director](personas/art-director.md)
- [Development peer](personas/development-peer.md)
- [Marketing strategist](personas/marketing-strategist.md)
- [Product owner](personas/product-owner.md)
- [Researcher](personas/researcher.md)
- [Delivery lead](personas/delivery-lead.md)

Either participant may name a custom role or share the same role. Use a custom role when no card fits; do not force a closest match.

## Workflow

1. Enter and anchor. Retain prior task, current step, and pending decision in conversation context. State: `Persona session start. Prior work resumes when you say “end persona.”` Use any agent role already supplied; otherwise list cards and ask which role the agent should take. Done when prior context is anchored and agent role is explicit.
2. Set user role. Use supplied role; otherwise ask which role the user will take. Read selected cards, once each. Done when both perspectives are explicit.
3. Frame shared work. Use supplied topic, intended outcome, and constraints. Ask one focused question at a time for missing details that materially affect the session. State bounded frame and each role's contribution. Done when topic and outcome are clear.
4. Collaborate. Use selected perspectives to structure questions, observations, options, and recommendations. Do not invent the user's opinions from their role. Keep facts, assumptions, and open decisions distinct. User retains final decisions unless explicitly delegated. Check recommendations against frame and card boundaries; correct drift before continuing. If a material gap prevents progress, name blocker and ask one focused question. Done when agreed outcome is reached; summarise result and wait for user direction rather than starting new work.
5. End and resume. At any step, `end persona`, `end the persona`, or equivalent ends session. Summarise result, discard session-only roles, and state: `Persona session ended. Resuming: <prior work or normal conversation>.` Restore prior step and pending decision; resume only within existing authority. Done when prior context is restored and roles no longer shape responses.

## Output shape

After framing, use a short session summary; do not repeat it every turn.

```text
Persona session

Agent persona: <role>
User role: <role>
Working on: <topic and intended outcome>

Role contributions:
- <agent perspective>
- <user perspective>

Next: <first question, option, or action>
```

## Rules

- Enter only through explicit `/skill:persona` invocation.
- Ask for the agent persona before the user's role; ask one question at a time.
- A role may be held by either participant and may be shared by both.
- Use a custom role when no card fits; do not force a closest match.
- Keep persona cards role-neutral. Decision authority, workflow, and response style remain outside this skill unless the user explicitly sets them for the session.
- Do not select or mutate persistent facets, start another skill, or alter task state merely because a persona session begins or ends.
