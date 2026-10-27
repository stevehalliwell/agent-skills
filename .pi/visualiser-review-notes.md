# Visualiser review notes

User feedback collected before deciding what to change. No implementation, priority or commitment implied. Clarify as needed during capture.

## Observations

- Purpose clarification: workbench data should support deriving/designing a level as good as Mario 1-1, not merely cataloguing an existing level or checking whether its objects/rules have records.
  - Assess what interactions, aesthetics and design philosophy are missing that would steer a designer toward that quality of level.
  - Consider missing categories of data, relationship types and analyses that could yield the information needed to arrive at such a level. Do not assume the answer is only more example records or more viewer features.
  - Evaluate the previous observation-by-observation comparison as evidence of generative design gaps, not only documentation coverage gaps.
  - Quality is the design goal, not a mechanically guaranteed result or a score inferred from graph richness. Concrete representation, analysis and implementation choices remain open until feedback review.

- Kicked shells exist: `interactions/kick-resting-shell.md` launches a resting shell through `mechanics/shell-travel`; related cases cover hitting enemies, wall reversal and danger to the hero.
  - User wants this described as transforming an enemy into an ally. Capture the role change explicitly during later design review, not only shell motion/contact mechanics.
  - Critical clarification: user frames player action as transforming the enemy into a non-enemy/ally before it can kill enemies. Shell attacks therefore must not be counted as evidence of rich enemy–enemy interactions merely because the shell originated as an enemy.
  - Distinguish current interaction role from entity origin, and distinguish that role from whether contact can harm the player. A transformed ally/non-enemy can remain hazardous to the player; that does not by itself make it an enemy again.
  - Representation and analysis must preserve this role transition so comparisons of enemy–enemy interaction richness are not misleading. Exact modeling approach remains undecided; no dataset change made during note capture.

- Interaction asymmetry and design-complexity exploration:
  - User contrasts rich player–enemy interactions with comparatively simple enemy–enemy interactions, describing most enemies as rebounding off one another. Capture as user-supplied Mario observation, not verified behavior or an adopted example rule.
  - Visualisation should help a designer notice comparatively sparse, uniform or repetitive interaction areas and consider richer/more varied alternatives. Do not treat sparsity as a defect or imply all entity pairs should follow the same rules.
  - System should help assess how much additional interaction complexity a proposed change introduces. Exact measures and comparison workflow remain undecided.
  - More interactions or a parallel intricate interaction web do not establish a better player experience. Visual analysis and framework lenses should help examine whether added complexity serves the player, without presenting structural richness as a quality/fun score.

- Interaction representation must answer “what happens when A + B?” and preserve state-dependent differences.
  - User identifies two possible approaches: keep interactions flat by representing big and little Mario as separate nouns; or retain one noun and support “A + B, but A is <condition>.”
  - Collection capabilities and authoring guidance must align: if separate state-specific nouns are discouraged or unsupported, conditional interactions must be supported explicitly.
  - Neither approach selected. Retain this as a representation/authoring question, not merely a viewer filter or prose-display issue.

- General requirement behind the death example: represent concepts so visualisation and returned analysis can expose shared destinations, converging paths and highly connected outcomes. Death is the first example noticed, not a special-purpose feature request.
  - “High traffic” here concerns many authored connections/routes reaching a concept, not measured runtime frequency. Preserve distinct routes and their conditions rather than equating connectivity with likelihood.
  - If current concept representation cannot support this diagramming/analysis, expand or adapt collection structures and/or workbench guidance. Do not constrain the design to what current consumers happen to expose.
  - Earlier form/hazard comparison is one application of this broader need. Concrete representation and analysis measures remain to review after note capture; no implementation started.

- Desired visualisation and analysis: distinguish lethal outcomes by player form and hazard. User wants to see that death is common for small form but rare for big form, while pits kill both forms; pits could therefore rank as the hardest enemy/hazard under some measure.
  - Need visual presentation and returned analysis data, not only rules buried in prose.
  - Distinguish prevalence of lethal authored cases/vulnerability from observed death frequency. No encounter probabilities or play measurements supplied; do not infer actual rates from case counts.
  - Candidate comparison: lethality across player forms. Exact meaning of “common,” “rare” and “hardest,” and chosen measure, remain open for later review.

- Resource modeling clarification:
  - A resource is always an existing game noun and must have an associated system.
  - User proposes resource records should point to those noun/system records rather than establish separate identities.
  - Describe player awareness of the resource, where it participates, and how it is gained, accrued, spent and depleted.
  - Keep the noun/system requirement distinct from the proposed resource-record shape. Exact fields and implementation remain undecided until feedback review.

- Resource definitions need improvement. User distinguishes:
  - Player-held resources across moment-to-moment and meta layers: coins, time, lives and “bigness” (power/form). User calls these permanent resources; exact persistence across layers remains to clarify during review.
  - Moment-to-moment environmental resources: number and types of enemies and blocks, because these provide different interaction opportunities. Resource framing should not be limited to player-held counters.
