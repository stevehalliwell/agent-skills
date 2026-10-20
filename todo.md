# Game Design Workbench

Status: implementing Slice 1 for review. Later slices remain unstarted. User authorized dedicated branch/worktree and commits per independent-developer step.

Implementation context:

- Branch: `feat/game-design-workbench`.
- Worktree: `D:/Repos/agent-skills-game-design-workbench`.
- Base: `99f0879c316377faf162841362310dc8b024b1e0`.
- Scope: first usable skill release, all 18 packs, collection templates, project-local generic/query tools, populated crate-puzzle example, tests, and offline decision report. No native viewer or simulation execution claimed.
- Step commits: (1) plan/spec baseline; (2) model/templates/example; (3) local setup/query/report tools; (4) researched packs/entrypoint/docs; (5) integrated verification/corrections; (6) offline review report.
- Per step: update evidence here, stage only assigned paths, commit before next step. No merge/push. Original worktree's unrelated files remain untouched.

## Goal

Build `game-design-workbench`: an agent skill, Attendant-compatible templates, and companion tools for generating, reviewing, analysing, and simulating game designs. Primary value: expose design gaps and help designers resolve them.

Sources:

- [game-design-workbench-skill-outline.md](game-design-workbench-skill-outline.md): starting brief, not an implementation contract.
- [game-design-frameworks-reference.md](game-design-frameworks-reference.md): initial framework/lens inventory and research references. All 18 named entries in sections A–D are intended initial catalog scope; section E supplies cross-cutting perspectives, not separate theories to implement wholesale.

This file tracks current specification, open decisions, and delivery steps. Update settled requirements here rather than keeping an answer log.

## Confirmed direction

- Repository is canonical skill source, committed to Git and symlinked into global Pi skills.
- Use this single `todo.md` for task tracking. Do not set up task collections in this repository.
- Start with how people use the workbench and what it provides. Refine implementation after behavior is clear.
- Use Attendant and templates to establish project-local game-design data.
- Support generation, review, analysis, and simulation; prioritize finding gaps.
- First usable slice includes skill, templates, worked design example, and attributed original question packs for all 18 named frameworks, including minimum data collections/query helpers needed to action them. Full workbench remains intended scope; broader deterministic tools, native views, and simulation follow. Do not defer an essential query while claiming its pack is actionable.
- Packs may require additional collections for distinct data and dedicated queries. Define those needs per pack; do not assume existing core tables make every framework actionable. Prefer C99 tools; `.mjs` is allowed where necessary, with reason and runtime dependency documented. Do not duplicate canonical facts across framework datasets.
- Bundle a tiny crate-and-switch puzzle worked example inside skill folder. It demonstrates usage and tests the integrated system; extend it as capabilities arrive.
- Workbench is an ongoing tool/approach, not a linear workflow. Engage from any stage of design.
- Guide designer to supply core elements, then continually refine, explore, and elaborate them. Do not require a fixed starting artifact or sequence.

## Starting constraints from brief

Retain during refinement; surface conflicts rather than silently changing them.

- Markdown records are authoritative. Derived indexes, reports, and views are disposable.
- Local-first; no required service, account, or persistent database.
- Start with Windows. Use C99 through `cjit`, with raylib where needed for views. Preserve portable source and file formats; conventional C compilers remain a target. Do not claim macOS/Linux verification without testing.
- Reuse Attendant's collection conventions and generic record operations. Do not build a second Markdown database.
- Human and agent use the same model, checks, and simulation results.
- Partial designs remain useful. No mandatory giant ontology.
- Findings distinguish broken data, unresolved design, opportunities, hypotheses, and deliberate exclusions.
- Analysis identifies questions; it does not prove fun or predict player behavior without stated assumptions.
- Simulation code runs only on explicit invocation, never on project open or document preview.
- Viewer is primarily exploratory, with explicit support for editing selected skill's authored rank and recomputing descendant display positions. Normal Markdown editing remains available. Other viewer editing remains out of scope unless agreed.

## User experience

### Engagement model

Workbench supports repeated, non-linear design work. Enter at current state, establish relevant context, and choose the useful operation. Do not force setup, generation, review, and simulation into a mandatory sequence.

Two explicit entry situations:

- **New repository:** install and configure all supported table sets, their templates, and required scripts so designer can enter information in any domain without another collection-setup step. Empty collections are valid; do not require records in every table.
- Guide designer to provide core elements; progressively flesh them out rather than requiring a complete design upfront.
- **Existing workbench project:** inspect existing design data and guidance, recover current state, and continue refining, exploring, or elaborating the design. Do not restart setup or repeat settled questions.

Setup copies required tool sources/scripts into project as a versioned snapshot. Project tools work without installed skill; updates are explicit and preserve local changes rather than silently inherit global skill changes. Git captures tool version alongside design data.

Setup/update/resume contract:

- Propose one concrete combined setup plan before applying Attendant collection creation. Show project root, role-to-collection mapping, all new/changed files, required runtimes, and collisions. Reuse compatible existing collections only after reading their schema/usage; stop for incompatible contracts rather than silently overwrite or migrate.
- Install all 19 core table sets listed below, each with `.schema.md`, `.usage.md`, and `.template.md`; empty tables are valid. Add any justified pack-specific collection/query requirements. Setup alone does not copy the tutorial game into designer's live records or invent initial game facts.
- Copy project-local references, all 18 pack documents, templates, available query/tool sources, launchers, licenses, and quickstart under `tools/game-design-workbench/`. Include Attendant runner dependencies when local generic operations depend on them: inspected runner imports sibling `yaml.mjs`, not a standalone single file. Document Node.js >=24.10.0 for Attendant.
- Keep `tools/game-design-workbench/manifest.json` as snapshot metadata: distribution revision, model/report versions, installed capabilities, role-to-collection mapping, dependency versions/licenses, and shipped-file hashes. Existing `.pi/attendant.tables` still owns collection discovery; manifest does not become a second record index. Model/role conflicts require resolution, not guessed remapping.
- Explicit update compares installed base hashes, current local files, and new distribution. Show additions, safe replacements, local edits, removals, and schema/data migrations separately. Preserve local edits; stop for conflicts. Never delete local extras or migrate records as an implicit tool update. Record new manifest version only after successful apply/validation; report exact partial state if a write fails.
- Resume reads manifest/capabilities, Attendant configuration/schema/usage, requested source records and relevant linked rationale/evidence. Resolve current scope from request; inspect old reports as historical context, not current findings. Do not load every pack, reread every record, repeat settled questions, or use a previous report's lens as standing permission.
- If source is usable but a tool/dependency is absent, continue available record/guided operations and state unsupported checks. Do not auto-install dependencies, upgrade snapshots, execute simulations, or run a broad review on resume.

### Core elements and input routing

- When designer needs guidance, ask for a natural-language description of player experience. Develop nouns, verbs, and mechanics from there, then skill atoms and other relevant design detail.
- This is a default guidance path, not a prerequisite chain. Designer may already know what to add or change.
- Accept direct input at any level and route it into appropriate records and relationships. Do not force an experience interview before capturing a supplied mechanic, skill atom, or other concrete detail.
- Capture explicit supplied facts directly into appropriate records and relationships. Routine record placement does not require a preview or approval gate. Report what was captured.
- Ask only about consequential ambiguity; leave unsupported detail unresolved. Do not silently invent rules, resolve conflicts, or turn agent suggestions into accepted design.
- Preserve supplied intent while translating prose into structured design data.
- Use current records to avoid repeating known questions. Progressively expose useful missing detail rather than demand complete collections.

### Available operations

- **Start a design:** user supplies a pitch, notes, or a narrow design problem. Establish current intent and choose only useful collections. Produce a small connected model, not a complete invented game.
- **Adopt an existing design:** inspect current Markdown, schemas, and usage guidance. Identify reusable data and propose a mapping. Do not overwrite documents or replace project conventions implicitly.
- **Develop part of a design:** focus on a mechanic, system, challenge, progression segment, or intended experience. Offer alternatives in context; distinguish proposals from accepted rules.
- **Review for gaps:** choose a scope, inspect records and relationships, run available checks, and report actionable findings with sources.
- **Explore a change:** trace affected records and assumptions before modifying the design. Explain reach and uncertainty, not just a list of backlinks.
- **Test a hypothesis:** define measurable question, controls, assumptions, and any target; explicitly run bounded experiment. Report measurements first. Requested expectation analysis can compare distributions against declared criteria without turning simulation into a design judge.
- **Resume:** recover current design intent, open questions, and experiment evidence from project records rather than agent memory.

### Consistency versus requested analysis

- Proactively follow connections to check consistency and identify change impacts. Inspect affected definitions and relationships, not only the edited record.
- Do not conduct analysis through a design lens unless requested. Scaffolding, interaction coverage, experience support, balance, and similar interpretations are requested operations, not automatic post-edit audits.
- Routine capture can report broken links, conflicting authoritative definitions, or directly affected records without launching a lens-based review.
- Analysis/review requests must select a specific lens or framework. Recover selection from context when clear; otherwise ask for it. Do not offer or run an all-lenses review: catalog may become very large.
- Bundle every named entry in sections A–D of framework reference, replacing earlier four-framework shortlist. Follow section F6 for phased delivery without dropping catalog scope.
- Keep framework distinctions and cautions from reference: Flow is not GameFlow; ZPD and scaffolding are related but not identical; working-memory counts are not hard game-design limits. Contextual or genre-specific frameworks do not become universal requirements.
- Quantitative expectations/balance remains a required perspective, supported by resource/economy methodology, experiments, and evidence; do not require a separate competing dataset for it.
- Each pack may provide only original questions/references where appropriate. Do not invent numerical validators or a universal design score.
- Preserve attribution and source links. Research relevant sources before authoring each pack; write original prompts rather than reproduce copyrighted books, decks, questionnaires, or scoring systems.
- Framework document examples are illustrative, not command contracts. Do not adopt an all-lenses option from an example.
- Initially support bundled framework documents only. Project-supplied/custom framework conventions are future scope, not part of initial delivery.
- Load and apply only selected bundled lens/framework guidance; keep catalog discoverable without loading every lens into context. Do not imply an executable plugin system.
- Audience, player capability, and motivation terminology are common framework concepts, not a requirement to model particular cohorts or derive player profiles from project records. Do not add audience/cohort collections, capability scores, personality scoring, or mandatory challenge-versus-capability curves. Apply these concepts through original framework guidance; existing design records and supplied evidence can inform discussion without turning conceptual vocabulary into measured player data.
- Explicit lens/framework analysis requests create dated reports identifying selected framework and coverage. Routine consistency/impact checks do not automatically create a full review report.

### Per-operation discipline

These are safeguards within an operation, not a linear design process.

1. Establish requested operation and scope from user request and existing project data.
2. Inspect relevant collections and their guidance. Read linked context needed for the question.
3. Return useful work: a proposed design addition, sourced findings, impact analysis, or experiment result.
4. Resolve material choices with designer. Do not silently turn suggestions into accepted design.
5. Persist authorized changes in source records and link relevant evidence or rationale.
6. Rerun affected consistency checks and any lens-based checks included in requested operation. Report what changed, what remains unknown, and what evidence supports the result.

A review request must not silently become a repair operation. Explicit design input authorizes direct capture. Persist generated alternatives as proposed; do not adopt them or change accepted intent without designer direction.

