# Mechanics, Dynamics, Aesthetics

Pack: `mda` · version 1
Attribution: Robin Hunicke, Marc LeBlanc, Robert Zubek (2004)

## Contents

- [Purpose and locus](#purpose-and-locus)
- [Three views and two perspectives](#three-views-and-two-perspectives)
- [Aesthetic vocabulary](#aesthetic-vocabulary)
- [Dynamic models, feedback, and tuning](#dynamic-models-feedback-and-tuning)
- [Inputs and workbench ownership](#inputs-and-workbench-ownership)
- [Apply to a selected slice](#apply-to-a-selected-slice)
- [Worked example and review artifact](#worked-example-and-review-artifact)
- [Practical questions and counterchecks](#practical-questions-and-counterchecks)
- [Common mistakes](#common-mistakes)
- [Source tensions and transfer limits](#source-tensions-and-transfer-limits)
- [References](#references)

## Purpose and locus

MDA supports translation between implemented rules, behavior during play, and desired experience. Use it to explain why a proposed rule might support an experience, trace an unexpected play pattern back toward its conditions, or examine how tuning one component could affect others. The paper connects design, criticism, and technical research through this shared vocabulary. It is not just a way to sort features into three lists.

**Unit of analysis:** a bounded relationship across layers. Select an encounter, recurring decision, subsystem interaction, or local experience intention. A useful account names the rules and content involved, what happens over time under particular inputs, and why that behavior might matter experientially. Whole-game discussion is possible, but a local trace cannot establish whole-game quality.

Use when a question crosses layers: “Could this resource rule encourage the kind of commitment we intend?” Do not use MDA alone to certify accessibility, audience fit, learning, commercial appeal, emotional response, or overall fun. It supplies neither a universal fun formula nor a validated player questionnaire. Do not run it as an automatic framework sweep.

**Review draft:** provisional pending human source/completeness review. Application below is an independently authored workbench procedure, not a procedure or instrument reproduced from the sources. Reading or applying this guide does not authorize record edits, experiment execution, goal revision, or design adoption.

## Three views and two perspectives

The paper first pairs the consumption terms rules, system, and “fun” with the design terms mechanics, dynamics, and aesthetics. Its figures show a relationship between layers, not three independent bins.

- **Mechanics:** particular game components at the level of data and algorithms. The later Mechanics section also includes afforded actions, control mechanisms, and supporting content. Read this as the specified material from which behavior develops: state, available operations, conditions, transitions, parameters, and relevant level/content arrangements. A rule can specify what happens on a push without specifying which pushes a player chooses.
- **Dynamics:** runtime behavior when mechanics act on player inputs and one another's outputs over time. Describe a sequence, recurring pattern, or state-dependent relationship. Betting rules are mechanics; bluffing can be a dynamic. Spawn locations are mechanical/content arrangements; camping can be a dynamic. Neither outcome follows from the feature name alone.
- **Aesthetics:** desirable experiential/emotional responses during interaction. These are design aims and ways of describing appeal, not only visual styling. An aesthetic intention says what interaction should be like for a player; it does not establish what any player felt.

**Workbench shorthand: Rules–Behaviours–Feelings (RBF).** Use this plain-language bridge when it helps: what rules allow or constrain, what happens during play, and what that experience could feel like. It is not the paper's terminology or a replacement framework. Rules also involve components/content; behaviours include system behavior, not only player choices; feelings include experiential qualities and appeals, not only discrete emotions. Trace the relationships rather than assume a rule guarantees a feeling. Design can start at any endpoint.

**Designer perspective:** start with operable mechanics, consider the dynamics they generate, and consider the experiences those dynamics could support. Small implementation changes can cascade across layers. This is a causal design account, not proof that any particular proposed chain is correct.

**Player perspective:** experience sets the tone; observable behavior and eventually operable rules make that experience available. The reverse-facing figure changes the vantage point, not the physical direction of causation. It does not assert that every player consciously discovers mechanics in a fixed order. Ask what a player can perceive and act on, not just what a designer knows exists.

**Experience-driven design:** reason backward from an aesthetic aim toward behaviors that could sustain it, then toward rules/content that could enable those behaviors. Inspect forward again for competing outcomes and side effects. Neither direction alone closes the loop; play evidence and iteration can revise the model.

Workbench distinction: `systems` are constructed arrangements of mechanics. They are not automatically MDA dynamics. The paper's broad word “system” does not define the workbench `systems` role. Likewise, an individual `aesthetics` record is not the overall concept or player story. Compare the local aesthetic with the broader purpose, desired arc, and constraints without translating the whole brief into taxonomy labels.

## Aesthetic vocabulary

**Start by naming the intended experience.** Use the [working emotion vocabulary](../references/emotional-experience/emotions.md) directly when articulating an aesthetic: quiet satisfaction, relief, curiosity, admiration, mixed feelings, or another supplied meaning. Specify whose feeling it is, what it concerns, and what could make it meaningful where that helps. Use [shared emotional-experience framing](../workflows/emotional-experience.md) for relational detail, overlap, or transitions. These resources support aesthetics authoring as well as explicit emotion-led briefs; no separate framework selection or emotion interview is required.

An `aesthetics` record can own a specific felt intention without choosing one of MDA's eight labels. An appeal such as Challenge or Fellowship may help explain the intended experience, but it does not substitute for naming the particular feeling. Conversely, an aesthetic can concern sensory, expressive, or other experiential qualities without naming a discrete emotion. Keep detailed meanings in the shared vocabulary rather than duplicate a separate emotion dictionary here.

The 2004 paper offers eight labels to replace an undifferentiated claim of “fun.” These are **non-exhaustive vocabulary**, not compulsory categories, personality types, exclusive genres, or scores. A game can pursue several, with different emphasis and possible tensions. Use only labels that clarify the selected question; retain a more specific authored intention where useful.

The following meanings paraphrase the paper's list and examples:

- **Sensation:** appeal through sensory experience. Ask which perceptible qualities of interaction matter; high asset detail alone does not establish sensory pleasure.
- **Fantasy:** appeal through make-believe or inhabiting an imagined role/world. This is not restricted to the fantasy genre.
- **Narrative:** appeal through drama. Ask how events, stakes, and their development matter during play; a supplied plot is not evidence that play delivers drama.
- **Challenge:** appeal through confronting obstacles. Identify what makes overcoming an obstacle engaging rather than equating difficulty or failure rate with enjoyment.
- **Fellowship:** appeal through a social framework. Consider how play creates shared activity and relationships; multiplayer support alone does not establish this experience.
- **Discovery:** appeal through encountering uncharted territory. Territory may be a world or an unfamiliar possibility within play; a large content inventory alone does not show discovery.
- **Expression:** appeal through self-discovery and leaving an individual mark. The paper's dynamic examples include personalized characters and changing worlds. A customization menu is a possible means, not experiential proof.
- **Submission:** appeal through play as a pastime. Do not read this as submission to other players, obedience, incompetence, or a measured mental state.

These practical distinctions are application guidance, not extra published categories. The paper itself uses Competition in its Quake example although Competition is not one of the eight listed labels. Do not silently add a ninth mandatory category or force competition into Challenge. Its presence illustrates vocabulary beyond a closed list.

LeBlanc's undated homepage keeps the eight labels but changes three glosses: Narrative becomes unfolding story rather than drama; Expression becomes soap box rather than self-discovery; Submission becomes mindless pastime rather than pastime. These emphasize somewhat different readings. Name the source/version when the difference matters. Do not infer a dated revision sequence from an undated page, or use “mindless” to infer a player's cognition. This guide anchors the model in the 2004 paper while retaining the homepage variation.

## Dynamic models, feedback, and tuning

A dynamic model makes an account specific enough to question. Identify relevant state, player inputs, transitions, information, and consequences across time. It may be a qualitative sequence, a feedback sketch, or a mathematical model with justified assumptions. MDA does not require equations or a simulator.

The paper gives two distinct tools:

- **Distribution model:** its two-six-sided-dice figure shows that sums are not equally likely. This helps reason about board progress under stated assumptions. It does not model a player's understanding, choices, or enjoyment. A correct local distribution is not a complete game model.
- **Feedback model:** a thermostat illustrates state being sensed and fed back into action that changes the state. The Monopoly figure instead traces unequal wealth through movement, property payments, and further wealth changes. The account describes a reinforcing gap: richer players can penalize others more effectively, while poorer players lose further ground. Reinforcing feedback can amplify a lead; corrective feedback can oppose a deviation. These are descriptions of loop behavior, not synonyms for good and bad design.

Distinguish **system feedback** from **player-facing feedback**. A recurring wealth loop changes future conditions. A message or animation makes state/consequence perceptible. Both can affect experience, but showing a clear message does not necessarily alter the underlying loop.

For a selected loop, explain what feeds back into what, through which rule, and over what timescale. Mark missing replenishment, exit, or reset conditions as unknown. Do not invent a cycle merely because two records link to each other.

The paper's Monopoly analysis proposes helping trailing players, constraining leaders, or accelerating resolution. These are alternative interventions, not universal fixes. A change intended to sustain competition may weaken a theme; a tax intended to improve pacing may add calculation burden and obscure standings. Evaluate the whole affected path rather than optimizing one number in isolation.

Tuning therefore means iterative translation: state an experiential aim, predict dynamic effects, change a rule or parameter with authority, inspect resulting behavior and experience, and revise. The guide authorizes analysis/proposals only. In this workbench, values belong in `control-vars`; experiment hypotheses, local measures, assumptions, observations, and results belong in `experiments`. Do not invent computed emotional effects or imply a proposed tuning value was tested.

## Inputs and workbench ownership

Primary roles: `mechanics`, `systems`, `dynamics`, `aesthetics`, `experiments`. Follow relevant links into existing `interactions`, `control-vars`, `resources`, challenges, or content only as needed for the selected slice. Keep existing collection roles; no separate measures, decisions, or skills collection.

Inspection topics: MDA relationships, experiment evidence, and relevant source records.

Read the actual concept/mode and linked record bodies, not only names or relationship arrays. Useful inputs include:

- Selected question, encounter boundary, current rule/content version, and intended experience if supplied.
- Actions, state transitions, costs, resources, timing, information available before/after action, and relevant configuration references.
- Predicted/observed behavior, competing explanations, and investigation context.
- Actual observations and player accounts when available, with source/date/context and limitations.

In `game-design` mode compare the chain with supplied intent. In `game-explorer` mode investigate candidate experiences without treating an absent destination as a defect. No mode change or adoption follows from framework fit.

Keep three evidence states distinct: **authored fact/intention**, **hypothetical interpretation**, and **supported observation**. An accepted record can contain an unverified prediction. An empty observations section means no recorded evidence, not that the claim failed. Missing optional links mean undocumented relationships, not automatically missing design.

## Apply to a selected slice

1. **Bound the question.** Choose one decision or encounter and one starting endpoint: rule, behavior, or intended aesthetic. Name scope and relevant version/draft state. Done: claims will not silently expand to every player or the whole game.
2. **Recover the authored account.** Read the concept, rules, linked dynamics/aesthetics, relevant configuration, and experiment bodies. Separate exact supplied facts from unknown conditions. Done: the starting chain cites actual owners and preserves missing data.
3. **Describe mechanics operationally.** State what action is available, under which conditions, what changes, what does not change, and what information a player gets. Include content arrangements only when known. Done: another reader can distinguish a specified transition from a guessed strategy.
4. **Build the dynamic account.** Describe a plausible sequence over time: what the player notices, chooses, encounters, and can do next. Name assumptions about information, goals, alternatives, timing, and consequences. Add a feedback model only when repetition actually closes a loop. Done: each behavioral claim has a mechanism and explicit evidence status.
5. **Translate between perspectives.** From mechanics, ask which experiences several possible behaviors could support. From the selected aesthetic, ask what behavior would make it plausible and whether the player can perceive the conditions. Compare the local A aspect with the concept/player story. Done: there is an explained bridge, not three adjacent labels.
6. **Construct a counterpath.** Keep the same rule and vary a consequential assumption, such as legibility, strategy, retry cost, or available alternatives. Trace a different possible dynamic/experience. Done: preferred interpretation has a serious rival and a condition that could weaken it.
7. **Identify a bounded discriminator.** Suggest an observation or comparison only when useful. Separate behavior seen from experience reported; place any later retained investigation in `experiments` under separate authority. Done: proposal distinguishes rival accounts without claiming execution or emotional measurement.
8. **Return a review artifact.** Summarize the scoped chain, assumptions, evidence, counterpath, relation to concept, and unresolved question. Offer tuning directions as hypothetical alternatives, not adopted changes. Done: human can challenge the reasoning and choose whether further work is warranted.

Stop at a useful partial chain when inputs are missing. Request only missing facts that materially block the question; do not require every aesthetic category or relationship to be filled.

## Worked example and review artifact

This example checks actual tutorial text but does not review a playable build. Sources: [concept](../examples/crate-puzzle/game-design/concept.md), [crate-pushing](../examples/crate-puzzle/game-design/mechanics/crate-pushing.md), [careful-positioning](../examples/crate-puzzle/game-design/dynamics/careful-positioning.md), [deliberate-tension](../examples/crate-puzzle/game-design/aesthetics/deliberate-tension.md), and [planning investigation](../examples/crate-puzzle/game-design/experiments/energy-encourages-planning.md).

**Authored facts and intentions:**

- Concept uses `game-explorer`; it offers a candidate direction, not a completed game or adopted destination for every proposal. Its player story is explicitly hypothetical, not a participant quote or room specification.
- Successful push requires a free destination and enough energy. It moves the crate one cell and spends `push-cost`. Wall-blocked and insufficient-energy pushes move nothing and cost nothing. Pulling is unavailable.
- Careful-positioning predicts consideration before action. Deliberate-tension names a local intention at commitment. Their accepted status does not turn prediction into observation.
- Concept aims more broadly at thoughtful commitment, satisfaction when a plan works, and willingness to retry. Tension alone cannot stand in for that arc.
- Planning investigation has no defined measures, no observations, and no execution. [Route-cost](../examples/crate-puzzle/game-design/experiments/route-cost.md) is also unexecuted; its assumed correct actions/delays and analytical timing reference do not establish human planning or feeling.

**Hypothetical interpretation:** finite successful pushes plus unavailable pulling could make players consider later positions before committing. If costs and consequences are understandable and meaningful, this could support deliberate tension. This is a plausible M–D–A account, not a demonstrated effect. Actual geometry, information presentation, and retry context remain insufficient to certify a solution or experience.

**Hypothetical rival:** the same spending rule could encourage conservative trial avoidance rather than useful planning if consequences are hard to anticipate. Pauses could reflect confusion; repeated blocked attempts could be cost-free probing rather than commitment. This does not establish that either problem occurs.

A concrete output could be this ordinary review note; it is not a new required schema or a record edit:

```text
MDA review draft — one crate-push commitment
Scope: documented rule and intentions; no build or play observations.
M: Successful push spends push-cost; blocked/low-energy attempt costs nothing;
   pulling unavailable. Layout and information presentation unresolved.
D hypothesis: Player compares future positions before a successful push.
Assumptions: Cost understood; consequences can be anticipated; choices matter.
A intention: Deliberate tension at commitment, not the entire player story.
Concept relation: Possible support for thoughtful commitment; possible tension
   with willingness to retry if scarcity discourages experimentation.
Rival: Unclear consequences produce hesitation or cost-free probing instead.
Evidence: Authored prediction/intention only; planning study not executed.
Discriminator proposal: In a future bounded playtest, inspect action sequences
   and player explanations of intended moves and costs; collect experience
   accounts separately. Pauses alone cannot distinguish the two accounts.
Open: What information is available before commitment, and what makes retry
   understandable? No tuning, record adoption, or experiment execution authorized.
```

If a hypothetical legibility change were later considered, predict both paths: clearer advance information might support planning, but might also remove uncertainty a designer wanted. The source does not select that trade-off for this game. Any measures, method, hypotheses, and observations retained later belong together in an experiment, not a new collection.

## Practical questions and counterchecks

- Which state transition makes the named behavior possible, and which player choice makes it likely? If either is absent, is the account incomplete or merely undocumented?
- What can a player perceive before choosing? Does the designer's explanation depend on hidden knowledge?
- What repeats, and what changes with repetition? Does a claimed feedback loop actually return to affect its starting condition?
- Could the same mechanism support several aesthetics, or undermine the selected one under another strategy? Which contextual condition separates those paths?
- Which part of the intended player story is outside this local aesthetic? Could maximizing tension weaken satisfaction, recovery, or another stated aim?
- Would behavior persist without the relevant constraint? What else changes in that comparison, and could it explain the result?
- What would count against the proposed chain? Is a suggested observation able to distinguish its rival, or merely confirm that an action occurred?
- What side effect could a tuning change introduce elsewhere: pacing, information burden, dominant strategy, or thematic conflict?

Observe behavior and reported experience separately. A pause may be planning, confusion, interruption, or something else. A player account supports a contextual interpretation; it does not establish universal appeal or causation. Keep counterexamples, strategies, and changed conditions visible. No inference of capability, mastery, or emotion follows from completion time or declared links.

## Common mistakes

- **Feature inventory as analysis:** three populated lists do not explain relationships. Replace adjacency with conditional, temporal accounts.
- **Renaming rules as dynamics:** “energy system” names an arrangement. Explain how energy state and choices change play over time.
- **Aesthetics as visual polish:** include sensory appeal where relevant, but do not reduce MDA A to art style.
- **Taxonomy completion:** eight labels are not eight design requirements. No aggregate fun or psychological score follows from category coverage.
- **One-to-one mapping:** one mechanic can support several dynamics; the same behavior can have different experiential meanings in different contexts.
- **Predictions as facts:** accepted links and well-formed chains are not causal evidence or measured emotion.
- **Tuning as number search:** a change may alter understanding, strategies, and other goals, not just a target outcome. Do not invent an optimal value.
- **Local aesthetic as whole concept:** a tension record cannot replace the player story or silently revise the project's direction.

## Source tensions and transfer limits

The paper describes mechanics both narrowly as data/algorithms and broadly through actions, objects, and content. Preserve this breadth when analyzing a game; use existing workbench owners to distinguish rules, entities, configuration, and arrangements rather than forcing everything into `mechanics`. This is a storage adaptation, not a correction of the source.

The paper calls the lenses causally linked and predicts experiential consequences. A source's causal design model does not validate a particular workbench chain. Local evidence must support local claims. The paper's illustrative Monopoly fixes and three AI design passes demonstrate translation and interdependence, not controlled trials or universally applicable prescriptions. Its demographic assumptions in the AI passes do not establish audience preferences and must not be transferred as player facts.

Homepage wording differs as noted above; neither source establishes a complete or exclusive aesthetic taxonomy. Do not merge their glosses into a supposedly definitive later version. Local sources do not establish completeness of MDA scholarship or this pack.

Both full captures were read, including the paper's conclusions/references and the homepage's workshop/lecture sections. Essential diagrams inspected: rules/system/fun, M/D/A, opposing designer/player viewpoints, two-dice distribution, thermostat, and Monopoly feedback. Text extraction loses original pagination and some punctuation; diagram labels have no separate OCR transcript. Use section names and retained images as locators; do not invent page numbers. No source-critical category/relationship figure remains unread for this guide, but original PDF layout was not independently rechecked here.

Linked slides, workshop archives, recordings, and *Tools for Creating Dramatic Game Dynamics* were not read or captured for this application. They cannot supply unstated definitions or validation. Captures are local-only and not publication-cleared; public availability is not a redistribution license.

## References

- Robin Hunicke, Marc LeBlanc, Robert Zubek (2004), [*MDA: A Formal Approach to Game Design and Game Research* — actual full local capture](../references/mda/captures/hunicke-leblanc-zubek-2004/MDA.md). Reviewed full text. Key locators: Introduction and Towards a Comprehensive Framework (translation/interdependence); MDA and MDA as Lens (definitions/perspectives); Aesthetics and Aesthetic Models (vocabulary and goals); Dynamic Models, Mechanics, Tuning (models and iteration); MDA at Work, First/Second/Third Pass, Wrapping Up, Conclusions (AI example and transfer). Essential figures inspected: [rules/system/fun](../references/mda/captures/hunicke-leblanc-zubek-2004/images/MDA-002.png), [M/D/A](../references/mda/captures/hunicke-leblanc-zubek-2004/images/MDA-003.png), [designer/player](../references/mda/captures/hunicke-leblanc-zubek-2004/images/MDA-004.png), [two dice](../references/mda/captures/hunicke-leblanc-zubek-2004/images/MDA-005.png), [thermostat](../references/mda/captures/hunicke-leblanc-zubek-2004/images/MDA-006.png), [Monopoly feedback](../references/mda/captures/hunicke-leblanc-zubek-2004/images/MDA-007.png).
- Marc LeBlanc (undated), [*8KindsOfFun.com: The collected game design rants of Marc “MAHK” LeBlanc* — actual full local homepage capture](../references/mda/captures/leblanc-8kindsoffun/page.md). Reviewed full page. Key locators: The List (alternate glosses); Papers / MDA (author's account of conceptual/prose contributions); Game Design Workshop (play/analyze/modify context); Lectures (bibliographic leads only, not reviewed slide content). No publication date inferred from HTTP metadata.
- [MDA capture index, provenance, fidelity, exclusions, and rights context](../references/mda/README.md). Captured 2026-10-21; original PDF not retained, seven figures retained. Independent notes are separate from actual captures. No explicit redistribution/adaptation license established; human rights review required before publication. Stepwise procedure, counterchecks, and worked review artifact above are workbench adaptations, not proprietary instruments or source-authored tests.
