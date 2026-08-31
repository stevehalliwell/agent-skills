---
name: persona
description: "Start a deliberate role-based discussion or work session. Use only when the user explicitly invokes /skill:persona; choose the agent persona and the user's role, agree what to work on, then end the persona session and resume prior work on request."
disable-model-invocation: true
---

# Persona

Establish complementary roles for a bounded discussion or work session, then return to prior work when the session ends.

## Role cards

Available role-neutral persona cards:

- [Art director](personas/art-director.md)
- [Development peer](personas/development-peer.md)
- [Marketing strategist](personas/marketing-strategist.md)
- [Product owner](personas/product-owner.md)
- [Researcher](personas/researcher.md)
- [Delivery lead](personas/delivery-lead.md)

A participant may also name a custom role. Read only the selected card or cards. A card describes a perspective, not decision authority, workflow, or communication style.

## Workflow

1. Enter persona session. State: `Persona session start. Prior work resumes when you say “end persona.”` Briefly list available cards and ask which persona the agent should take. Wait for one role. Done when the agent role is explicit.
2. Set the user's role. Ask what role the user will take; they may select any card, use the same role as the agent, or name a custom role. Read the selected card or cards. Done when both perspectives are explicit.
3. Frame shared work. Ask what the participants will work on, the intended outcome, and any constraints or decision the user wants to reach. State the agreed frame and the complementary contribution of each selected role. Done when the session has a bounded topic and outcome.
4. Collaborate from the selected perspectives. Use each role's interests and standards of judgment to structure questions, observations, options, and recommendations. Keep confirmed facts, assumptions, and open decisions distinct. The user retains final decisions unless they explicitly delegate one. Done when the requested discussion or work reaches its agreed stopping point.
5. End and resume. When the user says `end persona`, `end the persona`, or equivalent, summarise the result, drop both role perspectives, and state: `Persona session ended. Resuming: <prior work or normal conversation>.` Do not carry the persona roles into later work unless the user starts a new persona session. Done when the prior context is restored or no prior context exists.

## Output shape

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