### Alternatives and design memory

- Keep all generated design alternatives as retained proposal records, initially marked proposed. Start with idea/context/links rather than fully normalize every quick alternative across domain tables.
- Elaborate linked proposed domain records when exploration needs them. Adoption writes selected facts into canonical domain records; proposal/decision record retains rationale and links, not a competing authoritative rule copy.
- Lifecycle meanings: proposed = not adopted; accepted = current design; rejected = explicitly ruled out; superseded = replaced by linked design content; defunct = no longer applicable. Record disposition rationale when supplied; do not invent a reason.
- Treat lifecycle as current state, not an edit log. Ordinary edits to a record rely on Git history; they do not require making a superseded copy of every previous version.
- User direction authorizes adoption, rejection, replacement, retirement, or reopening. Matching an alias on inactive content must expose its disposition, not silently reactivate it.
- Acceptance means adopted design, not empirically proven behavior. Keep intention, assumption, prediction, and observation distinct from lifecycle.
- Selecting an alternative makes it accepted but does not automatically reject its siblings. Unchosen alternatives remain proposed until designer explicitly assigns another disposition.
- When an alternative is rejected, superseded, or defunct, retain it rather than delete it. Preserve enough context and rationale to avoid repeatedly redesigning something already ruled out.
- Proposed and inactive alternatives do not count as active rules or silently create contradictions in accepted-design analysis. Permit explicit exploration/review of alternatives.
- Consult relevant retained alternatives when generating or revisiting design choices. Surface prior rejection and its reason; do not present a ruled-out idea as new.
- Retained alternatives describe current disposition and useful rationale. Use Git for edit history rather than maintaining a chronological transcript in each record.

### Outputs by operation

- **Setup:** selected collections, schemas, usage guidance, body templates, and a small initial set of connected records. Explain what each collection contributes.
- **Generation:** concrete candidate mechanics, relationships, challenges, or alternatives tied to design intent. Identify assumptions, consequences, and questions each candidate introduces.
- **Review:** scoped findings with source records, reason, evidence class, and suggested investigation. State coverage and checks not performed.
- **Analysis:** matrices, prerequisite/progression views, focused relationship views, or change-impact results derived from the same records.
- **Simulation:** declared model/assumptions, reproducible run metadata, measurements, spread, and execution limitations. Simulation reports what happened; it does not judge meaning for design. Compare with expectations only through separately requested analysis, with design interpretation left to designer/agent.
- **Resolution:** updated source or recorded exclusion/rationale, followed by a rerun showing whether the original finding remains.

## Required capabilities

### Interaction matrix and graph exploration

- User can request a noun × verb matrix. Table is an entry view, not a one-result-per-cell data model.
- Each cell can contain multiple conditional outcomes, with links to their conditions, consequences, and defining records. Preserve distinct rules rather than flattening them into a single value.
- Describe conditions and consequences in readable prose, with structured links to involved concepts and controls. Do not require a general-purpose executable rules language.
- Deterministic checks inspect declared structure; agent review assesses prose-level conflicts. Generated C simulations explicitly implement the relevant prose rules and expose interpretation/assumptions.
- Use one interaction record per distinct conditional case. A case can have multiple consequences; do not restrict a case or cell to one scalar result. Matrix groups applicable cases into the same noun–verb cell; each case carries its own status, links, and findings.
- Each case explicitly names its applicable matrix noun(s) and verb, separately from contextual nouns. One case may apply to multiple nouns without duplicating its authoritative rule.
- Matrix membership and distinct-partner counts use declared pairs, not every noun mentioned in a condition/effect. Full impact graph still follows contextual links. Example: blocked push belongs to crate × push; wall is context unless separately declared as an applicable matrix noun.
- No interaction record means unconsidered/unknown, not automatically a defect. Do not require coverage records for every noun–verb pair.
- Record deliberate exclusions or non-applicability explicitly where needed. Keep these distinct from a defined interaction whose outcome is no effect.
- Support exploration from a cell into case details/source and a focused relationship neighborhood. Show case-level counts/classifications; a defined case does not establish exhaustive condition coverage for its cell. Do not default to a whole-project hairball.
- Measure primary noun/verb connectivity by distinct partners: distinct verbs per noun and distinct nouns per verb. Report conditional-case counts separately so many cases for one pair do not inflate breadth.
- Primary connectivity counts accepted, defined interaction pairs. Proposed/inactive cases and explicit exclusions/non-applicability do not inflate active connectivity; expose their counts/classifications separately when included.
- Include isolated nodes and allow sorting to expose highly connected and sparsely connected nodes within inspected scope. Expose supporting relationships; connectivity is not a quality score or automatic defect.

### Skill atom chains and ordered progression

- User can request skill atom chains and prerequisite relationships.
- Each skill atom defines its prerequisites; all listed prerequisites are required (AND semantics). Use `prerequisites`, not `dependencies`, for this skill relationship.
- Missing, null, and empty prerequisite lists mean the same empty set of declared relationships. Do not invent a separate unknown-prerequisite state or warning based only on YAML representation. Root means no declared prerequisites, not easy or required first.
- Skill prerequisites define the order in which skills must be acquired. Derived prerequisite depth does not determine when a skill appears in game progression.
- Track conceptual first-required placement in nullable integer `rank`. Keep separate from acquisition prerequisites, prerequisite depth, and physical level layout.
- Root skills need not appear or be acquired in first level. Show late-appearing roots and parallel chains at their intended progression points.
- Lay out all skill chains through their prerequisites. User can additionally request rank-ordered layout to show chains relative to designed progression.
- User can edit a selected skill's authored rank; recompute descendant display positions without overwriting their authored ranks. Show conflicts rather than fix them, visually and in JSON. See [Rank edit behavior](#rank-edit-behavior) for exact examples.
- Keep prerequisite order and designed progression placement distinct; make prerequisite/placement conflicts visible.
- Challenges explicitly distinguish introducing, practising, requiring, and demonstrating mastery of skills. A challenge may serve multiple roles; appearance alone is not evidence of learning, and declared roles do not prove observed understanding.
- Do not demand earlier teaching. When a challenge introduces a skill and requires it immediately, scaffolding review identifies a single-shot learn-and-demonstrate situation that may need attention, not an automatic missing-teaching error.
- A required skill without documented learning support raises an unresolved scaffolding question, not proof of impossibility. Support explicit intentional discovery or assumed prior knowledge with rationale; show that context with relevant findings. Repeated warnings are acceptable.
- Check every declared progression route to a challenge. Distinguish learning support guaranteed on all routes from support present on only some routes; identify routes lacking support.
- Incomplete route data yields unknown coverage, not a claim of complete scaffolding. Earlier rank alone does not establish that player traversed a teaching opportunity.
- Initial skill ranking is conceptual progression placement. Do not require correspondence to physical level layout; mapping skill atoms/ranks onto that layout is a future feature.
- Conceptual challenge/progression relationships still support requested scaffolding review; physical layout mapping is not a prerequisite for ranking or review.
- Initial progression routes are explicit next-challenge links plus marked entry challenges. Levels are optional groupings, not route authority. Skill rank does not imply challenge reachability.
- Route checks operate on this conceptual graph without requiring physical level layout. Incomplete routes remain unknown coverage.
- Challenge route conditions/gates remain prose initially. Structural checks inspect declared graph routes, not game-state reachability; report limitations where unencoded gates could change valid paths. Do not invent a progression rules engine.
- Cycles in challenge routes can represent revisits; cycles in skill prerequisites are conflicting acquisition requirements. Show issues, do not remove edges automatically.

### Canonical definitions, aliases, and templates

- Provide aliases for nouns, verbs, and other named design concepts so alternate terms resolve to one canonical definition rather than divergent duplicates.
- Resolve canonical names and declared aliases automatically to canonical records. Use canonical identity for relationships and analysis.
- Preserve display spelling; compare terms/aliases case-insensitively after trimming and collapsing incidental whitespace. Do not automatically equate punctuation, plurals, or other word forms; declare aliases for those variants.
- For an undeclared term that could refer to an existing concept, present likely matches and ask whether to reuse the concept/add an alias or create a distinct concept. Do not infer identity from similar wording alone.
- Require canonical terms and aliases to be unique across the entire design, including across collections. Do not permit a noun and verb to claim the same term as separate concepts.
- Report ambiguous aliases and collisions rather than silently picking a record. Glossary fields and normalization below define storage/matching; workbench supplies this layer because Attendant does not natively resolve concept aliases.
- Maintain a project glossary mapping alternative words to the agreed canonical term, with a short project-specific meaning and a relative link to concept documentation when it exists.
- For documented concepts, glossary defers to full concept documentation; do not maintain an independent competing definition.
- Permit inline glossary definitions when no concept document exists. Do not require a separate page for every term.
- Example: agreed term `HP`, alternative terms `health` and `lives`, and a sentence defining its meaning in this project can remain inline.
- Store one small Attendant glossary record per term. Generate a combined browsable glossary from those records; no separately authored copy.
- Glossary record owns agreed term, aliases, and optional relative concept-document link. Inline entries own their short definition; linked entries defer full definition to concept documentation.
- Domain records keep stable technical keys; glossary owns mutable terminology. Derive reverse terminology links rather than maintain duplicate alias lists in domain records.
- Glossary supports term discovery and global duplicate prevention across linked and inline entries. Promotion keeps glossary identity/aliases, moves inline definition into authorized concept document, and replaces definition with documentation link. Resolve paths from project root and rebase rendered links; broken links are findings, not permission to guess a target. Stable reference migration is required for concept-key/collection changes; an ordinary file move instead needs documentation-path repair.
- Construct each collection's template sections and usage guidance to give each definition one authoritative home.
- Other records reference that definition rather than restate independently editable rules or values. Views and exports derive from source definitions.
- Make ownership of conditions, consequences, shared rules, and control values explicit to prevent accidental drift or conflict between collections.
- Interaction cases own situation-specific conditions/outcomes. Mechanics own reusable rules; interactions reference those rules rather than create independently authoritative copies.
- Templates and tools must discourage duplicate rule definitions and help identify existing duplication, drift, and conflicts. Prose-level equivalence/conflict detection is agent review unless supported by declared structure.
- When a case-specific rule becomes shared, move its authoritative definition to the shared mechanic and replace former definitions with references; do not leave two authoritative versions.

### Control variables and exports