- Shell-jump/reward note, supplied by user: big jump off a shell; enough jumps off a shell award a coin for stunting; enough coins award a life; player can obtain effectively infinite lives by continuing the process. Preserve this account for later review, not as verified behavior of Mario or adopted rules for the fictional example. Exact jump/reward conditions and thresholds remain unspecified.

- Interaction matrix review:
  - Need zoom in/out.
  - Keyboard navigation should repeat while a key is held, not act only once per press.
  - Consider implementing matrix directly in ImGui; compare with current implementation before choosing.
  - Need to switch back and forth between interaction matrix and other visualiser views. User suggests these probably belong in the same tool; integration approach remains undecided.
  - Need both grid and force-directed node views, using attraction/repulsion to expose relationship shape rather than only grid hot spots. Whether existing graph mode meets this need remains unreviewed.

- Little plumber example, MDA layout: layout should try to avoid connection-line crossings where possible.
- Little plumber example: potentially missing skill atoms or related design coverage. User raised:
  - Jumping onto something versus running into it.
  - Head-butting from below versus landing on top.
  - No fall damage.
  - Hidden/invisible blocks.
  - Some pipes transport the player.
  - Some pipes contain enemies whose timing the player must read to avoid them.
  Classification and whether existing records already cover each point remain unreviewed; do not assume every point needs a new skill atom.
- Run should be a separate verb, not only a walk-speed modifier. User notes that running also changes air control and creates relative-speed breakpoints: what the player can outpace while walking versus running. Current `verbs/walk.md` folds run into horizontal movement; capture requested distinction without implementing it yet.
- Potentially missing system: proximity-triggered world/encounter activation. User describes Mario's world as not starting until Mario approaches, preserving designed encounter sequences regardless of time spent reaching them. Retain as user-supplied observation about Mario and a design point to consider for this example; exact activation boundaries, reset behavior and exceptions remain unspecified and unverified.

## Mario observations compared with current example

User supplied these observations to test coverage, not to authorize dataset changes. Comparison checks authored Little plumber records, not historical accuracy of the Mario observations.

1. Inaction kills on first screen; jumping over suffices. Partial/different: `first-walker` permits jumping past an approaching walker, but `start-apron` gives safe movement before that encounter; first-screen idle death from the enemy is not established.
2. Opening optimized to jump first enemy and head-hit a block, revealing physics, small-form brick limits, coin and mushroom rewards. Individual rules exist (`solid-collision`, `brick-response`, `supply-release`, `growth`); the coordinated encounter geometry/timing and instructional rationale do not. `first-walker` and `first-supply` are separate sequential challenges.
3. First mushroom bounces back toward an idle player. `growth` defines wall reversal; first-supply does not specify placement, travel speed or timing ensuring this demonstration.
4. Easily jumped enemies between large pipes demonstrate mutual rebound, not damage. `pipe-steps` defines pipes/landing/entrance, not that enemy arrangement or enemy–enemy rebound rules.
5. Invisible blocks, distinct extra-life mushroom, question whether mushrooms are always good. Concealed supplies and life-cap exist, including `bump-life-supply` and brick-canopy placement. No explicit learned generalization about mushroom benefits; hidden-block collision details remain incomplete.
6. Enemies fall into pits; pits kill hero regardless of size. Walker falling is allowed; every-form hero gap death is explicit. No staged enemy-fall demonstration.
7. Falling straight down onto enemies teaches directional contact. Descending top-contact rules exist and do not require an intentional jump. Specific repeated drop-onto-enemy teaching arrangements are absent.
8. Low-action turtle sequence accidentally demonstrates kick and enemy hit. Shellback-to-shell, automatic side-contact kick and shell defeating walker exist; shell-lane does not arrange an incidental chain or explicitly capture the enemy-to-ally role framing requested in feedback.
9. Raised block section shows camera does not scroll up and safe travel above viewport. Not documented: camera covers forward scrolling/left boundary, not vertical locking or safe top-of-screen behavior.
10. Big-form destruction is fun and sometimes reveals treasures. Breaking by power state exists; fun is not specifically documented as a destruction experience goal or observed result. Ordinary bricks explicitly contain no rewards; concealed supplies nearby are not treasures revealed by destruction.
11. Enticing star followed by hard enemy row, allowing powered player to land among and kill enemies. Missing star, its temporary invulnerability/contact-defeat rules, and corresponding encounter staging. Existing damage-recovery safety window is not that power.

Coverage lesson: having individual rules is not equivalent to recording how layout, timing, likely actions, feedback and contrast expose those rules to a player. Distinguish intended teaching opportunity from evidence that learning occurred. No fixes or representation choices made during this comparison.

## Continued review — skill chains, challenges, encounters and projection

