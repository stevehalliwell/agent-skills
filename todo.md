# Game Design Workbench

Status: Slice 1 delivered; framework guides remain provisional. Native matrix/connectivity, skill/progression drafts/views, focused relationship exploration, diagnostics browser, MDA CLI/explorer and typed control resolution/export/import planning implemented and checked. One historical crate timing baseline retains 1,000 trials and frozen evidence; no rerun during Review 01. Actor/state Review 01 is implemented and verified through full live-example migration, ready for review. Tasks 02–11 still need refinement and delivery; final integrated/manual review remains. User accepts explorer as starting point; usability review deferred. Review 01 checkpoint commit authorized; branch/worktree remains local. No push authorized.

Implementation context:

- Branch: `feat/game-design-workbench`.
- Worktree: `D:/Repos/agent-skills-game-design-workbench`.
- Base: `99f0879c316377faf162841362310dc8b024b1e0`.
- Scope: agent skill, 18 provisional guides, collection templates, populated crate-puzzle example, authoring checks, and first native matrix/connectivity CLI plus read-only matrix view. Use Attendant/Git directly; no generic/query/report lifecycle layer. No simulation execution claimed.
- Step commits: (1) plan/spec baseline; (2) model/templates/example; (3) local setup/query/report tools; (4) researched packs/entrypoint/docs; (5) integrated verification/corrections; (6) offline review report.
- Update evidence here; stage/commit only authorized paths when explicitly approved. No merge/push. Original worktree's unrelated files remain untouched.

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

- **New repository:** install and configure supported table contracts and required domain-tool sources so designer can enter information in any domain without another collection-setup step. Empty collections are valid; do not require records in every table.
- Guide designer to provide core elements; progressively flesh them out rather than requiring a complete design upfront.
- **Existing workbench project:** inspect existing design data and guidance, recover current state, and continue refining, exploring, or elaborating the design. Do not restart setup or repeat settled questions.

Use installed skill, existing Attendant operations, and ordinary Git. Copy wanted guidance/templates or actual domain-tool source only when needed; agent merges updates with project-specific content and reviews diff. Git records reviewed versions. No workbench installer, manifest, or hash inventory.

Setup/update/resume contract:

- Present concrete paths/aliases and changes; honor Attendant confirmation/migration workflows. Read existing contracts and preserve incompatible source until resolved.
- Current model has 15 role contracts with schema/usage/body template, including typed axes. Empty collections are valid. Setup creates no tutorial facts; migrate existing records deliberately.
- Registry owns collection discovery; project instructions identify concept owner and non-default role aliases. No parallel manifest.
- Agent merges useful upstream changes, preserving local edits/unknown files. Migration/deletion is not an implicit update. Inspect actual partial state and Git diff after failure.
- Resume reads project instructions, schemas/usages, concept/mode, relevant records/evidence. No automatic lens sweep, setup, installation, or execution.
- Retired local tool copies need reconciliation before removal. Preserve unique content and historical evidence.

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
- Explicit lens/framework requests return sourced findings and coverage. Save dated reports when requested; routine consistency/impact checks do not create a report automatically.

### Per-operation discipline

These are safeguards within an operation, not a linear design process.

1. Establish requested operation and scope from user request and existing project data.
2. Inspect relevant collections and their guidance. Read linked context needed for the question.
3. Return useful work: a proposed design addition, sourced findings, impact analysis, or experiment result.
4. Resolve material choices with designer. Do not silently turn suggestions into accepted design.
5. Persist authorized changes in source records and link relevant evidence or rationale.
6. Rerun affected consistency checks and any lens-based checks included in requested operation. Report what changed, what remains unknown, and what evidence supports the result.

A review request must not silently become a repair operation. Explicit design input authorizes direct capture. Generated alternatives remain proposals; persist only when wanted. Do not adopt them or change accepted intent without designer direction.

### Alternatives and design memory

- User owns alternative retention: keep wanted proposals in relevant domain records or existing project-level decisions, or leave discussion unpersisted. No workbench decisions collection or mandatory archive.
- Adoption writes selected facts into canonical domain records. Preserve supplied rationale when retaining an option; do not create competing rule definitions.
- Status is free text, default proposed; standard query recipes select accepted definitions. Common meanings: proposed = not adopted; accepted = current design; rejected = explicitly ruled out; superseded = replaced by linked design content; defunct = no longer applicable. Record disposition rationale when supplied; do not invent a reason.
- Treat lifecycle as current state, not an edit log. Ordinary edits to a record rely on Git history; they do not require making a superseded copy of every previous version.
- User direction authorizes adoption, rejection, replacement, retirement, or reopening. Matching an alias on inactive content must expose its disposition, not silently reactivate it.
- Acceptance means adopted design, not empirically proven behavior. Keep intention, assumption, prediction, and observation distinct from lifecycle.
- Selecting an alternative makes it accepted but does not automatically reject its siblings. Unchosen alternatives remain proposed until designer explicitly assigns another disposition.
- Users may retain ignored alternatives under rejected, superseded, defunct, inactive, disabled, or other statuses, or request deletion. Check affected references before authorized deletion; no forced retention.
- Proposed and inactive alternatives do not count as active rules or silently create contradictions in accepted-design analysis. Permit explicit exploration/review of alternatives.
- Consult relevant retained alternatives when generating or revisiting design choices. Surface prior rejection and its reason; do not present a ruled-out idea as new.
- Retained alternatives describe current disposition and useful rationale. Use Git for edit history rather than maintaining a chronological transcript in each record.

### Outputs by operation

