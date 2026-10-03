---
name: skill-craft
description: "Use when creating, editing, reviewing, or improving agent skills/SKILL.md files, especially when skill behavior is unpredictable, triggers misfire, agents skip steps, or skill content is hard to maintain. Goal: make skill invocation and execution predictable while keeping context load low and aligning skill behavior with user intent."
---

# Skill Craft

Make skill behavior predictable while leaving room for task-specific judgment.

## Trigger clarification

Confirm the request concerns a skill's invocation, instructions, references, resources, or behavior before loading bulky files. Stop when it concerns ordinary project work rather than skill design. Trigger clarification is post-load; discovery belongs in the frontmatter description.

Completion: skill use is confirmed or rejected before any extra files are loaded.

## Workflow

1. Identify invocation.
   - Model-invoked: agent or another skill must reach it autonomously.
   - User-invoked: human should choose it; set `disable-model-invocation: true`.
   - Completion: frontmatter matches intended reach.

2. Shape description.
   - Treat frontmatter description as the model's routing table, not a summary.
   - Say when to use the skill and what goal it serves.
   - Model-invoked: use concise, specific trigger phrases users/agents actually say. Include synonyms only when they clarify distinct likely requests.
   - Include both user phrases (`dig into details`, `flesh out`, `scope this`, `pause on design`) and agent situations (`before coding`, `unclear requirements`, `cross-cutting change`) when relevant.
   - Make scope distinguishable from neighboring skills; avoid both missed invocations and false positives. Add short, concrete negative cases to description when they prevent likely confusion.
   - Avoid listing internal steps; put procedure in body.
   - User-invoked: keep description human-facing.
   - Front-load strongest trigger phrases in first sentence; do not hide triggers behind abstract labels.
   - Completion: description states capability and use conditions with discriminative triggers, clear goals, no reliance on `AGENTS.md`, no duplicate branches, and no body-only discovery detail.

3. Build information hierarchy.
   - Keep trigger clarification before any required reads/references.
   - Keep always-needed workflow and reference inline when `SKILL.md` remains under about 1,000 words.
   - Treat about 1,000 words as a local review threshold, not an automatic split. Keep the body under 500 lines; move branch-only/bulky material out when it obscures the main path.
   - Link each operational reference directly from `SKILL.md` with an explicit read-when condition; avoid chained discovery through references.
   - Give reference files over 100 lines a table of contents so partial reads reveal their scope.
   - Completion: agent can find each relevant branch directly and follow the main path without loading unrelated material.

4. Match workflow specificity to risk.
   - Use heuristics when multiple approaches are valid and context determines the path; use preferred patterns or parameterized scripts when limited variation is useful; use exact commands and sequences for fragile operations.
   - Each ordered step ends with a checkable done condition. Prescribe sequence only where order matters.
   - Demand enough legwork: exhaustive where needed, narrow where not.
   - For quality-critical outputs, specify a check, correction, and recheck loop using a validator or explicit reference criteria. State how to stop or report a blocker when correction cannot succeed.
   - Completion: instructions allow appropriate judgment, protect fragile operations, and define both success and recovery from failed checks.

5. Add lifecycle for mode skills.
   - If skill temporarily interrupts current work, make lifecycle explicit: enter mode, anchor current work, perform bounded workflow, confirm/decide/write only at right gate, exit mode, state next step or resume prior task.
   - Add exact entry/exit phrases when consistency matters, e.g. `<Mode> start. Current work resumes after <condition>.` and `<Mode> resolved: <result>. Next: <resume/implement/docs/blocker>.`
   - Completion: mode skills cannot read as free-floating checklists; agent knows when it entered, what authority/gate ends it, and how to return.

6. Co-locate material.
   - Keep each concept's definition, rules, caveats, and examples together.
   - Split by workflow, branch, or domain according to what the agent needs together; do not force unrelated material into one reference file.
   - Completion: agent reading one heading gets nearby context needed to act.

7. Prune hard.
   - Remove duplication: one meaning, one home.
   - Remove sediment: stale or future-maybe content.
   - Remove no-ops and explanations the agent already knows; retain task-specific knowledge and constraints.
   - Rephrase negation as positive target unless hard guardrail requires ban.
   - Completion: every line changes invocation, execution, or safety.

8. Check invocation behavior and response shape.
   - A manually invoked skill must perform its primary safe action immediately; it must never respond only that the workflow or skill was loaded.
   - When that action needs user-specific scope or direction, ask one focused question instead of acknowledging the load.
   - Add output format only when consistent responses matter.
   - Keep format short enough agent will use it. Make templates strict for exact data contracts and flexible for context-dependent responses.
   - Add concrete input/output pairs when examples communicate expected behavior better than more prose; co-locate them with the relevant guidance.
   - Completion: user-facing response starts useful work or asks for required direction; examples and templates clarify behavior without overconstraining content.

9. Specify executable resources when present.
   - Prefer bundled scripts for repeatable deterministic operations rather than asking the agent to regenerate them.
   - State whether each script should be executed or read as reference; document commands and required dependencies. Attempt the documented operation and report actionable runtime or harness failures; do not add generic availability preflight instructions.
   - Require scripts to handle expected errors with actionable messages and document the rationale for non-obvious defaults and parameters.
   - For destructive or complex batch operations, validate a structured intermediate plan before applying changes, then verify the result.
   - Use forward-slash paths and tool identifiers supported by the target harness; do not assume another platform's naming or runtime conventions.
   - Completion: resources have clear execution intent, prerequisites, failure handling, and checks proportionate to risk. Skip this step for instruction-only skills.

10. Validate locally.
   - Resolve `<skill-craft-dir>` from this loaded skill, not the project's working directory. The bundled validators require Node.js 18+.
   - Run `node <skill-craft-dir>/validate-frontmatter.mjs <SKILL.md>`. It enforces quoted `description`, name format/64-char limit, and 1024-char description limit. Do not write ad-hoc validators for these checks.
   - Run `node <skill-craft-dir>/md-words.mjs <SKILL.md>` when checking information hierarchy; it excludes YAML frontmatter.
   - Run `node <skill-craft-dir>/validate-urls.mjs <SKILL.md> [<linked-file.md> ...]` when content has HTTP(S) URLs. It checks URL syntax and follows HTTP redirects; it fails on request errors and non-2xx/3xx responses.
   - Run `node <skill-craft-dir>/validate-links.mjs <SKILL.md> [<linked-file.md> ...]` for inline local links and heading anchors; it skips fenced examples and does not fetch URLs. Check bare resource paths and conditional reference discovery manually.
   - Fix validation errors and rerun affected checks; report unresolved failures rather than declaring completion.
   - Completion: bundled validation passes; structural checks alone do not establish execution quality.

## Section contract

Prefer this order. Omit empty/no-op sections.

```markdown
---
name: <lowercase-hyphen-name>
description: "<trigger-rich when-to-use + goal; for model-invoked include concrete user phrases and agent situations>"
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

Use the workflow's completion conditions as the review checklist. Inspect the actual entrypoint and applicable references/resources; distinguish demonstrated execution defects from static conformance gaps. Cite the smallest decisive file location and recommend the least change that fixes it. Report checks not run and unresolved failures.

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
- Prefer discriminative trigger phrases over abstract labels or synonym lists when model invocation matters.
- Prefer one strong leading word over repeated explanation.
- Preserve project-local skill conventions unless they harm predictability.
