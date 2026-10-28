# Game-design applications — provisional shortlist

## Result

Prioritize **declared combination coverage**, then **goal/hazard mechanic-chain inspection**, then **resource-consumer and budget inspection**. All three have documented research precedents. Their adaptations here remain hypotheses, not validated design advice.

Most useful near-term question: **Which authored combination, terminal rule or resource dependency deserves the designer's next inspection, and what declaration supports that choice?** Do not infer difficulty, teaching risk, mastery, balance or design quality from topology.

This is bounded orientation/source discovery, not a literature review or implementation plan. Seven unique candidate URLs retained, representing five works; three original papers support shortlist. One survey supplies a domain-dependence caveat; one original Machinations lead remains inaccessible. Sources, exact passages, limitations and local line ranges: [evidence-extracts.md](evidence-extracts.md). Search history: [../discovery-journal.md](../discovery-journal.md).

## 1. Discover declared mechanic/skill combinations and where they first appear

**Question.** Where does a challenge combine abilities previously listed separately? Which routes have prior opportunities for constituent skills, and which have prior opportunities for the actual combination?

**Documented application.** Butler et al., CHI 2015, used small solution graphlets plus an auxiliary math graph to generate Refraction progressions. Linked patterns distinguish composition from mere presence of individual pieces [E1–E2]. This was an implemented system and a 2,377-player field experiment, not a general result that graph features measure learning. Median time played was 184 seconds for generated progression versus 199 seconds for expert progression; reported nonsignificant difference does not establish equivalence. Authors explicitly lacked a random baseline and could not establish that progression affected engagement.

**Concrete local observation.** Crate `coordinate` has AND prerequisites `plan-push` and `route-switch`. `combined-room` both introduces and requires `coordinate`, and separately requires both constituents. `push-room` permits a bypass directly to `combined-room` or a visit through `switch-practice`. Combined-room's body explicitly says no physical map and prior planning support not documented. These are authored declarations, not observed learning.

**Proposed adaptation — hypothesis.** Search the full declared skill/challenge graph for explicit combination atoms, rather than making users choose every pair. Show a “combination first-demand” overlay: ring combined-room, emphasize the coordinate junction and its two prerequisite branches, and distinguish available prior constituent-opportunity witnesses from prior combination opportunities. Selecting a marker reveals exact role/prerequisite declarations and a retained route. Designer's next action is inspect intent: intentional discovery, missing authoring, or experiment candidate. Not automatically “add an earlier lesson.”

**Data boundary.** Prerequisites, roles and `next` already declared and consumed. Current native route analysis already expands AND prerequisites and returns prior-opportunity witnesses; do not relabel that as a new capability. Added interpretation would distinguish explicit combinations from their constituents and surface them graph-wide. Challenge schema also declares mechanic and interaction links, but current native challenge loader consumes skill roles/next, not every schema field. Actual solution graphs, required action sequences, geometry, alternative winning solutions and player traces are genuinely missing for Refraction-style claims. A declared prerequisite motif is not a validated solution graphlet.

**Failure modes/counter-evidence.** Combining independently practised skills can add demands, but topology cannot measure that demand. Skill definitions may be coarse, proposed or incomplete. First arrival differs from later revisits. Counts can overvalue duplicate content. Refraction's generator constrained all valid solutions to share a solution graph; arbitrary crate puzzles need not. Its automatic progression also repeated similar levels after feature novelty ran out. Bare topological sort does not control pacing [E2].

**Scale/confidence.** High confidence in documented method and current declarations; medium confidence that overlay aids local review; low confidence in predictive teaching value. Start with explicit AND junctions and existing route witnesses, not arbitrary subgraph mining. Fixed small motif extraction can still grow rapidly with node degree; all possible subsets grow combinatorially. Current native bounds are 4,096 records/32,768 relationships, not limits on authored data. Existing route checks have separate check/witness caps. Larger projects need indexed typed adjacency, bounded findings and explicit incompleteness, not exhaustive path/pair enumeration. No measured implementation cost claimed.

Local anchors: `skills/game-design-workbench/examples/crate-puzzle/game-design/skill-atoms/coordinate.md:8–13`; `challenges/combined-room.md:7–32`; `challenges/push-room.md:7–33` under same example root. Reader: `skills/game-design-workbench/native/gd_core.c:380–387`; route computation: `native/gd_routes.c:65–130` under workbench root.