- **Setup:** approved empty collections/contracts and concept/navigation. Create connected records only from explicitly supplied capture, not setup itself.
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
- Skill-atom prerequisites define the order in which skill atoms must be acquired. Derived prerequisite depth does not determine when a skill appears in game progression.
- Track conceptual first-required placement in nullable integer `rank`. Keep separate from acquisition prerequisites, prerequisite depth, and physical level layout.
- Root skill atoms need not appear or be acquired in first level. Show late-appearing roots and parallel chains at their intended progression points.
- Default skill-atom layout is prerequisite-directed, not unconstrained force layout with arrowheads. Use top-to-bottom chain direction by default, with left-to-right alternative: prerequisite nodes precede every descendant along chosen axis. Dynamic settling respects this ordering while arranging parallel branches and merges. Arrows point from prerequisite to dependent skill.
- Lay out all skill chains through their prerequisites. User can additionally request rank-ordered layout to show chains relative to designed progression. Directional display constraints do not assign or rewrite authored rank; prerequisite depth remains separate from designed progression. Prerequisite cycles cannot satisfy chain ordering: expose conflicting cycle rather than remove links or imply valid ordering.
- User can edit skill rank and prerequisite connections in memory; recompute descendant display positions without overwriting their authored ranks. Show conflicts rather than fix them, visually and in JSON. Viewer reports proposed edits for agent/Attendant handoff; it does not write authoritative records. See [Rank edit behavior](#rank-edit-behavior).
- Keep prerequisite order and designed progression placement distinct; make prerequisite/placement conflicts visible.
- Challenges explicitly distinguish introducing, practising, requiring, and demonstrating mastery of skill atoms. A challenge may serve multiple roles; appearance alone is not evidence of learning, and declared roles do not prove observed understanding.
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
- Example: `enemy-move-speed` uses expression `${control-vars/player-move-speed} * 0.75`. Preserve authored relationship; changes to referenced controls propagate to resolved values.
- Detect missing references and dependency cycles rather than inventing values or evaluating indefinitely.
- Export current control values to CSV, INI, JSON, and potentially other formats. Exports and simulations can consume resolved numeric values without losing authoritative formulas in source records.
- Import closes the tuning loop: export current controls, tweak values in-game, then deterministically update stored values for existing controls from the edited export. LLM may assist initial setup; re-import must not depend on LLM interpretation of changed values. Scope is value updates, not importing new controls or changing type/unit/constraint definitions. Validate and plan updates deterministically; apply authorized changes through public Attendant, never a native Markdown writer. Preserve formula definitions during import. Apply literal-control updates and recompute dependencies; flag direct derived-value tweaks that do not match the recomputed result rather than replacing formulas or guessing upstream edits. Support re-import of the same CSV/INI/JSON dialects used by exports. Compare imported values only with what a fresh export of current Markdown would generate; do not require a previous export or companion baseline snapshot. Imported values differing from current literals are proposed updates; unchanged values are no-ops. This intentionally cannot distinguish stale exports from intervening design edits. Show replacements before authorized application. Validate the complete candidate batch before any writes; unknown keys, type/unit/constraint failures, or unmatched formula values block application. Preserve omitted controls and all definition metadata. Explicit adoption uses public Attendant and verifies source has not changed since preview.
- Controls declare units; dimensionless controls are explicit. Validate dimensional consistency: compatible dimensions for addition/subtraction and derived dimensions for multiplication/division.
- Initial numeric representation follows Attendant/JSON numeric interoperability: binary64 continuous values and integers limited to exact safe range `-(2^53 - 1)` through `2^53 - 1`. Reject overflow/inexact integer storage rather than silently round. Integer-valued expressions still use binary64; integral floating results do not prove exact authored-decimal/rational mathematics. Larger numeric domains require an explicit later representation change.
- Do not silently convert compatible unit scales such as seconds and milliseconds; require explicitly compatible scales. Unit mismatches produce actionable errors.
- Formula constants may carry explicit units, e.g. `${control-vars/base-cooldown} + 0.2[s]`. Plain numbers are dimensionless; do not infer units from addition context. Separately tunable offsets may instead be control references.
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
- When storage is requested, save a new dated report with scope, time, and source-state context. Do not overwrite earlier results or mandate duplicate formats.
- Repeated warnings may appear in successive reports. Source changes make older reports historical; do not imply an old result was revalidated.
- Normal gap exploration succeeds when analysis completes, even with findings. Invalid input/runtime failures still fail. Explicit severity thresholds enable nonzero exits for automation; validation fails on broken data. Review/validation does not automatically fix source; use nonmutating Attendant validation when reviewing.
- [Reports and provenance](#reports-and-provenance) defines timestamps, source-state capture, and historical presentation. Findings stay in run reports unless designer requests a source question/decision/hypothesis; do not create an issue record for every warning. Generated alternatives follow user-owned retention policy.
- Recorded design intent and rationale remain useful even when a warning repeats; do not confuse repetition with a new design defect.

### Representative use cases

- **Crate interaction:** player can push crates, but pushing a crate against a wall has no defined outcome. Matrix shows an unresolved cell, not an automatic error. Review points to the missing consequence and asks whether movement is blocked, altered, or intentionally undefined.
- **Skill scaffolding:** a challenge requires crate planning, but declared prior progression establishes only movement and pushing. Tool reports the unsupported prerequisite with challenge and skill sources. It does not claim the player cannot discover the skill independently.
- **Experience support:** design intends tension, but no documented dynamic supports it. Report an unsupported design hypothesis, not proof that play lacks tension.
- **Progression experiment:** target names rank and time bound. Simulation reports time-to-rank distribution under stated assumptions. Separately requested analysis compares distribution with criterion; designer/agent interprets design significance. Missing assumptions block supported conclusions, not invite guessed player behavior.
- **Change impact:** removing pushing exposes dependent interactions, skill atoms, challenges, and experiments. Separate known dependency reach from speculative experiential effects.

## Capability boundaries

- Workbench manages design knowledge and evidence, not game runtime implementation.
- No engine, game scripting language, document editor, generic issue tracker, or replacement for Attendant.
- Do not require a simulation for every mechanic or intended experience.
- Setup installs all supported table sets, but populating every collection or linking every domain is not mandatory. [Table responsibilities](#table-responsibilities) owns initial inventory; Do not add a competing intent collection; use concept ownership specified below. Additional framework collections require a distinct fact/ownership need.
- Preserve mechanics/system/dynamic distinctions and intention/prediction/observation distinctions.
- Visualizations expose existing analysis; they do not introduce separate rules or authoritative state.
- Narrow automatic routing: activate for explicit game-design workbench, structured design-data, noun–verb matrix, skill-chain/progression, framework-review, and design-simulation requests. Skip ordinary game-code-only work and unrelated brainstorming, research, or technical review. Loading skill does not authorize setup, a lens review, or simulation execution beyond requested operation.
- Neighboring skills retain their responsibilities: brainstorming can generate ideas without setting up a project; coding/implementation handle game runtime code; technical review handles architecture. Workbench captures authorized design facts and proposals when requested, not every adjacent conversation.

## Refinement status

Control-variable import is an export/game-tweaking/re-import loop for deterministic updates to existing controls. Preserve formulas and flag unmatched derived-value tweaks; use existing CSV/INI/JSON export dialects. Import compares against a fresh current export, without a historical baseline or companion snapshot. Treat differences as proposed updates; preserve omitted controls/definitions. Full-batch validation and source freshness protect authorized adoption. Implement shared resolver/export/import planning before public-Attendant application. Other contracts below cover operations, source ownership, framework scope, partial data, aliases, progression, control variables, reports, views, and simulation checkpoint flow. First-slice walkthroughs and delivery plan define reviewable outcomes.

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

Current ownership, lifecycle, and partial-state contract: [model reference](skills/game-design-workbench/workflows/model.md). Distribution field/usage/template authority: [shipped templates](skills/game-design-workbench/templates/), edited directly. Live project schema/usage remains authoritative after setup. Do not keep another field dictionary in this task record.

Model version 2 consolidates investigation data into experiments. Future executable criteria encoding needs refinement as experiment work proceeds. Earlier outline/research files are planning inputs, not current field contracts.

New collections require distinct source ownership and an example showing why existing records are insufficient.

### Framework pack contracts

All 18 packs ship as attributed, independently authored guidance in first slice, with necessary collection templates and query helpers. Reuse Attendant queries or direct record inspection where sufficient; add C99 helpers when needed and justify `.mjs` exceptions. Each pack declares scope, useful source records, original questions, available queries, evidence limits, and missing-data behavior. Empty optional tables do not block conceptual discussion; reports identify unavailable data/checks. Do not add one collection per theory or require framework labels on every record.

Each [framework guide](skills/game-design-workbench/frameworks/) owns its scope, relevant inputs/inspection topics, questions, and source limits. Edit guides directly; catalog is discovery index, not an executable recipe or JavaScript content mirror.

Section E perspectives are cross-cutting prompts attached to relevant packs, not extra mandatory datasets or an all-perspectives command. Framework-specific tools must name their actual query and required records; prefer shared C99 operations. Justify any dedicated `.mjs` query and document Node requirement. A question-only pack is usable without pretending its interpretive review is implemented in C.

### Rank edit behavior

First editing increment:

- Edit skill `rank` and prerequisite connections between loaded skills in memory. No general Markdown/body editor or unrelated field editing in this increment.
- Recompute graph, placement, and findings from draft edits. Preserve authored ranks/prerequisites of records not directly edited; changed derived positions do not make descendants authored edits. Keep cycles/conflicts visible rather than silently repair them.
- Show when draft rank/prerequisite values differ from last loaded baseline. Track modified records and overall modified state; returning rank and prerequisite set to baseline clears that record's modification flag.
- Keep in-memory modification separate from externally changed source. Existing polling/stale indication continues; never silently discard modified drafts on refresh or close.
- [x] Support Ctrl+Z undo back to loaded disk sync point before overall task ends. Implemented now: rank/prerequisite edit history preserves sync point; refuses edits at 4,096 actions rather than evict history. Undo restores authored draft values, raw declarations at sync point, and derived state without writing sources. Focused text input keeps local text undo; Undo button always handles draft edits.
- Emit a JSON change report for handoff: these are proposed edits, not applied source changes. Include record reference/source path and only user-edited fields, with loaded baseline, current-disk, and edited values. Support both rank and prerequisite changes; mark `applied: false`. Do not report unrelated external changes as user edits.
- Re-read current disk values when reporting; preserve loaded baseline and draft. Expose source changes or unavailable current values with explicit errors/coverage rather than imply a safe merge. Distinguish unavailable values from authored null/empty prerequisites. A report is not a lock or permission to overwrite; applying actor rechecks current source.
- Finish/close editing session by returning one JSON result on stdout; diagnostics stay on stderr. Empty successful report has `changes: []`. No per-edit stream or automatic source application.
- Optional explicit `--output <file>` routes that JSON result to a UTF-8 report file instead of stdout. Never silently overwrite an existing file or write authoritative input records; report-output failure must not silently lose draft. Installed on skill view; reject non-JSON report destinations and keep interactive draft available on output failure.
- No authoritative source mutation in viewer. Agent can decide how to apply report, including Attendant updates or precise source edits. Viewer does not need a source serializer/patch writer. Revert-to-disk interaction remains deferred; no automatic merge, discard, or save to authoritative records. Undo to baseline then clean F5 is available; one-click discard/reload remains deferred.

- Graph supports prerequisite layout and conceptual rank layout. `rank` remains the field/CLI term; accept null or JSON-safe signed integer, including zero. Numeric rank is conceptual first-required placement, not prerequisite depth or a physical level number. Unranked atoms retain null authored rank even when graph constraints display them at a minimum position; distinguish authored rank from effective placement.
- Changing B from rank 2 to 4 changes only B's authored rank. Recompute downstream display constraints; upstream prerequisites and every descendant's authored rank stay unchanged.
- If C requires B and has authored rank 5, display at 5 or later. If C's rank is null, keep null and display at 4 or later. If C has authored rank 3, keep 3, display at 4 or later, and report conflict between intended rank and prerequisite constraint.
- Recompute display placement when B moves earlier too; never overwrite descendant intent as a layout side effect. Same-rank prerequisites remain allowed; prerequisite ordering still exists within coarse rank.
- Return proposed field changes separately from affected graph nodes, authored ranks, derived minimum/display positions, and consistency findings. Show issues visually and in JSON, not automatic corrections. Shared C99 computation handles draft analysis; CLI source analysis still describes disk, not an unreported viewer draft.
- Valid draft edits may expose conflicts; do not block them or silently repair prerequisites/challenges. Applying the handoff is a separate agent/Attendant action, limited to authorized rank/prerequisite changes while preserving bodies, IDs, unrelated metadata, and other records' authored values.
- Detect external source changes and expose them alongside draft edits rather than overwrite or silently merge. No native authoritative rank-write command or shared CLI/UI disk writer is required for this editing increment.

### Scope, impact, and coverage

- A review scope selects finding focus, not permission to ignore external prerequisites or prior routes. Read supporting records outside focus when needed; report focus and inspected support separately. Guarantees concern declared graph/routes only, never every possible play path.
- Matrix scope uses selected nouns/verbs and canonical cases for those pairs. Challenge context can identify candidate rules through declared links; prose conditions are not automatically solved to prove availability. Represent cases sparsely; do not allocate a complete noun × verb product merely to show unknown cells.
- Impact reports direct/transitive declared links with paths and relationship kinds. Distinguish definitional/formula/prerequisite use from contextual association; graph reach does not establish causal experience changes. Broken direct references are deterministic; speculative downstream effects remain questions.
- Partial source is valid when optional data is absent. Wrong types, unresolved supplied references, duplicate identities/terms, and malformed source remain data findings. Load usable records where safe and mark omitted/invalid scope; never turn a failed load into an apparently empty design.

### Reports and provenance

- Give each run a UTC timestamp and collision-safe suffix. Directory form: `YYYYMMDDTHHMMSSmmmZ-<suffix>`; metadata date/time uses RFC3339 UTC. Never overwrite an existing run directory.
- Run directory keeps reproduction/source-state metadata separate from bulk derived artifacts. Save readable findings with evidence links; add structured data when useful or required by a consumer. Place rebuildable matrices/traces/plots/binaries under ignored `artifacts/`; retain metadata/recipes and non-rebuildable agent/playtest evidence. Do not apply an ignore rule that hides all authored reports. Supporting queries within one requested review need not create separate review runs.
- Source records live in configured collections; requested reports/results stay outside them under `game-design-results/`. Track recipes, simulation source, and non-rebuildable evidence; ignore rebuildable artifacts.
- Record lens/framework, scope, cited paths/fields, source commit, relevant drafts/untracked paths, and selected intent comparison commit/path. Use recognizable labels and stable record IDs. No automatic fingerprints, frozen review snapshot, or generated-ID service.
- Reports distinguish completed checks, unsupported/unperformed checks, missing data, and incomplete scope. No findings is not completeness evidence.
- Each finding includes stable rule code/identity, kind, severity, evidence class, involved records, source field/section, trigger/evidence, consequence/question, and suggested investigation.
- Chat or Markdown suffices for agent reviews. Structured findings are optional when useful, required only by an actual consumer. C tools do not claim agent-only interpretation.
- Reopen reports as historical. Inspect relevant Git changes/current source before claiming applicability; never label old reports newly run. State reconstruction limits for uncommitted reviewed text.
- Explicitly requested multiple frameworks may be compared; preserve disagreements and evidence limits. No implicit all-framework execution or aggregate design score.

### Controls and experiments

- Formula syntax: decimal/scientific numeric literals, qualified references `{control-vars/record-name}`, unary `+`/`-`, binary `+`/`-`/`*`/`/`, and parentheses. Use standard arithmetic precedence; references resolve stable control keys, not glossary aliases.
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
- Do not keep old simulations synchronized with later rules. Results name experiment-local measures; viewers use their recorded type/unit/boundary definitions.
- Simulation request establishes model question and material assumptions; missing target permits descriptive exploration, not an invented pass criterion.
- Model/player-behavior assumptions are experiment-local by default, explicitly recorded with question inputs. Do not mix assumed action delays, policies, or success rates into exported game tuning. Shared reuse requires deliberate promotion into appropriate design data.
- Bound runs, steps, and elapsed execution; persist partial/error/limit status rather than label a stopped run successful. Incomplete evidence cannot establish a satisfied target.
- Initial defaults: 1000 batch runs, at most 1,000,000 steps per run, 30-second execution timeout. Expose positive explicit overrides; record effective limits. Compile/startup failures are distinct from simulated outcomes; verify these defaults against fixture workload before claiming usability.
- Seed random runs through a specified shared generator, not compiler-specific `rand()`. Save generator/model version, seed, effective controls/overrides, and checkpoint commit. Historical reproduction loads recorded source/inputs; a new answer about current rules gets a new simulation.
- Agent prepares an ordinary tracked recipe identifying code/dependencies, inputs/overrides, assumptions, seed, trials, and limits. Preparation does not execute simulation or require a lifecycle program.
- Agent/human obtains approval under project policy and commits with Git. Reuse committed recipe for retries; do not rewrite timestamps/inputs into a dirty-state loop.
- Agent inspects Git status/diff and checkpoints reviewed recipe/code/inputs under project policy. Execute from exact committed state, using ordinary checkout/worktree if needed. Identify external inputs/dependencies. No custom Git/checkpoint/hash subsystem. Do not force-add private/ignored files; changed inputs need a new reviewed checkpoint. Declined approval blocks execution.
- Record actual checkpoint commit in run result metadata after checkpoint; do not rewrite the committed recipe to contain its own future commit hash. A prepared recipe is not an executed result. Opening it or viewing results does not authorize execution.
- Support stepping, batch runs, distributions/percentiles, numeric parameter sweeps, and selection of an unusual run. Sweep `start:end:step` uses inclusive start and includes end only when reached; reject zero/wrong-direction step, non-finite values, or nonintegral points for integer controls. Compute points from index rather than cumulative addition; record effective points in prepared recipe. Initial maximum is 100 sweep points, overridable explicitly; trial/output/time limits still apply across whole prepared sweep. A deterministic model may have zero spread; do not invent randomness.
- Generated visualizations and headless outputs read same measures/results. A simulated prediction remains conditional on stated assumptions, not observed player behavior.

### Simulation program and measurement contract

- Run prepared model as a separate local process, not a dynamically loaded viewer plugin. Generated C99 program accepts `--input <frozen-input.json>` and emits versioned NDJSON on stdout; diagnostics go to stderr. Shared C helpers handle typed inputs, PCG32, step accounting, and event encoding. Conventional compiler and CJIT builds use same helper sources.
- Inputs contain recipe/experiment identity, effective typed controls with units, experiment-local assumptions, frozen measure/expectation definitions, trial count, seed, and limits. Model reads this snapshot, not current design files. Record source commit, code/helper/dependency versions, and actual build invocation/toolchain in provenance. No arbitrary expression or shell evaluation.
- Output begins with a header identifying protocol/model/recipe, emits trial results and optional step records, and ends with explicit completion/count/status. Each trial has configuration ID, stable zero-based trial index, and measurements keyed by canonical measure identity. Combined sweep trial identity is configuration ID plus trial index; repeated indices across distinct configurations are valid. Step records name trial, monotonically increasing step, and inspectable state/measurements. Validate identities, declared measurement types/units, duplicate indices, non-finite values, and missing/invalid completion; malformed output is not successful simulation.
- Default stores per-trial results; detailed trace is requested for selected trial(s). Viewer may step through saved trace without executing code. New trace generation or historical resimulation is an explicit execution with checkpoint/limits, not an automatic effect of opening a chart.
- Use PCG32 XSH-RR with explicit state initialization, algorithm/version recorded as `pcg32-xsh-rr-v1`. Master seed is uint64, serialized losslessly as a hexadecimal string; trial index selects stream with same master seed. Corresponding sweep trials reuse streams, with divergent model paths potentially consuming different draws. Uniform helper uses `ldexp(draw, -32)` on [0,1); bounded integers use rejection, not biased modulo. No compiler-specific `rand()` or cryptographic claims.
- [PCG minimal C API](https://www.pcg-random.org/using-pcg-c-basic.html) documents seed/stream repeatability and upstream golden sequence. Verify golden outputs before fixture counts; pin/review source and retain license if vendoring implementation. Prepared helper checks now verify upstream six-draw golden vector under CJIT 1.2.0 and Zig 0.17.0; this is engineering evidence, not experiment execution.
- Compute min/max/mean and nearest-rank p50/p90/p95 over defined numeric samples, reporting valid/undefined counts. For percentile p and N > 0, sorted one-based element is `ceil(p*N)`; N = 0 yields undefined, never zero or NaN. Label conditional summaries when some outcomes have no defined measure.
- A modeled goal not reached can produce explicitly undefined time-to-goal with reason; keep elapsed attempted time and completion state distinct. Unexpected missing measurements are output errors.
- Simulation summaries report finite seeded measurements, counts, and spread without design judgments or expectation-based verdicts. Completed execution returns exit 0 regardless of what measurements suggest about design. Do not add a simulation failure flag for unmet design expectations.
- Compile separately to ignored build output, then execute under supervisor. Initial compile timeout is 30 seconds; execution uses declared 30-second default and step/trial limits. Cap captured output at 64 MiB by default, with explicit positive override. Exceeding a limit stops execution, preserves available evidence, and reports incomplete status. Verify these engineering defaults against fixture; no unmeasured performance claim.
- Separate process and watchdog are not a security sandbox. Native C runs with user's permissions. Inspect generated code before execution; do not run untrusted repository code merely because a recipe names it. Generated models must keep I/O to declared inputs/results and avoid network, shell execution, or unrelated filesystem changes. Unsafe code needs resolution, not implied permission from a simulation request.
- Replay uses frozen code/helper/input versions in an isolated working directory without overwriting current design or checking out over user changes. Missing historical dependencies are actionable reproduction failures, not permission to substitute current controls/code. Each execution gets new dated result identity linked to original recipe/run.

### Agent-led experimentation and tuning

- Agent takes resolved control vars, builds an experiment for the requested question, runs it under approved execution/checkpoint policy, and uses results to propose control adjustments. No general `gd expectations` command or native consumer of experiment prose. No required experiment schema migration for expectation inspection.
- Experiment setup may expose temporary control tweaks for interactive refinement or implement an explicit algorithmic search. Keep objective, adjustable controls, bounds, assumptions, and stopping conditions explicit when needed. Reuse shared control resolution; do not duplicate formula evaluation. Candidate tweaks remain experiment-local until authorized adoption through the control import/public-Attendant boundary.
- Agent interprets results and declared criteria. Experiment-specific code may compute comparisons/sample fractions or drive refinement; this is not a general CLI expectation engine. Do not prove underlying player success probability or judge design quality from a simulation alone. No mandatory confidence-interval model or inferred statistical significance.
- Per-trial comparisons use declared type/unit: numeric order/range, boolean/enum equality. Aggregate numeric expectations do not coerce booleans/enums. Equality is exact; tolerance requires explicit range. A minimum pass rate uses all valid attempted trials as denominator, including modeled failure outcomes; do not drop incomplete routes to improve measured fraction.
- Undefined time-to-goal never counts as satisfying a completion-time criterion. Incomplete/timeout/runtime-error evidence cannot establish satisfied criteria. Aggregate comparisons with undefined inputs remain indeterminate unless experiment explicitly defines eligible population; never silently average away failures. Mathematical comparison is not a design verdict and does not change simulation execution status.

### Headless and viewer boundary

- C99 source loading, transient graph, and game-specific analysis are shared by headless tools and raylib views. Attendant owns generic record operations, collection/configuration discovery, search/query, schema interpretation/validation, reference hygiene, and projection lifecycle. Do not reproduce those services merely to remove its Node dependency.
- Direct reading may load Markdown from supplied collection folders, identify records by the established collection/name convention, and resolve references in relevant YAML fields into an in-memory graph. This is input preparation for domain analysis/views, not a second record platform. Keep relationship field identity, ambiguous/missing targets, source paths, and incomplete coverage explicit. Do not infer edges from arbitrary prose or every string resembling a reference.
- Check only safe parsing and inputs consumed by the domain operation. Loader failures mean the tool could not safely interpret its input; they are not a substitute for Attendant's project/schema validation. No generic schema engine, query language, CRUD API, automatic repair, or persistent projection.
- Watchpoint: if native loading/graph work starts reproducing substantial Attendant functionality, stop expansion and record the overlap. Consider an Attendant C99 port/shared native foundation as separate future work; do not build or approve it implicitly within this project.
- C99 headless core does not require raylib, a display, an agent, Node.js, or Attendant projection. Generic Attendant operations and any justified `.mjs` framework query retain their documented Node runtime dependency. Mark command-specific dependencies; do not silently make Node mandatory for unrelated C99 operations.
- Required native projections across full delivery: matrix; prerequisite/rank skill graph; conceptual challenge/skill-role progression with optional level groups; focused relationship graph; MDA trace; experiment traces/distributions/sensitivity; diagnostics browser. No required HTML counterpart or physical level map.
- Matrix expands multiple cases, conditions, outcomes, dispositions, context links, and source navigation. Focused graph starts at selected record/case and exposes adjacent links with explicit expansion. Reuse same relationship data; do not reimplement analysis in renderer. Connectivity sorting includes zero-degree nodes and case counts without a quality cutoff.
- Select skill node, edit rank/clear to null and prerequisite connections in memory, then report proposed changes as JSON for agent handoff. Display authored versus derived position, linked findings, and modified-versus-disk state. No viewer writes to authoritative Markdown. Other design authoring remains Markdown/agent-driven; transient experiment inputs and generated results are not baseline-control edits.
- Findings link to record path and field/section. Open through explicit user-selected editor or safe document association; otherwise show/copy path. Never execute a linked script/program as source navigation.
- Skill graph supports rank editing as specified. Other views are exploratory initially.
- Direct-source and structured-output parity applies to every deterministic visible finding; agent-only interpretations must be saved as structured findings before a viewer can display them.
- Viewer refresh must detect externally edited Markdown; keep last-good results clearly stale if current source cannot load. Do not save against a stale model.
- Copy actual domain-tool sources/licenses/dependency information when needed. Agent merges explicit updates with local versions and reviews diff. No generic bundle/update program or manifest. Resolve execution paths explicitly.

### Future domain-tool CLI contract requirements

Only computation/visualization belongs in executable tools. Setup, merge updates, framework interpretation, report writing, and Git checkpoints remain agent workflows. Examples below are planned, not installed interfaces.

- Domain commands cover game-specific matrix/graph analysis, skill placement, connectivity, declared impact, control-variable resolution/import/export, simulations/sweeps, and views. Experiment creation, criterion interpretation, and iterative tuning belong to the agent and experiment-specific setup, not a general expectation CLI. Parsing safeguards and consumed-field checks support those operations; generic record inspection, schema validation, reference hygiene, and queries remain Attendant's job. Agent reads catalog/guides and interprets framework findings; no general framework evaluator or report lifecycle command.
- stdout contains result data; diagnostics/progress/display tables use stderr. Explicit output format wins; piped/agent paths emit JSON without interactive prompts. Support field selection for large structured results.
- Version structured schema; include data, provenance/coverage, and structured errors. Empty successful results are empty data, not failures.
- Completed exploratory gaps succeed with findings. Explicit failure threshold can fail automation; invalid input/runtime failure and broken-data validation fail nonzero. Preserve partial results with explicit incomplete/error status where safe.
- Mutations report changed source paths. Missing required approval in noninteractive use fails with actionable message; noninteractive mode never implies commit consent.
- Use `gd` command name through project-local Windows launcher, with explicit `--project <path>` when not running from target project. Human instructions show launcher path rather than assume global installation.
- Common flags: `--json`, `--format <format>`, `--fields <comma-separated-fields>`, and `--output <path>` where supported. Explicit incompatible format flags fail rather than silently choose one. `GD_AGENT=1` selects noninteractive JSON mode.
- Exit codes: `0` completed success; `1` validation/finding threshold failed; `2` invalid input, missing prerequisite/approval, or runtime failure. Include partial data with explicit incomplete status when safe.
- Agent verifies checkpoint/execution location using ordinary Git before simulation/sweep. Domain executable consumes declared committed inputs and enforces numerical/resource limits, not a custom Git/approval lifecycle.

Representative commands (planned contract, not installed commands):

```text
# Generic record inspection/validation uses Attendant, not a parallel gd command.
gd matrix --json
gd connectivity --json
gd skill-atoms --skill-atoms skill-atoms=game-design/skill-atoms --json
# Skill edits are in-memory viewer drafts with JSON handoff; no native set-rank disk writer.
gd impact mechanics/crate-pushing --json
gd controls --json
gd controls export --format csv --output controls.csv
gd controls export --format ini --output controls.ini
gd controls export --format json --output controls.json
# Agent builds experiments from resolved controls and interprets results; no gd expectations command.
# Agent prepares recipe/code/resolved inputs, reviews diff, and checkpoints with Git.
# Execute from committed state; exact flags remain subject to delivery refinement.
gd simulate --input <committed-input.json>
gd sweep --input <committed-sweep-input.json>
gd replay <run-id>
gd view matrix
gd view skill-atoms --skill-atoms skill-atoms=game-design/skill-atoms --layout rank
# Planned draft handoff: one JSON result on finish; optional report file instead of stdout.
gd view skill-atoms --skill-atoms skill-atoms=game-design/skill-atoms --layout rank --json
gd view skill-atoms --skill-atoms skill-atoms=game-design/skill-atoms --layout rank --json --output changes.json
```

Framework IDs and any rule-specific flags are documented in bundled catalog; no `--all` framework selector. Prose review findings are agent-generated report data, not a claim that C performs every framework interpretation.

Structured envelope version 1:

```json
{
  "schema_version": 1,
  "ok": true,
  "data": {
    "applied": false,
    "changes": [{"reference": "skill-atoms/b", "source_path": "game-design/skill-atoms/b.md", "current_disk_available": true, "fields": {"rank": {"loaded": 2, "current_disk": 2, "edited": 4}}}],
    "affected": [{"record": "skill-atoms/c", "authored_rank": 3, "minimum_rank": 4, "display_rank": 4}],
    "findings": [{
      "code": "skill.rank_conflict",
      "origin": "deterministic",
      "kind": "conflicting_claims",
      "severity": "warning",
      "records": ["skill-atoms/b", "skill-atoms/c"],
      "sources": [{"path": "game-design/skill-atoms/c.md", "field": "rank"}],
      "reason": "Authored rank 3 precedes prerequisite constraint 4.",
      "suggested_investigation": "Review intended first requirement or prerequisite relationship."
    }]
  },
  "coverage": {"status": "complete", "operation": "skill-atoms.draft-changes"},
  "errors": []
}
```

Example shows proposed shape, not installed output or executed result. Read/report commands include provenance metadata; draft change report distinguishes proposed authored edits from affected display nodes and never claims source was written. Empty results preserve envelope with empty data collections. Failures set `ok: false`, nonzero exit, and stable `errors` entries containing code/message and source context. Data field selection never hides required error/coverage metadata.

## First-slice walkthroughs and acceptance

First slice delivers usable guided work, not a claim that later native tools exist. Record installed capabilities and use Attendant for available generic queries/validation. Preserve evidence origin when agent inspects relationships manually.

1. **Setup:** request “Set up game-design workbench here.” Return combined file/collection/dependency proposal. After approval, follow Attendant collection workflow and author approved contracts; no local runtime snapshot. Existing Markdown and custom configuration remain unchanged outside approved plan; no tutorial facts appear in live design.
2. **Direct capture:** supply “Crates can be pushed into free cells; wall-blocked pushes cost no energy.” Resolve declared box/shove aliases if used; capture canonical case/mechanic facts as accepted. Do not invent an energy amount, interview for experience first, or run a lens review. Return changed paths and unresolved material questions only.
3. **Generation:** request “Suggest alternatives to make blocked pushes interesting.” Keep ideas proposed; persist wanted ideas in domain records or existing project decisions according to user choice. Surface rejected pulling rationale if relevant. Choosing one adopts only selected facts; siblings remain proposed and rules retain one canonical home.
4. **Scoped review:** request “Review combined-room scaffolding using skill atoms.” Load that pack and relevant challenge/prerequisite/route context; return sourced findings; save dated Markdown/structured output when requested. Report missing planning opportunity, route-dependent switch support, single-shot roles, and rank conflict only to extent established by inspected data. No automatic Flow or other-lens review; no silent repair or invented player incapability.
5. **Change impact:** request “What would removing pushing affect?” Trace declared references to cases, skill atoms, challenges, and experiments. Return paths/link kinds and uncertain consequences; do not remove records or reinterpret historical simulations.
6. **Resolve:** explicitly change `coordinate` authored rank from 2 to 3 in baseline example. Rerun same skill review; rank conflict clears while missing/route-dependent learning questions remain. First slice may perform authorized source edit through Attendant; later GUI/CLI rank operation must produce same source outcome.
7. **Other packs:** exercise one explicit selected-pack request per pack against example or relevant partial source. Original questions and limitations remain useful with empty observations. Bartle can report limited applicability; Flow/cognitive/motivation packs do not demand cohort records. No all-lenses review command or aggregate score.
8. **Simulation handoff:** first slice documents question/control/assumption/expectation capture and prepare/checkpoint/resume contract. Do not claim executable simulation until its delivery slice passes tests. If an essential first-slice pack query needs a helper, deliver and test that helper rather than label an unavailable operation supported.

First-slice acceptance:

- All 14 current collection schemas/usages/templates validate with Attendant; representative empty, incomplete, accepted, proposed, excluded, and conflicting records have documented meaning. Required pack-specific additions follow same ownership rules.
- All 18 packs have citations, original actionable prompts, declared inputs/queries, evidence limits, and selected-pack walkthrough evidence. Research source materials before authoring; report inaccessible evidence and avoid unsupported claims.
- Use separately installed Attendant/public runner and matching workflows. No vendored files/private imports or runtime wrapper. Maintainer tests may locate dependency through `GD_ATTENDANT_SKILL`; normal use needs no such variable.
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
- Attendant owns its runner, dependencies, and operation guidance as a separate required skill. Workbench calls only public runner and consumes `schema` output; no internal YAML imports or copied Attendant files. Runtime imports `node:sqlite`; documented Node floor is >=24.10.0.
- No concept-alias mechanism was found in inspected runner paths. Workbench must provide terminology semantics on top of compatible fields, not claim Attendant resolves aliases natively.
- If direct frontmatter reading is needed, use a bounded parser such as the probed LibYAML event parser rather than write YAML syntax parsing. Windows/CJIT feasibility probe passed; this does not authorize a workbench schema/configuration engine.
- Native consumers must not silently reinterpret consumed strings/numbers or ambiguous YAML. Report unsupported/unsafe input with source context and operation coverage; leave generic record/schema validity to Attendant. Parser safety is not a claim of implementing Attendant validation.
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
- [x] Current model has 14 schema/usage/body-template sets. Setup/update/resume and relationship inspection use agent instructions, Attendant, and Git. Initial wrapper/manifest retired; no supplemental theory datasets.
- [x] Build bundled crate-and-switch source fixture and retained proposals, glossary, resource/control/formula examples, explicit case classifications, skill/challenge graph, and expected findings. No simulation execution claimed.
- [x] Test setup against empty/preconfigured temporary projects, preservation, approval gates, query/report/rank-edit walkthroughs, partial source, and unsupported-check disclosure. Selected-pack routing/agent authority checked statically; no autonomous-agent benchmark claimed.
- [x] Add identity/reference/alias tests with actual schemas; verify populated copied example and local Attendant rank update without global skill. Validate entrypoint/frontmatter/links and update repository skill index.

Acceptance: [First-slice walkthroughs and acceptance](#first-slice-walkthroughs-and-acceptance). No broad native application or working simulation is implied.

### Concept and working mode — before framework completeness pass

Reference inputs: root `Concept - Template.md` and `High Concept Doc.md`. Read both; retain originals unchanged. Older completed document is reference material, not automatically adopted current design or present-day market/platform evidence.

Confirmed behavior:
- Ship concept template and request existing concept/intention material during initial guided setup. Offer template when absent; incomplete input is valid, especially during exploration.
- Preserve overall intent, intended player, player story/voice, and play across time scales separately from individual MDA A aspects. Cross-check framework interpretations against original intent when available; report support, contradiction/tension, unknown evidence, discovery outside intent, and possible goal drift.
- One current project mode: `game-design` or `game-explorer`. Change explicitly; no per-session/investigation override. Switching preserves original intent, prior findings, and constraints; it does not adopt new goals.
- Commit completed game-concept changes so ordinary Git history preserves recoverable versions. No separate frozen concept copies. Scope commits to intended concept changes; preserve unrelated work. Saved comparisons identify concept path and exact committed revision, rather than a moving HEAD reference. Label uncommitted concept text as a working draft, not as content of that commit; read-only review does not silently commit existing user edits. Blank template is not an adopted intention; if no recoverable intent baseline exists, report that limit instead of inventing one. This does not authorize adopting or committing supplied reference documents as current game facts.
- Game design works toward intended experience for intended player. Game explorer assembles possibilities, investigates generated behavior, and follows promising leads toward potential experiences. Missing destination is not an explorer-mode defect. Emerging directions remain proposed until adopted.

Implemented delivery shape:
- One live concept document, default `game-design/concept.md`, prominently linked from README; no new collections merely for concept sections. Preserve an explicitly designated existing owner rather than creating duplicate live intent.
- Concise template based on supplied outline: razor/purpose, intended player/appeal, platform/context, core features, intended experience/player story, second/minute/hour play, hook, risks, and open questions. Related products/commercial material optional. Detailed rules/tuning and tests belong in existing collections; concept links to canonical definitions.
- Working mode, concept path/revision, and relevant drafts accompany saved review context. Read context before framework interpretation; do not repeat concept content in each guide.

- [x] Ship concept template and shared mode/intent instructions. Tutorial uses explorer mode with authored candidate brief and hypothetical player story, not observed evidence.
- [x] Agent preserves existing owner/body/navigation during setup and explicit header changes. Project instructions record binding; no managed-link installer.
- [x] Use ordinary Git log/show/diff for selected intent comparison and historical citations. No custom context/history/snapshot service. Earlier scripted tests remain historical evidence, not current interfaces.

Current source/workflow contract: [concept and working mode](skills/game-design-workbench/workflows/concept.md). Implementation verification recorded after checks below. Framework guides remain provisional; source acquisition and 14 guide expansions are delivered for review. First native matrix/connectivity slice now implemented; broader native consumers remain.

### Collection cleanup — before further review

- Implemented: rename `controls` to `control-vars`. Own named game configuration values, not only tuning inputs. Documentation and simulations reference names instead of repeating magic numbers. Later exports support experiments, game consumption, and CSV/spreadsheets. Keep runtime resource state and experiment-local assumptions distinct.
- Implemented: remove independent `measures` collection. Define measured outputs, units, and measurement boundaries inside owning experiment; no independent reuse requirement.
- Implemented: remove workbench-owned `decisions` collection and compulsory alternative archive. Existing project-level Attendant decisions may serve that role; do not create/require another. Users may retain ignored alternatives through their chosen statuses or delete them. This supersedes mandatory retention policy; do not automatically delete or discard existing content while removing collection support.
- Keep `challenges` as concrete gameplay tasks/situations with optional learning/route annotations. Current consumers versus planned checks: direct route inspection/Attendant queries and selected framework reviews now; deterministic all-route scaffolding/prerequisite checks in Slice 3; progression/graph views in Slice 4. Declared opportunity is not observed learning or game-state reachability.
- Import/export belongs in Slice 3 control-variable delivery. Export resolves JSON/INI/CSV through same value resolver used by experiments. Import formats, validation, and merge/overwrite behavior remain unresolved until refinement.

### Agent-led workflow correction

- [x] Remove generic lifecycle/query/report runtime and installed example copy; use Attendant, agent-guided merge, and ordinary Git instead.
- [x] Remove completed fixture migration programs and legacy wrapper export checker. Keep old evidence unchanged; Git retains deleted code.
- [x] Edit template files, example Markdown, and framework guides/catalog directly. Remove generators and JavaScript content mirrors. Tests validate shipped assets and ordinary temporary copies, not regeneration.
- [x] Update setup, capture, inspection, review, intent comparison, experiment checkpoint instructions, example, and repository index. No normal-use custom environment/manifest requirement.

### Framework guide expansion and review

- [x] Separate `workflows/`, `frameworks/`, and supporting `references/<framework>/`. All 18 guides end with References. Markdown `frameworks/index.md` replaces JSON catalog and explains when each guide is useful. Tests validate actual index and guides.
- [x] Retain actual source text/figures/captions and separate provenance/notes for 13 non-book/mixed-source frameworks. Preferred Markdown reading copies are indexed; HTML remains evidence. Record access, conversion, and rights limits per source; acquisition does not establish redistribution clearance.
- [x] Expand 13 source-backed guides with actual concepts, selected-slice application, output artifacts, worked examples, and counterchecks. Guides remain provisional for human review; word count and generation are not completeness evidence.
- [ ] Complete remaining human review where needed. User accepts resource economy, Caillois, and playcentric guides as sufficient for now; defer deeper source-backed expansion unless needed. These three do not block Slice 2. Schell's deck-backed and game grammar's chapter-backed expansions remain provisional, not full-book coverage. Preserve source limits; revisit a guide if implementation exposes a material contract gap.
- [x] Resolve Schell web-deck catalog: retain supplied raw JSON and searchable five-column table for all 116 English cards/451 questions. Preserve existing functional use notes; map descriptions/questions by internal index. Local-only/not-publication-cleared; local Git checkpoint explicitly approved, no push/publication.
- [x] Expand Schell application guide using retained individual lenses. Read supplied instructions and 17 selected cards; add question-led selection, scoped application, explicit artifacts/counterchecks, and failed-push example using Feedback (63), Punishment (47), and Emotion (1). Guide remains provisional pending human review; no whole-deck application or full-book coverage claimed.
- [x] Add shared emotional-experience procedure and non-exhaustive vocabulary: 64 primary working entries plus nine neighboring experiences/expressions, with meanings, targets, possible elicitors, source support, and explicit editorial/user-supplied senses. Support plain goals, specific relational feelings, mixtures, and rapid transitions; no dictionary gate or audience stereotypes. Retain Desmet source/figure and rights limits. Aesthetics authoring directly uses the vocabulary; templates, model guidance, and MDA surface naming feelings, targets, and experiential qualities. Rules–Behaviours–Feelings is an informal workbench bridge, not a renamed schema or claim about MDA's original terminology. Still 18 frameworks, no extra collections or compulsory taxonomy; provisional for human review.
- [x] Expand game grammar / meaningful play from the complete inspected chapter, printed pp. 31–37. Retain original teaching scan, raw Docling output, and checked full reading copy; repair OCR/column order against all seven page renders. Separate descriptive/evaluative meaning, discernibility/integration, delayed significance, and formal/social/cultural context from original matrix conventions. Correct worked example: failed pushes cost nothing; reason-specific feedback remains unspecified; overlapping blocked cases agree on state outcome. Add source-linked artifact/counterchecks, rights, and source limits. Provisional for human review; no game-record changes or full-book claim.
- [ ] Preserve overall player experience/player story and MDA A aspects as distinct lenses; do not treat one as canonical translation of the other.

### Slice 2 — C99 graph loading for domain tools

- [x] Implement direct Markdown/frontmatter loading for matrix/connectivity. User chose always-explicit `--nouns ALIAS=FOLDER`, `--verbs ALIAS=FOLDER`, `--interactions ALIAS=FOLDER`, optional repeated `--related ALIAS=FOLDER`. No path defaults, registry, or schema interpreter. LibYAML parser subset pinned/licensed; source identity uses collection/name, never UUID.
- [x] Share transient records, labelled declared links, sparse cells, and distinct-partner computation between `matrix`, `connectivity`, and `view matrix`. Preserve multiple cases, contextual nouns, dispositions, source/body, unresolved/ambiguous endpoints, zero-degree nodes, and operation coverage. JSON schema version 1; failed headless operations return partial data and exit 2.
- [x] Check consumed values and bounded parsing: malformed YAML, duplicate identities/fields, missing/wrong-role endpoints, null/text distinctions, escaped NUL/merge rejection, metadata skip, multi-target cases, text/file/frontmatter limits, changed sources, and successful/failed fresh-model loads. Crosscheck fixture identities/statuses/coverage/links through public Attendant on temporary copy. Matrix checks and authoring/public-Attendant checks pass; current combined coverage includes skill consumer below.
- [x] Verify Windows/CJIT headless CLI, explicit text/JSON selection, field envelope, failures, Unicode/space paths, and optional visible PNG capture at Unicode path. Windows Zig 0.17.0 C99 headless build emits identical matrix/connectivity JSON. No macOS/Linux execution claim. Headless executable requires no Node, raylib, display, agent, or projection.
- [x] Review current Attendant overlap: reader only consumes fields used by matrix and linked-record detail; no generic schema engine, CRUD, query, repair, collection discovery, or persistent projection. Keep overlap watchpoint before every loader expansion; separate future C99 Attendant port/shared foundation if needed.
- [x] Add bounded forward/reverse prerequisite adjacency for concrete skill-chain analysis. No generic traversal/query API or general impact analysis.
- [x] Verify live source polling/stale notice, F5 reload, invalid-source retention, and restored-source recovery in matrix table and skill rank graph. Reusable Windows `native/test-refresh.ps1` uses private copies and target-window F5 messages; captured/inspected state transitions and verified byte restoration. No product-code changes needed.
- [x] Close Slice 2 testing at user-accepted coverage. Native checks, inspected live captures, stale/F5/failure/recovery, and byte restoration are sufficient for this slice. Physical keyboard hardware, broader pointer controls, and clipboard were not systematically verified; no further test gate required for this slice.

#### Matrix graph extension

- User selected connected groups, not dense-community detection or shared-mechanic grouping. Add `view matrix --layout graph`; existing table remains default. Add `components` to matrix JSON and `--fields` selection; no new generic graph/query subsystem.
- Nouns/verbs are typed nodes; one edge per matrix cell retains all conditional cases. Compute undirected components from accepted, defined pairs only. Include singleton components for every isolated axis node. Context/mechanic links do not create matrix graph edges.
- Optional other-pair overlay shows unknown/excluded/not-applicable/proposed/inactive pairs without merging active components. Case details preserve statuses and source evidence. Graph/node/edge selection, pan/zoom/fit, and existing explicit refresh remain read-only; reuse case/source detail panel.
- C99 core owns membership; presentation owns only geometry/input. JSON groups expose stable anchor references, member references, and distinct active-pair counts. Grouping is derived structure, not authored categorization or proof of design quality.
- Graph view must be dynamic and offer different sorting/layout modes. Reuse graph presentation for later skill-atom view: selection, pan/zoom, node positioning, sorting, and layout switching. Keep matrix pairs/components and skill prerequisites/ranks in their domain computations; do not build a generic record graph/query subsystem. Layout positions remain transient; authored rank remains separate from derived layout.
- User selected live force-directed settling plus dragging, with pause and stable noun/verb-column alternative. Sort within components by name or active connectivity; force mode uses sorting as initial order, then settles freely. Shared graphics-free `gd_layout.h/.c` takes labelled nodes, caller-provided topology/groups/sides/weights, not records/status/prerequisites/ranks. Later skill view can adapt prerequisites/rank placement without changing domain ownership.
- cimgui/rlImGui supplies graph layout/sort/pause/overlay/fit/refresh controls and scrollable source details. Explicit `build-ui.ps1` uses installed Zig for vendored C++ dependency only, rebuilt against matching raylib 5.5 SDK; application remains C99/CJIT. No prebuilt prototype DLL, private framework module, implicit download/build, persistent layout, or ImGui settings file.
- Example acceptance: crate–push and switch–activate form two linked groups; six other nodes remain isolated. Three crate-push cases form one edge. Unknown exit–walk and not-applicable wall–activate do not merge groups, even when overlay is visible. Empty folders, chained components, duplicate cases, source changes, and reordered input must remain safe/deterministic.
- [x] Implement shared components/JSON, force/column layouts, sorting, drag/pan/zoom/fit/pause, graph/node/edge selection, source/case details, overlay, and explicit snapshot refresh. Focused native tests cover chained components, case deduplication/context exclusion, reordered input, stable sorting, held-node stepping, deterministic settling, invalid topology, and maximum-capacity bounded solver steps. Bounded table/force/column captures pass on Unicode paths; empty graph capture also passes. Windows Zig/CJIT matrix/components and connectivity JSON match. Opened live force-directed example graph with mechanics/control-vars/resources context after final checks. User interaction review remains; UX polish deferred. No rank/impact/context-graph expansion.

#### Skill prerequisite/rank consumer

- [x] Add `skill-atoms` and `view skill-atoms` with exactly one explicit `--skill-atoms ALIAS=FOLDER`, optional repeated `--related`. Consume only skill `prerequisites`, `rank`, and linked `mechanics` plus existing source/identity/status fields. Reviewed overlap: concrete domain consumer only; no discovery/schema/CRUD/repair/projection expansion.
- [x] Share exact authored ranks, prerequisite AND sets, depth, minimum/display rank, cycle membership, affected descendants, and sourced deterministic findings between headless JSON/text and view. Preserve all statuses, raw unresolved links, null/negative/zero/JSON-safe integer ranks, duplicate authored references, and isolated nodes. Deduplicate graph pairs only; no rank writes. Missing ancestry yields unknown derived values and unassessable conflict status, not independent roots.
- [x] Add iterative SCC/topological passes and relative prerequisite bands. Cycles remain conflicts with all original arrows; known links entering/leaving cycle bands and missing-ancestry branches still preserve relative direction. `layout_layer` is relative prerequisite presentation, not authored rank or a repaired graph.
- [x] Reuse cimgui graph controls, node/edge/source selection, path copy, explicit refresh, name/link sorting, fit, and pause. Default `--layout prerequisites --direction down`; `--direction right` alternative. Dynamic settling/dragging lock ordering axis, including paused drag. Rank mode compresses finite rank bands, orders same-rank prerequisites, retains parallel root lanes, and preserves authored intent.
- [x] Validate 14/14 native domain/layout checks under Windows CJIT and Zig C99; public-Attendant authoring tests 3/3, including skill field/reference parity and cleared coordinate conflict after rank-only update on temporary copy. Check branches/merges, late roots, null/negative/zero/exact-limit ranks, typed/null boundaries, cycles/self-loops versus descendants, missing/wrong-role references, source edits earlier/later, held/paused constraints in both layouts/directions, and 4,096-node/32,768-declaration chain/cycle capacity. CLI field/envelope/failure/empty/Unicode checks and bounded graph captures pass. Physical pointer/clipboard and large-project rendering review remain open; live source/F5 refresh is verified below.

Acceptance: concrete game-specific graph operations use source identities/declared relationships correctly and report loading/coverage limits without rewriting records. Attendant remains authoritative for generic operations. No standalone generic validator, schema engine, collection manager, or query layer.

### Slice 3 — deterministic semantics and control exports

- [x] Deliver sparse matrix/case inspection and accepted-pair connectivity in first native slice above.
- [x] Add read-only skill prerequisite/rank computation and deterministic consistency findings through concrete native consumer above.
- [x] Add in-memory skill rank/prerequisite editing, visible divergence from loaded disk baseline, and JSON change handoff, following [Rank edit behavior](#rank-edit-behavior). Viewer does not write authoritative records; agent decides how to apply report. Finish returns one JSON result on stdout, or optional explicit `--output <file>` writes that report instead. Revert interaction remains deferred.
- [x] Add headless challenge/route-opportunity checks with transitive skill demands, bypass/revisit witnesses, explicit incomplete coverage, and source-backed findings.
- [x] Implement MDA declared traces and scoped gaps as both CLI query and native visual exploration mode. Both interfaces use shared C99 analysis. Manual interaction acceptance is deferred to the final gate, not a blocker to remaining delivery slices.
  - CLI: query authored paths forward from a mechanic or backward from an aesthetic; structured results retain source records, relationship fields, authored direction, unknowns, and scope.
  - Visual: browse same declared graph, focus a record/path, explore connected records in either traversal direction, and inspect source details/findings. Keep systems distinct from dynamics, not a compulsory intermediate layer. Presentation must not turn authored association into proven causation.
  - Both remain source-read-only. No general record editor, inferred emotions, guaranteed experiences, or automatic gap repair. Missing links are scoped review questions, not automatic design defects.
  - Contract: `gd mda` and `gd view mda` require explicit `--mechanics`, `--dynamics`, `--aesthetics` mappings; optional `--systems` and repeatable `--related`. Optional `--from ALIAS/NAME` selects query root. `--direction forward|backward|both` defaults both: forward follows reverse authored basis links toward aesthetics, backward follows authored references toward basis, both explores connected associations. Preserve authored source/field/target independently from traversal direction. No implicit collection discovery.
  - Core trace links: systems.mechanics, dynamics.mechanics/systems, aesthetics.dynamics. Experiments and mechanic context links remain source context, not invented MDA edges. All statuses/duplicates/isolates remain visible. CLI data: nodes, links, paths, gaps, findings; field selection preserves envelope/errors. A rooted query retains one shortest witness per reached node plus all selected/boundary links, not exhaustive path enumeration. Bound path output to 64 trailing nodes with original length/truncation metadata.
  - Gaps are informational declared-structure questions: no declared basis, or no declared aesthetic trace. Unresolved graph references make negative trace claims unknown, conservatively across supplied graph. Unconfigured alias is unloaded context, not a proven missing record; mapped missing/ambiguous/wrong-role targets remain explicit findings. Under failed load, unresolved mapped targets are unavailable, not proven missing. Never infer unknown intent, observed dynamics, emotion, or guaranteed experience. Systems are optional constructed arrangements, never required as a bridge.
  - Visual explorer uses same query result with role bands/free graph, node/edge selection, direction/root controls, role/reference/query/path filters, connected-record navigation, gold shortest-witness highlight, source/finding panels, stale notice and explicit F5 refresh. Source remains read-only; refresh failure retains last-good model/query/layout. Missing query root rejects reload until cleared. No change report or stdout data on normal close.
  - Checks: 27/27 native tests under Windows CJIT and Zig; MDA adds 64 random graphs/2,304 focused queries against independent shortest-path oracle, exact 4,096-node/32,768-link capacity, 64-node path suffixes, finding omission, allocation failure, strict fields, custom aliases, duplicate/status/context/boundary handling, and partial ambiguity/unavailable-target preservation. Shared API JSON equals CLI output; CJIT/Zig CLI JSON matches. Structured screenshot failure returns exit 2 with error envelope. CLI regressions and 4/4 public-Attendant/authoring tests pass. Zig production C99 `-Wall -Wextra` clean; new viewer object compiles with existing cimgui typedef warnings, no vendor edits.
  - Static role-band/free/empty captures inspected. Seven private-copy MDA stale/valid-reload/invalid/rejected-reload/restore/recovery captures inspected at `skills/game-design-workbench/native/build/refresh-cd87c512c65d4190a2323588b3129328/mda/`; all private bytes restored, shipped source SHA-256 unchanged, child exited 0. Desktop helper uses Windows PowerShell 5.1 and owned-window F5 only; no pointer/global keyboard input. User performs mouse interaction checks; keep final crate-puzzle explorer open for review.
- [x] Implement shared C99 typed control resolution: integer/number/boolean/enum values, bounded arithmetic/unit grammar, iterative dependency resolution, temporary overrides, focused dependency scope, inspection, and raw CSV/INI/JSON exports. Resolver/export tests reached 33/33 on CJIT and Zig; CLI tests cover example propagation, Unicode paths, typed false/enums, field selection, empty output, errors without partial exports, and source preservation. Production Zig C99 `-Wall -Wextra` build clean; test-only vendor unused-function warnings unchanged. Configurable status/name admission is implemented; filters preserve dependency reads and never silently promote excluded statuses.
- [x] Implement deterministic value import planning and authorized public-Attendant adoption using current-export comparison; preserve formulas, reject unmatched derived tweaks, preserve omitted controls/metadata, validate full batch and affected status-admitted dependents, and recheck collection binding/contract, source, and planned writes. No historical baseline/companion snapshot. Use same CSV/INI/JSON dialects. Native import is read-only; documented workflow passes C99-validated `items` unchanged to public Attendant. No duplicate evaluator, new lifecycle program, or native Markdown writer. Attendant validates full batches before replacement but does not provide cross-file atomicity: report partial I/O failure accurately, never promise rollback or silently retry. Checks: 37/37 native tests on CJIT and Zig; strict production Zig build clean. CLI JSON/CSV/INI round-trips, selection, formula blocking, typed values, and source preservation pass. Both toolchains preserve finite nonzero subnormals through export/import and reject arithmetic underflow to zero. Public integration: 5/5 tests pass, including actual value adoption on private copies, stale-preview rejection evidence, immutable/body preservation, formula propagation, and invalid-batch empty items.
- [x] Document agent-led control tuning and verify shared-control boundary with existing example. `workflows/experiment-tuning.md` owns agent procedure; `examples/crate-puzzle/experiment-tuning.md` walks through timing investigation and temporary speed candidates. Actual native exports verified at speed 2/3/4: walk duration 0.5/0.3333333333333333/0.25 s; push duration 1/0.6666666666666666/0.5 s. Corresponding no-delay route calculations: 5/3.333333333333333/2.5 s. Source hashes unchanged. Agent builds experiment, interprets results, and refines interactively or through explicit experiment-local search policy; formulas stay shared and baseline adoption remains separate. No expectation-inspection CLI or experiment schema change. These checks are control resolution/calculation, not executed model trials. Runnable model/frozen recipe and end-to-end results-driven tuning verification remain Slice 5; clock boundaries, model code/helpers, checkpoint, and execution approval must be ready before execution.
- Export/import selection is configurable: accepted-only, all statuses, selected statuses, explicit include names, and explicit exclude names. Combine filters; exclusions win. Default game-facing set is accepted controls; inspection defaults to all. An explicit include list without explicit status filter selects those names across statuses. Explicit status filters intersect includes. Filters select output keys; resolve required dependencies without exporting omitted keys. In accepted/status-filtered mode, reject dependencies outside allowed statuses rather than silently promoting inactive definitions. Explicitly selected drafts retain their status; exporting values does not adopt them.
- [ ] Verify stable planned findings and source locations, explicit exclusions/no-effect/unknown distinctions, all declared routes including revisits, prerequisite cycles, null/zero/false, exact integers, enums, unit/formula errors, and override propagation.
- [ ] User manual draft-interaction pass at step end: rank/prerequisite controls, Ctrl+Z to disk sync point, dirty refresh guard, finish/close, failed-output retention/retry. See [manual checklist](skills/game-design-workbench/native/README.md#manual-draft-checks). Agent must not move pointer, click, type, or inject global keyboard input. Continue with code tests, screenshots, and private external data changes only.
- [x] Code-test rank/prerequisite draft edits, modified-state tracking, recomputation, and JSON handoff preserve baseline/body/metadata/descendant intent without writing sources. Cover external edits, unavailable current-disk values versus authored null/empty, cycles/conflicts, return-to-baseline, and no silent draft loss. Check parseable stdout/file JSON, empty changes, Unicode/space output paths, and safe output failure without overwriting existing files or authored sources. Rerun coordinate resolution fixture through agent/Attendant-applied handoff.

Acceptance: documented CLI examples and envelope work; completed exploration can contain findings; validation/threshold/runtime exits follow contract; export values agree with common resolver and do not overwrite source.

#### Challenge/route opportunity contract

- Headless `gd scaffolding --skill-atoms ALIAS=FOLDER --challenges ALIAS=FOLDER [--related ALIAS=FOLDER] --json`. Exactly one skills/challenges role each; no matrix roles. Reuse native loading, prerequisite analysis, typed consumed fields, allocator, errors, and output conventions. No GUI, source writes, rank edits, MDA checks, or player-state simulation.
- Earlier `introduces`/`practises` are support opportunities. `requires`/`tests_mastery` are demands, not assumed support. Include transitive AND prerequisites; deduplicate challenge/skill checks while preserving raw declarations/statuses. Rank never establishes traversal or learning.
- For every demanded skill and known prerequisite, classify prior opportunity as `all_routes`, `some_routes`, `absent`, or `unknown`. Inspect finite entry-to-visit walk prefixes, including revisits. Evaluate demand before current challenge opportunities. A later revisit never erases unsupported first arrival. Current introduction/demand overlap produces separate informational `scaffolding.single_shot`, not an automatic missing-teaching error.
- Missing/null/empty role/next lists mean no declared links. Empty next is a terminal in declared graph, not full physical-game coverage. Missing entry markers in nonempty graph, unresolved/wrong-role next links, and unreachable demand challenges prevent full opportunity claims. Preserve known supported/unsupported route witnesses even when coverage is unknown. Route cycles are valid revisits; prerequisite cycles remain findings. Unencoded gates/conditions stay prose and outside reachability claim.
- Shared C99 analysis owns a bounded result; borrowed record strings require loaded model to outlive result and remain immutable after analysis. Iterative prerequisite traversal and two-state (prior support seen/not seen) BFS avoid route enumeration, recursion, and infinite revisit walks. At most 32,768 challenge/skill checks; exhaustion returns partial output/exit 2. At most 128 witnesses, each retaining at most 256 trailing route nodes with explicit prefix truncation/full length and omitted count. Finding cap remains 128; coverage/check booleans do not depend on retained finding/witness capacity.
- JSON data: `challenges`, `skills`, `checks`, `witnesses`, `findings` plus omitted counts. Check example: `{"challenge":"challenges/combined-room","skill":"skill-atoms/route-switch","coverage":"some_routes","supported_route":true,"unsupported_route":true}`. Source paths/status/entry/raw references live on record summaries; checks also identify originating demand and direct versus prerequisite role. Witnesses link to check index and preserve explicit paths. Field selection retains envelope/errors. Completed analyses/findings exit 0; input/allocation/operation/output failure exits 2. No threshold flag.
- Acceptance: crate example exposes missing planning support, optional switch support/bypass witness, and same-challenge coordinate introduction/demand. Test multiple entries, joins, unreachable fragments, revisits/self-loops, missing/wrong-role references, unknown coverage, null/empty, typed entry/role fields, transitive prerequisites/cycles, duplicate declarations, exact limits, finding/witness truncation, and allocation failure. Verify JSON/text/field selection and existing command parity. Private fixtures only; leave manual draft UI pass pending.

### Slice 4 — native projections and source navigation

- [x] Verify Windows/raylib 5.5 build and bounded read-only matrix view on shared C99 model. Pin dependency notices; ignore SDK/DLL/build outputs. Copy owner-authorized MIT base/test headers unchanged for native checks.
- [x] Deliver read-only prerequisite/rank graph with shared source/finding inspection; default chain direction enforced during settling/dragging.
- [x] Deliver MDA trace explorer with Slice 3 shared CLI/query increment above.
- [x] Deliver conceptual progression as switchable Skills/Challenges modes in existing skill explorer, not a separate viewer. `view skill-atoms` takes optional explicit `--challenges` and initial `--mode skills|challenges`. Skills mode retains prerequisite/rank editing; Challenges shows authored routes/entries, raw skill roles, support checks and witness navigation/highlight. Direct/indirect skill-demand navigation and findings switch to appropriate mode. Draft/history/report destination survive mode switches; both analyses recompute after edit/undo. Challenge routes stay read-only. Shortest entry-hop bands are display only, with cycles/duplicates/unreachable shelf retained. Mode/source switching rejects invalid pending rank text; dirty refresh remains blocked. Manual interaction acceptance stays at final gate. Native tests 39/39 CJIT and Zig, including layout/draft alternation, prerequisite propagation, undo, and 4,096-node/32,768-link layout capacity. Both mode captures inspected; challenge bands stay stable (no force settling). Full graph-enabled CLI/capture suite and 5/5 public-Attendant tests pass. Private-copy challenge-route F5 checks verify changed graph, failed typed input retaining last-good model, restore/recovery, exact source restoration, and normal close. Final source-backed-error/stable-band captures inspected at `native/build/refresh-93390203967840df95c14f4178609a44/challenges/` (paths relative to skill); 7 stages pass, restored SHA-256 `E504AB761C89012FCFECD1A0D06FEEE45FDFF47E24B58BD44714C6F728F60D86`, source unchanged, exit 0. Final full graph suite rerun passes; viewer object builds with only existing vendor typedef warnings. Manual interaction remains deferred.
- [x] Deliver focused relationship exploration in existing matrix graph and skill/progression explorer. Source action starts from selected record/case; Relationships mode supports explicit one-hop incoming/outgoing expansion, recentering, all statuses/duplicates/self links/retained isolates, boundary declarations, authored arrows, field/source-line selection, and source navigation. `--mode relationships --from ALIAS/NAME` starts at a uniquely loaded record. Scope is already-consumed declarations, not every schema field, reference hygiene, transitive impact, causality, or learning. No loader expansion or generic query/CRUD/store. Shared graphics-free `native/gd_focus.c` keeps transient reveal/presentation state; generic layout solver stays domain-free. Draft/history/report destination survive switching; edits/undo rebuild declarations. F5 resets to root neighborhood; malformed input or missing root retains last-good snapshot. Details: `skills/game-design-workbench/native/README.md#focused-relationships`.
  - Checks: 41/41 CJIT and Zig; explicit expansion/direction/boundaries/duplicates/isolates, three-mode layout/draft alternation and undo, 4,096-node/32,768-link capacity. Full graph-enabled CLI/capture suite passes; inspected `native/build/space 雪-879dc0b8512e48c7bb267f8423753951/`. Private-copy relationship refresh: 9 stages at `native/build/refresh-737ae139553c4dc1b9dd87f396ca6af6/relationships/`; valid scope change, malformed input, removed root retaining snapshot, restored source/recovery inspected. SHA-256 restored `8FA4B512B53432F1B108EAA33624736700019303F0D2F0A92538BFA2BA7572BD`, canonical sources unchanged, exit 0. Production Zig `-Wall -Wextra` clean; viewer object has only existing vendor typedef warnings. Staged graph build at `native/build/focus-stage/gd-view.exe`; `-OutputDirectory` avoids replacing running user's explorer/draft. Artifact paths relative to skill. Mouse-driven expansion/mode/source acceptance remains final-gate; no global input automation.
- [x] Deliver shared diagnostics browser for matrix graph, skill/progression/relationship modes, and MDA explorer. `native/gd_diagnostics.c/.h` projects existing findings/errors and owns bounded rejected-reload error copies; shared C99/cimgui panel in `native/gd_diagnostics_ui.c/.h`. Filter metadata by plain substring/kind/selected record; preserve code/message/field/reference/path/line, source identity, omission and UTF-8 shortening flags. Inspect uniquely matched loaded source; rejected-input navigation explicitly shows last-good source, never failed candidate bytes. Unmapped/ambiguous/shortened paths do not invent identity. Retain all available rejected errors before freeing candidate, replace on next failure, clear on success; allocation failure explicit. No loader/schema expansion, generic validation/query/repair, severity ranking, record writes, or persistent store. Existing CLI contracts unchanged. Use matching prepared cimgui/rlImGui/raylib 5.5 dependencies.
  - Checks: 43/43 native tests on CJIT and Zig; projection/filter/source matching, separate finding arrays, owned error lifetime, UTF-8/code/path limits, shortened-path non-navigation, empty scope/invalid indices, 384 retained entries, producer omissions/overflow and allocation failure. Production Zig `-Wall -Wextra` clean; all three graph UI objects compile with only existing vendor typedef warnings. Full graph-enabled CLI/capture suite passes; captures `native/build/space 雪-507a4918d55f4df59494a820dc821187/`. Private-copy reload/failure/restore captures inspected: Relationships 9 stages at `native/build/refresh-0c03611fa58a4b07a55e871101f89753/relationships/`; MDA 7 stages at `native/build/refresh-ec3eeab2a9c84869ab0508206ba21053/mda/`; matrix graph 7 stages at `native/build/refresh-5127061a6b4c4e28a9c120bcd43f099f/matrix-graph/`. Rejected details survive candidate destruction; restored snapshots show zero retained reload errors. All canonical/private inputs restored byte-for-byte, owned processes exit 0; no pointer/global-keyboard automation. Artifact paths relative to skill. Staged executable `native/build/diagnostics-stage/gd-view.exe`; existing user explorer/draft untouched. Mouse-driven filters, inspect/collapse/copy and usability acceptance remain final-gate. Details: `skills/game-design-workbench/native/README.md#diagnostics-browser`.
- [x] Deliver selected skill rank/prerequisite draft editing, Ctrl+Z, modified-state markers, protected refresh, and JSON handoff. Source application remains outside viewer.
- [x] Deliver explicit relationship expansion over active consumer's loaded declarations; see focused explorer checks above. Matrix/skill node/edge/source detail, refresh/staleness and path navigation remain. No separate authoritative store, general record editor, or inferred full-project impact.
- [ ] Finish full deterministic finding/count/rank parity review, interrupted-save behavior, and broader domain/view parity. Shared computation, draft rank changes earlier/later, prerequisite edits/cycles, undo, and live external-edit/stale/invalid-source/restore/F5 paths are checked without authored descendant rewrites. Viewer intentionally has no authoritative source writer.
- [ ] Deferred, needs refinement: choose a better default font for raylib text. User accepts current graph as useful; UX/font work remains future task scope. Font choice and implementation details TBD.
- [ ] Deferred, needs refinement: investigate Neo4j and similar approaches for graph analysis. Current model feels like nodes with named connections. No graph-analysis backend selected or database adoption authorized; retain Markdown/Attendant authority and existing storage/runtime constraints pending explicit design decision.

Acceptance: human can inspect multi-case cell and parallel/late-root chains, navigate sources, resolve rank conflict, and see same results as agent. No physical level-layout mapping or HTML duplicate required.

### Slice 5 — explicit one-shot experiments and result views

Current scope: one concrete crate route-timing experiment in example data, then derive concise agent workflow from that artifact. Do not extend supervision, protocols, batches or result views for this request. Existing infrastructure stays untouched. [Experiment record](skills/game-design-workbench/examples/crate-puzzle/game-design/experiments/route-cost.md) links actual generated code/frozen inputs and first completed baseline evidence. [Workflow](skills/game-design-workbench/workflows/experiment.md) owns generate/check/run/record loop; tuning is a conditional extension. User subsequently authorized narrow local checkpoint and baseline execution; both completed. No push, publication, candidate run or control adoption.

Prepared increment delivered: shared C99 typed-input/PCG32/measurement/step/event helpers, inspectable route-timing baseline, frozen effective inputs, and [ordinary recipe](skills/game-design-workbench/examples/crate-puzzle/experiments/route-cost/recipe.md). Both CJIT 1.2.0 and Zig 0.17.0 compiled model. After approval, checkpoint `014af8bd477524a145d6ad077abf3ca2ad60c47a` froze 33 required files, including dependencies/licenses but no unrelated viewer/framework/research changes. Clean isolated checkout `D:/Repos/agent-skills-route-baseline` rebuilt with Zig 0.17.0; 100 input/random + 20 result helper checks passed there before one real model run. Engineering checks remain distinct from its simulated measurements.

Route clock confirmed: start before first action delay, include all seven independent delays and action durations, stop when final successful push finishes. Scope is prescribed legal route with four walks then three successful pushes; not puzzle solving or observed player behavior. Baseline controls retain sufficient energy. Insufficient-energy termination/recovery policy remains outside this increment; do not silently choose it.

- [x] Prepare C99 frozen-input/PCG32/measurement/step/event helpers and baseline crate model/input/recipe. Explicit compile-only model build and 100 helper checks pass under CJIT/Zig, including six-draw PCG golden vector, strict JSON/UTF-8/surrogates/duplicates/type/finite/integer/storage bounds, allocation refusal, encoding and cooperative budgets. No route trials or runtime acceptance claimed. PCG source/header/license retained unchanged at pinned commit; Apache-2.0 notice/provenance retained.
Preparation checks: 56 local documentation link targets resolve; pinned PCG source/header/license hashes match 3/3; frozen definitions/values match fresh native resolver 7/7; canonical design records unchanged. `git diff --check` passes apart from existing todo LF-to-CRLF notice. That preparation did not execute model; subsequent single baseline runtime/protocol check is recorded below. No broader workload performance claim.

- [x] Deliver explicit C99 Windows supervision and graphics-free result validation with [contract/commands](skills/game-design-workbench/native/experiments.md). `gd_experiment_run.c` launches suspended into owned kill-on-close job, restricts inherited handles/NUL stdin, arms independent watchdog before resume, captures raw stdout/stderr under combined frozen cap, locks exact recorded input copy, confirms owned-tree cleanup and retains bounded buffered tail. All files exclusive-create; existing output directory refused. No Git/checkpoint/approval automation, shell, source adoption or auto-run.
- [x] Deliver `gd_experiment_result.h/.c` and read-only CLI: strict versioned NDJSON, whole frozen input/provenance/dependency match, typed local measures/undefined reasons, scoped unique trial IDs, selected monotonic traces, final counts/status and process-exit checks. External stop always prevents complete evidence. Preserve invalid/partial records and counts; no criteria evaluation or repair. Input bounds stay 1 MiB/4,096 nodes/depth 32; embedded header has bounded 32 KiB/128-node/one-level allowance. One configuration per stream; later sweeps can validate separate files with repeated per-configuration indices.
- [x] Verify engineering infrastructure under CJIT 1.2.0 and Zig 0.17.0: 100 input/random + 20 result helper checks per compiler, plus 302 synthetic checks across both compilers. Cover reader allocation refusal/cleanup, malformed/provenance/type/undefined/trace/count/exit/stop failures, maximum input wrappers, raw NUL, stdin EOF, input-copy write lock, parent/descendant hangs, stdout/stderr/combined floods, startup failure and existing-evidence refusal. Zig C99 `-Wall -Wextra -Werror` clean. Latest retained synthetic artifacts: `skills/game-design-workbench/native/build/synthetic-ca0b7e6272e04111838096de45f8242f/`. No route model execution. Physical disk/OS/thread failure and non-Windows execution untested.
- [x] Review checkpoint scope/location and obtain explicit approval; create narrow local checkpoint `014af8b` and clean isolated checkout. Existing unrelated work remains uncommitted; no private/ignored inputs force-added.
- [ ] Deferred, separate scope: batch/sweep/replay automation and optional native result visualization. User confirmed saved experiment Markdown/summary/raw output is sufficient for the completed experiment workflow; no result-viewer milestone blocks the larger example. Recipe preparation/checkpoints use agent and ordinary Git.
- [x] Execute first approved baseline once: run `20261024T193020Z-baseline-speed-2`, 1,000 requested/attempted/completed, no invalid/undefined results, exit 0, native supervisor and separate reader both validate complete evidence. 245/1,000 (24.5%) finish within 8 s; supplied 90% criterion not met. Mean 8.510268915928435 s; range 6.472825908102095–10.635222096694633 s; every route complete with 0 energy. Continuous analytical fraction 26.03% is separate from sampled result. Raw files, hashes, exact build/cwd/toolchain and analysis retained under example `game-design-results/route-cost/20261024T193020Z-baseline-speed-2/`; actual record stage completed. Only checkpoint committed; results and post-run documentation remain uncommitted. No automatic adoption or game-quality verdict.
- [ ] If separately requested, test energy override 2 (requires explicit exhaustion policy), speed sweep, formula propagation, further criterion analysis/tuning, and unusual-trial trace. Do not extend this completed single-baseline request by implication.
- [ ] Test missing approvals/declined commits, dirty/untracked relevant source, unchanged recipe retry, stale recipe, compile/protocol/runtime/timeout/step/output failures, undefined measurements, missing historical dependencies, and replay without current controls or overwritten user files.
- [ ] Verify dated results, tracked reproduction provenance versus ignored rebuildable output, no auto-run on open/preview, CLI/view parity, and new simulation after rule changes. Use temporary Git repositories; never checkpoint this skills repo as a test.

Acceptance: reproduce recorded trials with frozen code/inputs/toolchain, show measurements/spread/percentiles and limits, inspect a selected run, and distinguish simulated predictions from observed play. Separately requested analysis can compare declared expectations; simulation does not decide design meaning. Full workbench scope is delivered across slices; current implementation covers Slice 1, native matrix/connectivity and skill prerequisite/rank CLI/views, plus in-memory skill rank/prerequisite editing, undo, JSON handoff, and headless challenge/route opportunity analysis. Remaining domain consumers and simulations are not delivered.

### Slice 6 — Little plumber guy NES example

Status: authored and headless-checked; manual integrated UX remains final-gate work. User clarified purpose: build familiar game-design data for future users, secondarily expose limits in current structures/tooling. Not decomposition/reverse engineering of Nintendo's game. All values and rules are authored for a fictional NES-style opening course. No external reference material, ROM, map, art or game screenshot retained; no reference lookup needed.

- [x] Keep crate-puzzle unchanged and add independent `examples/little-plumber-guy-nes/`, concept in game-design mode, 14 shipped collection contracts/aliases and public-Attendant identities/fields. No new schemas/runtime or data generator.
- [x] Author 202 meaningful records: 26 nouns, 12 verbs, 49 conditional interactions, 24 mechanics, 6 systems, 8 proposed dynamics, 6 intended aesthetics, 16 skill atoms, 19 challenges, 2 level groups, 20 controls, 4 resources, 3 proposed investigations and 7 glossary entries. All bodies filled; no transplanted crate facts. Rules cover movement, blocks, power states, directional enemy contacts, shells, pickups, retry/time/coin/score, optional cellar bypass and ending branches.
- [x] Add README with approximate consolidated LLM instruction, workbench parts used, linked reading paths, bounded requests and existing-tool commands. `model-limits.md` documents prose-only state predicates, conditional capability/route gates, instance/geometry absence, stateful retry limits, cross-control constraints and honest unknowns. No claim about actual Nintendo behavior or observed players.
- [x] Existing native consumers pass: 26 nouns/12 verbs/49 cases in 26 occupied cells; 16 skills/19 prerequisite links/no cycle findings; 19 challenges and 18 demanded-skill checks (17 all_routes, finish-approach absent before its first test); 20 controls resolved (walk/run tile times 0.5/0.25 s, stable coin capacity 49); 44 MDA nodes/74 links, backward tension trace accepted. Temporary walk-speed 3 override also resolves without source adoption. Example remains incomplete where stated, not silently repaired to make every finding disappear.
- [x] Public Attendant strict validation: 202 records, no diagnostics/corrections. Maintainer suite: 6 passed, 1 optional desktop-draft integration skipped. Tests read all bodies, validate a private copy and exercise existing native consumers; no duplicate evaluator. Skill frontmatter/local links checked. Check artifacts remain in ignored `native/build/little-plumber-checks/`.
- [ ] Include larger dataset in final user-owned integrated native-view review. No manual UX/rendering-performance claim from headless checks.

### Review task sequence

User accepted this starting order. Tasks remain needs-refinement, not implementation-ready. Refine one at a time and persist settled answers in the relevant task. Independent usability work and graph research may move earlier. Source observations: `.pi/visualiser-review-notes.md`.

Across all tasks: raw source must remain readable and straightforward for humans to edit without an agent, GUI or conversion step. User treats this as essential to usefulness, portability and agent friendliness, not optional presentation polish. Prefer compact, target-scoped language over serialized trees or function-heavy expressions. Tools interpret source; generated views must not become the only usable interface. Refinement/acceptance must include representative raw-text reading/editing examples.

Preserve authoritative Markdown, Attendant ownership and shared C99 computation. Update both examples with each relevant change; deepen connected Little plumber sequences and make their design contributions recoverable. Let gap exploration suggest possibilities beyond 1-1, including ideas found elsewhere in Mario games, without requiring a full-game catalogue or treating optional extensions as defects. Distinguish missing facts, authoring guidance, relationships, analysis and presentation. No automatic asset acquisition, game trials, framework/dependency adoption or commits.

#### Review 01 — Actor/state interaction model

Status: ready for review; implemented and verified through full affected-example/sample migration. Foundational for 02, 03, 04 and 09. Earlier checkpoint: `8942bd1`; subsequent implementation/migration included in this record's authorized checkpoint commit. No push, experiment rerun or manual UX acceptance.

- Result: interactions describe participants, applicable circumstances, actor state transitions and other effects so shared outcomes and role changes can be inspected without interpreting prose alone.
- Delivery boundary: include working headless inspection, selected by user, not authoring-only contracts. Deliver source contracts/usage, shared C99 reading/resolution, facts/diagnostics output and tests. Keep new actor-state visualisations, ghosts and broad graph queries in later tasks. Choose CLI/API details using established conventions; report checked versus unexamined semantic scope explicitly.
- Example migration: change every current example to match the new structures, clarified by user; no representative-only migration. Apply the contract to all affected interactions, mechanics, actor/axis definitions and formula examples across the current example projects and relevant authoring samples. Preserve existing identities, dispositions, unresolved design questions and frozen experiment evidence; migrating every record does not authorize inventing missing rules, quantities or observations. No affected live example remains in the legacy format at task completion. CLI details can follow established conventions rather than become another broad preference exercise; include observable headless acceptance.
- Accepted authoring specimen: user reviewed and accepted `skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/interactions/touch-walker-powered.md` as the source-format shape. It contains a body rule with canonical participant bindings, ordered contact context, explicit conditions/ANY, state changes, numeric control lookup and preserved life budget. Existing identity/status retained. Native declaration reader now parses this specimen. Typed axes, explicit noun/axis bindings, effect state names and quantity targets now resolve. State predicates compile with unknown truth preserved; ordered context now binds local participants and explicit rule vocabularies without inferring contact. Numeric comparisons and quantity-expression primitives now use shared C99 math. Conditional applicability/outcomes, explicit composition, whole-case preservation and bounded consistency checks now work headlessly. Full live migration covers 85 rule records / 180 cases. Strict Attendant validation: 67 crate / 222 plumber records, zero diagnostics/corrections. Declaration or scenario success does not establish gameplay validity. Finish concrete headless/acceptance details under the agreed constraints without further broad format-preference polling; implementation remains separately scoped.
- Requirements: answer A + B under relevant actor states; retain which participant changes; allow multiple effects on different participants; expose a shared death destination without confusing player death with enemy death. Distinguish current role from origin and ability to harm the player. Shell attacks after enemy-to-non-enemy/ally transformation must not inflate enemy–enemy richness.
- Starting examples: pit kills player across forms; enemy side contact kills small/unprotected player but removes power from big player; descending top contact defeats eligible enemy and rebounds player; turtle transformations and player motion effects. Exact shell transitions are not adopted from review illustrations without resolving the authored rule.
- Preserve existing noun identities, conditions/cases, dispositions, source links and incomplete designs. Resources remain existing nouns; resource flows are developed in 02.
- Mechanics and interactions: user wants their definitions to reinforce each other or expose conflicts, rather than select one as an implicit winner. Mechanics describe shared/general rules; interaction cases describe concrete conditions/effects. Retain both declarations and their provenance; compare where their scopes overlap. Agreement supports consistency; incompatible declared effects expose a conflict; missing/unspecified details do not establish agreement or contradiction. Explicitly referenced rule composition may supply derived outcomes, but a reference alone is not independent corroboration. Do not silently inherit effects, overwrite cases, auto-repair or resolve conflicts by source kind. Implemented scope: uniquely typed hypothetical participant alignment, ordered context/action-link checks, bounded declared enum product, matching/redundant/compatible/conflicting/disjoint/undetermined results, copied conflict witnesses and explicit unexamined numeric/open/limited scope. No guessed invocation bindings or universal algebra/physics proof.
- State composition: independent dimensions, selected by user. Life, form and protection can coexist; interactions may inspect/change several dimensions. Do not require enumeration of every combined configuration.
- Axis types: use the same typed axis-definition mechanism for named states, integer quantities and numeric quantities, selected by user. Actors declare the axes they use. Numeric axes need units and relevant bounds; actor quantities are not mutable global control variables. Control variables retain configuration ownership. Native contract now reads flat `value_type`, `allowed_values`, `unit`, `minimum`, `maximum`, `closed`; unspecified types/units remain unknown. Initial actor values are not inferred. Effect targets and RHS quantity expressions now bind/evaluate through shared numeric machinery. Target-scoped `this` reads supplied starting facts, never configuration defaults.
- Actor applicability: explicit axis references, clarified by user. An axis definition in Markdown declares its set of states; actor type declares that it uses that axis, rather than repeating a per-actor allowed-value list. Reusable definitions and incomplete designs must be possible. Do not infer applicability solely from interaction mentions or infer reachability of every cross-axis combination. State sets remain open by default; native `closed: true` explicitly closes only a named-state vocabulary, not cross-axis reachability. Noun `axes` holds explicit references; omitted/null applicability stays unspecified, while `axes: []` records an empty current inventory.
- Interaction conditions: omitted state conditions mean unspecified, selected by user. An explicit unrestricted/any condition is distinct from omission and need not enumerate every allowed value. Support a target/case-local shorthand making other actor axes unrestricted, selected by user; accepted specimen uses `other_axes: ANY`. It does not change axis values or establish cross-axis reachability. Without that explicit shorthand, omitted axes remain unspecified. State/numeric predicates and explicit ANY/unspecified modes combine with supplied context facts and case-local exclusions; omitted scope remains unknown.
- Interaction effects: an unmentioned resulting state dimension is unspecified unless explicitly preserved, selected by user. Support an explicit shorthand preserving other dimensions without enumerating each one. Do not infer unchanged state from an incomplete effect list. Resolve OTHER against the entire expanded case, selected by user: collect explicit caller/invoked-rule effects first, then preserve remaining axes. Do not turn component-local OTHER declarations into artificial conflicts with explicit effects elsewhere in that expansion. Conflicting explicit assignments and explicit promises to preserve a particular axis remain constraints to check, not last-write-wins.
- State effect authoring: name resulting states and infer their axes through the affected actor, selected by user. Example: `becomes: small, protected`; qualify ambiguous names as `axis.state`. Explicit axis assignment is not required. Effects assign states rather than express Boolean alternatives; contradictory assignments within an axis need a diagnostic. `becomes` names bind; simultaneous proposals retain immutable starting values, source paths and contradictory/uncertain write checks.
- State identity: each axis has a Markdown record listing its states directly, selected by user. Actors reference the axis. No separate record per state value required now; user is open to revisiting if a concrete need emerges. Graph nodes for individual values resolve within their axis and retain the affected actor context. Preserve shared state meaning; do not duplicate actor nouns for every combination.
- Condition logic: support nested Boolean conditions with grouped AND/OR/NOT, selected by user rather than limiting conditions to simple conjunctions or splitting every alternative into separate cases. Retain the agreed unspecified-versus-any distinction. Boolean support alone does not imply physical simulation or a general gameplay runtime.
- Condition authoring: human-readable, target-scoped expressions. User example: `alive and ((big && !protection) or (small && protection))`. Resolve state names through the target actor's declared axes rather than repeating actor references and state lookup functions. User rejected both JSON trees and function-heavy expression examples. User offers lowercase or PascalCase state names and SCREAMING_SNAKE_CASE logic keywords if this simplifies parsing. Current grammar accepts `alive AND ((big AND NOT protection) OR (small AND protection))`, case-insensitive ASCII logic keywords, `&&`/`||`/`!`, and parentheses. State/axis names remain exact and case-sensitive. Precedence NOT, AND, OR; no implicit operators. Bare/qualified names reuse explicit actor-axis lookup, never body mentions. Numeric comparisons now reuse shared bounded control math, explicit actor quantities and typed configuration references. Source contracts: `native/conditions.md`, `native/quantities.md`.
- Name disambiguation: use bare state names when unique within the target actor's axes; support `axis.state` qualification when ambiguous, selected by user. Example: `shield.active AND NOT weapon.active`. Never guess an ambiguous match or require full record paths/state lookup functions in expressions.
- Raw-text organization: group conditions and effects by named participant, selected by user rather than repeat the participant in every statement. Participant names are local bindings to canonical nouns; they do not create new entities or a role taxonomy. Preserve ordered relational context between these bindings.
- Rule storage: one labelled code block in the Markdown body is authoritative, selected by user rather than a multiline frontmatter field. Accepted fence label `gd-rule`; current declaration grammar documented in `native/rules.md`. Ordinary frontmatter retains flat metadata and reference links; prose explains intent without becoming a competing machine-readable definition. C99 validates domain rule text; Attendant retains record/reference/projection responsibilities.
- Named cases: allow multiple named cases within one mechanic or interaction rule block, accepted by user after confirming body placement. Cases are unordered, not an if/else sequence. Retain record identity plus local case identity/source location in diagnostics, comparison results and paths. Native reader and conditional evaluator retain named-case identities and source locations; no unordered case becomes an if/else winner.
- Condition sets: user clarifies `guard: dead` was intended as a short circuit/early-out, not a positive requirement. Short circuits are case-local, selected by user: when true, exclude this interaction case, not all interactions involving the actor. Do not introduce implicit actor-wide inheritance. Reader compiles `skip_if: dead` separately; read-only Boolean evaluation consumes explicit participant-scoped predicate facts. Combined exclusion/applicability remains case-local; guard compilation introduces no actor-wide inheritance. Requirements express conditions that must hold. Preserve unknown conditions rather than treating them as false. Boolean grouping and explicitly mapped composition retain separate declarations and provenance. User clarifies omitted `skip_if`: no guard declared, no assumption. Case exclusion checks inspect only authored guards; do not evaluate missing fields, invent false predicates or inherit actor-wide guards. Missing positive requirements/context remain unspecified. Source-presence distinction is separate from an authored guard evaluating false or unknown.
- Overlap semantics: interaction cases are unordered declarations, selected by user. Expose applicable and uncertain cases and incompatible effects; do not choose an implicit winner from record order or priority. Compatible overlap is not itself a defect. Check across Markdown files and collection boundaries as well as within one record: user notes that semantically the same thing may be declared in several files. Distinguish equivalent/redundant declarations, compatible partial overlap, conflicting effects and unresolved ambiguity. Preserve separate source identities; do not silently merge, deduplicate, choose a file winner or treat differently named declarations as necessarily different behavior. Findings retain both sources, conditions, witnesses where available and checked/undetermined scope. Detection scope, fixed capacities/work budgets and result fields are documented in `native/outcomes.md`; limits retain checked and unexamined portions.
- Role terminology: user questions any explicit role concept and proposes ordinary object states instead. Do not add a mandatory Role axis or separate role/classification mechanism. Walking/hiding/kicked states and their interactions can describe the transformation; enemy/ally labels, if useful, can be ordinary authored states. Analysis still needs explicit actor/state scope for enemy–enemy comparisons rather than inferring enemy status from origin or harmful contact. Exact query-set definition belongs with analysis refinement.
- Interaction context: user identifies direction/contact circumstances as context elements belonging to the interaction, not extra actor state axes. Context elements are optional; do not manufacture them for every case. They distinguish interactions between the same participants, such as player/goomba contacts that determine which actor becomes dead. Reuse the axis-definition format for context vocabularies, selected by user; context axes belong to the interaction rather than implicitly becoming actor state axes. Use the same omission convention as actor conditions: omitted context is unspecified; explicitly write ANY when no extra context restriction is intended. User selects this uniform convention to try in concrete authoring examples; revisit if those examples expose friction.
- Relational context: expressive and ordered, clarified by user. `a <operator> b` is not interchangeable with `b <operator> a`. Preserve explicit participant bindings and operand order in source, analysis and diagrams; do not sort participants or assume commutativity. Local names must distinguish multiple participants of the same actor type. Implemented fragment: rule `context_axes` explicitly selects existing enum-axis definitions; body accepts ANY or LEFT OPERATOR RIGHT between case-local labels. Preserve operand order. Bare/qualified operators resolve only within that inventory, not actor axes or all loaded definitions. Omitted context stays unspecified; omitted/null vocabulary stays unknown, `[]` explicitly empty. Compound/numeric context and physical scene truth remain unsupported; combined logical applicability consumes explicit ordered relation assumptions. Contract: `native/context.md`.
- Quantitative effects: include structured quantitative changes in task 01, selected by user rather than deferring all non-state effects to mechanic prose. Cover actor-specific quantities such as rebound/jump-height changes alongside state transitions; keep resource-role/flow modeling in 02 coordinated with this effect contract. Do not invent numeric values or executed physics. Use assignment expressions, selected by user, with special `this` for the assigned axis's starting value so delta changes remain assignments, e.g. `Amount: this + 1[life]` under proposed `set`. Control-variable lookups use `{...}` without a dollar prefix, e.g. `{control-vars/safety-window}`. User explicitly requires the new syntax, rejects legacy `${...}` acceptance, and requests migration of existing formulas. Apply the new syntax across the shared formula reader, live designs, templates, guidance and relevant tests; diagnose legacy lookup syntax rather than accepting it. Preserve historical frozen experiment inputs/results unchanged under the existing evidence constraint; do not present their old syntax as valid new input or rerun experiments to rewrite history. Implemented quantity contract: scoped actor-axis LHS, shared bounded arithmetic/control references on RHS, assignment-only `this`, explicit `UNKNOWN`, exact units and consumer constraints. Configuration is never an automatic actor starting value. Bounds findings retain proposed values; no implicit gameplay policy. See `native/quantities.md`.
- Effect timing: one interaction describes a simultaneous transition; all effect expressions read the same starting values, selected by user. Incidental presentation order must not change meaning. Do not introduce sequential execution or implicit stages. Conflicting writes remain diagnostics, not last-write-wins.
- Numeric bounds: bounds diagnose out-of-range results, selected by user; no implicit clamp/wrap/reject policy changing game behavior. Preserve the proposed result and report violation or uncertainty. Capping, wrapping and other boundary behavior belong in explicit authored rules. Types/units/safe-number limits remain consumer constraints, not hidden gameplay corrections.
- Equivalent rule formulations: support both direct conditional effects and explicitly composed rules. User example: side contact causes power loss; power loss on small player causes death, while on big player it causes small. Authoring preference must not force different terminal-result interpretation. Compare conditioned derived outcomes while preserving intermediate concepts, ordered participants, guards, all declared effects and source paths; structural topology need not be identical. Do not infer unrestricted conditions from omission, equate bare reachability with applicability, repeatedly apply power loss to a newly small actor for the same occurrence, or assert universal equivalence where information/analysis is insufficient. Invocation syntax and bounded comparison contract: `native/outcomes.md`. Structural numeric agreement is not proof of algebraic equivalence or arithmetic validity over all inputs.
- Rule invocation binding: allow a shorthand inside a participant group when the referenced rule has exactly one participant, selected by user. Bind that callee participant to the enclosing caller participant, independent of local label spelling. Multi-participant rules require explicit callee-to-caller binding maps. Do not guess bindings by names or noun type; diagnose incompatible types, missing/ambiguous bindings and use of the shorthand when additional participants are required. Invocation expansion retains the same starting-value semantics and source paths; it does not imply a later tick or sequential mutation. Implemented spelling: `causes: ALIAS/NAME[#CASE] with Callee=Caller, Other=OtherCaller`; semicolons separate calls. One-participant shorthand remains explicit and type-checked.
- Comparison coverage: include bounded comparison across declared state branches, selected by user rather than supplied-scenario checks only. Return matching results within checked scope, concrete conflict witnesses where established, and explicit undetermined/unexamined scope for open state sets, missing information, unsupported numeric reasoning or limits. Preserve conditions and provenance. Do not claim universal equivalence from sampled scenarios or adopt a general mathematical-equivalence solver by implication. Implemented capacities: 4,096 pairs, 128 enum branches/pair, 16 dimensions, 16 expansion levels, 4,096 expansions, 8,192 proposals; separate charged invocation/comparison/evaluation work. Numeric continuum remains unexamined.
- Implementation checkpoint: `529ff3c` commits current workbench changes, examples, retained experiment evidence and refinement records before implementation. Unrelated root concept drafts excluded; no push. Result evidence has byte-preserving Git attributes and includes its empty stderr log.
- Implemented slice — formula lookups: shared C99 control resolver requires `{ALIAS/NAME}` and rejects dollar-prefixed references with sourced `controls.reference_syntax`. Updated import dependent-impact discovery, all six live derived-control formulas across both examples, existing native fixtures and formula guidance. Public Attendant updates preserved record identities/values; its standard serialization normalized the three crate metadata headers. Historical frozen input/results unchanged.
- Checks — formula slice: new braced-reference regression failed on the old reader; after implementation native domain/layout/draft/route/MDA/control checks pass 45/45, including malformed/mixed/legacy references, source locations/offsets and omitted dependent validation during import. Staged headless CJIT builds succeed under `native/build/review01-formulas/`, preserving existing viewers/binaries. Authoring tests pass 6 with 1 optional desktop test skipped. Live Attendant strict baseline validates 63 crate and 202 plumber records without diagnostics/corrections; post-change copied-project validation passes in authoring tests. Scoped whitespace check passes; frozen-input/result diff empty. No experiment reruns or manual UX acceptance claimed.
- Implemented slice — rule declarations: `gd rules` reads one labelled body block per explicitly bound interaction/mechanic record. C99 retains named cases, distinct local participants, raw conditions/context/effects, numeric assignments, metadata/source links and Markdown line provenance. Missing blocks remain absent; omitted fields remain null. Keep repeated assignments and cross-file case names; reject malformed fields/indentation/duplicate local identities without returning half-parsed records. JSON/text and field selection follow existing CLI conventions. At declaration checkpoint only accepted plumber specimen was structured. Subsequent slices now cover all 85 live rule records, typed evaluation, explicit expansion, preservation and bounded comparison; original source identities retained.
- Checks — rule reader: native 50/50; combined authoring/CLI tests 8 passed, 1 optional desktop test skipped. Includes bounded text/case/participant/assignment limits, allocation failure, rollback, same-type bindings, fences/CRLF, original source lines, metadata/links, partial/empty/error output and source immutability. New module compiles with Zig C99 `-Wall -Wextra -Werror` for Windows and Linux targets; Linux execution untested. Staged under `native/build/review01-rules/`; existing viewers preserved. Frozen experiment/model evidence diff empty. Reader and axis slices committed as user-requested checkpoint `065f11b`, after formula/deferred-JSON-note commits `0fb2965` and `309ccbd`. Unrelated root drafts excluded; nothing pushed.
- Implemented slice — typed axes/applicability: shared typed-metadata reader plus `gd_rule_axes.c` validate enum/integer/number definitions, explicit open/closed vocabularies, shared units and numeric bounds. `gd rules --axes` exposes `axes`, `actors`, `findings`, sourced noun/axis resolutions and separate readiness flags. Preserve duplicate declarations and distinct same-type local bindings; never infer actor applicability from body mentions. Missing type/unit/applicability stays informative unknown, not false or automatic repair. Native source contract: `native/axes.md`.
- Live contracts/actor declarations: user approved adding both axes collections and continuing. Public Attendant registered `game-design/axes as axes`; both examples now have 15 collections. Installed shared axis schema/usage/template and optional noun `axes` references. Created 3 crate and 8 plumber provisional axis definitions from existing authored rules/quantities. Updated all 32 nouns through public Attendant: 9 inventories reference known dimensions; 23 remain explicitly null. Existing noun identities/statuses/bodies retained; no new initial values or unreported rules. Bootstrap checkpoint counts were 66 crate / 211 plumber; later full migration now has 67 / 222 records. Contact remains context-only unless explicitly selected for another use. All 85 live rule records are now structured.
- Checks — typed axes/live contracts: native 55/55; combined authoring/CLI 10 passed, 1 optional desktop test skipped. Covers typed/partial definitions, open/closed/empty vocabularies, numeric units/bounds/negative zero, duplicate retention, omitted/null/empty applicability, all reference-resolution states, source locations, value/finding limits, JSON/text, field selection and source immutability. Both live projects pass strict public Attendant validation without diagnostics/corrections; verified all 32 noun IDs and bodies unchanged. Native axes readiness true for both supplied collections; global binding readiness false because other inventories remain unknown, not empty. Axis body source links checked. New module compiles with Zig C99 `-Wall -Wextra -Werror` for Windows/Linux; Linux execution untested. Staged under `native/build/review01-axes/`; frozen experiment/model evidence unchanged. No new commit, control-format migration or viewer interaction.
- Implemented slice — effect symbol bindings: `gd_rule_logic.c` resolves bare/qualified state names and integer/number assignment targets through explicit participant noun inventories. JSON retains raw text and adds canonical target/state facts; duplicate references do not create matches, while repeated effects/assignments remain separate. Unknown/open information stays undetermined, not false or invented applicability. Closed missing names, ambiguous matches, wrong types and invalid definitions produce sourced errors. No condition applicability, resulting quantity evaluation, preservation or contradictory-write/composed-result claims. Fixed 8,192 state-declaration cap; 16,777,216 charged comparison/traversal work units; malformed/overflowing lists return no partial list, work exhaustion marks remaining lookups unavailable. No allocation inside symbol resolution.
- Checks — effect symbols: native 56/56; combined authoring/CLI 12 passed, 1 optional desktop test skipped. Covers approved specimen bindings, same-type local participants, duplicate declarations, bare/qualified/unknown/closed-missing/ambiguous names, wrong types/invalid definitions, Unicode/quote/backslash JSON spans, source lines/immutability, exact capacity and overflow, bounded work and checked/unavailable portions. Shared JSON string writer accepts bounded spans without copying source text. Changed application modules compile as Zig C99 with `-Wall -Wextra -Werror` for Windows/Linux; Linux execution untested. Staged builds under `native/build/review01-effects/`, existing viewers preserved. Frozen experiment/model evidence unchanged. User-requested checkpoint `d837e40` commits this slice after `065f11b`; unrelated drafts excluded, nothing pushed.
- Implemented slice — state predicate logic: renamed effect-only module to `gd_rule_logic.c` and reused its state lookup/work budget for `requires`/`skip_if`. Iterative bounded parser retains grouped AND/OR/NOT, symbolic aliases, explicit ANY, exact names, raw byte offsets and source lines. Omission stays unspecified; `other_axes` exposes unspecified/any/invalid modes. Derived postfix programs are output facts, never authoritative JSON authoring. Read-only C99 truth uses explicit facts keyed by snapshot participant/axis/state; missing, conflicting and unresolved facts remain unknown. This is predicate logic, not overall applicability, scene validation or effect execution. Numeric comparisons initially remained unsupported at `be06ee3`; subsequent quantity slice shares existing evaluator rather than adding a second one. Limits: 256 input tokens per field, 64 groups, 65,536 compiled tokens per result; no recursion/allocation, no half-compiled failed fields.
- Checks — state conditions: native 57/57; combined authoring/CLI 14 passed, 1 optional desktop test skipped. Covers full three-valued NOT/AND/OR tables, absent/conflicting/cross-participant facts, explicit ANY versus omission, independent skips, grouped precedence, case-insensitive keyword/symbol aliases, exact state spelling, qualified/unknown/unsupported/type-error paths, original byte offsets/Unicode, source immutability and exact/exceeded depth/per-field/global capacities. Shared condition/effect work exhaustion retains checked predicates/effects and marks later targets unavailable; bounded byte-work fixtures cover that path. Changed application modules compile with Zig C99 `-Wall -Wextra -Werror` for Windows/Linux; Linux execution untested. Staged under `native/build/review01-conditions/`; user-requested checkpoint `be06ee3` commits this slice after `d837e40`. No push, simulation run or viewer interaction.
- Implemented slice — ordered context: C99 parses ANY or one ordered local-participant triple, resolving operator states through explicit interaction/mechanic `context_axes`. Definitions reuse existing axes collection/format, without actor-state leakage, automatic contact, commutativity, noun-label guessing or cross-case binding. Shared state lookup preserves qualified/bare names, unknown/open vocabularies, duplicates and source spans. Local-name comparisons and context/effect/condition lookups share bounded symbol work; unavailable results never become false/missing. Expanded metadata uses bounded byte markers instead of a 32-bit consumed-field mask; duplicate `closed` still detected beyond bit 31.
- Live context contracts: added optional `context_axes` ref-array and usage to both live examples and shared templates. Public Attendant created provisional plumber `axes/Contact` with only already-authored `sideContacts`, then added that reference to accepted powered-contact specimen. Verified public update preserved ID/status/body exactly before explanatory prose update. Eight actor axes and all noun inventories unchanged; plumber at context checkpoint had 9 axes / 211 records. Full migration now has 20 axes / 222 records and all 73 structured rule sources; context symbols are not scene observations.
- Checks — ordered context: native 57/57; authoring/CLI 15 passed, 1 optional desktop test skipped. Covers operand reversal, same-type/self bindings, cross-case rejection, ANY/omission, explicit/empty/unknown inventories, qualification, duplicates, ambiguous/open/closed/type/invalid-definition/wrong-role paths, Unicode/raw offsets, source immutability, late metadata duplicate detection and work exhaustion. Strict public Attendant validation: 66 crate/211 plumber, no diagnostics/corrections. Changed application modules compile Zig C99 `-Wall -Wextra -Werror` for Windows/Linux; Linux execution untested. Staged under `native/build/review01-context/`; no further commit or push, no experiment rerun or viewer interaction. Historical experiment inputs/results unchanged.
- Implemented slice — local exclusions: read-only `gd_rule_case_exclusions` checks authored guards only. Distinguish absent guard list, all authored guards false, known true exclusion and unknown/unexamined exclusion. Preserve case-local witness participant, total/checked guard counts, conservative charged work and limit flag. Any checked true guard excludes only that case even if another is unknown; no-guard result does not fabricate false or establish positive applicability. CLI adds authored `skip_if_count`, not scenario truth. Separate 16,777,216 evaluation-work budget reserves each whole guard before predicate evaluation; no allocation or source/result mutation.
- Checks — local exclusions: native 58/58; authoring/CLI 15 passed, 1 optional desktop check skipped. Covers no authored guard versus explicit false/true/unknown guards, case-local scope despite same noun type, missing facts, true witness after an unknown guard, early witness/checked counts, invalid fact/case inputs and aggregate evaluation-budget exhaustion without false exclusion claims or source/result mutation. CLI retains raw absent fields and reports authored counts only. Changed application modules compile strict Zig C99 for Windows/Linux; Linux execution untested. Staged under `native/build/review01-exclusions/`; user-requested checkpoint `8942bd1` commits context/exclusion work after `be06ee3`. No additional approval gate from supplied guard clarification.
- Implemented slice — quantity primitives: `gd_controls.c` now owns one span-aware bounded numeric compiler/evaluator for controls, numeric predicates and assignment RHS. `gd_rule_quantities.c` resolves selected dependencies, actor-target starting facts, typed values, units, integer/safe arithmetic and diagnostic bounds. Numeric comparisons integrate into Boolean programs; malformed/limited numeric syntax rolls back complete field, not just failing atom. Enum/boolean control references retain types/provenance. Case `unknown_conditions`/`unknown_effects` preserve readable unmodeled scope without invented defaults. Same-frame primitive reads never mutate facts. Quantity budget: 16,384 programs, 32,768 tokens, 67,108,864 charged work units; Boolean query budget remains 16,777,216. Primitive inspection alone makes no applicability/composition/consistency claim; optional analysis now provides separate conditional checks.
- Checks — quantity primitives: native 60/60; combined authoring/CLI 16 passed, 1 optional desktop check skipped. Covers same starting values, distinct same-type actors, numeric grouping/comparisons/fractional thresholds, explicit unknown, missing/conflicting facts, mixed-role control lookup, typed enum/boolean references, units, finite/safe/integer constraints, bounds without clamp, complete-field syntax rollback and immutable source. Strict Zig C99 compilation passes for Windows/Linux changed modules; Linux execution untested. Staged under `native/build/review01-quantities/`. Read-only audit covers all 85 live rule records; temporary proposal is not an authoring format. Public-Attendant metadata adoption and readable body migration are now applied across both examples. Frozen evidence unchanged; no commit/push/experiment rerun/viewer interaction.
- Implemented outcomes: `gd_rule_analysis.c/.h` compiles explicit call maps, evaluates supplied typed actor/context assumptions, expands iteratively, collects simultaneous writes, resolves whole-case OTHER and checks explicit preservation conflicts. CLI adds `--case`, `--assume`, `--context`, `--context-not`, `--analyze`, `--fields analysis`; reports inputs, source paths, witnesses, unknowns and bounded coverage. Runtime arithmetic/type failures stay uncheckable even when bytecode matches. Unexamined invocations cannot manufacture unconditional preservation.
- Full migration: crate 12 records / 16 cases / 4 axes; plumber 73 records / 164 cases / 20 axes / 6 explicit calls. Preserve original IDs, statuses, dispositions, flat links, original prose, null inventories not explicitly extended and unresolved event/geometry/reset facts. Shell remains shellback state; spent block remains supply-block state; descriptor nouns retained. Death and later retry are separate occurrences. Public Attendant owns metadata; no projection edits or new authoring format. Templates/usages and guidance updated; no generators/content mirrors exist. Reusable guarded maintenance tool: `scripts/migrate-workbench-actor-rules.mjs`, staged copy before adoption and no-op rerun.
- Checks — integrated outcomes/migration: native 61/61; combined authoring/rule/quantity/analysis Node tests 22 passed, 1 optional desktop integration skipped. Includes direct/composed agreement, same starting facts, preservation conflict, source/fact immutability, typed assumptions/context, cycles, invalid numeric dependencies, reference/literal integer-safety parity, arithmetic failure, whole-field rollback and branch/pair/invocation caps. Strict Zig C99 compilation of changed application modules passes Windows/Linux; Linux execution untested. Test translation unit also compiles with existing vendor static-function warning exemption; Linux additionally needs POSIX feature macros and vendor unused-parameter exemption. Vendor sources unchanged. Source audit verified all 85 rule records / 318 original prose paragraphs, IDs/statuses/dispositions and all 405 original paths retained; 19 frozen result files plus frozen input remain byte-identical. Accepted specimen receives historical-checkpoint context without deleting original prose. Strict public Attendant: 67 / 222 records, no diagnostics/corrections. Artifacts: `native/build/review01-outcomes/`. Final source/link/whitespace checks pass. Migration plan succeeds; applied rerun changes zero fields/fences. Outside-project mutation collection rejected before changes. Full independent analysis review retrieved: invocation-limit preservation, runtime-error compatibility and C API input ownership/type findings fixed and regression-checked. Shared math review defects fixed; binary64 limitation explicit. Current model/rules/controls/analysis structs total 50,466,480 bytes before rule text/allocator overhead; no broad peak-memory/performance claim. No UX, Linux execution or physics acceptance claimed.

#### Review 02 — Resource roles and flows

Status: needs-refinement; direction supported by user. Depends on relevant effect contracts from 01.

- Result: existing canonical nouns participate as resources within associated systems; describe awareness, ownership/location, persistence, availability, gain/accrual, use/depletion, conversion and renewal. Include player-held pools and environmental opportunities.
- Interactions own effects; resource views expose flows without duplicating rules. Gap analysis must distinguish missing declarations, limited role diversity and pivotal connections; low degree is not automatically a flaw and high degree is not the definition of pivotal.
- Open: concrete schema/links, relationship to state/capability, source ownership, structural findings/evidence and CLI contract. No general rules engine or quality score.

#### Review 03 — Contextual skills and skill relationships

Status: needs-refinement. Uses 01; informs 04.

- Result: distinguish verbs from contextual skills and represent necessary prerequisites separately from alternatives, teaching sequence, contrast, transfer and combinations.
- User clarification: jumping is a verb, not the end of the skill chain; circumstances of deployment make distinct skills. Avoiding an enemy, defeating it by descending contact and applying the same understanding when falling are not interchangeable requirements.
- Preserve genuine AND prerequisites. Open: added relationships, understanding/expression/evidence guidance, analysis and examples.

#### Review 04 — Challenges, encounters and generative design guidance

Status: needs-refinement. Uses 01 and 03; relevant resource roles from 02.

- Result: connect design intent, player understanding, pressure, arrangement/timing, anticipated actions/inaction, feedback, recovery and subsequent contrasts/combinations. Data should help derive/design a level of Mario 1-1 quality, not merely catalogue parts or label teaching after the fact.
- Parked options: retain task/situation facets within challenges; separate encounters (arrangements) from challenges (goals/constraints); reusable challenge patterns with concrete encounter instances. No option selected; do not assume a new collection.
- Test with opening enemy/block/mushroom, pipes/pit and shell role-transformation sequences. Open: record ownership, links, useful analysis and bounded delivery slices.

#### Review 05 — Graph-query reference study

Status: needs-refinement; may run earlier to inform model contracts. Informs 08–10.

- Result: bounded, source-linked study of useful graph-query/analysis patterns, using Neo4j as a reference. Shortlist design questions, candidate algorithms, required data and interpretation limits.
- Investigate high traffic, convergence, linchpins, alternative paths, clusters/isolates and transformation/role patterns. Exact research question and source/deliverable bounds remain to refine.
- No Neo4j adoption, database migration or replacement of Attendant/Markdown implied.

#### Review 06 — Unified viewer navigation

Status: needs-refinement; independent base work can move earlier. Foundation for 09.

- Result: one tool with independently switchable data layer and view mode, preserving useful inspection context.
- Open: selection/filter/history retention, available combinations, incompatible-view behavior, draft/reload protection and shell integration. Preserve existing analyses and source ownership.

#### Review 07 — Viewer usability improvements

Status: needs-refinement; independent fixes can move earlier. Coordinate integration with 06.

- Result: matrix zoom, held-key repeat, clearer navigation/inspection and fewer avoidable edge crossings, including Little plumber MDA layout.
- Compare current matrix renderer improvements with ImGui integration. Open: concrete input/zoom/layout behavior, renderer choice, delivery slices and user-owned acceptance. No agent mouse/global-keyboard automation.

#### Review 08 — Expand analysis of existing data

Status: needs-refinement. Use 05 where relevant; do not wait for new schemas to expose current facts.

- Result: better structural inspection and gap-finding through shared CLI/viewer computation over already authored relationships.
- Assess current matrix/force-directed and other consumers before assuming data is missing. Open: supported relationships, scope, metrics, witnesses, omissions and acceptance. Do not infer runtime frequency, learning or quality from topology.

#### Review 09 — Analysis and views for expanded model

Status: needs-refinement. Depends on relevant contracts from 01–04 and viewer integration from 06.

- Result: actor/state navigation, state-specific ghosts, conditional comparisons, shared outcome nodes, resource flows and role changes in shared analysis and visual inspection.
- Select actor and move through its states to reveal relevant partners; alternatively select actors and show ghosts for states in which they interact. Preserve underlying actor identity. Include resource gap/pivotal-connection evidence.
- Open: projections, filters, density controls, CLI contracts and per-capability delivery slices. Do not draw every combination or add executable simulation by implication.

#### Review 10 — Predefined design-query catalogue

Status: needs-refinement. Uses 05 and available capabilities from 08/09.

- Result: named, reusable design questions for high traffic, linchpins, alternatives, transformations and other selected patterns, with matching CLI/viewer results.
- Return supporting relationships/paths, conditions and incomplete-data limits, not unexplained scores. Distinguish structural importance, role variety and player-experience value.
- Open: exact query set, parameters, algorithms, bounds and acceptance. Catalogue implementation is separate from reference research.

#### Review 11 — Higher-dimensional projection layouts

Status: needs-refinement. Uses selected feature/relationship contracts and viewer integration.

- Result: inspect associative data through an arbitrary number of selected dimensions projected into 2D.
- Parked candidates: UMAP, t-SNE, PCA and MDS; no algorithm/dependency selected. Keep separate from ordinary force-directed layouts.
- Open: feature representation, weights, missing/unknown values, interpretation, reproducibility, implementation/dependency constraints and useful-inspection acceptance.

Commits require explicit user approval; user approved the pre-implementation checkpoint and completed formula slice. No experiment rerun, control adoption, UI automation or publication implied.

### Deferred — control-variable JSON files

Status: draft; explicitly deferred by user.

- Store control variables in JSON files rather than Markdown files, per user correction. Record for future work; do not implement this format migration in current slice.

### Final gate — review committed branch assets before push/completion

- [ ] Complete user-owned manual interaction review of delivered native views after remaining delivery slices and World 1-1 example. Include MDA node/edge selection, source navigation, root/direction changes, query/witness filters, pan/zoom/drag, and layouts; skill draft edits, undo to loaded baseline, dirty-refresh protection, and successful/failed JSON handoff. Include controls added by later slices. Keep Slice 2 acceptance closed; this is integrated final acceptance, not reopening its delivery gate. No agent mouse/global-keyboard automation. Continue implementation until this gate; do not pause each slice for these deferred checks.
- [ ] Review all assets committed on `feat/game-design-workbench` before any push or declaring this task complete. Cover full committed branch state and branch history, not only the last commit or current working diff.
- [ ] Check code/tests, instructions/guides, templates/examples, source captures/data, reports, and other committed files for relevance, correctness, provenance, rights/publication limits, and accidental private or unrelated content. Validate the final committed snapshot; report coverage and unresolved findings.
- [ ] Resolve blockers or obtain explicit acceptance of remaining limits before completion. Local-only/not-publication-cleared references need a publication disposition before push. Removing a file from the final tree does not remove it from earlier commits; any history rewrite requires separate approval. Push still requires explicit user approval.

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
- Honor user-owned retention/deletion of alternatives; verify inactive ideas do not act as accepted rules and later generation consults relevant retained rationale.
- Resolve alternate terms to canonical records without creating divergent duplicates; test ambiguous aliases.
- Change one canonical definition and verify all dependent views use updated source without manual duplicate edits.
- Change absolute and relative control values; verify resolved CSV/INI/JSON exports and simulation inputs agree.
- Generate a small C99 interaction simulation from supplied prose, recording assumptions and using current controls.
- Demonstrate authorized Git checkpoint and comparison/recovery workflow without a document-level edit log or loss of unrelated changes.

## Historical implementation evidence

These checks describe earlier deliveries, including retired runtime. They are not current commands or requirements. Git retains deleted code; historical reports/baselines remain unchanged. Current checks belong to workflow simplification section below.

### Initial implementation checks — dependency packaging superseded

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
- Step 5: packaged **227 project-local tool/reference files** into tutorial using guarded setup. Initial reviewed proposal: 229 file changes, 19 reused collections, no new collections/conflicts/candidates. Field-selector bug found during direct use; fixed matching column metadata and unknown fields on empty results, then applied guarded two-file package update.
- Final integrated run: **23/23 passed** (**16 workbench + 7 existing Attendant tests**), Windows/Node v24.16.0. Delivered folder copied outside repository: local-only validation, populated queries/aliases/catalog, unchanged sources, and selected rank update all passed. Only selected record changed; body/immutable metadata/other authored ranks preserved.
- Source and vendored references: **61 local links pass**; frontmatter and generated-pack checks pass. External access warnings remain as recorded above.
- Saved scoped tutorial skill-atoms interpretation: `20261020T030047047Z-3af7318c`; one agent finding, partial coverage, no repair/simulation. Reopen confirms matching current source, historical status, and `freshly_run: false`. Setup re-plan reports zero changes/conflicts.
- Step 6: offline review report at `reports/game-design-workbench-slice-1.html`; machine verification, final 23-test log, browser result, and original demo setup proposal retained in `reports/`.
- Report static check passes: six sections, no charts, no packaging errors. Headed Chromium with HTTP(S) blocked passes anchors/direct hash/history, progress, theme, keyboard focus, mobile 320/390 px, reduced motion, JavaScript-disabled content, print CSS/PDF generation. Zero HTTP requests/browser errors; desktop/mobile screenshots inspected. Not an exhaustive accessibility/PDF-pagination audit.
- Reusable report browser audit: `scripts/check-offline-report.mjs --help`. First audit hit disabled-page callback timeout; changed waits to Node timers and reran successfully on final HTML. Preview left open.
- Fresh Git-index export passes setup idempotency and saved-review source matching. Fixed source vendor index/working CRLF mismatch by preserving existing bytes; no vendor logic changed. Strict owned-file whitespace check passes; default check's vendor CRLF warnings are not silently rewritten.
- Slice 1 complete for review. Slices 2–5 remain unimplemented. No merge/push/publication performed.

### Dependency correction

- User correction: Attendant is another required skill, not vendored infrastructure. Revision authorized; previous standalone-without-Attendant claims are superseded.
- Remove canonical/example Attendant copies. Require loaded skill directory via `GD_ATTENDANT_SKILL`; fail before writes when absent. Call public runner only; compare schemas through public API. Fixture generator uses JSON-compatible YAML without private parser imports.
- Fingerprint installed dependency separately. Preserve old reviews as historical/stale; capture fresh scoped review after package update. Guard legacy managed-file deletion; block local edits and preserve unknown files.
- Dependency regression: 27/27 tests pass (20 workbench, 7 Attendant). Package contains 219 files, no vendored Attendant. Report refresh remains.
- [x] Final documentation step: reviewed/expanded all 18 framework docs to 803–925 words each. Added working concepts, scoped procedures, worked examples, evidence/counterchecks, and traps. Preserved original prompts, source-access limits, selected-lens routing, and no-instrument/no-score boundaries. Maintainer generator validates coverage; retired one-line fixture fields removed.
- Final source/package checks: 27/27 tests pass; 18/18 generated packs current; frontmatter passes; 231 local links pass; entrypoint 797 words; 219 installed files; zero setup changes/conflicts after update. No new source fetches; eight original HTTP 403 warnings retained.
- Handoff refreshed: corrected scoped review `20261020T091047165Z-8da3234e` matches current source. Original run remains unchanged/stale; finding IDs stay stable. Fresh-index export passes setup idempotency and current/stale review checks.
- Added reusable `scripts/check-workbench-export.mjs`; use `--help` for staged-export workflow. Verified JSON/text success, usage failure, and missing-dependency failure. No stage/commit/install side effects; private export removed.
- Offline report/browser checks pass again: six sections, zero HTTP requests/errors, desktop/mobile, history/progress/theme/focus, reduced motion, no-JS, print generation. Eight original citation-access warnings retained, not relabeled as a new URL check.
- Correction commits: `d2d028d` dependency boundary; `b755438` expanded guides. No merge/push/publication. Original worktree remains `dev` with its original untracked files.

### Collection naming correction

- Rename domain collection/role/query to `skill-atoms`, including paths, schemas, references, glossary documents, templates, examples, model contracts, and selected-framework guidance. No old query alias.
- Preserve agent-skill install paths and published GameFlow theme name. Keep frozen reports/baselines unchanged; old paths remain historical evidence, not active configuration.
- Preserve all 68 record UUIDs. Explicit fixture identities keep regenerated records stable after collection and glossary-path renames.
- Source verification: 27/27 tests pass; 231 local links pass; all 18 generated packs current; all 68 UUIDs match pre-rename commit. Fixture regeneration preserves complete UUID set. Old query is rejected; legacy manifests stop with `COLLECTION_RENAME_REQUIRED` rather than creating duplicate source.
- Maintainer rename tool is idempotent. Guarded example update reuses all 19 collections, with zero final changes/conflicts.
- Handoff refreshed: current review `20261020T192957597Z-4698820e` uses renamed paths and matches current source. Both earlier runs/baselines remain unchanged and stale. Staged export passes; finding IDs remain stable.
- Current offline static checks pass: six section anchors, no remote resources, unchanged script/style bytes. Headed-browser recheck blocked because agent-browser CDP discovery exceeded 100-second and 90-second command limits. Prior browser results remain explicitly historical, not relabeled as current.
- Source rename committed as `0f7f433`; no merge/push/publication.

### Model/documentation feedback pass

- Model 2: consolidate hypotheses, expectations, and observations into experiments; current templates and fixture migrated. Record stage is extensible text, not a prematurely fixed transition engine.
- Preserve original planning-hypothesis UUID as experiment; embed timing criterion with original UUID, threshold, rate, and authored nulls. Do not conflate human-planning investigation with assumed-delay timing model. Current fixture has 67 records across 16 collections.
- Runtime prerequisites have one reference owner. Shared model/authority rules load before operation branches; collection usages keep local meaning only. Framework guides no longer link back to already-read review parent or repeat shared boilerplate.
- Example README surfaces unauthored overall experience/player story separately from local MDA aesthetic. No new game goals or player voice invented.
- Verification: 27/27 tests pass; 219 local links pass; frontmatter and pack generation checks pass. Staged export verifies 16 collections, two proposed-stage investigations, six skill atoms, unchanged setup, and historical/stale reviews with stable finding IDs. 67 original UUIDs retained; merged criterion UUID/values/nulls preserved in experiment provenance.
- Current implementation commit: `087021c`. Fresh evidence lives in `reports/game-design-workbench-model-2-*`; earlier handoff/report evidence remains unchanged and historical. No new framework review, experiment execution, viewer, or browser audit performed.

### Concept/mode verification

- 30/30 tests pass on Windows / Node.js 24.16.0. Existing/new concept setup, mode switches, drafts, exact historical citations, and read-only review behavior covered.
- Staged export passes: 61 local file-link targets, 16 collections, six skill atoms, two proposed investigations, explorer context, no inferred original intent, and zero setup changes/conflicts. Installed snapshot has 205 files. No domain records changed.
- All 18 generated packs current; both canonical migrations remain no-ops. Framework completeness remains provisional, not established by these checks. Earlier reviews stay unchanged/stale; no new framework review, simulation, viewer, or browser audit.
- Evidence: [verification](reports/game-design-workbench-concept-verification.json), [tests](reports/game-design-workbench-concept-tests.txt), [staged export](reports/game-design-workbench-concept-export.json). Supplied root concept documents remain unchanged/untracked; not adopted or committed.
- Next: framework-by-framework completeness review before Slice 2.

### Collection cleanup verification

- Model 3: 14 collections, 63 records, 189 installed files. `control-vars` replaces controls in roles, fields, references, formulas, queries, templates, and fixture. No old query alias. Local measures live in experiments; no workbench decisions collection or mandatory archive.
- Existing pulling/crate-color proposals moved to rejected verb/proposed mechanic records with identity, rationale, and provenance intact. All 63 surviving UUIDs match `82a97f6`; four measure UUIDs retained beside embedded definitions. No new game facts or executed evidence.
- 31/31 tests pass, including custom ignored statuses, independent project decisions preservation, legacy update gates, hyphenated field selection, and refreshed installed example. 223 local links/anchors and skill frontmatter pass; all 18 generated packs current. Framework completeness remains provisional.
- Canonical cleanup helper ran guarded apply, then no-op check. Both earlier migration helpers remain no-ops. Updated example setup and staged export have zero pending changes/conflicts. Export verifies 14 collections, seven control variables, six skill atoms, two proposed investigations, and unchanged/stale historical findings. No new report files generated; existing reports/baselines remain historical and unchanged.

#### Example concept follow-up

- Filled tutorial concept with candidate purpose, player/appeal, intended experience, hypothetical player story, play over time, linked features, constraints, and open questions. Kept `game-explorer`; no platform, market, or playtest evidence invented.
- Generator now copies this authored concept instead of blank setup template. Updated MDA relationship and installed snapshot; no new modules or reports. Generator/installed-example tests pass (2/2), 11 local links pass, setup remains idempotent. No full-suite rerun for this follow-up.

## Workflow simplification verification

- Removed six runtime modules including `lib.mjs`, wrapper CLI/projections/report storage/Git parser, installed seven-module example copy, three completed migration programs, legacy export checker, and document generator/content mirrors. Removed tests of retired interfaces.
- Removed remaining template/example generator and both JavaScript data mirrors. Shipped templates and example Markdown own their respective content. No `scripts/` directory remains in skill; `test/authoring.test.mjs` validates real assets through public Attendant. Repository `check-offline-report.mjs` remains historical HTML verification, not skill runtime.
- Checked installed copy before removal: all 189 files matched manifest; no local extras/edits. Canonical 63 records, authored concept, contract field definitions, and reports/baselines preserved. Removed obsolete generator notices from 28 schema bodies; frontmatter unchanged.
- Current checks: 3/3 tests pass against real assets and ordinary temporary copies. External Attendant validates all 14 copied empty template collections and 63 example records/unique UUIDs; tests cover null rank, proposals, and rank-only update preservation. Ordinary Git index export passes same 3 tests. Without dependency, 2 pass/1 integration skip. Skill frontmatter and local links pass, including this record. No active generator imports/commands/notices remain. Earlier 4-test generator-based results are historical. No new report generation or framework review.

## Framework references and card-table checkpoint

- 13 framework source folders retain full reading copies alongside independent notes, source metadata, and documented rights/conversion gaps. Root reference index maps HTML originals to readable Markdown.
- 13 framework guides expanded; five book-based guides remain provisional without equivalent expansion. No claim of completed manual framework review or release readiness.
- Schell adds `references/schell/deck-en.json`, exact raw JSON supplied by user, and `deck-table.md`: internal index, exact title, functional use note, description, question list. All 116 titles/indices/notes and 451 ordered questions checked. Table flattens layout whitespace; raw JSON preserves supplied strings/formatting. No new fetch needed for conversion.
- Added Git attributes to preserve reference bytes and treat current PDF/image formats as binary. All 381 reference files match staged blobs byte-for-byte; no automatic newline conversion of raw evidence or supplied JSON.
- Validation: 3/3 authoring/public-Attendant tests pass; skill frontmatter and local entrypoint/guide/workflow/reference-index/task links pass. Authored-change whitespace checks pass. Full-source whitespace check reports 22,348 formatting findings in preserved capture/reference material; do not rewrite original evidence merely to satisfy whitespace lint.
- User explicitly selected complete local checkpoint including references, about 74 MB, after scope/rights review. Retain local-only/not-publication-cleared warnings. No merge, push, or publication. Exclude unrelated root `Concept - Template.md` and `High Concept Doc.md`.
- Schell card-reference task complete. Framework guide review/deeper book-based application work remains; then Slice 2 C99 source core. Control-variable import formats and merge/overwrite behavior remain unresolved Slice 3 scope.

## Native matrix verification (earlier checkpoint)

- Run from worktree root: `powershell.exe -NoProfile -ExecutionPolicy Bypass -File skills/game-design-workbench/native/test.ps1 -Graph -RaylibDirectory skills/game-design-workbench/native/.cache/raylib-5.5_win64_mingw-w64`. UI DLL is explicitly prepared with `build-ui.ps1`; SDK is local ignored input, not a shipped requirement for headless operation.
- Run authoring integration with `GD_ATTENDANT_SKILL=C:/Users/steve/.pi/agent/skills/attendant` and `GD_NATIVE_EXE=D:/Repos/agent-skills-game-design-workbench/skills/game-design-workbench/native/build/gd.exe`: `node --test skills/game-design-workbench/test/authoring.test.mjs`. Crosscheck uses public queries on temporary copy; source fixture remains untouched.
- Conventional compiler command is in `native/README.md`; checked Windows Zig 0.17.0 with `-std=c99 -Wall -Wextra`. Both command outputs match CJIT JSON for shipped fixture.
- Inspected bounded PNG: matrix selects crate × push, retains three cases, shows status/coverage, labelled links and Markdown. Fixed raylib 5.5 `TakeScreenshot` discarding directory components by explicit image encoding and UTF-8-aware file output; checked actual Unicode destination. Fixed paragraph wrapping to account for complete rendered string width.
- Owner confirmed copied proto headers are MIT and freely reusable. Exact bytes/hashes and notice retained; build/thread/JSON helpers not copied without a current use. cimgui graph extension now explicitly rebuilds that backend source against matching raylib 5.5 SDK/runtime, with retained license/provenance and bounded captures; no prebuilt prototype DLL reused.
- Remaining coverage: interactive controls/clipboard/live refresh, full large-project rendering timing, and non-Windows execution. Graphics-free layout capacity probe measured 4,096 nodes/32,768 edges for ten fixed steps in 20.0 ms under Windows/CJIT; this is not a renderer timing or universal performance guarantee. No rank editing, skill/route/impact analysis, simulation, commits, or push claimed by this slice. Whole-branch rights/history review still applies to retained reference assets.

## Native skill verification (earlier checkpoint)

- Same native/authoring commands above now cover skill consumer. Latest Windows CJIT and Zig C99 native checks: 14/14 each. Authoring/public-Attendant integration: 3/3, with native skill identities/status/rank/prerequisite/mechanic crosscheck and fresh recomputation after temporary Attendant coordinate rank update.
- Windows Zig `-std=c99 -Wall -Wextra` headless build passes without warnings. CJIT/Zig JSON parity checked for matrix, connectivity, skill/context results, field selection, argument failure, and unavailable headless view. Explicit text overrides agent/CI JSON defaults.
- Final bounded captures under `native/build/space 雪-44bbed73dbab4dd286d49abf9d3d5048/`: matrix table/force/columns/empty; skill prerequisite/rank in both directions; incomplete/cyclic and empty skills. Inspected default chains, rank/late-root layout, and retained cycle/self-loop arrows. Shared-axis lock is also tested headlessly before/after held-node settling.
- Live desktop refresh run: `native/test-refresh.ps1`, matrix and skill views, artifacts `native/build/refresh-6c0dd7d1314a4c2fa39049d182fb9298/`. Saved 14 client-area captures; inspected baseline, detected-stale snapshot, refreshed valid state, retained last-good state on invalid input, and successful restored baseline. Matrix free-push coverage defined→unknown; skill coordinate authored rank 2→4 clears conflict, then restoration returns authored 2/display 3/conflict. Source/private-copy SHA-256 matches after restoration; owned processes exit 0. Targeted F5 messages exercise live window input, not physical hardware. Screenshot states were visually inspected, not automatically OCR-asserted. Canonical records and user clipboard untouched.
- Rank editing, challenge routes, font replacement, and Neo4j investigation remain deferred. No shipped example records changed. Native files and integration/docs remain uncommitted; no push or publication. Whole-branch rights/history gate remains open.

## Native draft verification

- Implemented `native/gd_draft.c`: concrete skill-only in-memory edits, baseline-borrowed text, shared recomputation, modified-field tracking, bounded history, and report delivery. No generic CRUD, new dependencies, or authoritative Markdown writer.
- Native domain/layout/draft checks: 18/18 on Windows CJIT and Zig C99. Includes 4,096 undo actions without eviction; 32,768 relationship boundary; duplicate declaration restoration; exact/null ranks; cycles; three-way reports; source identity/unavailable errors; existing-file rejection and failed-output draft retention. Production headless Zig `-Wall -Wextra` clean; test build reports unused functions in unchanged vendored proto header.
- `native/test.ps1 -Graph` passes CLI, stdout/file JSON, Unicode/space destinations, empty/partial/cyclic cases, and bounded matrix/skill captures. Explicit `--output` is skill-view-only and `.json`-only; skill view report is JSON-only, not a new source-update CLI.
- Draft interaction acceptance remains pending user pass at step end. Earlier injected-input probes produced screenshots/reports but intermittently missed clicks; not reliable final acceptance evidence. Removed `-Draft`/`-ReportFile` automation and all pointer/global-keyboard injection from `native/test-refresh.ps1`. Keep code tests, bounded screenshots, private external data edits, and owned-window F5 only. Earlier private inputs restored by SHA-256; canonical examples/clipboard untouched. No live viewer left running.
- Authoring/public-Attendant integration: 4/4, including actual viewer JSON applied explicitly by test consumer to another private copy after matching current-disk values/ID. Only requested record fields change; body, other records, and descendant null rank remain intact; fresh native findings agree with draft report. Set `GD_NATIVE_DRAFT_REPORT` alongside existing Attendant/native paths to repeat handoff check.
- No commits, push, source publication, broad editor, source-writer implementation, or Mario example creation. World 1-1 remains Slice 6 before whole-branch final review.

## Native route verification

- Delivered `gd scaffolding` over explicit skill/challenge collections. `gd_routes.c` uses shared loaded records/prerequisite analysis, bounded iterative closure/two-state BFS, source-backed checks, and route witnesses. No GUI projection, source writes, new dependencies, or automatic repairs. Only earlier introduction/practice counts as support; requiring/mastery-testing stays a demand.
- Native tests: 22/22 under Windows CJIT and Zig. New checks cover seeded opportunity roles, duplicates/all statuses, transitive demands, unknown references/entries, unreachable fragments, strict entry types, 96 small graphs against independent reachability oracle, 32,768-check success/overflow, witness/path/finding caps, and result/scratch allocation failures. Existing 18 tests still pass. Zig production C99 `-Wall -Wextra` build clean; graph-enabled CJIT build compiles without launching a window.
- Headless CLI checks pass: crate example 4 challenges/1 entry/10 checks/12 witnesses; combined-room planning absent, switch some routes, two single-shot overlaps. Exact bypass path and practice path asserted. JSON/text/field selection, empty inputs, missing required roles, and Unicode/space folders pass. No pointer or global-keyboard interaction used.
- Authoring/public-Attendant checks: 4/4, including independent challenge IDs/statuses/entry/role/next arrays against public queries on private copy. Rank-only Attendant update clears rank conflict without changing route checks/opportunity findings. Existing actual-report private-copy handoff still passes. No shipped example records changed.
- Skill frontmatter and 103 local links across entrypoints/native guide/inspect workflow/skill-atoms guide/task pass validation. Manual draft UI pass remains pending; MDA, controls, route GUI, Mario example, and final whole-branch review remain outside this increment.

## Comments

- Route verification: default Zig cache referenced missing object files; rerun succeeded using isolated ignored `native/build/zig-global` and `zig-local` caches. No shared-cache deletion or tool installation.
- Unrelated untracked `skills/unity-cli/` remains untouched. Starting brief and supplied framework reference remain untracked.
- Specification checks: 8 local links and 2 HTTP(S) URLs passed bundled skill-craft validators; whitespace check reported no errors. These are documentation checks, not workbench execution evidence.
- Implementation authorized after refinement; current work isolated from original worktree. No task-management collection created.
- First-slice engineering choice validated by local/copy/checkout tests: queries reuse Attendant SQL/Node runtime rather than duplicate domain analysis in `.mjs`. C99 analysis remains in later agreed slice; reviewer can assess bootstrap usability separately.
