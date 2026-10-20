---
name: game-design-workbench
description: "Use for game-design-workbench requests, structured game-design records, noun–verb case exploration, skill/challenge-chain inspection, or an explicitly selected game-design framework review. Organize and question one Markdown design model while retaining alternatives and evidence. Skip ordinary game implementation, generic brainstorming, and unsolicited framework reviews."
---

# Game Design Workbench

Use one human-editable design model through selected operations and lenses. This release delivers guided design work, 19 collection templates, 18 original attributed packs, and local Node helpers. Native C99 analysis, graphical views, formula resolution/exports, and simulation execution are **not installed**.

## Trigger clarification

- `Review this encounter using GameFlow`: use selected pack only; review does not authorize repairs.
- `Capture that pushing costs one energy`: record supplied fact without an interview when owner is clear.
- `Explore three alternatives to pulling`: retain proposed alternatives and rationale; do not adopt them.
- `Implement crate movement in Unity`: skip; use implementation workflow.
- `Make my game better`: no unsolicited 18-lens sweep. Ask for a material scope only if no bounded operation is apparent.

On explicit invocation without an operation, inspect `.pi/attendant.tables` if present, then ask one focused operation question. Default: inspect existing model; no setup or review. Use `ask_user` for a focused choice when available.

## Workflow

1. Identify target project, requested operation, source authority, and scope. Start anywhere: existing facts bypass guided input; experience-first input is optional. Read the matching operation reference below. Done: requested action and its write authority are clear.
2. Inspect `.pi/attendant.tables` and, if present, `tools/game-design-workbench/manifest.json`. Read live `.schema.md` and `.usage.md` for relevant roles before interpreting fields or changing records. Require and load installed `attendant` skill for collection/record operations. Follow its operation references. Set `GD_ATTENDANT_SKILL` to that loaded skill's absolute directory for helper commands; never copy or vendor Attendant. Done: actual paths/aliases and source owners are known.
3. Perform bounded action. Use supplied non-tentative facts as facts; mark generated options proposed. Resolve declared aliases before creating identities. Ask only when consequential ambiguity remains. Done: output matches requested scope without inventing requirements, evidence, or exclusions.
4. Check affected source with read-only `validate`; inspect scoped queries and exact source diff after edits. Correct only authorized in-scope errors; otherwise report blockers. Never treat a failed load as an empty design. Done: source authority, unsupported checks, and unresolved issues remain visible.
5. Return result, changed paths or report path, and material limits. No aggregate quality score. Do not commit, publish, repair, set up collections, or execute experiments merely because a review was requested.

Run bundled helper; resolve `<workbench>` to this skill folder or installed project-local snapshot:

```sh
node <workbench>/scripts/workbench.mjs capabilities --project <project> --json
node <workbench>/scripts/workbench.mjs query skills --project <project> --json
```

Requires installed `attendant` skill and Node.js `>=24.10.0`; no npm dependency or running service. Missing Attendant configuration fails `ATTENDANT_REQUIRED` before source writes. Query helpers use temporary Attendant projections; source Markdown remains authoritative. Execute documented command rather than guessing tool availability. On failure, retain sources and report actionable error/code.

## Conditional operation reads

Read only reference matching current operation:

- Setup, bind existing collections, update tools, or resume installation: [Setup](references/setup.md).
- Capture supplied rules, generate alternatives, adopt/reject/revise decisions, define terms, or requested rank edits: [Capture and revision](references/capture.md).
- Inspect cases/relationships, search declared terms, or assess change impact: [Inspection](references/inspect.md).
- Review a selected lens or save/read a dated review: [Review and provenance](references/review.md), plus selected pack below.
- Clarify field ownership, lifecycle, or partial-model interpretation: [Model contract](references/model.md).
- Run helpers directly, interpret errors/limits, or use machine output: [CLI contract](references/cli.md).
- Try worked source without creating a new design: [Crate-puzzle walkthrough](examples/crate-puzzle/README.md).

## Selected framework reads

Load only selected pack. If user supplies a clear lens, do not ask again. If lens choice materially changes requested review, present a narrow choice; default to nearest stated concern. No `all` selector or automatic framework sequence.

- Rule/experience trace: [MDA](references/frameworks/mda.md).
- Actions, conditions, consequences, feedback: [Game grammar](references/frameworks/game-grammar.md).
- Resources and feedback loops: [Resource economy](references/frameworks/resource-economy.md).
- Recurring configurations: [Gameplay patterns](references/frameworks/gameplay-patterns.md).
- Learning loops/prerequisites: [Skill atoms](references/frameworks/skill-atoms.md).
- Pattern learning/novelty: [Theory of Fun](references/frameworks/theory-of-fun.md).
- Challenge relative to capability: [Flow](references/frameworks/flow.md).
- Enjoyment conditions: [GameFlow](references/frameworks/gameflow.md).
- Unfamiliar concurrent demands: [Cognitive load](references/frameworks/cognitive-load.md).
- Memory, cues, chunking: [Working memory](references/frameworks/working-memory.md).
- Temporary support and independent transfer: [ZPD/scaffolding](references/frameworks/zpd-scaffolding.md).
- Autonomy, competence, relatedness: [Self-Determination Theory](references/frameworks/self-determination.md).
- Published interest vocabulary: [Quantic Foundry](references/frameworks/quantic-foundry.md).
- Routes to emotion: [Four Keys](references/frameworks/four-keys.md).
- Historical multiplayer participation: [Bartle](references/frameworks/bartle.md).
- Forms/freedoms of play: [Caillois](references/frameworks/caillois.md).
- Contextual reflective questions: [Schell](references/frameworks/schell.md).
- Goals, prototypes, play evidence: [Playcentric](references/frameworks/playcentric.md).

## Authority rules

- Review/inspection reads source; requested report writes results only. No repair/adoption permission is implied.
- Optional absence is valid. Distinguish unknown, explicit exclusion/non-applicability, defined no effect, invalid data, and unsupported checks.
- Preserve alternatives and useful rationale. Choosing one leaves siblings proposed unless user states another disposition.
- Keep definitions in one owner. Resolve declared aliases only; do not guess synonyms or revive inactive concepts silently.
- Frameworks ask questions, not verdicts. Missing teaching is not incapability; declared experience links are not observed feelings. No cohort/profile datasets or psychological scores.
- Proposed experiments remain proposals. Measurements, execution status, expectation comparison, and design judgment are separate. Never invent executed trials or player observations.
