# Decision structure — working vocabulary

Status: working vocabulary. User confirmed shared contextual representation and test/quiz distinction, and added reaction/dexterity tests. Remaining terms/examples stay under review. Not a universal taxonomy, schema, classifier or quality score.

- [Purpose](#purpose)
- [Context required](#context-required)
- [Design intent checks](#design-intent-checks)
- [Terms and examples](#terms-and-examples)
- [Time dimension](#time-dimension)
- [Classification shape](#classification-shape--proposed-not-schema)
- [Existing data owners](#existing-data-layer-candidates)
- [Open questions](#open-questions)

## Purpose

Describe quality and nature of decisions available to players, not merely count available actions. Supply definitions, examples, boundary cases and evidence requirements before proposing stored classifications. Use terms both to describe existing situations and to question whether a proposed mechanic asks for the kind of performance designer wants. Terms not established by user below remain provisional; promoting this reference does not adopt every category or example.

User-established distinctions:
- Available options do not by themselves establish meaningful alternatives.
- Calculation requires work to determine which option is best; players may externalize that work through spreadsheets, guides or searching for an answer.
- Optimization concerns choosing constrained improvements toward a known goal, such as developing an RPG build when desired end state cannot simply be obtained. It can involve calculation but is not the same demand.
- Dilemma has no plainly good or bad answer on its face, yet something must happen. Trolley problem is reference example; do not reduce this to every sacrifice, all outcomes being bad, or mixed Nash equilibrium.
- Time available to deliberate is a first-class dimension across decision kinds. A dilemma or math problem in a <200ms instant-feedback loop may demand too much deliberation; this is a design concern, not a measured universal threshold.
- Include tests/quizzes: game can judge a response as wrong or failed, rather than merely less beneficial or a matter of play style. User's example is Mario walking into an enemy and dying. In existing source, harmful side contact by small, unprotected Hero is the relevant lethal case; do not generalize to protected/powered contact.

- Reaction tests demand a response within a specific short time span. Keep this distinct from dexterity and knowledge demands.
- Dexterity tests demand performance of particular controller buttons, motions or sequences. Changing input mapping can reduce this demand without removing knowledge needed to choose the action.

Examples below are hypothetical unless explicitly identified as existing source evidence. Proposed classifications are not new authored facts or observed player results.

## Context required

Assess a decision relative to player, goals, available information, state, constraints and time horizon. Distinguish actual consequences from what player knows or believes. Designer knowledge is not automatically player knowledge. Include available deliberation time, preparation opportunities, familiarity, execution demands and feedback timing; these can change nature of experienced decision.

Record relevant outcomes, costs, uncertainty and reasons to prefer alternatives. Missing evidence stays unknown. A category does not establish enjoyment, difficulty or quality.

## Design intent checks

Use vocabulary during design, not only after data exists. Ask: **“This is a dexterity test. Do you want it to be?”** Ground demand in specific input requirements; qualify as a candidate when facts are incomplete.

Declared input/timing requirements can establish a mechanical demand without a playtest. Whether players find it difficult or enjoyable is a separate empirical question.

Keep three claims separate:
- Current/proposed demand: what rules, controller mapping, information and timing ask player to do.
- Intended demand: what designer wants player to understand, choose or perform, and how that serves intended experience.
- Observed demand/difficulty: what actual players experience, supported by contextual evidence.

If demand is wanted, review its fit, support and consequences. If unwanted, propose alternatives that reduce it while preserving wanted demands. Fighting-game motion inputs versus one button per action can expose this distinction; simplifying input need not remove action-choice knowledge. Do not change mechanic or assume intended demand without authorization.

Useful review questions:
- Is this intended to test knowledge, reaction, dexterity, planning, or some combination?
- Does the response window allow intended deliberation, or primarily demand recognition and execution of a prepared response?
- Is a failed response wrong under this task, merely less beneficial, or valid under another preference?
- Would simpler input preserve meaningful choice while removing unwanted execution demand?

A mismatch is a design question, not automatically a defect. No required dexterity level, mandatory simplification, automatic redesign or aggregate quality score follows. Use Challenges for local task/assessment and unresolved intent fit; retain rules, behavioral hypotheses and observations in their existing owners.

## Terms and examples

### Option

Action or course of action available under stated conditions. Describes availability, not value or difficulty.

Example: accept 1000 dollars or accept 999 dollars.

Boundary: an apparent button that cannot be chosen is not an available option under those conditions.

### Nominal selection / dominated alternative

Options exist, but one is worse under all relevant stated criteria, with no compensating benefit. “Nominal selection” describes decision when superior alternative is apparent to player.

Example: receive 1000 dollars versus 999 dollars under otherwise identical conditions, when goal is more money and no relevant downside exists.

Boundary: greater reward with greater risk is not the same example. A dominated option can also have a learning, expressive or role-playing purpose; establish relevant goals rather than assume them.

Evidence: same conditions, relevant preferences/criteria, consequences and player-visible information. Unknown downside prevents a dominance claim.

### Calculation

Work required to determine which option is best under supplied facts and criteria. Relevant demand is discovering or comparing answers, not merely selecting an already-obvious winner.

Example: compare damage, attack speed and modifiers to determine which equipment combination performs best under specified conditions. Work may move into a spreadsheet, guide or web search.

Boundary: once player knows answer or follows a guide, same situation may no longer require calculation from that player. Unknown consequences or competing values add other demands.

Evidence: inputs, relationships, criterion, constraints, player knowledge, available time and access to external aids. Do not infer difficulty or player research behavior from presence of numbers alone.

### Optimization

Choose constrained improvements toward a known goal when desired result cannot simply be obtained. Emphasis is constructing or improving an available solution, potentially through successive decisions, not merely computing an answer.

Example: develop an RPG build toward a desired combat role while choosing among currently affordable equipment, skill points and prerequisites. Each choice attempts to bring build closer to goal; ideal build is not immediately available.

Boundary: calculation may help compare improvements; trade-offs may prevent improving every property. These descriptors can coexist. Do not require a numeric distance-to-goal or a provably optimal final build.

Evidence: target, current capabilities, feasible improvements, constraints, dependencies and what counts as progress. Keep this working game-design meaning distinct from a mathematical claim of global optimality.

### Puzzle

Bounded problem organized around discovering a solution or satisfying success conditions through reasoning, inference or search. May contain calculations and optimization; not a synonym for every decision with a correct answer.

Example: arrange crates so a route opens to an exit.

Boundary: knowing required move but struggling to time its input primarily tests execution. A problem can combine both demands.

Evidence: goal/success conditions, relevant rules, information available and solution constraints. Player's prior knowledge affects experienced demand.

### Trade-off

Alternatives improve different relevant criteria at expense of others. Which is preferred depends on goals, priorities or circumstances.

Example: take shorter hazardous route or longer safer route.

Boundary: explicit preferences and known consequences may turn comparison into a calculation. Choosing a feasible improvement toward a target can also involve optimization. No universal classification independent of context.

Evidence: competing criteria, consequences, preferences and circumstances. Sacrifice alone does not establish dilemma.

### Risk-based decision

Consequences have specified or estimated probabilities; decision depends on possible outcomes and attitude to risk.

Example: certain reward of 5 versus a 50% chance of 12 and otherwise 0.

Boundary: larger expected reward is not automatically preferred. Unknown probabilities are not known risk merely because a designer could assign a number.

Evidence: outcome distribution and its provenance, stakes, goals and relevant risk preferences. No inferred observed success rates.

### Decision under uncertainty / information gathering

Relevant facts, consequences or probabilities are unknown to player; investigation, inference or commitment under incomplete information matters.

Example: scout an unknown route at a time cost before committing to it.

Boundary: incomplete design documentation is not proof that player experiences uncertainty. Unknown model facts and deliberately hidden player information are different.

Evidence: what is known, hidden, learnable, signalled and costly to discover.

### Strategic / adversarial decision

Value of an action depends on other decision-makers' actions or responses. Prediction, adaptation, coordination or avoiding exploitation can matter.

Example: rock-paper-scissors; no fixed pure action beats every opponent action.

Boundary: a fixed authored enemy script is not automatically a strategic opponent. A mixed strategy is not a measure of bad outcomes or emotional conflict.

Evidence: agents, action timing, information, consequences/preferences and response possibilities. Do not infer equilibrium without a specified model.

### Dilemma

Situation with no plainly good or bad answer on its face, yet something must happen. Competing considerations do not reduce to an uncontested answer. Acting and allowing an outcome through inaction may both matter.

Example: trolley problem. Different interventions or non-intervention engage conflicting consequences, duties and responsibility; counting outcomes alone does not settle every relevant consideration.

Boundary: not every trade-off, sacrifice or unknown fact establishes a dilemma. Do not reduce category to “all outcomes are bad.” Rock-paper-scissors and mixed Nash equilibrium describe a different strategic property, not an automatic dilemma classification. A payoff matrix may model selected considerations without settling whether it captures what matters.

Evidence: alternatives including consequential inaction, conflicting values/considerations, consequences, framing, what player understands and why no answer is plainly preferable. No universal moral ranking inferred.

### Test

Decision or performance situation with a game-defined criterion for success/failure or correct/incorrect response. An available action can be a failed response, not merely an alternative with a smaller reward or a different play style.

Example supplied by user: Mario walks into an enemy and dies. For existing data, use `interactions/touch-walker-small#lethal-side`: small, unprotected Hero makes harmful side contact and becomes dead. Under task of getting past encounter alive, this is a failed response.

Boundary: failure is relative to task and conditions, not a universal verdict on an action. Protected/powered contact differs. A death may serve another explicitly stated goal; do not silently reuse survival criterion in that context. A declared lethal consequence is not an observed player failure or proof of why it occurred.

Evidence: task, conditions, success/failure criterion, relevant rule outcomes and game feedback. Keep knowledge error, execution error, experimentation, deliberate sacrifice and unknown explanation separate.

### Quiz

Knowledge-oriented test whose primary intended demand is recognizing, recalling, inferring or selecting a correct response, rather than performing it accurately. Knowledge and execution demands can overlap.

Example: identify which described contact defeats an enemy versus harms unprotected Hero, using rules player has been given opportunity to understand.

Boundary: failing a live platforming encounter does not by itself prove failed knowledge. Player may know correct response but mistime it. Avoid automatically classifying every hazard as a quiz.

Evidence: knowledge being assessed, available information, response criterion, player learning opportunity and execution/time demands. Declared skill links identify intent/opportunity, not demonstrated understanding.

### Reaction test

Mechanic or situation demands a response within a specific short time span following a cue, event or state change. Test concerns responding in time; choosing correct response may also test knowledge, and performing input may also test dexterity.

Example: recognize an incoming attack and trigger an appropriate response before its response window closes. A single-button response can still impose a reaction test.

Boundary: not every timer creates a reaction test. An extended course clock or turn deadline is not enough. Anticipation and familiarity may allow preparation rather than fresh recognition; distinguish reaction from executing a preplanned action at a known time. No universal millisecond cutoff or inferred player capability.

Evidence: trigger/cue, when it becomes perceptible, required response, window start/end, input acceptance/latency and consequence of missing window. Separate designed window from measured player reaction time.

### Dexterity test

Test whose demand includes accurately performing specific controller buttons, motions, sequences or coordinated inputs. Input performance is distinct from knowing which game action to choose. Relevant demand depends on controller, mapping, assistance and player capability, not only game action's name.

Example supplied by user: fighting-game moves/combos use motion or sequence inputs. A controller with a separate button for each action could reduce dexterity demand while largely preserving knowledge needed to choose which action to use and when. This is a hypothetical comparison, not a claim about a particular game or controller.

Boundary: accepting a button press does not by itself establish a substantial dexterity challenge. A one-button action may still demand precise timing, aiming or coordination. Simplifying mapping does not automatically remove reaction demand, and may alter timing too; compare resulting demands rather than assume all other conditions stay identical.

Evidence: action, device/mapping, required buttons/motions/order, coordination, precision, timing tolerances, assistance and failure criterion. Keep knowledge errors and input-performance errors separate; do not infer either from failure alone.

### Execution challenge

Main demand is performing an intended action accurately, quickly or consistently rather than deciding which action is preferable.

Example: jump a known gap with demanding timing.

Boundary: choosing when to jump may also involve prediction or a trade-off. Decision and execution demands can coexist.

Evidence: input/timing demands, feedback, recovery and relevant player capability. No measured difficulty without evidence.

## Time dimension

Decision kind alone does not describe demand placed on player. Distinguish:
- Deliberation window: how long player has before action or inaction commits an outcome.
- Preparation: whether relevant reasoning can happen beforehand, between encounters or outside game.
- Familiarity: whether decision is novel, learned, rehearsed or delegated to an external answer.
- Execution window: time/precision required to perform intended action, separate from deciding it.
- Feedback timing: when consequences become apparent and whether there is time to revise.

Contrast: RPG build planning may allow extended calculation and external tools; combat may require rapid execution of a previously chosen policy. A trolley-like dilemma with <200ms to respond demands something different from an unhurried one. It may become a test of recognition, reflex or prior commitment rather than considered deliberation. These are hypotheses to investigate, not observed player outcomes.

Do not treat <200ms as universal capability limit, assume instant feedback implies equal deliberation time, or silently give every decision a time limit. Unknown timing stays unknown.

## Classification shape — proposed, not schema

Use overlapping descriptors rather than one exclusive category. A timed puzzle can combine reasoning, uncertainty and execution. Risk can coexist with a trade-off or strategic interaction.

Keep separate:
- Structure: availability, dominance, competing criteria, target, feasible improvements and constraints.
- Information: known facts, estimated probabilities, hidden information, other agents and access to external aids.
- Demands: calculation, goal-directed improvement, search, inference, recognition/reaction, dexterity, execution and other supported demands. Reaction and dexterity can coexist with knowledge tests; they are not exclusive categories.
- Time: deliberation window, preparation, familiarity, execution window and feedback timing.
- Game judgment: explicit success/failure or correct/incorrect criterion, graded benefit, preference-dependent outcome, or unresolved judgment. These may coexist in one situation.
- Evaluation: why alternatives matter, intended experience, readability of consequences and supporting evidence.

A classification needs context, rationale, source references, epistemic status and unknowns. Human-authored intent, analytical hypothesis, model result and observed player behavior are not interchangeable. No compulsory label or aggregate score.

Illustrative assessment, not a proposed frontmatter contract:

```text
Subject: one route decision in a challenge
Goal: reach exit while conserving lives
Alternatives: short hazardous route; longer safer route
Descriptors: trade-off; uncertainty
Basis: route costs declared; hazard probability not established
Unknowns: player's knowledge; probability; preference between time and lives
Status: predicted decision structure, not observed behavior
```

## Existing data-layer candidates

Agreed direction: use existing data owners for contextual descriptions and consider optional structured classifications after definitions/examples are settled. No new collection or machine-readable field has been adopted.

Current source model has no dedicated decision collection or decision-classification field.

- Shared vocabulary/reference owns definitions. Existing glossary can point to canonical definitions; do not copy independent definitions into every record.
- Challenges own concrete goals/context and can identify a decision or test opportunity. Existing `introduces`, `practises`, `requires` and `tests_mastery` links support intended learning/assessment relationships; they do not encode complete response criteria or prove a player passed/failed.
- Interactions, mechanics and systems own relevant rules and arrangements, not automatically player judgments.
- Dynamics own intended/predicted/observed behavior with explicit epistemic status. Contextual claims about how players deliberate may fit here.
- Experiments own hypotheses, method, observations and interpretation used to test classification or quality claims.

Choose exact owner and optional machine-readable fields only after vocabulary and classification purpose are agreed. Do not add collections, migrate examples or implement automatic classification from this draft.

## Review outcome needed

- Definitions with examples and counterexamples sufficiently precise for discussion.
- Distinguish overlapping descriptors from mutually exclusive categories.
- Establish what can be authored, derived under explicit assumptions, or only investigated through play evidence.
- Test representative Mario/Crate situations without inventing source facts.
- Recommend data owner and minimal optional representation, retaining provenance and unknowns.

## Open questions

- Which remaining terms need different boundaries or additional examples?
- How should optional timing context be expressed without fabricating player capability or a universal deliberation threshold?
- Which task/response criteria, reaction windows and device-dependent input demands need structured representation?
- Which optional structured fields and queries best serve confirmed uses: reviewing existing decision opportunities and questioning whether proposed demands fit design intent? Six-record prose pilot precedes schema/query design.

## Comments

Formal terminology check: [Asuman Ozdaglar, Strategic Form Games and Nash Equilibrium](https://web.mit.edu/Asuman/Desktop/asuman/www/documents/Staticgameschapter.pdf), definition 4 and matching-pennies/penalty-kick examples, consulted for formal terminology only. Mixed Nash equilibrium is defined by absence of profitable unilateral deviation. Example payoff matrices demonstrate that a game without a pure equilibrium can still have a matrix representation and mixed equilibrium. This source does not validate proposed game-design vocabulary or classify repository examples.
