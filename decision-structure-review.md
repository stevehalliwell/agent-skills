# Decision structure — example review and proposed additions

Status: P1–P4 confirmed. Initial six-record pilot applied: M2, M6, M10, M15, M18 and C4. Follow-up M5/M7 applied as separate local tasks, with unclassified demands preserved. Other example proposals pending. Rules, learning-role metadata and schema fields unchanged. Eight approved records now contain contextual YAML assessments; CLI-only demand/intent query implemented. Viewer integration unavailable.

## Scope and review method

Reviewed all 19 Mario challenge records and all 4 Crate challenge records, plus supporting Dynamics, Experiments and selected rules. Mario is an authored fictional design, not a factual reconstruction of an existing game. Crate is an exploratory tutorial brief. Neither example supplies observed player decision-making.

Read each proposal as: **this is what source says; this is what I suggest adding; this is what remains unknown.** Confirm, correct or reject individual proposals here before they become source changes. Agreement on one example does not approve all classifications or a schema.

This review covers decision content and data placement. It does not assess every rule, validate physical levels, or implement new graph features.

- [Shared representation](#shared-representation)
- [Mario proposals](#mario-proposals)
- [Crate proposals](#crate-proposals)
- [Dynamics additions](#dynamics-additions)
- [Experiment additions](#experiment-additions)
- [Categories not yet established](#categories-not-yet-established)
- [Proposed application order](#proposed-application-order)
- [Multi-situation follow-up and capability assessment](#q4-multi-situation-review-and-capability-assessment)

Definitions now live in [shared workbench reference](skills/game-design-workbench/references/decision-structure.md). This document applies them; it does not create competing definitions.

## Shared representation

### P1. Classify situations, not whole games or isolated buttons

Confirmed: use contextual descriptions in existing Challenges. One challenge may contain several situations: choosing whether to engage, choosing a response, then executing that response. Do not give all three one exclusive label.

Keep these separate:

- **Task and conditions:** what player is trying to accomplish; relevant state and prior knowledge.
- **Alternatives:** available actions, avoidance and consequential inaction where supported.
- **Decision demands:** reasoning, calculation, optimization, uncertainty, trade-off, recognition, reaction, dexterity or execution.
- **Game judgment:** explicit success/failure, graded benefit or preference-dependent result.
- **Timing:** deliberation, preparation, execution and feedback; familiarity changes experienced demand.
- **Basis:** declared rule, authored intention, prediction, model result or observation.
- **Unknowns:** absent conditions, missing player knowledge, geometry or evidence.

These are confirmed body concepts, not adopted frontmatter keys. Use only relevant items, not empty checklists in every record.

### P2. Keep test, quiz and mastery distinct

Confirmed: **test** for a situation with a stated success/failure criterion; **quiz** for a knowledge-oriented test. An execution test need not be a quiz; a correct explanation need not prove performance.

User-added base vocabulary:

- **Reaction test:** demands a response within a specific short time span. See [definition, example and evidence boundaries](skills/game-design-workbench/references/decision-structure.md#reaction-test).
- **Dexterity test:** demands accurate performance of particular controller buttons, motions or sequences. See [definition and controller-remapping example](skills/game-design-workbench/references/decision-structure.md#dexterity-test).

Knowledge, reaction and dexterity demands can overlap but must not be conflated. Simplifying a fighting-game action to its own button can reduce dexterity demand while preserving action-choice knowledge. These labels do not establish universal timing limits, observed difficulty or player ability.

Existing `tests_mastery` links identify an intended learning-assessment relationship. They do not mean every harmful response tests mastery, or that a player has passed. Leave existing learning-role metadata unchanged during first classification pass.

A researcher's contact-prediction question can be a quiz in an experiment without becoming an in-game quiz. None of reviewed records establishes an implemented in-game question-and-answer interface.

### P3. Start with prose; structure only after review

Confirmed approach: add a concise **Decision and assessment** section to selected Challenges after their example-specific wording is reviewed. Existing rules continue owning conditions and consequences. Dynamics owns behavioral predictions; Experiments owns methods/results. Shared reference owns vocabulary.

Reviewed framing for `first-walker`, applied in expanded form by six-record pilot:

> **Task:** get past Walker alive. Jumping over, attempting a stomp and retreat are available; defeating Walker is not required. With small, unprotected Hero, harmful side contact produces death and fails this survival task. This is a test combining contact understanding and execution, not evidence of a knowledge-only quiz or acquired mastery. Deliberation time and execution windows are unspecified. Failure alone cannot distinguish misunderstanding from mistiming.

“Get past alive” is proposed explicit assessment framing, not a verbatim existing goal. Existing goal is getting past by jumping over or attempting a stomp, with retreat space and possible damage.

No new collection, classifier, quality score or graph is proposed for adoption in this review. Optional queryable fields can follow once examples establish what needs querying.

### P4. Question intended demand, not just classify content

Confirmed use: **“This is a dexterity test. Do you want it to be?”** Ground current/proposed demand in mechanics, input mapping, information and timing. Ask whether it serves intended experience; do not assume test is wanted or defective.

Keep current demand, intended demand and observed player difficulty separate. Where unwanted, discuss alternatives that preserve wanted knowledge, reaction or decision demands. No automatic redesign or observation claim follows. Each pilot record now contains an unresolved intent check rather than a fabricated answer.

## Mario proposals

### M1. Start apron — exploration before assessment

[Source: start-apron](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/start-apron.md)

**Found:** broad safe ground; room to start/stop without lethal edge; introduces moving right and stopping at an edge. Input knowledge is not proven.

**Propose:** movement discovery and low-consequence execution practice. No explicit local pass/fail test or meaningful trade-off is established. Do not label merely moving right a calculation or optimization.

**Timing:** room to experiment is declared; no precise preparation duration or input window. Safe ground does not mean course clock is paused.

### M2. First Walker — failed response versus valid alternatives

[Source: first-walker](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/first-walker.md). [Exact lethal case: touch-walker-small](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/interactions/touch-walker-small.md), `lethal-side`.

**Found:** jump over, attempt stomp or retreat; multiple skills introduced together. Small, unprotected Hero dies from harmful side contact. Valid stomp and harmful side contact are different conditions.

**Propose:** survival test combining recognition/contact understanding and execution. Treat lethal side contact as failed response under survival task, not a merely lower-value play style. Bypass and stomp can both satisfy getting past; do not invent one correct solution.

**Boundary:** not every Walker contact is lethal. Protected/powered states differ. Retreat delays commitment; whether and for how long it remains feasible depends on situation. First encounter is not automatically a mastery test or proof player was taught the rule.

**Timing/evidence:** no reaction window or observed mistake. Quiz demand is a hypothesis until player knowledge is isolated from performance.

### M3. First supply — optional acquisition, not compulsory correctness

[Source: first-supply](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/first-supply.md)

**Found:** optional growth reward over safe ground; missing it does not block progress; bumping and collecting introduce several concepts.

**Propose:** distinguish choice to pursue reward from execution of bump/collection. Missing a chosen collection goal can fail that local goal, but skipping reward is not failure of course progress.

**Unknowns:** cost, visibility of benefit, collection window and why a player skips. Do not assert meaningful trade-off or optimization merely because power is useful.

### M4. Brick canopy — infer a state-dependent rule

[Source: brick-canopy](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/brick-canopy.md)

**Found:** small and powered forms produce different brick responses; mixed blocks, optional concealed life supply and shorter-jump practice over broad support.

**Propose:** information gathering/inference plus execution practice. A knowledge-test candidate is predicting response for current form; the actual encounter is not established as a quiz. Optional hidden reward is not a mandatory answer.

**Unknowns:** what player has seen, whether contrast communicates rule, discovery time and feedback latency. Do not infer literal backtracking from a changed understanding.

### M5. Pipe steps — separate discovery, route choice and input

Applied in approved follow-up: three assessments for landing, entrance discovery and route choice. Discovery/route demand lists remain empty; no compelled search or best-route claim. See [follow-up capability assessment](#q4-multi-situation-review-and-capability-assessment).

[Source: pipe-steps](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/pipe-steps.md)

**Found:** jumping/landing practice; only one designated pipe offers cellar entry; hint/discoverability unresolved; entry requires alignment and down input.

**Propose:** three situations: landing execution, discovering entrance, choosing whether to enter. Entrance discovery concerns player information; input/alignment concerns successful performance. Continuing overworld is valid, not a wrong answer.

**Unknowns:** what player knows about destination, route costs, entry window and readable signals. Plain route arrows do not supply those predicates.

### M6. First gap — clearest existing execution test

[Source: first-gap](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/first-gap.md)

**Found:** committed jump across lethal floor break to broad landing. Existing `tests_mastery` targets landing. Earlier practice opportunities are not proof of learning, and cellar route bypasses this challenge.

**Propose:** explicit pass/fail survival test with launch/landing execution and edge recognition. Crossing alive is success; falling through lethal boundary is failure. No competing-values dilemma is established.

**Timing:** distinguish time available to inspect approach from airborne control/landing window. Neither has a numeric value. Familiarity may reduce deliberation but not eliminate execution demand.

### M7. Low ceiling — cleaner execution is not compulsory success

Applied in approved follow-up: separate traversal and chosen shortened-jump reward task. Ground traversal has no classified demand; optional technique retains execution/undecided. Success criteria remain prose, not new fields.

[Source: low-ceiling](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/low-ceiling.md)

**Found:** shortened jump can improve reward access; ground route remains available. Tall stand-up case is unresolved.

**Propose:** execution refinement and optional reward access. Distinguish successful traversal from cleaner reward-taking; a non-shortened jump is not automatically a failed encounter.

**Boundary:** no general optimization claim without goal and constrained improvements. No pass/fail criterion should rely on unresolved clearance behavior.

### M8. Flower shelf — capability pursuit under state conditions

[Source: flower-shelf](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/flower-shelf.md)

**Found:** optional raised supply; tall Hero can gain flower, small Hero receives cap instead. Requires collection and targeting; arrival alone does not grant ranged capability.

**Propose:** state-dependent reward choice plus execution. Candidate constrained improvement toward a chosen capability goal, not proof every player is optimizing. Skipping remains valid.

**Unknowns:** benefit understood by player, detour cost, available clock margin and precise collection demands. One cannot classify same pickup as same improvement regardless of form.

### M9. Bonus entry — commitment, not automatic best route

[Source: bonus-entry](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/bonus-entry.md)

**Found:** designated aligned down-entry action commits optional transfer; discovery unknown.

**Propose:** route commitment with an input condition. Unknown destination information may matter, but documentation uncertainty is not evidence that players are uncertain. Choosing not to enter is not failure.

**Unknowns:** information before commitment and whether player can revise choice before transfer. Do not infer hesitation/reaction time from a route edge.

### M10. Coin cellar — strongest authored time/reward trade-off

[Source: coin-cellar](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/coin-cellar.md)

**Found:** optional low/raised coins in room without enemies or lethal gaps; shared clock continues. Explore or leave quickly; no compulsory collection total.

**Propose:** trade-off between optional reward and conserving time. Not collecting all coins is not wrong. If budget is generous, nominal cost might not matter to player; preserve this as unverified meaningfulness, not automatic quality.

**Calculation/optimization:** possible when player deliberately budgets remaining time or selects constrained reward collection. Do not assert either demand from timer/coin numbers alone.

**Timing:** room supports relatively safer exploration, not unlimited deliberation. No measured decision windows or collection costs.

### M11. Bonus return — transition with a consequence, not necessarily a choice

[Source: bonus-return](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/bonus-return.md)

**Found:** one-way return preserves carried state, time and rewards; skips three authored challenges.

**Propose:** describe commitment consequence and location feedback. No separate meaningful alternative at return itself is established. Do not force a decision label onto every route node.

**Boundary:** skipped opportunities are not missing competence. One-way transfer does not prove player anticipated or regretted bypass.

### M12. Shell lane — avoid, engage or use carried capability

[Source: shell-lane](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/shell-lane.md)

**Found:** bypass, stomp/manage shell, or use spark if available. Resting/sliding states can be observed; no mandatory damage or shell mastery.

**Propose:** separate engagement choice, state recognition and execution. Conditional response tests can be described once precise contact/state/task is supplied. Bypass is a valid alternative, not a failed shell quiz.

**Boundary:** useful shell can create returning danger, but known probabilities are absent. Use uncertainty/consequence management rather than claiming quantified risk. Fixed enemy behavior alone does not establish strategic/adversarial decision-making.

**Timing:** observation opportunity exists; launch/return/response windows unknown. An unfamiliar player and a rehearsed player may face different deliberation demands.

### M13. Coin bridge — graded benefit and voluntary practice

[Source: coin-bridge](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/coin-bridge.md)

**Found:** raised coin jumps versus direct safer floor travel; missed platform need not kill; completion does not require coins.

**Propose:** optional reward/time/effort trade-off plus execution practice. Missing a coin is reduced benefit or failed self-chosen collection goal, not automatically course failure.

**Unknowns:** meaningful cost, player preference and landing demands. Safer continuous floor does not support a quantified death-risk comparison.

### M14. Checkpoint strip — preparation and recovery context

[Source: checkpoint-strip](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/checkpoint-strip.md)

**Found:** safe retry-anchor strip before ending sequence; opportunity to inspect time; no refill or proof of awareness.

**Propose:** preparation context and altered cost of later failure, not a standalone dilemma or test without a response criterion. Retain difference between crossing trigger and understanding checkpoint.

**Timing:** safe space does not imply clock pause. Retry relationships are stateful rules, not ordinary forward route arrows.

### M15. Stair fork — traversal preference with possible time pressure

[Source: stair-fork](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/stair-fork.md)

**Found:** exposed narrow upper steps versus lower, longer ground passage. Both rejoin final gap; lengths and times unmeasured.

**Propose:** candidate trade-off between execution demands and travel/time cost; legitimate preference-dependent alternatives. No universally correct branch established.

**Boundary:** source calls lower route longer; do not convert that to a measured time saving on upper route. Calculation needs concrete inputs and a criterion; dilemma needs more than two routes with costs.

### M16. High stair — optional precision practice

[Source: high-stair](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/high-stair.md)

**Found:** narrow ascending supports; additional landing opportunity; recovery geometry unspecified.

**Propose:** execution practice after route selection. Individual landing can have a local success criterion, but no authored lethal-failure consequence for every miss is established.

**Unknowns:** widths, timing, recovery and actual difficulty. Do not credit this practice to players choosing low path.

### M17. Low path — valid alternative, not an incorrect answer

[Source: low-path](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/low-path.md)

**Found:** longer ground route skips upper landing practice and offers space to reassess remaining time. No measured length/time.

**Propose:** chosen traversal alternative and preparation opportunity. Choosing it may prioritize easier travel over conserving time; this motive is a hypothesis, not encoded by path selection.

**Boundary:** no automatic optimization, failure or missing-skill label.

### M18. Final gap — composite test with route-dependent preparation

[Source: final-gap](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/final-gap.md)

**Found:** visible lethal floor break; combines launch, landing and time awareness. Existing mastery target is finish-approach; no prior explicit introduction of that composite skill. Upper stair practice is optional.

**Propose:** composite survival/execution test. Assess preparation opportunities separately for each route; do not assume mastery from graph reachability or label design impossible from missing introduction alone.

**Timing:** distinguish approach deliberation, time-pressure context and execution window. No numeric windows. Recognizing correct response and executing it are separate evidence targets.

### M19. Finish — explicit course-success feedback

[Source: finish](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/finish.md)

**Found:** reach visible post to commit success beyond final landing. Feedback should distinguish finish from merely surviving jump.

**Propose:** course success criterion and feedback event, not necessarily a rich decision. No further route, campaign or post-finish choice is authored.

**Unknowns:** feedback latency and simultaneous death/finish ordering. Do not invent outcome precedence to complete a classification.

## Crate proposals

Crate records are much thinner than Mario's. They describe conceptual progression without physical layouts. More precise decision descriptions require authored situations, not inferred geometry.

### C1. Move demo — introduction without a defined test

[Source: move-demo](skills/game-design-workbench/examples/crate-puzzle/game-design/challenges/move-demo.md)

**Found:** movement introduction; no prior skill requirements; assistance and observed learning unspecified.

**Propose:** mark decision/assessment context as not yet specified. Candidate execution/discovery opportunity, not established puzzle or quiz. First useful addition is a local task, available actions and feedback—not a categorical verdict.

**Timing:** no deliberation limit, response window or pressure established.

### C2. Push room — legal action versus useful action

[Source: push-room](skills/game-design-workbench/examples/crate-puzzle/game-design/challenges/push-room.md). [Rule: crate-pushing](skills/game-design-workbench/examples/crate-puzzle/game-design/mechanics/crate-pushing.md).

**Found:** introduces push while also requiring push; requires movement. Successful push requires free destination and sufficient energy, spends energy; wall-blocked/insufficient-energy attempts move nothing and spend nothing. Pulling unavailable.

**Propose:** distinguish three judgments: input/action permitted, push succeeds, push helps solve intended task. A successful but poorly chosen push could matter differently from a blocked attempt, but no specific room establishes either as a failed puzzle response yet.

**Quiz candidate:** predicting whether a supplied push succeeds and whether energy is spent. This would need a concrete visible state and response criterion; it is not an existing in-game quiz.

**Review issue:** introducing and requiring push in same room merits checking intended assistance, not silently changing learning-role metadata.

### C3. Switch practice — optional learning opportunity

[Source: switch-practice](skills/game-design-workbench/examples/crate-puzzle/game-design/challenges/switch-practice.md)

**Found:** introduces reading/routing switch; practises push; optional route before combined room. Assistance unspecified.

**Propose:** candidate rule-understanding and planning situation. Need explicit local task and switch/exit conditions before calling it a concrete puzzle test. Bypassing practice is not itself wrong or proof of ignorance.

**Timing:** no speed criterion. More time thinking is not automatically worse performance.

### C4. Combined room — strongest puzzle candidate, incomplete assessment

[Source: combined-room](skills/game-design-workbench/examples/crate-puzzle/game-design/challenges/combined-room.md). [Concept](skills/game-design-workbench/examples/crate-puzzle/game-design/concept.md).

**Found:** combines push planning and switch routing; introduces coordination while requiring it; prior planning support missing. Concept aims to open exit by placing crate on switch with finite pushes and no pulling.

**Propose:** intended puzzle/planning test combining rule knowledge and constrained resource use. A concrete layout must establish solution choices, failure states and recovery. Do not claim a particular push causes an irreversible dead end without geometry.

**Calculation/optimization:** counting required pushes may become calculation if nontrivial comparison is required. Finding a feasible solution is not automatically optimizing; minimizing pushes/time would need a goal. A finite budget is a constraint, not proof of meaningful alternatives or an optimum.

**Timing:** no game-imposed deliberation deadline established. Energy limits successful actions, not thinking duration.

## Dynamics additions

Keep these as behavioral predictions. Record status such as `accepted` does not turn a prediction into observation.

### D1. Crate: careful positioning

[Source](skills/game-design-workbench/examples/crate-puzzle/game-design/dynamics/careful-positioning.md)

**Found:** finite pushes predicted to encourage consideration; no observed behavior.

**Propose:** explain candidate demand as planning under action budget. Contrast desired deliberation with avoidance of experimentation, confusion or copied solutions. More hesitation alone cannot identify calculation or thoughtful planning. Keep concrete room criteria in Challenges.

### D2. Mario: clock trade-off and reward detours

[Clock trade-off](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/dynamics/clock-tradeoff.md). [Reward detours](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/dynamics/reward-detours.md).

**Found:** optional exploration/reward versus time conservation predicted; generous clock may erase relevant cost. Preferences and detour costs absent.

**Propose:** distinguish authored expenditure from experienced trade-off. Preserve alternatives as potentially preference-dependent, not correct/wrong. Calculation/optimization should remain conditional on player goal, information and actual comparison.

### D3. Mario: shell risk and power protection

[Shell risk](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/dynamics/shell-risk.md). [Power protection](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/dynamics/power-protection.md).

**Found:** shell utility can create danger; power could prompt caution or aggressive experimentation.

**Propose:** describe state-dependent consequences and uncertain player response without invented probabilities. Separate avoiding damage, preserving a wanted capability and intentionally accepting a cost. Do not label every damage event a wrong strategic choice.

### D4. Mario: capability recontextualization and learning by retry

[Capability](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/dynamics/capability-recontextualizes.md). [Retry](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/dynamics/learn-by-retry.md).

**Found:** changed form may change understanding; retry permits revised timing/routes/contact. Neither establishes learning.

**Propose:** track hypothesized change from discovering a response to recognizing it and executing a learned policy. Repeated failure could reflect understanding, execution, feedback or access needs; keep those alternatives open. Similar upcoming objects are not the same as physically revisiting prior objects.

### D5. Mario: forward commitment and practice bypass

[Forward momentum](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/dynamics/forward-momentum.md). [Practice bypass](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/dynamics/optional-practice-bypass.md).

**Found:** forward camera may promote commitment; routes provide different opportunities, not measured knowledge.

**Propose:** add preparation/information context to later test hypotheses. Distinguish unavailable retreat from deliberate commitment, and missed opportunity from inability. Do not derive player preference or competence from graph path alone.

## Experiment additions

### E1. Contact prediction versus actual performance

[Source: first-course-comprehension](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/experiments/first-course-comprehension.md)

**Found:** proposed contact-prediction measure already asks whether participant correctly predicts a supplied case before trying it. No build, participants or results.

**Propose:** identify this prompt as a knowledge-oriented test/quiz in research method. Pair predictions with separately observed execution when method is refined. Correct prediction plus failed action differs from wrong prediction plus successful action. Record familiarity, prompts, assistance and timing; do not assume retrospective explanation proves original knowledge.

**Boundary:** a pre-action prompt may teach, slow or otherwise change encounter. Keep prompted and unprompted conditions distinct. No threshold, participant count or new result proposed here.

### E2. Why a route was chosen

[Source: route-choice](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/experiments/route-choice.md)

**Found:** proposed reason categories already include reward/time/safety/curiosity/other/unknown, based on participant explanation rather than path alone.

**Propose:** test whether alternatives were noticed and costs understood before classifying experienced trade-off. Distinguish ignoring an unimportant reward, not discovering it and being unable to reach it. Record available time and relevant goal; no universal best route.

### E3. Ideal travel budget is not player calculation

[Source: travel-budget](skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/experiments/travel-budget.md)

**Found:** proposed designer-side ideal travel calculation; blocked on missing lengths. Excludes hesitation, jumps and enemies.

**Propose:** explicitly separate calculation performed by designer/model from calculation demanded of player. A computed margin may inform context but cannot establish deliberation time, reachability or decision quality. No execution proposed.

### E4. Crate planning investigation

[Source: energy-encourages-planning](skills/game-design-workbench/examples/crate-puzzle/game-design/experiments/energy-encourages-planning.md)

**Found:** proposed playtest with no method, measures or observations.

**Propose:** investigate whether players can predict a push result, explain a plan and revise after feedback. Compare understanding with execution, successful actions with useful actions, and deliberation with confusion. Assistance/guide use can change experienced demand. No causal conclusion from longer pauses alone; detailed study design remains separate work.

### E5. Crate route timing — keep research criterion separate

[Source: route-cost](skills/game-design-workbench/examples/crate-puzzle/game-design/experiments/route-cost.md)

**Found:** completed simulation prescribes four walks and three successful pushes, assuming correct decisions and route legality. All 1,000 routes completed; 245 met 8 s. Research criterion required at least 90% by 8 s. Uniform 0–1 s delays are assumptions, not observed deliberation.

**Propose:** label this an experiment/model evaluation, not an in-game quiz or evidence players failed a test. Preserve distinction between route completion and meeting research timing criterion. Do not transfer its 8 s deadline into Crate Challenges or reuse random delays as measured player thinking time.

## Categories not yet established

These are absences of sufficient evidence in reviewed situations, not claims that categories cannot apply.

- **Dilemma:** no reviewed situation establishes unresolved competing considerations in intended trolley-like sense. Optional rewards, damage and route costs are not enough.
- **Nontrivial player calculation:** plausible in budgeting, but no concrete supplied comparison establishes required work for a particular player. Designer formulas do not prove player calculation.
- **Optimization:** plausible for chosen reward, capability or push-efficiency goals. Source does not yet establish enough contextual constrained-improvement decisions to label every resource choice optimization.
- **Dominated alternatives:** no sourced equivalent of otherwise identical $1000 versus $999 established. Harmful contact fails survival task, but available responses differ in conditions and execution demands; do not collapse test failure into a blanket dominance claim.
- **Known probabilistic risk:** no player-facing outcome distribution or measured failure probability established. Hazards can have deterministic rules and uncertain execution.
- **Strategic/adversarial decision:** enemy presence alone does not establish choices by another responsive decision-maker.
- **In-game quiz:** no implemented question/response presentation established. Contact prediction is already a proposed research quiz; encounter recognition is a candidate knowledge demand.
- **Numeric deliberation windows:** absent. No reviewed encounter can be labelled a <200ms decision from these records.
- **Observed decision quality or learning:** no human play evidence establishes either. Crate's executed timing simulation is not a substitute.

## Proposed application order

1. Review shared representation P1–P3 and concrete examples here. Amend this document from confirmations/corrections.
2. Apply first approved Challenge additions to `first-walker`, `first-gap`, `coin-cellar`, `stair-fork`, `final-gap` and `combined-room`. These contrast failed response, execution test, trade-off, preference and incomplete puzzle context.
3. Apply remaining approved Challenge/Dynamics wording without altering rule outcomes, constants or learning-role links implicitly.
4. Refine approved Experiment descriptions; do not run studies, simulations or viewers as a side effect.
5. Only then choose optional fields needed for specific queries. Separate game judgment, decision demands, timing and evidence status rather than introduce one exclusive `decision_kind` label.
6. Consider graph annotations/filters only after stored meaning and a concrete question are agreed. Declared relationship graphs cannot currently display or derive this full contextual assessment.

Current disposition: P1–P4 confirmed, including reaction/dexterity distinctions and design-intent review. M2, M6, M10, M15, M18 and C4 applied as contextual prose and structured body assessments. M5/M7 multi-situation follow-up also applied; other M/C/D/E proposals remain pending. User subsequently chose contextual YAML storage and CLI-first query delivery. No automatic inference, aggregate quality score, experiment execution or viewer integration adopted.

### Pilot checks and frontmatter conversion

Pilot changed six Challenge bodies only; preserved metadata values, identities, routes, learning-role links, rules, constants and historical results. Subsequent authorized format conversion below changes header syntax only. Definitions moved to shared reference; former report now points there. Skill entrypoint links reference for explicit decision/test and intent-fit questions.

Initial installed-Attendant validation rejected legacy JSON-style flow frontmatter. User authorized conversion to block YAML. Converted 67 live headers: 14 shipped template schemas, 39 Crate files (14 schemas and 25 records), and 14 Mario schemas. No field/type/value or body changes in conversion; no new schema fields.

Checks:
- Strict Attendant validation passes: Crate 67 records, Mario 225 records, no diagnostics or pending corrections.
- Copied templates validate with 15 collections, 0 records and no diagnostics/corrections.
- Schema discovery succeeds for both examples; challenge queries return 4 Crate and 19 Mario rows without truncation.
- Ten before/after native comparisons preserve controls, matrix, rules, scaffolding and MDA facts across both examples; only source-line provenance excluded.
- Exactly 67 planned files changed from pre-conversion snapshot. All converted Markdown bodies and 400 other template/example/evidence files remain byte-identical. Ordinary JSON and frozen experiment evidence untouched.
- Converter regression tests 8/8; focused authoring checks 3 passed, optional legacy-runner integration skipped. Native Attendant integration performed directly above.
- Each pilot record retains one assessment section. Skill frontmatter, local links and reference HTTP URL checks pass.

Reusable tool defaults to read-only plan, rejects unsupported/duplicate/unsafe headers before writing, skips block YAML on rerun and preserves BOM/newline/body bytes. Per-file atomic writes are not a batch transaction; errors report completed paths.

```sh
node scripts/convert-workbench-frontmatter.mjs --json
# Apply an explicitly authorized plan:
node scripts/convert-workbench-frontmatter.mjs --apply --json
node --test skills/game-design-workbench/test/frontmatter-migration.test.mjs
```

Current conversion plan reports 0 remaining conversions. Conversion did not run experiments, refresh viewers or build compilers. Vocabulary/pilot/YAML-authoring checkpoint committed as `68b21bb`; later query work described below remains a separate slice.

### Query direction — not implemented

Queries should support both description and design review:
- Which situations demand knowledge, reaction or dexterity, under which conditions?
- Is that demand intended, unwanted or not yet decided?
- What source supports assessment, and is it authored intent, prediction, model result or observation?
- Would an input/timing alternative remove unwanted demand while preserving wanted choice?

Any structured representation must keep current and intended demand separate, preserve unknowns and bind each assessment to its own situation. One Challenge can contain multiple situations; combining unrelated tags must not manufacture a matching context. No new frontmatter fields, IPC fields or collection. First body contract and CLI selectors adopted below; remaining dimensions are prose. Plain prose/text search is not an exact typed classification query.

### Q1. Contextual assessment storage — confirmed

First question: **which situations demand a selected kind of performance, and is that demand wanted, unwanted or undecided?** Example: find dexterity tests with undecided intent, then ask designer whether that demand should remain.

Before this slice, native queries returned declared rule, actor/effect, structural, similarity, skill and Resource facts but did not parse decision assessments. New query below reads explicit stored blocks; neither path classifies prose.

**Confirmed storage:** one fenced YAML `decision-assessment` block per situation inside existing Challenge body. Keep narrative explanation alongside it. Frontmatter remains flat block YAML; no new collection or nested frontmatter. Stable assessment ID is local to exact Challenge identity. Multiple situations retain separate context, demands and intent.

Reviewed shape for first-Walker context, now adopted in expanded wording in its source record:

```yaml
id: walker-survival-small
task: Get past Walker alive.
context: Small, unprotected Hero; harmful side contact is lethal.
evidence_status: authored-assessment
rule_cases:
  - "interactions/touch-walker-small#lethal-side"
demands:
  - kind: knowledge
    intent: undecided
  - kind: execution
    intent: undecided
```

Confirmed first-slice rules:
- `demands` entries bind one performance kind to its own `intent`: wanted, unwanted or undecided. Missing/unreviewed intent remains undecided, not unwanted. Absence of a demand is not proof it is absent from gameplay.
- Filter demand and intent on same entry within same assessment. Never combine knowledge intent from one situation with dexterity demand from another.
- Return exact Challenge/assessment identity, context, evidence status and rule/source references. State these are authored assessments, not observed difficulty or inferred player knowledge.
- No automatic prose classification, timing cutoff, score, mechanic change or experiment execution. No automatic Run or new graph implied.
- Existing pilot has no established dexterity classification. A dexterity filter must not invent a match; regression fixtures can supply explicit illustrative assessments without rewriting authored examples.
- Intended-but-not-present demands, remaining vocabulary dimensions and numeric timing queries are later contract work, not silently represented by this first filter.

User selected contextual YAML blocks. Prose-only storage was not selected for typed queries; flat per-Challenge tag unions cannot preserve required situation-level binding. Structured blocks now added only to six approved pilot records; all intent entries remain undecided. Reader/query contract follows below.

### Q2. First native query slice — implemented CLI

Question name: `decision-assessments`, projection `challenges`. Read explicit assessment blocks only; do not infer classifications from prose, button counts, timers or linked rule bodies.

CLI, available in fresh CJIT build:

```sh
gd query --question decision-assessments --projection challenges \
  --challenges challenges=PATH/TO/challenges \
  --demand dexterity --intent undecided --json
```

Inputs and matching:
- Require explicit Challenge collection binding. Preserve exact alias, record identity and local assessment ID; do not rely on row order.
- Optional `--demand` selects one exact authored demand label. Labels describe assessed demands, not a universal exclusive taxonomy; no synonym inference.
- Optional `--intent` selects wanted, unwanted or undecided. Omitted intent on a demand means undecided. Unknown is not false or unwanted.
- Both filters apply to same demand entry in same assessment. With no demand selector, intent can match any demand entry. With no selectors, return all authored assessments, including ones with no classified demands.
- Return each matching assessment once, with matching demand entries identified. Counts refer to assessments and demand entries, not executions, frequency or player failures.

Result retains Challenge reference/ID, local assessment ID, task, context, declared evidence status, authored rule references, full demand/intent entries, matching entries and source location. References are supporting declarations, not proof that linked case executes or applies. Evidence status is an authored claim, not independently verified by query.

Empty result is successful and means no retained explicit assessment matches. It does not establish absence of a gameplay demand. Unsupported/malformed blocks, duplicate local IDs, duplicate/conflicting demand entries, invalid intent or exhausted source limits must be disclosed rather than silently skipped or coerced. Reader bounds and atomic failure behavior are fixed below. Viewer integration remains excluded; no last-good GUI result is replaced.

Acceptance examples:
- Knowledge wanted in one situation plus dexterity undecided in another must not match dexterity wanted.
- An explicit dexterity/undecided entry matches and returns its exact situation/context.
- Missing intent remains undecided; knowledge failure and input-performance failure are not inferred from outcome.
- Prose-only records produce no typed assessment rows. Six pilot blocks adopted; no synthetic dexterity/reaction matches added to game examples.
- Existing rules, other queries, routes, learning roles, drafts, selections and analytical inputs remain unchanged.

**Confirmed scope:** CLI first. Viewer controls/results, new graphs and automatic analytical Run are excluded.

Implementation contract: named `decision-assessments` query on `challenges`; single exact `--demand` and/or `--intent` filters; `--fields summary,assessments` prunes data only, retaining parameters, coverage and errors. Demand labels are bounded open technical names, not a closed universal taxonomy. Intent values are wanted, unwanted and undecided; missing intent defaults to undecided. Context/evidence may be unknown; rule references remain authored strings, not applicability checks. Parser rejects unknown keys, duplicate IDs/kinds/fields, aliases, anchors, explicit YAML tags, malformed types and unsupported documents.

Fixed bounds: 4,096 total assessments, 16 per Challenge, 16 demand entries and 16 rule references per assessment, 32,768 aggregate demand entries/references each, 64 KiB per YAML block, 2,048 YAML events per block, nesting depth8, 4 MiB retained assessment text. Technical names at most128 bytes; other scalar text at most4,095 bytes. Reuse source-loader bounds. All result storage stays heap-owned; result borrows immutable model identities. Parse/load/allocation/limit failure returns nonzero with explicit diagnostics and no success-shaped rows; no clipped findings presented as a complete empty result. Repeated CLI selectors fail rather than replace earlier values. Existing query families reject these new selectors.

Adopt structured blocks only in six approved pilot records; retain prose and unknown intent rather than fabricate answers. Verification uses explicit synthetic dexterity/wanted/unwanted fixtures, cross-situation counterexamples, parser/limit failures, source-byte invariance and unchanged existing native facts. Dedicated Challenge-only source loader shares existing scan/identity/remap path without loosening legacy `gd_load` role combinations. No experiment or viewer mutation authorized.

### Q3. Query verification

Delivered [source/output contract](skills/game-design-workbench/native/decisions.md), bounded native reader and [CLI regression fixtures](skills/game-design-workbench/test/native-decisions.test.mjs). Fresh executable: `skills/game-design-workbench/native/build/decision-assessments-work/gd.exe`. Query slice committed as separate checkpoint; vocabulary/YAML checkpoint is `68b21bb`.

Observed inventory at first CLI checkpoint:
- Crate: 4 source Challenges, 1 assessment, 1 demand entry.
- Mario: 19 source Challenges, 5 assessments, 7 demand entries.
- All intent entries undecided. Knowledge + undecided matches first-Walker/final-gap contexts. No authored dexterity/reaction entries; filters return no rows without implying no such gameplay demand.
- Two example summary queries: 29.4 ms combined in one local run. Not a platform/performance guarantee.

Checks:
- Warning-free focused CJIT CLI/core builds. Native core148/148.
- Native decision/actor query tests14/14 with explicit `GD_DEEP_CHECKS=1`, including exact aggregate limits and text exhaustion. Default path skips those two capacity probes.
- Combined authoring/migration/native default run:24 passed,6 skipped. Skips:4 externally configured integration tests and2 opt-in capacity probes. Direct native Attendant validation performed separately below.
- Strict Attendant: Crate67 records, Mario225; zero diagnostics/corrections. Six pilot frontmatter blocks unchanged from checkpoint; prior prose preserved, only assessment blocks added.
- Ten before/after native controls/matrix/rules/scaffolding/MDA comparisons unchanged across both examples, excluding source-line provenance. Legacy structural Challenge-only source rejection remains intact.
- Skill frontmatter, new source/query links, shared reference/model/review links and diff checks pass. Broader README audit reports three pre-existing stale `controls.md` heading anchors; new query link is valid. Unrelated anchor repair deferred.

No experiment run, viewer launch/mutation, alternate compiler, full platform matrix or visual acceptance. No new frontmatter field, collection, automatic classification or quality score.

### Q4. Multi-situation review and capability assessment

User approved Pipe steps/Low ceiling follow-up. Added five assessments across two existing Challenges; kept prior prose, headers, route links and rules. Brick canopy and other pending proposals remain outside this slice. No new field, selector, game rule or intended-demand decision.

**Pipe steps:**
- `pipe-landing`: local chosen-support task; execution/undecided. Missing support does not alone establish lethal failure or a knowledge error.
- `entrance-discovery`: optional opportunity. Hint, discovery criterion and player knowledge unknown; demands empty rather than invent a knowledge/search requirement.
- `cellar-route-choice`: valid cellar/overworld alternatives. Destination knowledge, comparative costs and preferences unknown; demands empty rather than assume a trade-off or best route. Entry input is separate from preference; linked entrance predicate remains unencoded.

**Low ceiling:**
- `ceiling-traversal`: ground route remains valid; demands empty. Skipping optional reward does not establish encounter failure.
- `shortened-jump-reward`: player chooses that technique and collection goal; execution/undecided. Reward miss leaves local goal unmet, not necessarily failed traversal or a game-declared wrong answer. Geometry does not prove technique is compulsory. Tall stand-up response remains unresolved.

Current Mario inventory:19 source Challenges,10 assessments,9 demand entries. Eight records across both examples now carry11 assessments total. Unclassified means unknown assessment demands, not no gameplay demand.

#### Supported questions and limits

- **Which situations explicitly assess execution with undecided intent?** Exact native demand/intent query. Pipe landing and chosen shortened-jump reward match; their discovery, route-choice and ground-traversal siblings do not. Conditions remain returned context, not executable predicates.
- **Which situations have no classified demand yet?** Read all assessment rows and inspect empty `demands`; no new selector needed. Three Mario follow-up rows are empty. `--intent undecided` matches7 assessments, not all10: unknown demand classification and undecided intent on an existing entry are different states.
- **What counts as success, failure or a valid alternative?** Agent can review task/context and supporting rules. Criteria remain prose; no exact native correctness/optional-goal filter or game-outcome classifier. Distinguish author-selected local goals from declared game success/failure/feedback.
- **Is this knowledge, discovery, reaction or dexterity?** Native returns explicit labels, not diagnoses. Agent can compare available information, controller requirements and timing declarations; missing hints/windows/mapping prevent firm claims. A timer or failed action alone is insufficient.
- **Is this calculation, optimization, trade-off, puzzle or dilemma?** Exact label lookup works if authored. Establishing category needs goals, alternatives, constraints, information and relevant criteria. No algorithm derives meaningfulness, probabilities, dominance, global optimum or conflicting values from route counts/skill links.
- **Did player learn, misunderstand, mistime or choose a sacrifice?** Not established. Declared practice/mastery links and rule outcomes cannot identify player knowledge, performance cause, enjoyment or difficulty. Need appropriately scoped evidence.
- **Would wider timing or destination disclosure preserve wanted choice?** Agent can propose a comparison and identify intended invariants. No alternative adopted or measured here; this is not causal evidence or automatic redesign.

**Recommendation:** keep task/judgment distinctions in prose for now. Current reader already preserves separate situations and exposes unclassified rows. Before extending schema/query, choose an exact unanswered question and distinguish local goal criteria, game-declared judgment and observed player outcome. Do not add `correct_choice`, infer unwanted demand, or tag discovery merely to populate filters. Remaining vocabulary/example proposals still need review; no viewer work follows automatically.

#### Follow-up checks

- Default decision/actor/authoring tests:17 passed,6 skipped. Four external integration checks unconfigured; two deep capacity probes deliberately not rerun. New real-example assertions verify five distinct local IDs, empty demand lists and execution/intent filtering without cross-situation leakage.
- Strict native Attendant:225 Mario records; zero diagnostics/corrections. Both changed headers and Route sections unchanged from Git checkpoint.
- Five Mario controls/matrix/rules/scaffolding/MDA semantic comparisons unchanged, excluding source-line provenance. Crate sources unchanged.
- No native code change or compiler build; existing focused CJIT executable used. No experiment, viewer launch/mutation or new visual acceptance. Follow-up committed as checkpoint.
