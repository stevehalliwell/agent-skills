---
name: skill-craft
description: "Use when creating, editing, reviewing, or improving agent skills/SKILL.md files, especially when skill behavior is unpredictable, triggers misfire, agents skip steps, or skill content is hard to maintain. Goal: make skill invocation and execution predictable while keeping context load low and aligning skill behavior with user intent."
---

# Skill Craft

Make skills predictable: same process every run, not same output.

## Trigger clarification

Frontmatter description is the primary trigger surface. It must carry enough plain-language trigger phrases for the agent to load the skill without relying on global `AGENTS.md` instructions. Add Trigger clarification only for checks after the skill has already loaded.

Use this skill for:

- creating new skills
- editing or reviewing `SKILL.md` files
- improving skill descriptions, trigger branches, steps, completion criteria, output shapes, or rules
- deciding whether content belongs in `SKILL.md`, disclosed files, or another skill

Before loading bulky workflow/reference files, confirm the clarified trigger still fits. If it does not fit, stop and do not load them.

Completion: skill use is confirmed or rejected before any extra files are loaded.

## Workflow

1. Identify invocation.
   - Model-invoked: agent or another skill must reach it autonomously.
   - User-invoked: human should choose it; set `disable-model-invocation: true`.
   - Completion: frontmatter matches intended reach.

2. Shape description.
   - Treat frontmatter description as the model's routing table, not a summary.
   - Say when to use the skill and what goal it serves.
   - Model-invoked: pack description with concrete trigger phrases users/agents actually say, including synonyms, natural phrasing, and near-miss wording.
   - Include both user phrases (`dig into details`, `flesh out`, `scope this`, `pause on design`) and agent situations (`before coding`, `unclear requirements`, `cross-cutting change`) when relevant.
   - Prefer a slightly over-triggering description plus clear skip/stop criteria in body over under-triggering that prevents the skill from loading. Add short, concrete negative cases to description when they prevent likely false positives.
   - Avoid listing internal steps; put procedure in body.
   - User-invoked: keep description human-facing.
   - Front-load strongest trigger phrases in first sentence; do not hide triggers behind abstract labels.
   - Completion: description has rich trigger phrases, clear goals, real trigger branches, no reliance on `AGENTS.md`, no duplicate branches, no body-only trigger detail, no vague summary.

3. Build information hierarchy.
   - Keep trigger clarification before any required reads/references.
   - Keep always-needed workflow and reference inline when `SKILL.md` remains under about 1,000 words.
   - Split or move branch-only/bulky material to linked files with strong context pointers once it exceeds about 1,000 words.
   - Completion: agent can follow main path without wading through rarely used material.

4. Write steps with completion criteria.
   - Each ordered step ends with a checkable done condition.
   - Demand enough legwork: exhaustive where needed, narrow where not.
   - Completion: no step can be honestly skipped by vague “done enough” reading.

5. Add lifecycle for mode skills.
   - If skill temporarily interrupts current work, make lifecycle explicit: enter mode, anchor current work, perform bounded workflow, confirm/decide/write only at right gate, exit mode, state next step or resume prior task.
   - Add exact entry/exit phrases when consistency matters, e.g. `<Mode> start. Current work resumes after <condition>.` and `<Mode> resolved: <result>. Next: <resume/implement/docs/blocker>.`
   - Completion: mode skills cannot read as free-floating checklists; agent knows when it entered, what authority/gate ends it, and how to return.

6. Co-locate material.
   - Keep each concept's definition, rules, caveats, and examples together.
   - If splitting files, prefer one action/workflow file and one reference/context file.
   - Completion: agent reading one heading gets nearby context needed to act.

7. Prune hard.
   - Remove duplication: one meaning, one home.
   - Remove sediment: stale or future-maybe content.
   - Remove no-ops: lines that do not change agent behaviour.
   - Rephrase negation as positive target unless hard guardrail requires ban.
   - Completion: every line changes invocation, execution, or safety.

