# Game Design Workbench — Skill Outline

> **Status:** Concept / starting brief for agent-led refinement. This is not an implementation specification.
>
> **Core proposition:** Describe a game in human-readable Markdown once, explore that same data through multiple useful views, and make missing or questionable relationships obvious to both the designer and their agent.

## 1. Shape of the project

Build this primarily as an **agent skill with companion command-line tools, analysis scripts, and optional visualisers**, not as a standalone game-design application or bespoke document editor.

The workbench is intended for small game prototypes first, but should remain useful on larger designs. The author edits Markdown in VS Code (or any editor); the skill guides agents through the same data and runs the same analysis tools available to the human.

**Core principles**

- **Local-first and portable.** No cloud, service, account, ORM, or persistent database required to use it.
- **C99-first.** Keep the analysis and simulation code in standard C99 so it can run with `cjit` and conventional C compilers. Use other dependencies only if they deliver clear value.
- **Markdown is authoritative.** One record per `.md` file, with frontmatter for structured fields and a body for explanations and design notes. Git-friendly, agent-friendly, human-editable.
- **Attendant-compatible where practical.** Reuse [Attendant](https://github.com/stevehalliwell/agent-skills/tree/main/skills/attendant) collection conventions, schemas, flat frontmatter, cross-collection references, validation, search and generated projections. Do not invent a competing Markdown database.
- **One model, many projections.** A matrix, graph, progression view, diagnostics report and simulation plot are different ways of inspecting the same source records.
- **Human/agent parity.** A visual finding should have an equivalent structured CLI result. The GUI must not contain its own separate analysis logic.
- **Useful while incomplete.** Highlight unknowns, holes and contradictions without requiring designers to finish a giant schema before they can begin.
- **Keep it small.** Add view types and rules when real prototypes demonstrate their value. No general-purpose game engine, giant ontology or rich-text editor.

## 2. Source data and ownership

A project may contain collections such as:

```text
game-design/
  nouns/              # objects/entities/resources in the design
  verbs/              # actions available to players or systems
  interactions/       # noun-verb and other defined interactions
  mechanics/          # rules and state transitions
  systems/            # groups of interacting mechanics
  dynamics/           # expected behaviours emerging from mechanics
  aesthetics/         # intended player responses/experiences
  skill-atoms/             # skill atoms and prerequisite structure
  challenges/         # encounters, puzzles, teaching moments
  levels/             # progression/placement of challenges
  controls/           # tunable values and assumptions
  expectations/       # measurable design targets/hypotheses
  experiments/        # experiments and simulation links
  decisions/          # deliberate exclusions and design rationale
```

These are **candidate categories**, not a mandatory hierarchy. Start with the minimum needed for the first example, and permit project-specific collections. Do not conflate a *system* (the constructed arrangement of rules) with a *dynamic* (the behaviour the arrangement produces).

Each collection can carry Attendant's `.schema.md`, `.usage.md` and optional `.template.md`, configured through `.pi/attendant.tables`. Keep Attendant's `.attendant/` projection disposable. The workbench may build its own **in-memory** indexes, which are likewise not authoritative.

Structured links should use Attendant-compatible collection-qualified references where feasible (`skill-atoms/basic-movement`, `mechanics/pushing`, etc.). Make reverse links and transitive dependencies derived, not manually duplicated. Stable reference identity should survive a file being moved where possible; confirm exactly what Attendant currently guarantees before locking this in.

Prefer **flat fields and arrays** in frontmatter. Put explanations in the Markdown body. Put elaborate numeric tables, executable rules, or large datasets in referenced sidecar files rather than inventing deeply nested YAML.

**Illustrative record** (actual fields/schema syntax to be validated against Attendant):

```yaml
---
requires: [skill-atoms/basic-movement, skill-atoms/push-crate]
mechanics: [mechanics/crate-pushing]
introduced_in: levels/02
---
# Plan crate placement

Player predicts the resulting position of a crate before moving it.

## Evidence of understanding
Player deliberately avoids pushing a crate into an unrecoverable position.
```

The project should remain readable and modifiable if the visualiser, skill or Attendant are not installed.

## 3. Connected design domains

The model should accommodate, and make connections across:

1. **Game grammar:** nouns, verbs, interactions, preconditions, consequences and invalid/unsupported combinations. Noun–verb interaction matrices should expose defined, unknown and deliberately excluded cells.
2. **Mechanics and systems:** rules, state changes, resource flows and interactions between mechanics; both intended and observed/emergent dynamics.
3. **MDA / aesthetics:** links from mechanics through dynamics to intended player feelings and behaviours. Treat aesthetics as hypotheses, not facts automatically provable by a graph.
4. **Skill atoms and scaffolding:** abilities/insights, prerequisite chains, opportunities to introduce, exercise, combine, demonstrate mastery and transfer understanding. Track both understanding and expression, not just whether a mechanic appeared earlier.
5. **Levels and challenges:** which skills, rules, situations and dynamics a challenge assumes or exercises; how these are sequenced.
6. **Control values and measurable expectations:** tunable numbers, assumptions, output measures, targets and recorded experiment results.

No requirement that every node have every kind of link. Some gaps are errors, some are incomplete design, and some are intentionally absent.

## 4. Diagnostics: the main product feature

The workbench should identify **questions worth asking of the design**, not merely schema violations. Examples:

- A level requires a skill whose prerequisites were not introduced or exercised earlier.
- A skill is introduced but is never meaningfully applied, combined, or developed.
- A mechanic is documented but is unused by any challenge or dynamic.
- A noun–verb interaction is unconsidered, or multiple rules appear to disagree about its outcome.
- A desired aesthetic has no specified supporting dynamic; a claimed dynamic has no supporting mechanic.
- A design expectation depends on an undefined control or assumption.
- Changing/removing one mechanic may affect several levels, skill chains, or experiments.
- A simulation indicates the predicted outcome differs from the designer's stated target.

Represent finding types distinctly: **invalid/broken**, **missing required relationship**, **unknown/unresolved**, **possible opportunity**, **conflicting claims**, **unverified hypothesis**, **deliberately excluded**. Never treat every empty matrix cell as a defect.

Every finding should carry enough provenance to act on it: stable issue code, involved record IDs, paths/fields, why the rule fired, suggested investigation and severity or category. Support deliberate exclusions or recorded design decisions to suppress repeat warnings without erasing their rationale.

Analysis rules should be deterministic wherever possible. Agents may suggest new connections or interpretations, but the source model—not LLM recall—determines what exists.

## 5. Shared CLI and visualisations

**One C99 model loader / query / analysis library.** CLI tools and visualisers call the same functions and produce the same findings. The visualiser may aggregate or lay out results, but should not reimplement the underlying checks.

Indicative CLI commands (names and flags are placeholders to refine):

```sh
gd validate                  # local model semantics, on top of Attendant validation
gd gaps                      # unresolved questions and structural findings
gd matrix nouns verbs       # inspect noun–verb coverage
gd skill-atoms                    # dependencies, missing prerequisites, progression
gd impact mechanics/pushing # relationships affected by a design change
gd inspect skill-atoms/crate-plan # direct/reverse references and source paths
gd expectations             # targets and their dependencies
gd simulate rank3 --runs 1000 --seed 42
gd sweep rank3 --param spawn_rate=0.2:0.8:0.1
gd view                      # optional graphical workbench
```

Support machine-readable output (`--json`, CSV for numeric measures) and readable text output. A stable JSON result contract should be designed early so an agent, a GUI and automated tests can consume the same output. Include source paths so an item can be opened immediately in VS Code.

**Visual projections** (not all required for the first release):

- Noun × verb matrix: interactions, missing/unknown cells, coverage and selection details.
- Skill atom dependency graph: prerequisites, disconnected areas, introduction/practice/mastery and challenge placement.
- Level progression view: challenges by level/time versus required and previously developed skills.
- Local relationship graph: focused neighbourhood of a selected record, rather than a huge hairball as the default interface.
- MDA/experience view: intended mechanics → dynamics → aesthetics and unverified connections.
- Experiment panel: control values, stepper, traces, run distributions, parameter sweeps, result comparisons.
- Diagnostics browser: findings grouped by source, level, subsystem or kind; click through to source or relevant view.

Keep the GUI primarily **read-only/exploratory initially**. Editing is through normal Markdown tools. Later, tiny edits such as adding a connection or recording an exclusion could patch frontmatter if there is an obvious safe implementation.

## 6. Lightweight C99 experiments

Simulation is for **exploring specific dynamics and evaluating design expectations**, not recreating the full game. Each experiment can be a small independent C99 program or module, linked to Markdown controls, measures and target expectations.

Examples:

- Time to reach rank 3 given XP thresholds, enemy spawns, drop probabilities and player performance assumptions.
- Distribution of loot acquisition time and the effects of a pity mechanism.
- Currency stockpile, income curves and upgrade intervals.
- Feedback loops, scarcity, strategy comparisons or other rule-driven systems.

**Essential experiment abilities:**

- Read configurable control values and player behaviour assumptions.
- Step a state forward, optionally inspecting each state transition.
- Run batches of reproducible seeded simulations with varied randomness.
- Sweep selected parameter values to measure sensitivity.
- Report outcome distributions, percentiles, target pass rates and unusual outliers.
- Save measurements and sufficient run metadata (seed, inputs, simulation version) to reproduce a result.
- Plot the **spread**, not only a mean. Make it possible to inspect a particular unusual run.

Do not assume an agent or model can infer real player behaviour from mechanics alone. State modelling assumptions. Where the system is only approximate, label its predictions accordingly.

Executable C is project-authored code. It should run **only when explicitly invoked**, not simply when opening a project or previewing a Markdown file.

## 7. Skill packaging and Attendant integration

Possible package shape:

```text
skills/game-design-workbench/
  SKILL.md                # when to use; workflow; core commands
  references/
    model.md              # domain vocabulary and relationship meaning
    diagnostics.md        # checks and their interpretation
    experiments.md        # sim contract / measurement conventions
    attendant.md          # how to coexist with Attendant
  scripts/
    ...                   # small C99 core, CLI and optional viewer
  examples/
    crate-puzzle/         # fixture covering grammar + skills + levels
    progression/          # fixture covering controls + stochastic sim
```

The skill should teach an agent to:

1. Inspect existing project collections and `.usage.md` guidance, rather than assume schemas.
2. Use Attendant for generic records, search, schemas, validation and reference hygiene when installed.
3. Use the C analysis CLI for game-specific semantics and diagnostics.
4. Follow the finding's sources, update the Markdown design only when appropriate, and rerun the same check.
5. Use the same simulation entry points and output data as the human visualiser.
6. Distinguish observed evidence, model predictions, design intentions and open questions.

**Boundary:** Do not fork or reimplement Attendant's general database/query features. Conversely, the visualisation/analysis C tools should be useful directly from source files without requiring an agent, Node.js, the Attendant CLI or an always-running service. Investigate the smallest parsing subset that achieves this.

## 8. Suggested proof-of-concept sequence

1. **Fixture and vocabulary.** Create a tiny puzzle game design in Attendant-compatible Markdown: nouns, verbs, interactions, mechanics, 4–6 skill atoms, a handful of challenges and levels. Establish unambiguous reference rules.
2. **Headless loader.** Build a C99 reader and in-memory indexes for those records and links. Print entities, relationships, reverse links and unresolved references. Verify behaviour with and without Attendant-generated indexes.
3. **First semantic diagnostics.** Detect undeclared/missing relationships, broken skill prerequisites and levels requiring unestablished skills. Give every result a source path and stable machine-readable shape.
4. **Two projections of the same data.** Build a noun–verb matrix and a skill progression/dependency view. Verify that updating a Markdown link changes both and changes CLI diagnostics consistently.
5. **First executable dynamic.** Add a tiny rank progression/economy C99 simulation with a Markdown expectation, controls, seeded runs, trace and distribution plot. Output the same measurements to CLI/GUI.
6. **Usability loop.** Edit in VS Code, rerun/reload, jump from a view/finding to the source document; test the full workflow with a coding agent using only the skill and CLI.
7. **Extend only from use.** Add MDA views, more relationship checks, parameter sweeps, editing conveniences and other domains after actual prototype needs justify them.

## 9. Explicit non-goals

- A full game engine or game scripting language.
- Simulating real human psychology or proving that a game will be fun.
- A general-purpose rich-text editor, wiki, issue tracker or ORM.
- A replacement for Attendant's search, SQL projection, schema operations or record workflows.
- A mandatory large ontology or exhaustive set of fields for every game.
- A giant graph as the primary/default UI.
- Automatic rewriting of design decisions without designer review.
- Requiring simulation code for every mechanic or aesthetic.

## 10. Questions to resolve during refinement

- What is the **smallest useful typed vocabulary** that can support a noun–verb matrix and skill scaffolding without locking the tool to one genre?
- Which relationship conventions are native to Attendant today, and which game-specific semantics belong in the workbench's own layer?
- How should the model represent *unknown*, *not applicable*, *intentionally excluded* and *genuinely missing* without excessive authoring overhead?
- Should the viewer be a tiny raylib/cimgui native executable, a static HTML export, or both? Preserve the shared analysis core regardless.
- How should an experiment declare its inputs/outputs while keeping C99 code independently compilable/runnable through cjit?
- What is the minimum stable JSON output shape for CLI, viewer and agent parity?
- How much source rewriting, if any, should be supported by the GUI?

## Definition of early success

With one small sample game, a designer can edit Markdown in VS Code, run the same deterministic checks from the CLI that appear in the visualiser, inspect both an interaction matrix and a skill progression view derived from the same files, identify missing connections, and run one C99 experiment showing a distribution of outcomes against a declared target. An agent can independently reproduce and explain those findings using the skill and scripts.

**Success is not feature count; it is being able to discover and resolve a design question through multiple views of one shared source of truth.**