- User requests viewer clustering around an arbitrary number of axes, flattened algorithmically into 2D for inspecting higher-dimensional associative data. Algorithm name not recalled. Assistant suggests UMAP/t-SNE and distinguishes PCA/MDS as alternatives; no algorithm chosen.
- User places situation/pressure within skill atoms and challenges. Separate avoiding enemies by jumping over from defeating eligible enemies by descending onto them; other-direction contact can kill the player under applicable form/protection rules. These are distinct responses/understandings, not necessarily one prerequisite chain.
- User asks whether missing design/teaching relationships should be expressed through richer skill-atom chains or elsewhere. Current schema has prerequisites, rank and mechanic references; body template already includes understanding, expression, learning loop and evidence sought.
- User requests further exploration of how challenges and encounters are related and whether they differ. Ground in both user observations and preceding assistant proposals: intent, pressure, arrangement/timing, likely actions, feedback, recovery and subsequent contrasts/combinations. Distinction and representation remain open.
- Follow-up captured under Slice 6 in `todo.md`. Continue ideation; no schema, dependency or implementation decision made.
- User clarifies: jumping is a verb, not the end of a skill chain; circumstances in which the player deploys it make distinct skills. Preserve this distinction when reviewing existing broad jump skill records.
- Park algorithms and challenge/encounter options for later discussion. Algorithms: UMAP/t-SNE, with PCA/MDS as alternatives; no selection. Representation options: task/situation facets within challenges; separate encounters (arrangements) from challenges (goals/constraints); reusable challenge patterns with concrete encounter instances. No selection.
- Continue the five-area review with area 2, interaction representation and analysis. Expansion is exploratory, not approval to modify schemas, example rules or native consumers.

## Interaction proposal — actors, state transitions and resource effects

- User proposes thinking in terms of objects/actors, the states they transition to during interactions, and resources exchanged or created by interactions.
- User's illustrative transitions: falling into a pit makes player dead; landing on a goomba makes that enemy dead; landing on a turtle makes it hiding; landing on a hiding turtle makes it a kicked turtle and adds jump height to the player. These are candidate design examples, not verified Mario behavior or adopted changes to the existing shell rules.
- Purpose: more expressive interaction representation, shared outcomes across many causes, and death visible as a node in graph views.
- Review needs: preserve which participant changes state; allow several effects on different participants; distinguish shared state meaning from actor-specific state; retain direction/form/other conditions. Resource flows can accompany transitions; jump-height changes need not automatically be classified as resources. Representation remains exploratory.

## Resource review direction and gap analysis

- User supports implementing the resource model discussed as an improvement: existing noun plus associated systems; player awareness, ownership/location, persistence, availability, acquisition, use/depletion, conversion and renewal; environmental opportunities as well as held pools. Interactions own effects; resource views expose flows without duplicating rule authority. Concrete fields and implementation remain to refine.
- Additional requirement: like every other view/analysis CLI, resource views and analysis must help find gaps, including resources serving few roles and connections that are particularly pivotal.
- Review distinction: missing declarations, low role diversity and structurally pivotal connections are different findings. Low connectivity is not automatically a flaw; pivotal connections need not have high degree. Preserve context, conditions, incomplete-data limits and supporting paths rather than infer quality or play frequency from graph structure.

## Example review direction

- User supports area 4 approaches A (develop connected sequences in depth), B (make their design contributions recoverable) and C (use examples to expose representation/guidance/analysis gaps).
- C should also point outward toward possibilities beyond World 1-1 that occur elsewhere in this or other Mario games. Treat these as exploration of the design space, not an automatic requirement to implement every feature or make a complete Mario catalogue.
- Sequence work: improve the other discussed workbench areas, then revise examples to match. Some missing content will be filled naturally while exercising improved interactions, resources, skills and design guidance. Do not make exhaustive example gap-filling a prerequisite for those improvements.
- Preserve distinction between an actual model/tool limitation, incomplete authored design and an optional extension. No new game rules, schema choices or implementation details settled by this clarification.

## Viewer work split and actor/state inspection

- User identifies an actor-oriented view: select an actor and move through its states to inspect which other actors it interacts with in each state. Alternatively select a set of actors and show state-specific “ghosts” for each state in which they have an interaction. Preserve association with the underlying actors; exact presentation and state representation remain to refine.
- User explicitly splits viewer/analysis work into several tasks:
  1. Combine tools so data layer and view mode can be switched freely while retaining some context.
  2. Make the tool more user-friendly.
  3. Expand visualisation and analysis over data already present.
  4. Expand visualisation and analysis as new data types and connections are added, including actor/state inspection.
  5. Provide predefined graph queries for high-traffic, linchpin and related patterns. Investigate Neo4j as a reference for available/useful graph querying; this does not select Neo4j as project storage/runtime.
- Exact context retention, task boundaries, query semantics and implementation order remain to refine. Distinguish structural traffic/importance from runtime frequency or player-experience quality.

## Raw-text usability constraint

- User expects agents to operate on much of the data, but raw source must remain readable and non-trivial editing must remain practical for humans. Otherwise the data loses usefulness, portability and agent friendliness.
- Treat human raw-text authoring as a cross-cutting requirement, not merely viewer polish. Do not require a GUI, agent or conversion step to understand/edit canonical data. Use representative raw-text examples while refining contracts.
- For conditions, user prefers target-scoped state names and readable logic rather than JSON trees or repeated actor/state lookup functions. User offers lowercase/PascalCase states and uppercase logic keywords to simplify parsing; exact grammar remains to refine. Short-circuit/early-out conditions are intended, rather than `guard` necessarily meaning a positive prerequisite.