8. Check invocation behavior and response shape.
   - A manually invoked skill must perform its primary safe action immediately; it must never respond only that the workflow or skill was loaded.
   - When that action needs user-specific scope or direction, ask one focused question instead of acknowledging the load.
   - Add output format only when consistent responses matter.
   - Keep format short enough agent will use it.
   - Completion: user-facing response starts useful work or asks for required direction; it is predictable without overconstraining content.

9. Validate locally.
   - Run `node ./validate-frontmatter.mjs <SKILL.md>`. It enforces quoted `description`, name format/64-char limit, and 1024-char description limit. Do not write ad-hoc validators for these checks.
   - Run `node ./md-words.mjs <SKILL.md>` when checking information hierarchy; it excludes YAML frontmatter.
   - Run `node ./validate-urls.mjs <SKILL.md> [<linked-file.md> ...]` when content has HTTP(S) URLs. It checks URL syntax and follows HTTP redirects; it fails on request errors and non-2xx/3xx responses.
   - Check relative links resolve from skill dir.
   - Completion: bundled validation passes and skill should load cleanly in pi.

## Section contract

Prefer this order. Omit empty/no-op sections.

```markdown
---
name: <lowercase-hyphen-name>
description: <trigger-rich when-to-use + goal; for model-invoked include concrete user phrases and agent situations>
disable-model-invocation: true # only for user-invoked
---

# <Title>

<One-sentence purpose / leading word.>

## Trigger clarification
<Only when frontmatter cannot safely carry all trigger checks.>

## Required read
<Only after trigger clarification; point to disclosed workflow/reference if needed.>

## Workflow
<Ordered steps. Each step has completion criterion. If very long, split into separate .md files.>

## Output shape
<Only if user response shape matters.>

## Rules
<Guardrails and invariants. Positive phrasing preferred.>

## References
<Context pointers to linked files, only if needed.>
```

## Review checklist

- Invocation correct: model-invoked only when agent/other skills must reach it.
- Description is strong enough to trigger without global `AGENTS.md` help.
- Description says when to use the skill and what goal it serves, not internal steps.
- Description front-loads concrete user phrases and agent situations, including likely synonyms and near-misses.
- Description prefers controlled false positives over missed invocations for model-invoked skills.
- Trigger clarification exists only for post-load checks, not primary discovery.
- Required reads/references come after trigger clarification.
- Branches are real branches, not synonyms.
- `SKILL.md` stays self-contained unless it grows beyond about 1,000 words or has branch-only bulk.
- Steps are ordered and have checkable completion criteria.
- Mode skills have explicit enter/anchor/gate/exit/resume lifecycle, not just steps to run.
- Reference sits at right level: inline if every run needs it, linked if branch-only/bulky.
- Related reference is co-located.
- One meaning has one source of truth.
- No no-op advice, stale sediment, or unneeded prose.
- Negations become positive target behaviour where possible.
- Skill stays narrow; split only when invocation, sequence, or noise reduction earns the load.
- Output shape exists only when it improves predictable user response.
- Manual invocation begins the skill’s primary safe action; it never returns a load-only acknowledgement.
- If manual invocation lacks direction needed for that action, the skill asks one focused question.

## Output shape for reviews

```text
Skill craft review: <skill/path>

Invocation: <model/user> — <fit>
Main issues:
- <issue> -> <fix>

Recommended edits:
1. <edit> — <why>
2. <edit> — <why>

Keep:
- <parts already working>
```

## Rules

- Do not add boilerplate sections to satisfy contract; empty sections are no-ops.
- Do not split skills for neatness alone.
- Prefer deleting weak prose over rewriting it.
- Prefer several concrete trigger phrases over one abstract label when model invocation matters.
- Prefer one strong leading word over repeated explanation.
- Make manual invocation operational: begin the primary safe action, or ask one focused question when direction is required. Never use a load-only acknowledgement as the response.
- For skills that act as temporary modes, prefer explicit lifecycle phrases over implied control flow.
- Preserve project-local skill conventions unless they harm predictability.