## 2. Inspect goal/hazard mechanic chains, with explicit unknown links

**Question.** Which declared actions and transformations connect to a goal or loss condition? Which necessary-looking steps lack encoded rule support or depend on scene facts?

**Documented application.** AtDelfi (FDG 2018, arXiv v2) represents rules as a mechanic graph and chooses player-input-to-terminal paths to generate tutorial text and recorded gameplay examples [E3]. Eight GVG-AI games demonstrate the approach. This is a tutorial-generation demonstration, not evidence of production adoption or improved player learning.

**Concrete local observation.** Plumber `finish` requires move-right, practises finish-approach and declares course-finish. `finish-post-contact` describes alive hero contact committing completion, stopping clock spending and disabling hazards. Those latter outcomes remain `unknown_effects`; map placement and same-frame death/finish priority are unresolved. A record-link path to this interaction therefore does not prove an executable path to victory.

**Proposed adaptation — hypothesis.** A goal-focused overlay would emphasize declared finish-related mechanic/case nodes and known condition edges, while showing prose-only completion effects as explicit unknown boundaries. Keep alternatives visible. Select a highlighted chain to inspect conditions, source lines and unresolved terminal semantics. Designer's next inspection: decide whether completion semantics, simultaneous event ordering or physical placement need clarification before a playable strategy or tutorial can be derived.

**Data boundary.** Current records expose rule cases, participants, guards and source declarations. Current resource preview draws some typed declarations/invocations; generic focused relationships exposes only fields consumed by its loader. Challenge mechanics are schema-declared but not consumed by challenge-role loader. There is no established complete input-to-terminal graph here: explicit terminal semantics, action occurrence identity, map reachability, timing and cross-rule composition are not guaranteed. These are genuinely additional requirements, not data that can be invented by traversing more record links.

**Failure modes/counter-evidence.** AtDelfi missed mandatory portal travel in RealPortals because abstract VGDL mechanics did not encode map necessity; no agent won that game, leaving winning video examples incomplete. Its Plants tutorial also failed to explain an essential creation chain. These failures closely challenge claims based on sparse authored topology. Shortest record path is neither sufficient plan nor most useful lesson. Merging similar rules can erase consequential state distinctions. No stomp/shell teaching-risk claim established here.

**Scale/confidence.** High confidence that explicit-rule chains have been used for tutorial selection; medium-low confidence in utility before local terminal semantics exist. Treat as review view, not automatic tutorial feature. AtDelfi describes enumerating possible paths before choosing longest-shortest witnesses; do not copy that into cyclic/larger graphs. Bounded reachability/shortest witnesses can avoid enumeration, but condition satisfaction and simultaneous rules need separate semantics. No effort/performance estimate supported.

Local anchors: `skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/finish.md:8–28`; `interactions/finish-post-contact.md:18–39` under same example root. Existing preview limit: `skills/game-design-workbench/native/README.md:33–48`; direct graph construction: `native/gd_resource_graph.c:90–131,157–185` under workbench root.

## 3. Find resource consumers and inspect budget assumptions before balancing

**Question.** Which actions draw on the same actual pool, and can a proposed sequence exhaust its budget? Where are identity, event count or resource-flow semantics missing?

**Documented application.** GEEvo (IEEE CEC 2024, arXiv v1) uses executable graph economies with weighted flows, repeated simulation and fitness objectives [E4]. It benchmarks generated economies and illustrates damage balancing with fictional mage/archer economies. This is a simulation demonstration, not validation of real class balance or production use. Its model covers fixed systems whose resources originate within the economy.

**Concrete local observation.** Crate player-energy declares initial/capacity controls and per-player-per-challenge ownership, with no regeneration/conversion defined. Crate-pushing spends push-cost on success; wall-blocked and insufficient-energy cases preserve Amount. Free-push repeats the guard/write but does not explicitly bind its Energy actor to the player-energy pool. Do not merge matching Energy labels or sum mechanic and interaction writes as two independent occurrences.

**Proposed adaptation — hypothesis.** First show resource-consumer families and unknown bindings. Highlight requirement/write/preservation edges using distinct encodings, retain separate unbound facets, and mark player-energy's identity boundary. Once an explicit scenario binds the pool and supplies successful-push count, show a budget overlay on that chosen scenario, with source-cost declarations and starting-value assumptions inspectable. Designer's next experiment: compare route push counts/budgets under stated initial conditions, not “increase energy because this node is central.”