- Support integer, continuous-number, boolean, and enum controls. Arithmetic applies only to numeric controls; using booleans/enums as numeric operands is an error. Preserve types in exports where supported by target format.
- Integer controls reject fractional results unless an explicit rounding operation produces an integer. Never silently truncate exports or simulation inputs. Invalid/non-finite arithmetic is an error.
- Manage numeric control variables with absolute values and relative values expressed through named-control references and simple arithmetic. Support ratios, offsets, and formulas combining controls.
- Example: `enemy-move-speed` uses expression `${controls/player-move-speed} * 0.75`. Preserve authored relationship; changes to referenced controls propagate to resolved values.
- Detect missing references and dependency cycles rather than inventing values or evaluating indefinitely.
- Export current control values to CSV, INI, JSON, and potentially other formats. Exports and simulations can consume resolved numeric values without losing authoritative formulas in source records.
- Controls declare units; dimensionless controls are explicit. Validate dimensional consistency: compatible dimensions for addition/subtraction and derived dimensions for multiplication/division.
- Initial numeric representation follows Attendant/JSON numeric interoperability: binary64 continuous values and integers limited to exact safe range `-(2^53 - 1)` through `2^53 - 1`. Reject overflow/inexact integer storage rather than silently round. Larger numeric domains require an explicit later representation change.
- Do not silently convert compatible unit scales such as seconds and milliseconds; require explicitly compatible scales. Unit mismatches produce actionable errors.
- Formula constants may carry explicit units, e.g. `${controls/base-cooldown} + 0.2[s]`. Plain numbers are dimensionless; do not infer units from addition context. Separately tunable offsets may instead be control references.
- [Controls and experiments](#controls-and-experiments) defines evaluator/export behavior. Exact implementation limits must be documented and checked, never silently truncate.
- Simulations and exports consume the same current control definitions; do not maintain separate hand-copied values.
- Experiment/sweep overrides are temporary. Resolve dependent formulas using effective overrides and save the resolved input snapshot with each run. Do not mutate canonical baseline controls unless designer explicitly adopts a configuration.

### Player resources

- Formalize explicit game-state resources available to player: health, stamina, ammo, currency, inventory capacity, and similar pools. Do not expand initial resource model to attention, information, or other qualitative advantages.
- Describe pool limits, acquisition, spending, regeneration, loss, and conversions where relevant.
- Keep runtime resource state distinct from tuning controls. Resource definitions may reference nouns and canonical control values rather than duplicate their definitions.
- Resource records extend canonical noun concepts, not independent duplicate identities. Matrix and glossary use noun identity; resource record owns pool-specific metadata.
- Controls own tunable amounts. Mechanics/interactions own state-changing rules. Resource acquisition, consumption, regeneration, loss, and conversion remain prose initially; no mandatory structured consumes/produces fields or deterministic directional flow graph.
- Agent may interpret resource flows during requested framework review, citing canonical rules. Do not present inferred flow direction as tool-proven structure. Generated C experiments explicitly implement selected prose rules using current controls.
- At most one accepted resource extension per canonical noun; proposals remain alternative metadata, not competing active pools. Runtime instances can have different owners/balances without creating duplicate noun identities.
- Initial/capacity control references, when supplied, must resolve to numeric controls with compatible units. Missing capacity is unknown unless prose explicitly declares uncapped behavior; do not silently infer unlimited or zero. Capacity may be a hard cap or soft threshold according to canonical rules. Do not invent clamping, negative-balance policy, regeneration, or an initial-versus-capacity error without declared rules.

### Generated simulations and saved results

- Skill creates a simple C99 simulation as a one-shot answer to a specific question against a specific design snapshot, not a maintained model expected to follow future rule changes.
- New simulations pull in current control values and freeze effective inputs for that investigation. Expose modelling assumptions and unresolved rules rather than silently inventing behavior.
- If rules change and designer wants to check again, create a new simulation; retain old code/results as historical evidence. Do not silently update or reinterpret the old answer.
- One-shot describes question scope, not one random trial: stepping, batches, sweeps, and outlier inspection remain supported within the investigation. Exact historical reproduction uses recorded code/inputs, never current controls.
- Code generation and execution are distinct actions. A request to simulate authorizes generation, inspection, compilation, and bounded execution once material rules/assumptions are clear. A request only to write simulation code does not authorize running it.
- Ask about consequential missing assumptions or unsafe actions, not routine compile/run steps already authorized. Never execute on project open or document preview.
- Before simulation execution, commit any uncommitted changes as a checkpoint. Prepare generated simulation source/run inputs first so checkpoint captures code and inputs used.
- Confirm commit with user unless explicit auto-confirm permission is active. Show pending commit scope, including unrelated changes; respect project policy and permission scope. Declined/missing approval blocks simulation rather than silently running dirty.
- Record checkpoint commit and effective inputs/seed/model version with results. Do not rely on a clean baseline commit while relevant source remains dirty.
- Save simulation results with date stamps and reproduction metadata under [Reports and provenance](#reports-and-provenance). Retain prior evidence without accidental overwrite; no automatic retention deletion.

### Git-backed iteration

- Workbench operates with awareness that project data, scripts, and design documents live in Git.
- Use Git commits as checkpoints for exploration, comparison, recovery, and inspection of prior designs. Documents describe current design and rationale; they do not maintain a rigid duplicate change history.
- Keep decision rationale and experiment provenance where useful; Git history replaces edit logs, not explanations of current choices.
- Version source, configuration, reproduction recipes/input snapshots, and evidence that cannot be rebuilt. Rebuildable intermediates/build results do not belong in Git: executables, caches, and reproducible generated artifacts.
- Saving dated results and tracking them in Git are separate decisions. Results remain saved locally even when ignored as reproducible output. Preserve enough tracked provenance to regenerate ignored evidence; retain non-rebuildable observations rather than treating them as disposable.
- After an authorized checkpoint, make requested exploratory changes without redundant per-edit approval. Git reversibility does not authorize unrelated changes or new design decisions.
- Follow project commit policy. Request explicit approval when required before committing; do not interpret this capability requirement as authorization to commit current repository work.
- Inspect working tree and preserve unrelated user changes. Never use automatic destructive resets or history rewriting as a shortcut to recovery.
- Simulation execution requires a checkpoint of uncommitted changes, confirmed unless user explicitly enabled auto-confirm. Other exploratory checkpoint requests follow project policy; Git history must actually contain the state claimed recoverable.
- Compare requested Git versions read-only, showing changed source paths and declared impact. Inspect older data in temporary location; do not run historical code merely to compare documents. Recovery targets only explicitly selected paths/state. Show destructive scope and protect uncommitted work before replacement; never reset unrelated files or rewrite history.
- This specification does not authorize committing current repository work.

## Gap finding

### Two distinct evidence layers

- **Deterministic checks:** inspect declared fields, references, ordering, dependencies, and measurable experiment results. Same input and tool version produce same finding.
- **Agent review:** inspect prose, intent, plausibility, and omissions not encoded in rules. Cite records and label interpretation or suggestion. Never present an inferred omission as a tool-verified failure.

A clean deterministic report means only that implemented checks found no issues in inspected scope. It does not establish complete design coverage.

### Finding contract

Each actionable finding needs:

- Stable rule code and enough identity to recognize recurrence.
- Kind, scope, involved records, and source paths/fields.
- Specific trigger and supporting evidence.
- Consequence or design question, not just “missing link.”
- Suggested investigation or resolution options.
- Explicit uncertainty and applicable limitations.

Finding kinds: invalid/broken; missing required relationship; unknown/unresolved; possible opportunity; conflicting claims; unverified hypothesis; deliberately excluded.

Severity is separate: error, warning, or information, justified by consequences and declared constraints. Unknowns, sparse nodes, and opportunities are not automatically errors. Preserve deterministic/agent evidence class separately from kind and severity.

Absence alone is not a defect. Check applicability and project expectations before reporting a missing relationship. Deliberate exclusions retain rationale and qualify matching findings, not unrelated issues. Repeated warnings are acceptable; automatic suppression is not required.

### Result freshness

- Distinguish old analysis results from newly generated results. Do not present a saved historical report as a current check.
- Save a dated report for each requested analysis/review run, with run identity, scope, generation time, and inspected source-state context. Reruns create new reports rather than overwrite previous results.
- Repeated warnings may appear in successive reports. Source changes make older reports historical; do not imply an old result was revalidated.
- Normal gap exploration succeeds when analysis completes, even with findings. Invalid input/runtime failures still fail. Explicit severity thresholds enable nonzero exits for automation; validation fails on broken data. Review/validation does not automatically fix source; use nonmutating Attendant validation when reviewing.
- [Reports and provenance](#reports-and-provenance) defines timestamps, source-state capture, and historical presentation. Findings stay in run reports unless designer requests a source question/decision/hypothesis; do not create an issue record for every warning. Concrete generated design alternatives still follow retained-proposal policy.
- Recorded design intent and rationale remain useful even when a warning repeats; do not confuse repetition with a new design defect.

### Representative use cases

- **Crate interaction:** player can push crates, but pushing a crate against a wall has no defined outcome. Matrix shows an unresolved cell, not an automatic error. Review points to the missing consequence and asks whether movement is blocked, altered, or intentionally undefined.
- **Skill scaffolding:** a challenge requires crate planning, but declared prior progression establishes only movement and pushing. Tool reports the unsupported prerequisite with challenge and skill sources. It does not claim the player cannot discover the skill independently.
- **Experience support:** design intends tension, but no documented dynamic supports it. Report an unsupported design hypothesis, not proof that play lacks tension.
- **Progression experiment:** target names rank and time bound. Simulation reports time-to-rank distribution under stated assumptions. Separately requested analysis compares distribution with criterion; designer/agent interprets design significance. Missing assumptions block supported conclusions, not invite guessed player behavior.
- **Change impact:** removing pushing exposes dependent interactions, skills, challenges, and experiments. Separate known dependency reach from speculative experiential effects.

## Capability boundaries

- Workbench manages design knowledge and evidence, not game runtime implementation.
- No engine, game scripting language, document editor, generic issue tracker, or replacement for Attendant.
- Do not require a simulation for every mechanic or intended experience.
- Setup installs all supported table sets, but populating every collection or linking every domain is not mandatory. [Table responsibilities](#table-responsibilities) owns initial inventory; aesthetics owns natural-language experience/intent rather than requiring a competing intent table. Additional framework collections require a distinct fact/ownership need.
- Preserve mechanics/system/dynamic distinctions and intention/prediction/observation distinctions.
- Visualizations expose existing analysis; they do not introduce separate rules or authoritative state.
- Narrow automatic routing: activate for explicit game-design workbench, structured design-data, noun–verb matrix, skill-chain/progression, framework-review, and design-simulation requests. Skip ordinary game-code-only work and unrelated brainstorming, research, or technical review. Loading skill does not authorize setup, a lens review, or simulation execution beyond requested operation.
- Neighboring skills retain their responsibilities: brainstorming can generate ideas without setting up a project; coding/implementation handle game runtime code; technical review handles architecture. Workbench captures authorized design facts and proposals when requested, not every adjacent conversation.

## Refinement status

No unresolved product decision currently identified. Contracts below cover operations, source ownership, framework scope, partial data, aliases, progression, controls, reports, views, and simulation checkpoint flow. First-slice walkthroughs and delivery plan define reviewable outcomes.

Not yet established by execution: authored pack quality/source research, schema/fixture compatibility, full C99 loader/evaluator, native raylib build, safe rank patching, simulation supervisor/reproduction, seeded sample counts, performance/resource limits, and non-Windows portability. These are explicit delivery checks, not facts inferred from the LibYAML probe. If implementation reveals a material behavior/data/compatibility choice, return to refinement instead of silently changing this contract.

## Concrete operation contracts

### Source records and partial data

- Core collection aliases identify game-design roles; Attendant configuration determines directories. Extra collections remain allowed. Do not infer a new core role solely from similar field names.
- Preserve Attendant `id`, `name`, and `created_at`. Structured relationships use qualified record names, not display terms or filenames guessed from glossary words.
- Use flat fields and arrays. Keep detailed explanation in body; put experiment data/code in referenced sidecars. Use Attendant-safe technical keys matching `[A-Za-z0-9][A-Za-z0-9_-]*`; mutable glossary/display terms are not restricted to these filename stems. Resolve design paths within project root; reject traversal/escaping sidecar or document paths rather than read/execute arbitrary external files.
- Missing numeric values remain unknown, not zero. Missing rank remains unplaced, not first rank. Empty collections and incomplete optional relationships remain valid design states.
- Lifecycle defaults to proposed for generated content. Explicit supplied facts are captured as accepted unless user wording itself is tentative. Accepted means design authority, not observed truth.
- On current-design projections, use accepted rules by default; label proposed/inactive alternatives when explicitly included. Terminology checks and prior-decision lookup still inspect retained alternatives.
- Templates identify authoritative sections and distinguish links/summaries from definitions. Do not copy a source rule into every referring record.
- Relationship-set membership is based on declared links; missing/null/empty prerequisite representation does not change graph edges or create a separate design state. Scalar rank/value null remains distinct from numeric zero.

### Table responsibilities

Install these supported table sets; records need not populate every set:

- `nouns`: entity/resource identity and meaning, not duplicated pool properties.
- `verbs`: action identity and meaning.
- `interactions`: applicable noun(s), verb, context links, shared mechanic/control links, case coverage classification, and canonical case-specific conditions/consequences in prose.
- `mechanics`: reusable rules and state transitions; refer to controls for tuning.
- `systems`: membership/grouping of mechanics and constructed rule arrangements.
- `dynamics`: intended/predicted/observed behavior, supporting mechanics/systems, assumptions and evidence.
- `aesthetics`: natural-language intended player experience and supporting dynamics. Capture initial experience description here; do not require controlled aesthetic vocabulary before starting.
- `skills`: prerequisite AND set, conceptual rank, related mechanics, and descriptions of understanding/expression.
- `challenges`: introduced/practised/required/mastery-demonstration skills, entry marker, possible next challenges, and relevant mechanic/context links.
- `levels`: optional challenge grouping and design context. No initial physical-layout mapping or second route authority.
- `controls`: typed integer/continuous/boolean/enum settings, literal value or relative formula, units, optional declared bounds/allowed values. One authoritative setting per control.
- `measures`: named output quantities, meaning, type, and units shared by experiments, observations, and expectations.
- `expectations`: measurable criteria/targets over measures, with scope and relevant assumptions. Targets are not proof or observed results.
- `experiments`: question, fixed design snapshot, model/test method, assumptions, controls/measures/expectations, code or test-plan references, and linked results. Simulations answer one question; create a new experiment for a changed design. Distinguish simulation from player testing.
- `hypotheses`: unverified design claims and supporting/refuting evidence links; do not confuse hypothesis with accepted factual observation.
- `observations`: actual player/playtest/telemetry observations with source, date, context, and interpretation separated. Simulated outcomes are not observed player behavior.
- `decisions`: retained proposal records and adopted rationale, distinguished by kind and lifecycle. Group related alternatives and link elaborated domain records; do not require full domain normalization for each idea. No rigid edit transcript.
- `glossary`: canonical term, aliases, brief inline meaning or concept-document link; derived combined glossary.
- `resources`: noun extension owning pool properties/ownership/lifetime and canonical control/rule references. Runtime balances belong to simulation/game state, not baseline tuning records.

These responsibilities define core source ownership, not a closed collection inventory. Framework packs reuse canonical facts and may add collections for distinct required data. Such collections remain Attendant-compatible Markdown under the same project, not separate framework databases. Each addition needs an ownership/relationship contract and an example showing why core records are insufficient; shared inputs should serve multiple packs where useful.

### Initial field contract

Use Attendant built-ins for `id`, `name`, `created_at`, `desc`, and `tags`; do not redeclare them in schemas. Common domain lifecycle field: `status` with proposed/accepted/rejected/superseded/defunct values. Relation arrays are sets of qualified references; derive reverse links.

Core typed fields:

- Interactions: `nouns` → nouns array; `verb` → verb; `context_nouns` → nouns array; `mechanics` → mechanics array; `controls` → controls array; `resources` → resources array; `coverage` = unknown/defined/excluded/not_applicable. Conditional rules/consequences stay in body.
- Mechanics: relevant noun/verb/control/resource reference arrays. Body owns reusable rule, assumptions, and open questions; no structured consumes/produces fields initially.
- Systems: `mechanics` membership array. Dynamics: supporting `mechanics`/`systems`, related `hypotheses`, `experiments`, `observations` arrays. Aesthetics: supporting `dynamics`, related `hypotheses`/`observations` arrays.
- Skills: `prerequisites` → skills array; `rank` → nullable integer; `mechanics` → mechanics array. Body sections: learning loop, understanding, expression, evidence sought, open questions. Evidence sought is not an observation.
- Challenges: `introduces`, `practises`, `requires`, `tests_mastery` → skill arrays; `entry` boolean; `next` → challenge array; optional `mechanics`, `interactions`, `nouns`, `verbs`, `resources` reference arrays for declared use/context. Body records goals, assistance, intentional discovery/prior-knowledge context, and route-gate prose. Mentioning a mechanic does not prove every case is reachable in that challenge. Teaching/mastery roles describe design intent, not measured achievement.
- Levels: `challenges` membership array. No competing next-level route graph or physical map field in initial model.
- Controls: `value_type` = unspecified/integer/number/boolean/enum; `value` nullable scalar; `expression` string; `unit` string; `allowed_values` string array; nullable numeric `minimum`/`maximum`. Define literal or expression, not two active values. Empty fields preserve incompleteness; numeric unit `1` explicitly means dimensionless.
- Measures: `value_type`, `unit`, optional enum `allowed_values`; body defines quantity and measurement boundaries. Experiments reference this definition instead of redefining units.
- Expectations: `measure` reference; `statistic` = unspecified/per_run/mean/min/max/p50/p90/p95; `operator` = unspecified/lt/le/eq/ge/gt/between; nullable scalar `target`/`upper_target`; optional `target_control`/`upper_target_control` references instead of duplicate literals; nullable `minimum_pass_rate` in [0,1] for per-run criteria. Target values use measure units; reject competing literal/reference definitions. Missing aggregate target stays descriptive, not assumed 100% success.
- Experiments: `experiment_kind` = unspecified/simulation/playtest; `controls`, `measures`, `expectations` arrays; relevant mechanic/interaction references; project-relative `program`, `inputs`, `results` paths where present. Assumptions belong to experiment input sidecar/body, not exported game controls. Body states question, model scope, limitations, and evidence context.
- Hypotheses: relevant mechanics/dynamics/aesthetics, experiments, and observations references; body owns claim/assumptions/evidence interpretation. Observations: optional measure/hypothesis/experiment references, source/date/context, and optional data sidecar. Keep firsthand observation, reported evidence, synthetic data, and interpretation explicit.
- Decisions: `record_kind` = proposal/decision; optional `alternative_group` string, `related_decisions` → decisions array, and `subjects` string array of qualified subject/domain links. Attendant cannot type one reference field against multiple collections; workbench validates/indexes these polymorphic links rather than inventing `ref:any` syntax. Body retains proposal context or adopted rationale. Accepted proposal details become canonical domain definitions; decision retains links, not independently current rule text.
- Glossary: `term`, `aliases` string array, optional `documentation` path. Inline body owns short meaning only when no full document exists. Structured paths are project-relative; rendered Markdown links are rebased to containing document. Promote inline meaning into full concept document and replace former definition with link; do not copy into two editable homes.
- Resources: `noun` reference, optional `initial_control`/`capacity_control` references. Body owns pool ownership/scope/lifetime and points to canonical behavior rules. Related mechanics/interaction involvement is reverse-derived; flow direction remains prose.

Schema encoding uses optional `?ref:<collection>` / `?ref[]:<collection>` where partial records are allowed, `string[]` for simple lists, and null schema declarations for nullable scalar fields where a numeric default would invent a value. Attendant treats null schema declarations as optional/untyped; workbench enforces numeric/integer/scalar subtype constraints. Do not invent nullable numeric marker syntax. Workbench checks subtype/unit/range/cross-field rules that Attendant's flat schema does not express.

Source identity and typed links must still validate when provided. Missing/null relationship arrays describe no declared links; never infer reverse edges from prose. Required relationships mean explicit schema/role invariants or declared design constraints, not whatever a framework usually discusses. Prose-level constraints are agent-interpreted unless a documented deterministic check implements them. Accepted case classified defined/excluded/not_applicable must identify its applicable noun(s) and verb; incomplete cases use unknown coverage rather than fabricate links. Do not confuse no-effect outcome with exclusion.

Body templates need only relevant sections; do not demand every optional annotation or every framework's inputs on each record. Template implementation must pass Attendant schema/record tests and preserve this null/empty/ownership contract.

### Framework pack contracts

All 18 packs ship as attributed, independently authored guidance in first slice, with necessary collection templates and query helpers. Reuse Attendant queries or direct record inspection where sufficient; add C99 helpers when needed and justify `.mjs` exceptions. Each pack declares scope, useful source records, original questions, available queries, evidence limits, and missing-data behavior. Empty optional tables do not block conceptual discussion; reports identify unavailable data/checks. Do not add one collection per theory or require framework labels on every record.

Initial mapping uses core collections below. Collections such as skills, resources, and observations already supply framework-specific data. Add a supplemental collection only for a distinct source fact without an adequate existing owner; common framework vocabulary alone does not justify one.

- `mda`: mechanics/systems → dynamics → aesthetics, plus hypotheses/experiments/observations. Queries expose declared forward/reverse links and disconnected records. Agent asks which behavior and experience each rule supports. Missing links are documentation questions; links do not prove emotional outcomes.
- `game-grammar`: nouns, verbs, interactions, mechanics, challenges. Matrix/connectivity queries expose declared cases, exclusions, unknown cells, and contextual links. Agent examines conditions, consequences, discernible feedback, and inconsistent prose. Templates include case-specific feedback with consequences; shared feedback rules remain in mechanics.
- `resource-economy`: resources, controls, mechanics/interactions, measures, expectations, experiments. Queries resolve controls and inspect declared involvement; agent interprets prose flows/feedback loops. Explicit simulation requests can investigate distributions/sensitivity. No deterministic flow graph or automatic execution.
- `gameplay-patterns`: mechanics, systems, dynamics, linked proposals/decisions, and cited external pattern references in relevant bodies. Agent compares structures, alternatives, and applicability. No bundled exhaustive pattern database, automatic pattern identification, or copied catalog.
- `skill-atoms`: skills, challenges, optional levels, and related mechanics. Shared prerequisite/rank/route checks expose cycles, intent conflicts, and declared learning opportunities. Agent examines learning loop, understanding, expression, feedback, combination, and transfer; opportunity is not acquired mastery.
- `theory-of-fun`: skills, challenge progression, mechanics, and evidence. Reuse chains/role views; ask whether progression introduces patterns, combinations, or only numerical repetition. Do not compute fun or equate repeated actions with lack of learning.
- `flow`: challenges, skills, goals/feedback in source bodies, and relevant evidence. Original qualitative questions examine challenge, capability, clarity, and feedback. No required cohort data, estimated capability curve, or computed flow/anxiety/boredom score.
- `gameflow`: scoped mechanics/challenges, goals, controls, feedback, and observations. Original evaluative questions cover applicable enjoyment conditions; social criteria can be inapplicable. Keep distinct from Flow; do not reproduce a proprietary questionnaire or aggregate score.
- `cognitive-load`: declared required skills, prerequisite combinations, challenge context, information/feedback, and support. Queries show declared relationships; agent asks what must be coordinated concurrently and what remains unfamiliar. A required-skills list does not prove simultaneous demand or calibrated cognitive load.
- `working-memory`: same relevant skills/challenges and source descriptions of cues, memory demands, and learned combinations. Original chunking/external-memory questions; no fixed four/seven-item failure threshold or player-capacity dataset.
- `zpd-scaffolding`: skills/challenges, route opportunities, documented assistance, and observations. Reuse route/role queries; ask about demonstrations, hints, safe practice, intermediate combinations, and withdrawal of support. No teaching prerequisite, player diagnosis, or claim that unsupported skills make success impossible.
- `self-determination`: aesthetics, mechanics/dynamics, choices/constraints, challenges, hypotheses, and observations. Ask about autonomy, competence, and relatedness as common concepts. No personality profiles or inferred psychological measurements.
- `quantic-foundry`: intended experiences, mechanics/dynamics, rewards, and evidence. Use attributed published motivation vocabulary in original support questions. No audience/cohort collection, questionnaire reproduction, preference scoring, or single player type.
- `four-keys`: dynamics, aesthetics, interactions, and observations. Original questions explore challenge/triumph, curiosity, meaning, and social amusement. Keep distinct from MDA vocabulary and Koster's account; category links remain interpretations.
- `bartle`: player activities, exploration, social/competitive mechanics, and rewards. Original genre-aware questions; solo puzzle fixture reports limited applicability. No universal typing or requirement to serve every category.
- `caillois`: mechanics/dynamics, randomness, competition, role-play, sensory/movement descriptions, freedoms, and constraints. Original forms-of-play questions; no numeric profile or requirement to cover every form.
- `schell`: any scoped records, retained alternatives, and design rationale. Original contextual questions reuse relevant queries; no reproduction of complete deck/book or requirement for another lens run.
- `playcentric`: aesthetics/intentions, hypotheses, experiments, observations, and decisions. Query evidence links and distinguish untested claims, simulated predictions, and observed play. Original questions connect goals to prototypes/tests and resulting choices; do not invent evidence.

Section E perspectives are cross-cutting prompts attached to relevant packs, not extra mandatory datasets or an all-perspectives command. Framework-specific tools must name their actual query and required records; prefer shared C99 operations. Justify any dedicated `.mjs` query and document Node requirement. A question-only pack is usable without pretending its interpretive review is implemented in C.

### Rank edit behavior

- Graph supports prerequisite layout and conceptual rank layout. `rank` remains the field/CLI term; accept null or JSON-safe signed integer, including zero. Numeric rank is conceptual first-required placement, not prerequisite depth or a physical level number. Unranked atoms retain null authored rank even when graph constraints display them at a minimum position; distinguish authored rank from effective placement.
- Changing B from rank 2 to 4 changes only B's authored rank. Recompute downstream display constraints; upstream prerequisites and every descendant's authored rank stay unchanged.
- If C requires B and has authored rank 5, display at 5 or later. If C's rank is null, keep null and display at 4 or later. If C has authored rank 3, keep 3, display at 4 or later, and report conflict between intended rank and prerequisite constraint.
- Recompute display placement when B moves earlier too; never overwrite descendant intent as a layout side effect. Same-rank prerequisites remain allowed; prerequisite ordering still exists within coarse rank.
- Return changed source record, affected graph nodes, authored ranks, derived minimum/display positions, and consistency findings. Show issues visually and in JSON, not automatic corrections.
- UI and CLI use same rank-update operation. Patch only selected skill's authorized rank field, preserving bodies, IDs, and unrelated metadata. Requested valid rank edits may expose conflicts; do not block them or silently repair prerequisites/challenges.
- Detect source changes since view/read and reload or fail with actionable conflict rather than overwrite external edits. Rank edit changes one source file; use atomic replacement after checks. No descendant multi-file rewrite/transaction is needed.

### Scope, impact, and coverage

- A review scope selects finding focus, not permission to ignore external prerequisites or prior routes. Read supporting records outside focus when needed; report focus and inspected support separately. Guarantees concern declared graph/routes only, never every possible play path.
- Matrix scope uses selected nouns/verbs and canonical cases for those pairs. Challenge context can identify candidate rules through declared links; prose conditions are not automatically solved to prove availability. Represent cases sparsely; do not allocate a complete noun × verb product merely to show unknown cells.
- Impact reports direct/transitive declared links with paths and relationship kinds. Distinguish definitional/formula/prerequisite use from contextual association; graph reach does not establish causal experience changes. Broken direct references are deterministic; speculative downstream effects remain questions.
- Partial source is valid when optional data is absent. Wrong types, unresolved supplied references, duplicate identities/terms, and malformed source remain data findings. Load usable records where safe and mark omitted/invalid scope; never turn a failed load into an apparently empty design.

### Reports and provenance

- Give each run a UTC timestamp and collision-safe suffix. Directory form: `YYYYMMDDTHHMMSSmmmZ-<suffix>`; metadata date/time uses RFC3339 UTC. Never overwrite an existing run directory.
- Run directory keeps reproduction/source-state metadata separate from bulk derived artifacts. Save structured findings and readable report with links to evidence. Place rebuildable matrices/traces/plots/binaries under ignored `artifacts/`; retain metadata/recipes and non-rebuildable agent/playtest evidence. Do not apply an ignore rule that hides all authored reports. Supporting queries within one requested review need not create separate review runs.
- Source records live under configured `game-design/` collections, project-local tool snapshot under `tools/game-design-workbench/`, and generated reports/results under `game-design-results/`. Run recipes, simulation source, and non-rebuildable evidence are tracked; rebuildable artifacts are ignored. Keep generated reports out of record discovery.
- Record requested lens/framework, scope, source paths/SHA-256 fingerprints, relevant schema/usage/guidance hashes, tool/pack versions, baseline Git commit, and dirty-state information where applicable. Derive stable finding identity from rule code, canonical record keys, field/section, and trigger discriminator, not generation time or prose wording.
- Reports distinguish completed checks, unsupported/unperformed checks, missing data, and incomplete scope. No findings is not completeness evidence.
- Each finding includes stable rule code/identity, kind, severity, evidence class, involved records, source field/section, trigger/evidence, consequence/question, and suggested investigation.
- Agent findings use same machine-readable report structure but remain marked agent interpretation. C tools do not claim to perform unavailable agent-only reviews.
- Reopen a saved report as historical unless inspected source/tool state still matches; never relabel it as newly run. Source changes during a run must be visible, not silently mix snapshots.
- Explicitly requested multiple frameworks may be compared; preserve disagreements and evidence limits. No implicit all-framework execution or aggregate design score.

### Controls and experiments

- Formula syntax: decimal/scientific numeric literals, qualified references `${controls/record-name}`, unary `+`/`-`, binary `+`/`-`/`*`/`/`, and parentheses. Use standard arithmetic precedence; references resolve stable control keys, not glossary aliases.
- Permit unit-bearing literals such as `0.2[s]`; unadorned literals are dimensionless. Unit expressions use case-sensitive named symbols, products/divisions, and integer powers; `1` is dimensionless. Canonicalize algebraically equivalent forms such as `m/s` and `m*s^-1`. Symbols are opaque, including game-specific units; no implicit SI scale/offset registry. Conversions require explicit formulas, e.g. multiply an ms value by `0.001[s/ms]`.
- Support `min(a,b)`, `max(a,b)`, `clamp(value,lower,upper)`, and unary `abs`, `floor`, `ceil`, and `round`. `round` uses ties away from zero. Comparisons/conditional expressions, assignments, arbitrary functions, and code execution are not part of control expressions.
- Preserve units through rounding/absolute value; min/max/clamp require compatible units. Unknown functions, invalid bounds, or excessive expression complexity produce actionable errors. Never use a shell or arbitrary code evaluator.
- Invalid references, cycles, unit/type mismatches, division by zero, non-finite values, or invalid integer results block affected export/execution. Inspection can still explain partial/invalid source.
- Export CSV/INI/JSON from same resolved values used by simulations; stable control keys do not change when glossary display terms change. Default export is resolved values, not executable formulas; metadata remains available through control inspection.
- CSV: one row per control, columns `key,type,value,unit`, quoted/escaped using conventional CSV rules. Preserve data values; warn about spreadsheet-formula-like strings rather than silently prefixing/changing enum data, and never auto-open exports in a spreadsheet. JSON: object keyed by stable control name with typed scalar values. INI: `[controls]` section, numeric/boolean literals and quoted escaped enum strings; document this dialect rather than claim universal INI typing.
- Keep full type/unit/formula/source metadata in structured control inspection and run snapshots. Exports are generated artifacts, not another editable authority. Sort keys for stable output.
- Refuse to replace an existing output file without explicit overwrite permission. Never overwrite canonical design records/configuration/tool source through export.
- Temporary overrides recompute formulas without changing baseline; explicit adoption changes canonical controls.
- Each one-shot experiment uses common input/measurement helpers rather than hand-copied control constants. Freeze declared controls and their transitive formula dependencies; unrelated unknown controls do not block the investigation. Missing required inputs do block execution.
- Do not keep old simulations synchronized with later rules. Results name canonical measures; viewers use their recorded type/unit definitions.
- Simulation request establishes model question and material assumptions; missing target permits descriptive exploration, not an invented pass criterion.
- Model/player-behavior assumptions are experiment-local by default, explicitly recorded with question inputs. Do not mix assumed action delays, policies, or success rates into exported game tuning. Shared reuse requires deliberate promotion into appropriate design data.
- Bound runs, steps, and elapsed execution; persist partial/error/limit status rather than label a stopped run successful. Incomplete evidence cannot establish a satisfied target.
- Initial defaults: 1000 batch runs, at most 1,000,000 steps per run, 30-second execution timeout. Expose positive explicit overrides; record effective limits. Compile/startup failures are distinct from simulated outcomes; verify these defaults against fixture workload before claiming usability.
- Seed random runs through a specified shared generator, not compiler-specific `rand()`. Save generator/model version, seed, effective controls/overrides, and checkpoint commit. Historical reproduction loads recorded source/inputs; a new answer about current rules gets a new simulation.
- Use prepare/checkpoint/resume flow. Prepare freezes generated code/dependency identities, effective inputs/overrides, assumptions, seed, requested trials, and limits into a tracked run recipe. Return recipe ID, prepared source paths, and checkpoint requirements without executing simulation.
- Agent or human obtains approval and makes Git checkpoint through ordinary Git operations. C tools do not automatically commit. Resume references same recipe ID; retry must not regenerate inputs or timestamps into a different recipe and create another dirty-state loop.
- Resume verifies recipe integrity, relevant source fingerprints, committed recipe/code/input state, and required clean working tree. Ignored/untracked relevant sources do not count as checkpointed; report them and never silently force-add ignored/private files. Whole-tree pending scope uses ordinary Git status; ignored unrelated private files are not staged. Keep approved rebuildable outputs ignored. Changed material inputs require a new preparation/checkpoint rather than silently updating frozen recipe. Declined approval blocks execution.
- Record actual checkpoint commit in run result metadata after checkpoint; do not rewrite the committed recipe to contain its own future commit hash. A prepared recipe is not an executed result. Opening it or viewing results does not authorize execution.
- Support stepping, batch runs, distributions/percentiles, numeric parameter sweeps, and selection of an unusual run. Sweep `start:end:step` uses inclusive start and includes end only when reached; reject zero/wrong-direction step, non-finite values, or nonintegral points for integer controls. Compute points from index rather than cumulative addition; record effective points in prepared recipe. Initial maximum is 100 sweep points, overridable explicitly; trial/output/time limits still apply across whole prepared sweep. A deterministic model may have zero spread; do not invent randomness.
- Generated visualizations and headless outputs read same measures/results. A simulated prediction remains conditional on stated assumptions, not observed player behavior.

### Simulation program and measurement contract

- Run prepared model as a separate local process, not a dynamically loaded viewer plugin. Generated C99 program accepts `--input <frozen-input.json>` and emits versioned NDJSON on stdout; diagnostics go to stderr. Shared C helpers handle typed inputs, PCG32, step accounting, and event encoding. Conventional compiler and CJIT builds use same helper sources.
- Inputs contain recipe/experiment identity, effective typed controls with units, experiment-local assumptions, frozen measure/expectation definitions, trial count, seed, and limits. Model reads this snapshot, not current design files. Record code/helper/dependency hashes and build invocation/toolchain in provenance. No arbitrary expression or shell evaluation.
- Output begins with a header identifying protocol/model/recipe, emits trial results and optional step records, and ends with explicit completion/count/status. Each trial has configuration ID, stable zero-based trial index, and measurements keyed by canonical measure identity. Combined sweep trial identity is configuration ID plus trial index; repeated indices across distinct configurations are valid. Step records name trial, monotonically increasing step, and inspectable state/measurements. Validate identities, declared measurement types/units, duplicate indices, non-finite values, and missing/invalid completion; malformed output is not successful simulation.
- Default stores per-trial results; detailed trace is requested for selected trial(s). Viewer may step through saved trace without executing code. New trace generation or historical resimulation is an explicit execution with checkpoint/limits, not an automatic effect of opening a chart.
- Use PCG32 XSH-RR with explicit state initialization, algorithm/version recorded as `pcg32-xsh-rr-v1`. Master seed is uint64, serialized losslessly as a hexadecimal string; trial index selects stream with same master seed. Corresponding sweep trials reuse streams, with divergent model paths potentially consuming different draws. Uniform helper uses `ldexp(draw, -32)` on [0,1); bounded integers use rejection, not biased modulo. No compiler-specific `rand()` or cryptographic claims.
- [PCG minimal C API](https://www.pcg-random.org/using-pcg-c-basic.html) documents seed/stream repeatability and upstream golden sequence. Verify golden outputs before fixture counts; pin/review source and retain license if vendoring implementation. Source inspection is not yet a local PRNG/compiler test.
- Compute min/max/mean and nearest-rank p50/p90/p95 over defined numeric samples, reporting valid/undefined counts. For percentile p and N > 0, sorted one-based element is `ceil(p*N)`; N = 0 yields undefined, never zero or NaN. Label conditional summaries when some outcomes have no defined measure.
- A modeled goal not reached can produce explicitly undefined time-to-goal with reason; keep elapsed attempted time and completion state distinct. Unexpected missing measurements are output errors.
- Simulation summaries report finite seeded measurements, counts, and spread without design judgments or expectation-based verdicts. Completed execution returns exit 0 regardless of what measurements suggest about design. Do not add a simulation failure flag for unmet design expectations.
- Compile separately to ignored build output, then execute under supervisor. Initial compile timeout is 30 seconds; execution uses declared 30-second default and step/trial limits. Cap captured output at 64 MiB by default, with explicit positive override. Exceeding a limit stops execution, preserves available evidence, and reports incomplete status. Verify these engineering defaults against fixture; no unmeasured performance claim.
- Separate process and watchdog are not a security sandbox. Native C runs with user's permissions. Inspect generated code before execution; do not run untrusted repository code merely because a recipe names it. Generated models must keep I/O to declared inputs/results and avoid network, shell execution, or unrelated filesystem changes. Unsafe code needs resolution, not implied permission from a simulation request.
- Replay uses frozen code/helper/input versions in an isolated working directory without overwriting current design or checking out over user changes. Missing historical dependencies are actionable reproduction failures, not permission to substitute current controls/code. Each execution gets new dated result identity linked to original recipe/run.

### Requested expectation analysis

- Evaluate expectations only as separately requested analysis over saved results, e.g. `gd expectations --run <run-id>`. Report literal criterion comparisons and sample fractions; do not prove underlying player success probability or judge design quality. Designer/agent owns interpretation. No mandatory confidence-interval model or inferred statistical significance.
- Per-trial comparisons use declared type/unit: numeric order/range, boolean/enum equality. Aggregate numeric expectations do not coerce booleans/enums. Equality is exact; tolerance requires explicit range. A minimum pass rate uses all valid attempted trials as denominator, including modeled failure outcomes; do not drop incomplete routes to improve measured fraction.
- Undefined time-to-goal never counts as satisfying a completion-time criterion. Incomplete/timeout/runtime-error evidence cannot establish satisfied criteria. Aggregate comparisons with undefined inputs remain indeterminate unless experiment explicitly defines eligible population; never silently average away failures. Mathematical comparison is not a design verdict and does not change simulation execution status.

### Headless and viewer boundary

- C99 source loader/index/queries/analysis are shared by headless tools and raylib views. Attendant remains generic record/search/schema/projection tool.
- C99 headless core does not require raylib, a display, an agent, Node.js, or Attendant projection. Generic Attendant operations and any justified `.mjs` framework query retain their documented Node runtime dependency. Mark command-specific dependencies; do not silently make Node mandatory for unrelated C99 operations.
- Required native projections across full delivery: matrix; prerequisite/rank skill graph; conceptual challenge/skill-role progression with optional level groups; focused relationship graph; MDA trace; experiment traces/distributions/sensitivity; diagnostics browser. No required HTML counterpart or physical level map.
- Matrix expands multiple cases, conditions, outcomes, dispositions, context links, and source navigation. Focused graph starts at selected record/case and exposes adjacent links with explicit expansion. Reuse same relationship data; do not reimplement analysis in renderer. Connectivity sorting includes zero-degree nodes and case counts without a quality cutoff.
- Select skill node, edit rank or clear to null, then save selected field. Display authored versus derived position and linked findings. Other design authoring remains Markdown/agent-driven; transient experiment inputs and generated results are not baseline-control edits.
- Findings link to record path and field/section. Open through explicit user-selected editor or safe document association; otherwise show/copy path. Never execute a linked script/program as source navigation.
- Skill graph supports rank editing as specified. Other views are exploratory initially.
- Direct-source and structured-output parity applies to every deterministic visible finding; agent-only interpretations must be saved as structured findings before a viewer can display them.
- Viewer refresh must detect externally edited Markdown; keep last-good results clearly stale if current source cannot load. Do not save against a stale model.
- Project-local tool snapshot includes required sources, licenses, scripts, and explicit dependency/version information. Resolve paths relative to project/tool location, not current shell directory assumptions.
- Upgrades are explicit. Compare local snapshot changes; stop/report conflicts rather than overwrite local tool or template edits.

### CLI contract requirements

- Commands cover source inspection, validation, selected-lens gaps, interaction matrix/graph, skill chains/rank updates, connectivity, impact, controls/export, expectations, simulations/sweeps, views, and report inspection.
- `gaps` requires explicit framework selection unless calling context already supplies it. Framework catalog/listing is discovery, not execution of every pack.
- stdout contains result data; diagnostics/progress/display tables use stderr. Explicit output format wins; piped/agent paths emit JSON without interactive prompts. Support field selection for large structured results.
- Version structured schema; include data, provenance/coverage, and structured errors. Empty successful results are empty data, not failures.
- Completed exploratory gaps succeed with findings. Explicit failure threshold can fail automation; invalid input/runtime failure and broken-data validation fail nonzero. Preserve partial results with explicit incomplete/error status where safe.
- Mutations report changed source paths. Missing required approval in noninteractive use fails with actionable message; noninteractive mode never implies commit consent.
- Use `gd` command name through project-local Windows launcher, with explicit `--project <path>` when not running from target project. Human instructions show launcher path rather than assume global installation.
- Common flags: `--json`, `--format <format>`, `--fields <comma-separated-fields>`, and `--output <path>` where supported. Explicit incompatible format flags fail rather than silently choose one. `GD_AGENT=1` selects noninteractive JSON mode.
- Exit codes: `0` completed success; `1` validation/finding threshold failed; `2` invalid input, missing prerequisite/approval, or runtime failure. Include partial data with explicit incomplete status when safe.
- `gd simulate`/`sweep` enforce clean checkpoint and tracked run sources. Skill uses ordinary Git commands and user confirmation to satisfy checkpoint requirement; C tool does not become an automatic Git commit/recovery subsystem.

Representative commands (planned contract, not installed commands):

```text
gd validate
gd lenses
gd inspect nouns/crate --json
gd matrix --json
gd connectivity --json
gd skills --json
gd skills set-rank skills/coordinate --rank 3 --json
gd impact mechanics/crate-pushing --json
gd gaps --lens skill-atoms --scope challenges/combined-room --json
gd controls --json
gd controls export --format csv --output controls.csv
gd controls export --format ini --output controls.ini
gd controls export --format json --output controls.json
gd expectations --json
gd expectations --run <run-id> --json
gd simulate prepare experiments/route-cost --runs 1000 --seed 42
# Review pending files and obtain required approval; make ordinary Git checkpoint.
gd simulate run <recipe-id>
gd sweep prepare experiments/route-cost --param walk-speed=1:3:0.5 --seed 42
# Checkpoint prepared sweep inputs before execution.
gd sweep run <recipe-id>
gd replay <run-id>
gd view matrix
gd view skills --layout rank
gd report <run-id> --json
```

Framework IDs and any rule-specific flags are documented in bundled catalog; no `--all` framework selector. Prose review findings are agent-generated report data, not a claim that C performs every framework interpretation.

Structured envelope version 1:

```json
{
  "schema_version": 1,
  "ok": true,
  "data": {
    "changed": [{"record": "skills/b", "rank_before": 2, "rank_after": 4}],
    "affected": [{"record": "skills/c", "authored_rank": 3, "minimum_rank": 4, "display_rank": 4}],
    "findings": [{
      "code": "skill.rank_conflict",
      "origin": "deterministic",
      "kind": "conflicting_claims",
      "severity": "warning",
      "records": ["skills/b", "skills/c"],
      "sources": [{"path": "game-design/skills/c.md", "field": "rank"}],
      "reason": "Authored rank 3 precedes prerequisite constraint 4.",
      "suggested_investigation": "Review intended first requirement or prerequisite relationship."
    }]
  },
  "coverage": {"status": "complete", "operation": "skills.set-rank"},
  "errors": []
}
```

Example shows expected shape, not executed result. Read/report commands include provenance metadata; mutation result distinguishes changed source from affected display nodes. Empty results preserve envelope with empty data collections. Failures set `ok: false`, nonzero exit, and stable `errors` entries containing code/message and source context. Data field selection never hides required error/coverage metadata.

## First-slice walkthroughs and acceptance

First slice delivers usable guided work, not a claim that later native tools exist. Record installed capabilities and use Attendant for available generic queries/validation. Preserve evidence origin when agent inspects relationships manually.

1. **Setup:** request “Set up game-design workbench here.” Return combined file/collection/dependency proposal. After approval, install empty configured collections and local snapshot. Existing Markdown and custom configuration remain unchanged outside approved plan; no tutorial facts appear in live design.
2. **Direct capture:** supply “Crates can be pushed into free cells; wall-blocked pushes cost no energy.” Resolve declared box/shove aliases if used; capture canonical case/mechanic facts as accepted. Do not invent an energy amount, interview for experience first, or run a lens review. Return changed paths and unresolved material questions only.
3. **Generation:** request “Suggest alternatives to make blocked pushes interesting.” Retain each idea as a proposed decision with context/group/subject links. Surface rejected pulling rationale if relevant. Choosing one adopts only selected facts; siblings remain proposed and rules retain one canonical home.
4. **Scoped review:** request “Review combined-room scaffolding using skill atoms.” Load that pack and relevant challenge/prerequisite/route context; save dated structured and readable findings. Report missing planning opportunity, route-dependent switch support, single-shot roles, and rank conflict only to extent established by inspected data. No automatic Flow or other-lens review; no silent repair or invented player incapability.
5. **Change impact:** request “What would removing pushing affect?” Trace declared references to cases, skills, challenges, and experiments. Return paths/link kinds and uncertain consequences; do not remove records or reinterpret historical simulations.
6. **Resolve:** explicitly change `coordinate` authored rank from 2 to 3 in baseline example. Rerun same skill review; rank conflict clears while missing/route-dependent learning questions remain. First slice may perform authorized source edit through Attendant; later GUI/CLI rank operation must produce same source outcome.
7. **Other packs:** exercise one explicit selected-pack request per pack against example or relevant partial source. Original questions and limitations remain useful with empty observations. Bartle can report limited applicability; Flow/cognitive/motivation packs do not demand cohort records. No all-lenses review command or aggregate score.
8. **Simulation handoff:** first slice documents question/control/assumption/expectation capture and prepare/checkpoint/resume contract. Do not claim executable simulation until its delivery slice passes tests. If an essential first-slice pack query needs a helper, deliver and test that helper rather than label an unavailable operation supported.

First-slice acceptance:

- All 19 core collection schemas/usages/templates validate with Attendant; representative empty, incomplete, accepted, proposed, excluded, and conflicting records have documented meaning. Required pack-specific additions follow same ownership rules.
- All 18 packs have citations, original actionable prompts, declared inputs/queries, evidence limits, and selected-pack walkthrough evidence. Research source materials before authoring; report inaccessible evidence and avoid unsupported claims.
- Local quickstart/references and runtime dependencies suffice after removing global skill from test environment. Generic Attendant snapshot includes runner and YAML dependency. No Node-free claim for these commands.
- Tests cover direct capture without unsolicited review, alias resolution/collision, retained alternatives/adoption, review without repair, current/historical reports, and supplied versus invented facts.
- Validate skill frontmatter, links, conditional reference discovery, and repository index entry. No workbench implementation, workflow execution, or pack-source research is implied by this specification alone.

## Worked example contract

Use tiny solo crate-and-switch puzzle as illustrative test data, not a second production game.

### Baseline rules and data

- Player moves on grid, pushes crates into free cells, and uses an occupied switch to open exit. Walls block movement/pushing. Pulling is not available.
- Successful push spends configured energy; blocked push does not spend energy. Zero energy prevents a successful push but not walking. Resource is per-challenge player pool initialized from canonical controls.
- Nouns cover player, crate, wall, switch, exit, and energy. Resource record extends energy noun. Aliases include `box` for crate and `shove` for push; add one inline-only glossary term to test lightweight entries.
- Interaction records distinguish energy-sufficient free push, wall-blocked push, insufficient-energy push, and switch activation. Crate × push has multiple cases; wall/switch/player are contextual links where appropriate. Include wall × activate as explicitly non-applicable and exit × walk while closed as an explicit unresolved case. Do not fill every pair or present defined cases as exhaustive coverage.
- Shared movement, switch, and energy rules have one canonical home; interaction cases reference them. Include one documented but unlinked mechanic for requested review, not an automatic defect.
- Preserve rejected pulling proposal with rationale about irreversible-push design. Keep another unchosen idea proposed, without fully modelling it. Future idea generation must surface prior rejection when relevant.

### Skill graph and challenges

Six atoms across two roots/chains with a join:

```text
move (rank 0) -> push (rank 1) -> plan-push (rank null) ----+
                                                        +-> coordinate (authored rank 2)
read-switch (root, rank 3) -> route-switch (rank 3) -------+
```

- `coordinate` requires both `plan-push` and `route-switch`. Display at rank 3 or later; retain authored 2 and flag conflict.
- Late root `read-switch` proves root does not mean first-level skill. Null `plan-push` remains null even when prerequisite placement constrains display.
- Editing only `push` to rank 4 moves descendant display constraints to 4 or later without changing `plan-push` null or `coordinate` 2. Upstream/parallel authored ranks remain unchanged.
- Challenge graph: `move-demo` (entry, introduces move) → `push-room` (requires move/push, introduces push) → either `switch-practice` or directly `combined-room`; `switch-practice` → `combined-room`. Optional practice introduces read-switch/route-switch and practises push. Combined challenge requires plan-push/route-switch/coordinate and introduces coordinate. Its empty next set means no further declared links, not complete physical-game reachability. Route authority is next-challenge links, not level geometry.
- Seed single-shot learning, a missing planning-support opportunity, and switch support available on only some declared routes. Roles stay explicit; findings remain questions, not proof a player cannot succeed.
- Levels optionally group challenges. No physical level-layout implementation.

### One-shot simulation question

Question: under explicitly assumed execution delays, does a known valid route complete within 8 seconds in at least 90% of trials?

- Route executes four walk actions followed by three successful pushes. Assume route legality and successful decisions; do not simulate player puzzle solving.
- Canonical controls: tile distance 1 m; walking speed 2 m/s; walk duration derives as distance/speed; push duration derives as twice walk duration; push cost 1 energy; initial energy/capacity 3 energy.
- Experiment-local assumption: independent uniform delay from 0 to 1 second per action. This is a modelling assumption, not measured player behavior or exported game tuning.
- Without delay, route time is 5 seconds. With assumed delays, supported time range is 5–12 seconds. Continuous-model analytical pass probability at 8 seconds is `1312/5040`, approximately 0.2603; this is a mathematical reference, not an executed simulation result.
- Seeded batch reports distribution and completion times only. Separate requested analysis can compute fraction finishing within 8 seconds and compare it with declared 90% criterion; simulation itself does not issue a design verdict. Verify actual seeded results after implementation; do not invent exact sample counts now.
- Temporary initial-energy override 2 produces route failure without changing baseline 3. Sweep walking speed to demonstrate dependent formula resolution and sensitivity.
- Record elapsed time, route completion, remaining energy, input snapshot, seed, limits, and checkpoint commit. Save dated results; inspect one unusual run by recorded identity.
- Changed rules prompt a new simulation/question instance. Historical replay uses old snapshot, not updated controls.

### Expected evidence and test method

- Base model is structurally readable while intentionally incomplete. Requested grammar/scaffolding/MDA reviews expose seeded questions with sources and distinguish agent interpretation from deterministic structure.
- Flow/cognitive/motivation frameworks use common conceptual vocabulary, not modeled cohorts or required numeric capability estimates. Do not invent thresholds or observations. Bartle's MUD-specific perspective can report limited applicability to solo puzzle rather than require multiplayer features.
- No fabricated playtest evidence. Observation tables can remain empty; use explicitly marked synthetic records only in isolated data-validation tests when needed.
- Tests copy example into temporary project/Git repository. Do not mutate bundled source, checkpoint this skills repository, or include unrelated user files.
- Planned stable findings: `skill.rank_conflict` (deterministic warning) for coordinate; `interaction.explicit_unknown` (deterministic info) for declared unresolved case; `scaffolding.single_shot` (deterministic info) for declared introduction/requirement overlap; `scaffolding.support_some_routes` and `scaffolding.support_absent` describe declared opportunity coverage, not actual learning. Agent prose findings retain separate origin. Injection fixtures cover `skill.prerequisite_cycle`, `reference.missing`, and `term.collision`; do not claim these occur in baseline.
- Apply explicit test edits and rerun same checks: coordinate rank 2 → 3 clears baseline rank conflict; unrelated learning/coverage questions remain. Alias collisions, malformed data, missing refs, and scalar/enum/units boundaries use small temporary mutations.
- Verify CLI/view parity, dated report freshness, current-versus-historical source, exports, temporary overrides, and reproducible simulation. Primitive boolean/enum codec tests need not add artificial gameplay mechanics to tiny example.
- Framework packs use original questions and reference-specific limits. Static pack validation does not establish full execution quality; run selected agent workflows and record checks actually performed.

## Attendant grounding

Static inspection of `skills/attendant/scripts/attendant.mjs` and `references/commands.md` establishes:

- Flat scalar/null fields and scalar arrays are supported; `string[]` supports alias lists. Nested objects are not supported record data.
- Structured references use `collection/name`; resolution uses record `name`, falling back to filename when absent. UUID `id` is not the reference spelling.
- Runner updates reject changes to `id`, `name`, and `created_at`. Keep stable technical record keys separate from mutable agreed terms and aliases.
- File location is not itself the reference key when record `name` and collection identity are preserved. Cross-collection moves or name changes need explicit reference migration; do not promise automatic stable-reference repair.
- Schema inspection confirms null declarations are optional/untyped, scalar defaults determine scalar type, and enum declarations supply allowed values. Missing/null enum values are tolerated by Attendant validation; workbench must not silently infer accepted authority from that tolerance. Explicit lifecycle values belong in templates/captured records; unset authority is reported rather than treated as accepted.
- Runner imports sibling `scripts/yaml.mjs`; project-local Attendant snapshots must include it. Runtime imports `node:sqlite`; documented Node floor is >=24.10.0.
- No concept-alias mechanism was found in inspected runner paths. Workbench must provide terminology semantics on top of compatible fields, not claim Attendant resolves aliases natively.
- Use LibYAML event parser rather than write YAML syntax parser: Windows/CJIT feasibility probe passed. Enforce Attendant-compatible flat record semantics in workbench wrapper; parser library alone is not schema validation.
- Wrapper must reject duplicate keys, unsupported nesting/aliases, and incompatible field values explicitly, and match supported YAML scalar/quoting semantics rather than silently reinterpret strings/numbers.
- Generic record operations still use Attendant; C tools load source for game-specific analysis.
- Inspected Attendant setup, empty-collection, and migration workflows: preserve existing sources/configuration; collection creation and migration have explicit proposal/apply gates. Workbench setup must honor these gates, using a concrete combined setup proposal rather than silently creating collections.
- `node --test skills/attendant/test/attendant.test.js`: 7 tests passed on current Windows environment. Coverage includes collection setup, live usage/schema, record operations, strict validation, query containment, and reviewed migration. This does not yet test workbench schemas or specialized move/alias behavior.
- Local `cjit --help` reports CJIT v1.2.0 and supports multi-source input, `-I`, `-l`, `-L`, compile-only `-c`, and executable output `-o`. Node.js v24.16.0 is available. Raylib integration and actual workbench compilation remain untested.
- LibYAML feasibility, checked 2026-10-19: [MIT license](https://github.com/yaml/libyaml/blob/master/License); release 0.2.5 commit `2c891fc7a770e8ba2fec34fc6b545c672beb37e6`. Full source/header set is 377,662 bytes; read-only event parser needs `api.c`, `parser.c`, `reader.c`, `scanner.c`, `yaml_private.h`, and `yaml.h`: 282,505 bytes.
- Temporary probe compiled/ran through local CJIT v1.2.0 using `HAVE_CONFIG_H=1`, `YAML_DECLARE_STATIC`, and generated version config. Both full build and four-C-file parser build passed flow arrays/quoted formula, block arrays/quoted colon/null, and malformed-YAML rejection checks. No sources vendored into repository yet.
- Probe establishes parser/link feasibility, not full Attendant compatibility, resource-limit behavior, upstream security review, or conventional-compiler portability. Retain license, pin reviewed vendor revision, and port focused cases into implementation tests.

## Work plan

### Refinement — complete

- [x] Ground starting brief, framework reference, Attendant contracts, and Windows/CJIT parser feasibility.
- [x] Define non-linear operations, routing/authority, lifecycle, source ownership, and partial-data semantics.
- [x] Specify all 18 pack contracts, first-slice walkthroughs/acceptance, model fields, CLI/results, projections, controls, and one-shot simulation flow.
- [x] Separate settled behavior from unverified implementation claims. Refinement ended before implementation; user subsequently authorized implementation and step commits.

### Slice 1 — usable skill, model, and all framework packs

- [x] Create `skills/game-design-workbench/SKILL.md`, conditionally loaded operation references, 18 pack documents, source citations, and original prompts. Each pack records actual source-access limits, including abstract/search-only evidence.
- [x] Implement 19 core schema/usage/body-template sets, shared query support, setup/update/resume instructions, snapshot manifest, and local quickstart/dependencies. No supplemental theory datasets needed.
- [x] Build bundled crate-and-switch source fixture and retained proposals, glossary, resource/control/formula examples, explicit case classifications, skill/challenge graph, and expected findings. No simulation execution claimed.
- [ ] Test setup against empty and preconfigured temporary projects; preserve user files/configuration and honor Attendant proposal/apply gates. Test all first-slice walkthroughs, one selected framework per run, partial source, and unsupported-check disclosure.
- [ ] Add identity/reference/alias tests with actual schemas; verify generic Attendant operations locally without global skill. Validate entrypoint/frontmatter/links and update repository skill index.

Acceptance: [First-slice walkthroughs and acceptance](#first-slice-walkthroughs-and-acceptance). No broad native application or working simulation is implied.

### Slice 2 — C99 source core and structured inspection

- [ ] Vendor reviewed/pinned LibYAML subset with license; implement bounded flat source/config/schema loading, typed scalar semantics, identity/term lookup, declared relationships, and reverse indexes. No database fork.
- [ ] Deliver launcher and headless `inspect`, `validate`, `impact`, catalog/report operations with versioned JSON, structured errors/coverage, source fingerprints, and dated reports.
- [ ] Verify supported flat YAML against Attendant; reject unsupported syntax, duplicate keys, invalid references/types, unsafe numbers, and malformed input explicitly. Document/test finite input/expression/graph/output limits; no silent truncation.
- [ ] Run without Node, raylib, display, agent, or `.attendant/` projection. Test paths with spaces/non-ASCII text, external edits, source changes during checks, malformed/oversized input, and Windows/CJIT build; test conventional compiler separately before claiming support.

Acceptance: same source identities/declared links as fixture and Attendant; invalid data fails predictably without rewriting source or pretending failed loads are empty designs.

### Slice 3 — deterministic semantics and control exports

- [ ] Add sparse matrix/case inspection, accepted-pair connectivity, skill prerequisites/ranks/route-opportunity checks, MDA declared traces, scoped gaps, and shared minimal rank-update operation.
- [ ] Implement control grammar/units/types, dependency resolution, temporary overrides, expectation inspection, and CSV/INI/JSON resolved exports.
- [ ] Verify stable planned findings and source locations, explicit exclusions/no-effect/unknown distinctions, all declared routes including revisits, prerequisite cycles, null/zero/false, exact integers, enums, unit/formula errors, and override propagation.
- [ ] Verify rank update changes only selected authored field, preserves body/metadata/descendant intent, allows conflicts, detects stale source, and survives failed writes without data loss. Rerun coordinate resolution fixture.

Acceptance: documented CLI examples and envelope work; completed exploration can contain findings; validation/threshold/runtime exits follow contract; export values agree with common resolver and do not overwrite source.

### Slice 4 — native projections and source navigation

- [ ] Verify raylib build/dependencies, then deliver matrix, prerequisite/rank graph, conceptual progression, focused graph, MDA trace, and diagnostics browser on shared C99 results.
- [ ] Implement selection/expansion, case/source detail, explicit refresh/stale indication, source navigation, and selected rank editing. Do not introduce separate view authority or an editor.
- [ ] Compare every deterministic visible finding/count/rank with CLI JSON; edit Markdown externally and verify refresh. Test stale/invalid source, interrupted save, and movement earlier/later without authored descendant rewrites.

Acceptance: human can inspect multi-case cell and parallel/late-root chains, navigate sources, resolve rank conflict, and see same results as agent. No physical level-layout mapping or HTML duplicate required.

### Slice 5 — explicit one-shot experiments and result views

- [ ] Deliver shared C input/measurement/PCG32 helpers, generated-model instructions, recipe preparation, checkpoint verification/resume, compilation/supervision, batch/sweep/replay, and experiment trace/distribution/sensitivity views.
- [ ] Implement fixture question and PCG golden tests; verify actual seeded distribution against analytical reference without inventing sample counts. Test energy override 2, speed sweep, formula propagation, separate requested expectation analysis, and unusual-trial trace. Verify simulation reports measurements without design verdicts or expectation-based command failure.
- [ ] Test missing approvals/declined commits, dirty/untracked relevant source, unchanged recipe retry, stale recipe, compile/protocol/runtime/timeout/step/output failures, undefined measurements, missing historical dependencies, and replay without current controls or overwritten user files.
- [ ] Verify dated results, tracked reproduction provenance versus ignored rebuildable output, no auto-run on open/preview, CLI/view parity, and new simulation after rule changes. Use temporary Git repositories; never checkpoint this skills repo as a test.

Acceptance: reproduce recorded trials with frozen code/inputs/toolchain, show measurements/spread/percentiles and limits, inspect a selected run, and distinguish simulated predictions from observed play. Separately requested analysis can compare declared expectations; simulation does not decide design meaning. Full workbench scope is delivered across slices; current implementation covers Slice 1 only.

## Acceptance approach

- Bundle a tiny worked example in skill folder, serving as both tutorial and system test.
- Include deliberately seeded gaps and exclusions with expected findings and a resolved variant or repeatable resolution procedure.
- Grow example coverage with delivery slices; final example must demonstrate integrated design data, deterministic analysis, views, and reproducible simulation.
- Show each supported workflow with an actual request and expected artifact or finding.
- Resolve a gap in Markdown and demonstrate corresponding result changes across supported outputs.
- Verify agent interpretations remain distinct from deterministic findings and measured evidence.
- Verify useful review while model remains incomplete.
- At tool delivery, verify direct CLI use without an agent and analysis without Attendant projections.
- At simulation delivery, reproduce a date-stamped result from saved metadata and inspect spread, not only mean.
- Demonstrate multiple conditional outcomes within one matrix cell and exploration of their source relationships.
- Show same skill atom chains by dependency depth and authored order, exposing parallel skillset development.
- Seed highly connected and sparse graph nodes; verify reported connectivity against fixture relationships.
- Retain generated alternatives with disposition and rationale; verify inactive ideas do not act as accepted rules and later generation surfaces relevant prior rejection.
- Resolve alternate terms to canonical records without creating divergent duplicates; test ambiguous aliases.
- Change one canonical definition and verify all dependent views use updated source without manual duplicate edits.
- Change absolute and relative control values; verify resolved CSV/INI/JSON exports and simulation inputs agree.
- Generate a small C99 interaction simulation from supplied prose, recording assumptions and using current controls.
- Demonstrate authorized Git checkpoint and comparison/recovery workflow without a document-level edit log or loss of unrelated changes.

## Implementation checks

- Plan checkpoint: `2a69e10`.
- Step 2: generated 19 schema/usage/body-template sets and 68 tutorial records; `attendant validate --strict` returned zero diagnostics/corrections. Observation collection intentionally empty; analytical timing reference is not executed evidence.
- Vendored unchanged Attendant runner plus YAML dependency and license notices for project-local operation. No new database/query engine.
- Step 3: bootstrap CLI, guarded setup/update, source-preserving queries, term resolution, snapshots, and report-storage module. Windows/Node tests: **8/8 passed**. Report integration remains pending catalog delivery.
- Tests cover empty setup/idempotency, local-only runner, stale approval, migration gate, local edit conflicts, all eight query recipes, non-correcting reads, inactive terms/collisions, bounded output/fields, validation findings, source fingerprints, and path escapes.
- Rejected weakening Attendant authorizer after `json_each` denial. Kept vendor unchanged; matrix selects declared arrays and expands bounded output in helper. No domain analysis duplicated in Node.
- Preserved Attendant `scripts/` and `references/` vendor layout so notice links resolve.
- Step 4: all 18 original attributed packs, narrow entrypoint (769 body words), operation/CLI/model references, direct quickstart, tutorial walkthrough, repository index, and report input template.
- Integrated Node tests: **14/14 passed**, including reports/history/freshness/stable finding identity, explicit source capture, all-lenses rejection, custom role reuse/local schema preservation, oversize sources, and Windows junction rejection.
- Frontmatter passes; **55 local links pass** after fixing one template-directory link. No skipped internal-link failures.
- External URL validator reports **8 HTTP 403 responses** across publisher/academic/model links. Sources retained as canonical citations; source access limitations documented per pack. No full-book/questionnaire/deck inspection claimed.
- Tool snapshot includes tutorial documentation/source but never its installed tools/results, avoiding recursive bundles. Canonical empty collections stay empty; tutorial copy under tools is reference material.
- Owned package text uses LF; vendored bytes remain unmodified. Attribute files accompany installed snapshot so recorded hashes survive checkout.
- Step 4 final regression: **15/15 passed**. Added explicit untyped-scalar coverage and actual temporary Git commit/Windows autocrlf checkout round trip. Found/fixed CRLF false conflicts in managed instructions; package hashes and setup idempotency survive checkout.
- Setup candidate scan now includes root Markdown and runs alongside existing registries; configured collection directories are excluded. Shallow scan depth/exclusions are explicit, not claimed exhaustive.
- Remaining: final example-local snapshot, integrated verification, offline report.

## Comments

- Unrelated untracked `skills/unity-cli/` remains untouched. Starting brief and supplied framework reference remain untracked.
- Specification checks: 8 local links and 2 HTTP(S) URLs passed bundled skill-craft validators; whitespace check reported no errors. These are documentation checks, not workbench execution evidence.
- Implementation authorized after refinement; current work isolated from original worktree. No task-management collection created.
- Provisional engineering choice: first-slice queries reuse Attendant SQL/Node runtime rather than duplicate domain analysis in `.mjs`. C99 analysis belongs to later agreed slice. Cross-check usability during review.