**Data boundary.** Resource noun plus initial/capacity controls exist in schema. Current native reader consumes resource `noun`, not initial_control/capacity_control as general resource-role links. Rules already encode guards and quantity writes; graph builder retains direct declaration types and explicit bindings. Resource preview does not implement resource routes, composed outcomes or source navigation. Explicit shared-pool binding, reset occurrence, action/event identity, admissible route/action sequence and success/blocked counts must be supplied or validated separately. Timing and player policy needed for dynamic flow claims.

**Failure modes/counter-evidence.** Multiple declarations of the same event risk double counting. Geometry can block otherwise affordable actions. Unmodeled replenishment cannot be assumed absent in an actual game merely because a fixture omits it. GEEvo's equal-damage objective erased the distinction between normal and critical damage in one case: objective met, intended mechanic lost. Some targets were unattainable and not every graph balanced [E4]. Topology detects candidate relationships, not economy balance or desirable player experience.

**Scale/confidence.** High confidence in direct numeric declarations and simulation precedent; medium confidence in consumer review, low confidence in automatic budget/path claims without identity/occurrence semantics. Direct declaration indexing is modest relative to solver/state-space work. Cycles, replenishment, multiple actors, ranges and stochastic policies can explode possible states; simulation multiplies graph work by steps, trials and search candidates. Bound each dimension, expose unresolved facets, keep coefficients/units/assumptions sourced. No benchmark extrapolation from GEEvo to current C99 tools.

Local anchors: `skills/game-design-workbench/examples/crate-puzzle/game-design/resources/player-energy.md:9–16`; `mechanics/crate-pushing.md:26–68`; `interactions/free-push.md:34–45` under same example root. Reader: `skills/game-design-workbench/native/gd_core.c:356–387`. Identity handling: `native/gd_resource_graph.c:31–68` under workbench root.

## Prioritized deeper-pass questions

1. Can a graph-wide declared-combination overlay expose a useful inspection beyond existing per-skill opportunity witnesses? Compare against current UI before adding metrics. Parent's structure/similarity lanes own general algorithms.
2. What minimal scenario/solution representation distinguishes mandatory composition from merely co-declared skills? Test authored crate examples conceptually, not by claiming player evidence. Original puzzle-solver/state-space papers could change this shortlist.
3. Which explicit terminal, occurrence and pool-identity declarations are missing? Until answered, rank mechanic-chain/budget views as conditional research, not ready features.
4. Can designer-chosen invariants prevent “balanced but no longer intended” outcomes? Any deeper simulation work needs separate approved scope.
5. Retrieve original Machinations source if permitted tools can access it without environment changes. Current failure prevents judging that alternative. No claim about comparative superiority.

## Checks, gaps and handoff

- Read shared brief/assignment completely before discovery. Read mandatory AI-assisted guide and research artifact formats. Follow bounded lane contract instead of full register/delegation workflow.
- Search/fetch observations retained chronologically; CSV preserves repeated observation of progression paper. Seven unique candidate URLs, under cap of eight. Alternate mirrors/secondary leads not pursued.
- Retained exact bounded passages with stable page/section locations. Search snippets used only for discovery. Public PDFs genuinely extracted; their wrapper messages were not treated as paper content.
- Read relevant native README completely; inspected actual challenge/skill/resource reader branches, route computation and resource graph construction. Read crate schemas/usages and concrete crate/plumber records. No native command, experiment, build or UI rerun.
- No implementation, installs, environment changes, shared edits, commits or additional agents. Deliverables confined to assigned folder.
- Gaps: no dedicated empirical encounter-design paper; no demonstrated effect on local design decisions; no player observations for fixtures; no full-state puzzle solving source reviewed. Three strongest methods selected rather than filling a fourth slot with analogy.
- Deeper pass could change shortlist, especially if puzzle-state search offers more useful deadlock/alternative-solution inspection than the conditional tutorial-path idea. Parent approval required before deeper campaign.

## Comments

Visual encodings above are proposed adaptations, not tested comprehension results. Drawing/perception lane owns evidence about layout effectiveness. Diagram beauty is not support for comprehension, learning or causal design advice.
