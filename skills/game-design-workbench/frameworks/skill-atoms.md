# Skill atoms and learning chains

Pack: `skill-atoms` · version 1
Attribution: Daniel Cook, The Chemistry of Game Design (original article; 2021 repost)

Provisional application guide. Human source/model review remains open. Cook's model and workbench record conventions are distinguished below; neither establishes observed learning.

## Contents

- [Purpose and locus](#purpose-and-locus)
- [The learning cycle and atom boundary](#the-learning-cycle-and-atom-boundary)
- [Chains, value, and learning states](#chains-value-and-learning-states)
- [Loops, arcs, and source versions](#loops-arcs-and-source-versions)
- [Inputs and workbench interpretation](#inputs-and-workbench-interpretation)
- [Apply to a selected design slice](#apply-to-a-selected-design-slice)
- [Worked example and review artifact](#worked-example-and-review-artifact)
- [Practical questions and counterchecks](#practical-questions-and-counterchecks)
- [Common mistakes](#common-mistakes)
- [Source and transfer limits](#source-and-transfer-limits)
- [References](#references)

## Purpose and locus

Use this lens to ask how interaction could change a player's usable understanding. Trace what the player expects, tries, perceives, and might learn; then ask where that learning becomes useful. A prerequisite graph alone cannot answer those questions.

**Unit of analysis:** a player–system learning interaction, represented as a skill atom, and its connections to other interactions. Focus can range from discovering one input's effect to coordinating a familiar set of actions into a new strategy. Cook's later loop account explicitly allows nested loops at different timescales. Choose a scale where the learned relationship can be stated and inspected.

Useful requests include:

- Diagnose an unclear causal relationship or feedback gap in one interaction.
- Examine how a learned tool supports a later challenge, including optional routes and assumed prior knowledge.
- Compare repeated experimentation with one-use information delivery.
- Form a bounded question about where learning might stall or lose perceived value.

Do not use this lens as a complete account of enjoyment, a diagnosis of a player's mind, or a reason to maximize tutorial steps. A familiar action may remain useful without teaching a new skill. Story, expression, social activity, and sensory experience need not justify themselves as prerequisites.

Read concept and working mode first. In `game-explorer`, explore candidate learning relationships without treating an absent destination as a defect. In `game-design`, compare them with supplied intent. Overall concept/player experience is not an individual MDA aesthetic. Cook's criticism of MDA in his bibliography is his position, not authority to merge those owners or replace the brief.

## The learning cycle and atom boundary

### Five elements, not a button list

In *Chemistry*, §3, Cook describes five elements:

1. **Decision:** player notices known affordances and weighs possible outcomes. Prior understanding shapes which actions seem possible or worthwhile.
2. **Action:** player acts. This may be a button press or a coordinated batch, such as navigating a maze. Motor input is not always the useful analysis scale.
3. **Simulation:** game rules process the action and update state. Identify relevant entities and conditions, not every implementation detail.
4. **Feedback:** game communicates the change through visible, audible, tactile, or symbolic information. A state transition and the player's access to it are different things.
5. **Modeling:** player interprets feedback and may revise a causal mental model. That revised model informs another attempt or another loop.

The shorthand atom diagram shows action, simulation, feedback, and a resulting model update. It compresses decision/modeling; it does not eliminate them. Diagram 4 additionally separates rules, tokens, and state change inside simulation, and depicts filtering feedback into useful information, tools, or information that does not advance understanding. Its labels/numbering differ from the five-element prose. Use the prose cycle for explanation, with the diagram as elaboration rather than a second mandatory taxonomy.

An atom's meaningful output is a usable relationship, not just success or an animation. Cook's jumping example connects a press to an avatar jump; reaching a platform is a further relationship learned by using jumping. Multiple attempts may be needed to distinguish causation from coincidence. Conversely, a player may already know the relationship before encountering this game.

### Choose a boundary by the learning claim

State a candidate model update in plain language: “Under condition X, action Y changes Z, so I can use it for purpose P.” This sentence is a workbench aid, not Cook's required notation.

Then bound the atom:

- Include conditions that change the causal claim. “Push moves a crate” may hide blocked destinations and insufficient energy.
- Keep the simulation distinct from feedback. Knowing that a rule updates the exit does not establish that a player can notice why it changed.
- Split when two different relationships can be learned or fail independently. Combine when already familiar actions serve one new coordinated understanding.
- Stop splitting when detail no longer changes the review question. Do not require an atom for every key, tile, UI cue, or rule clause.

A failed action can still supply useful feedback and support model revision. A successful action can teach nothing, or support a mistaken explanation. Record both possibilities rather than equating outcome with learning.

## Chains, value, and learning states

### What connects atoms

Cook's skill chain is a directed network of interaction loops. A skill gained from one atom enables actions in another. This represents learning through use: newly discovered jumping becomes a tool for reaching platforms, rather than merely a node scheduled before a platform node.

**Pre-existing skills** seed the network. Familiarity with controllers, reading a display, spatial reasoning, and genre conventions are possible assumptions to investigate, not universal player traits. Underestimating prior knowledge can make instruction redundant; overestimating it can make initial interactions inaccessible.

**Perceived value** helps explain why a player might continue. Cook argues that nearby clues to useful learning can sustain exploration even when distant benefits are unclear. Inspect the visible promise of a next use. Do not turn his “couple atoms” prediction horizon into a fixed numerical design rule or infer motivation from a connected edge.

A chain and a challenge route answer different questions:

- Chain: what understanding supplies a tool for what further interaction?
- Route: what opportunities might a player encounter, and in what declared order?
- Observed play: what did this player actually encounter, attempt, notice, and use?

None substitutes for the other two. A missing link can be an undocumented assumption rather than proof that play is impossible.

### Cook's five status labels

*Chemistry*, §4 and Diagram 10, distinguish:

- **Mastered:** recently acquired skill, represented as a completed model update.
- **Partially mastered:** player is exploring a relationship without yet mastering it.
- **Unexercised:** interaction has not yet been attempted.
- **Active:** already mastered skill is being used as a tool for further activity; repetition need not mean continued discovery of that same skill.
- **Burned out:** player has lost interest in exercising the interaction loop.

These are Cook's player-state interpretations, not required workbench fields, mutually exclusive permanent traits, or values inferable from rank. A declaration that a challenge `tests_mastery` states intent, not an observed status. Even repeated performance needs contextual interpretation; chance, imitation, assistance, or a familiar strategy can explain success.

### Burnout and uncertainty

Cook describes two directions of lost value:

- **Early blockage:** a player does not bridge from an acquired tool to an interesting next use. Further learning may remain unexplored.
- **Later loss of value:** a later activity no longer offers anything worthwhile, reducing reasons to exercise earlier tools. Diagram 13 shows this backward devaluation.

He treats burnout as a promising test signal. Workbench application must be more cautious than simply marking a dark bulb: stopping an action can reflect completion, another goal, fatigue, accessibility barriers, interface confusion, or time limits. State observed behavior separately from the proposed burnout explanation. Do not infer boredom, frustration, capability, or mastery from route position or inactivity.

Keep two uncertainties visible: **player uncertainty** about what an action does or why it matters, and **review uncertainty** about the player's model. Feedback may help resolve the former; authored records alone cannot resolve the latter. Reducing every unknown is not the goal. A puzzle can depend on discovering consequences, provided the designer can examine what makes those consequences interpretable and retry meaningful.

## Loops, arcs, and source versions

The retained *Chemistry* is a 2021 repost of the 2007 essay, with author-declared minor edits. Its §5 initially defines evocative arcs as special atoms that activate prior associations but never yield useful in-game skills. The same section then says evocative arcs can be useful by cueing an existing causal association, as in Cook's mushroom/size example. Preserve this tension: evocative recognition and usable understanding can interact, even within his early account.

The 2012 *Loops and Arcs* gives a broader structural distinction:

- **Loop:** model informs action; system responds; feedback updates model; player returns or starts a connected loop. Repeated trials explore cause/effect and a space of possible outcomes. Loops can nest and depend on one another.
- **Arc:** similar interaction elements, but the information payload is consumed and the player exits instead of repeatedly exercising the same interaction. Simple actions/systems may carry rich evocative feedback. An arc can teach; it is not defined by lack of all learning.
- **Sequence of arcs:** successive fresh payloads, not necessarily a reusable system. New content can sustain this sequence without creating a new loop.
- **Expanded loop:** one playthrough written as successive interactions. Useful as a trace, but only one sample of the loop's possibility space.
- **Mixture:** arcs can precede, follow, accompany, or sit inside looping activity. A level's one-way traversal can frame repeated local experimentation.

Ask what repeats **at the chosen scale**, what changes through repetition, and what is exhausted after delivery. Do not classify entire genres by whether they have narrative. Repeating a cutscene is not enough to make its payload a mastery loop; a one-off explanation can support a later loop.

Do not silently merge the two arc definitions or claim the repost is a verified revision of the 2012 model. Explain which usage serves the question. The later essay's loop notation foregrounds model/action/system/feedback rather than listing decision separately; this is a change in presentation, not evidence that decisions ceased to matter. Its commercial history and preference for exploring loops are arguments, not instructions to remove authored content or redesign for retention.

## Inputs and workbench interpretation

Roles: `skill-atoms`, `challenges`, `levels`, `mechanics`.

Inspection topics: skill atoms, challenge routes, and relevant source records.

Read selected concept, atom bodies, immediate prerequisite descriptions, linked rules/interaction feedback, relevant challenge routes, and any actual experiment evidence. Optional levels supply grouping only. Named configuration belongs to `control-vars`; do not copy values into a parallel rule account.

These are **workbench conventions**, not a transcription of Cook's whole theory:

- `skill-atoms` owns intended learning loop, understanding, expression, and evidence sought. Preserve existing 14-role model; do not add a `skills` collection or player-state registry.
- Atom `prerequisites` declares an AND set. All declared prerequisites belong to the authored dependency claim. Cook's directed examples do not establish this local AND/null/rank contract.
- `rank` is nullable signed authored conceptual placement, not mastery, graph depth, difficulty, chronology, or physical room order. Preserve negative/zero/null values. A late root is valid; an unranked atom is not rank zero.
- Absent/null/empty relationship arrays declare no links, not missing proof to invent. Broken references are a different issue.
- Challenge `introduces`, `practises`, `requires`, and `tests_mastery` label intended opportunities/demands. `next` links can declare alternatives; they are not AND prerequisites. A challenge may introduce an understanding it also requires during play; inspect intent before calling that inconsistent.
- Experiments own hypotheses, local measures, methods, observations, results, and limitations. No separate measures or decisions collection. An atom's “evidence sought” is not a completed experiment.

Do not run graph recipes as psychological evaluation. Native C99 tools now check declared prerequisite cycles, rank conflicts, and prior introduction/practice opportunities across entry-to-visit routes, including revisits. Use [native route contract](../native/README.md#challenge-route-opportunities); unknown/incomplete coverage and witness limits remain explicit. Manual tracing still names inspected nodes/routes and does not imply exhaustive coverage.

## Apply to a selected design slice

1. **Frame the question and authority.** Select one atom/challenge and a learning question. Read mode and candidate/intended experience. State source/draft context and whether any record edits are authorized. Done: bounded question, not an automatic framework sweep.
2. **Reconstruct the interaction.** Identify expected prior model, decision/action, relevant simulation conditions, perceivable feedback, and candidate model update. Cite owners; leave unspecified feedback unknown. Done: an intelligible loop account with facts separated from proposals.
3. **Test the atom boundary.** Ask whether success could hide a different explanation and whether a failed attempt could teach the relationship. Split/merge only as an analytical suggestion when distinct learning claims warrant it. Done: chosen unit and counterexample are explicit; no automatic record restructuring.
4. **Trace immediate learning connections.** Read prerequisite bodies and the next useful application. Separate genuine conceptual dependencies, assumptions about prior knowledge, and conventional teaching order. Compare ranks without filling or recalculating them. Done: dependency question explains what becomes usable, not just which number precedes another.
5. **Inspect selected opportunities.** Follow relevant declared routes, including a bypass when present. Locate introduction, practice, combination, or changed-context use where documented. Ask what repeats and what is a consumed payload. Done: inspected paths and gaps are named; no claim of physical reachability or all-route proof.
6. **Separate interpretation from evidence.** Identify what could distinguish causal understanding from chance, copying, or rote repetition. If useful, suggest an investigation and its confounds; actual measures and observations stay with an authorized experiment. Done: unsupported mastery/burnout claims remain hypotheses.
7. **Return a review artifact.** Explain the loop, relationship or feedback concern, consequence for the selected question, countercheck, and evidence gap. Mark suggestions proposed. Done: human can assess reasoning without adopting changes. Chat is enough; saving a review or editing records needs its own authority.

## Worked example and review artifact

### Authored tutorial facts

Sources below are selected crate-puzzle records, not a playable build:

- [Concept](../examples/crate-puzzle/game-design/concept.md): `game-explorer`; candidate direction concerns committing to moves and connecting placement, switch state, and energy. Player story is explicitly hypothetical.
- [Coordinate](../examples/crate-puzzle/game-design/skill-atoms/coordinate.md): rank 2; AND prerequisites `plan-push` and `route-switch`; body requests explanation and repetition as evidence sought, not observed mastery.
- [Plan-push](../examples/crate-puzzle/game-design/skill-atoms/plan-push.md): rank null; prerequisite `push`. [Route-switch](../examples/crate-puzzle/game-design/skill-atoms/route-switch.md): rank 3; prerequisite `read-switch`. [Read-switch](../examples/crate-puzzle/game-design/skill-atoms/read-switch.md): root at rank 3.
- [Push-room](../examples/crate-puzzle/game-design/challenges/push-room.md) links directly to combined-room or through [switch-practice](../examples/crate-puzzle/game-design/challenges/switch-practice.md). Practice introduces read-switch and route-switch. [Combined-room](../examples/crate-puzzle/game-design/challenges/combined-room.md) introduces coordinate and requires plan-push, route-switch, and coordinate. No physical level map is supplied.
- [Pushing rule](../examples/crate-puzzle/game-design/mechanics/crate-pushing.md): successful push needs a free destination and sufficient energy, moves one cell, then spends push-cost. Blocked/insufficient-energy pushes move nothing and cost nothing; pulling is unavailable.
- [Switch rule](../examples/crate-puzzle/game-design/mechanics/switch-exit.md): occupied switch opens exit; removing crate closes it. [Occupied-switch interaction](../examples/crate-puzzle/game-design/interactions/occupied-switch.md) says exit state is visible; sound is unspecified. This is not evidence that a player noticed the cause.

### Hypothetical interpretation, not a recorded play sequence

Suppose a player sees a crate open an exit by occupying the switch. A hypothetical local atom account could be:

- Prior model/decision: player suspects switch occupancy matters and chooses to test that prediction. Neither suspicion nor choice is observed.
- Action: push a crate onto the switch, assuming an arrangement where the move is legal. No such layout is specified.
- Simulation: successful push moves the crate and spends push-cost; occupancy opens the exit under the authored rules.
- Feedback: exit state is authored as visible. Whether the player connects that change to occupancy, and how energy use is communicated, remain unknown.
- Candidate model update: occupancy controls exit state. Further experimentation could distinguish this from an explanation based only on having pushed once.

A further candidate update is that routing the crate must preserve the ability to reach/use the needed state with the remaining push budget. That second claim is a proposed interpretation of coordination, not the record's exact authored explanation or a certified solution.

At this scale, one push can be an action inside a larger planning loop. Repeatedly predicting a route, attempting it, and revising a plan could exercise that loop. Seeing the exit open is feedback; a celebratory flourish, if later proposed, might be a parallel evocative arc. No such flourish is an existing game fact. A single prescribed solution trace would be an expanded loop, not evidence of the whole possibility space.

### Example review output

> **SA-feedback — possible opportunity; origin: agent; impact: unresolved.**
>
> Scope: coordinate, its immediate prerequisite records, push-room/switch-practice/combined-room links, and linked pushing/switch feedback. Manual source reading only; no all-route or physical-layout validation.
>
> Trigger: atom bodies say to inspect state and adjust a plan, but do not specify what feedback distinguishes a successful local push from a plan that preserves a useful later state. Visible exit state supports one relationship, not necessarily the combined planning claim.
>
> Question: what should the player predict before committing, and what result would let them revise the prediction? Hypothesis: the optional practice route could support switch understanding before combination; the shortcut may instead rely on prior knowledge or discovery in combined-room. Neither route proves learning or incapability.
>
> Countercheck: a player might copy a successful sequence without understanding why it works. A changed hypothetical arrangement, with a prediction explained before acting, could help examine transfer. Explanation alone also depends on language and prompting; nonverbal choices may supply different evidence. No arrangement, test, threshold, or observation is supplied here.
>
> Separate authored-order question: coordinate rank 2 depends on route-switch rank 3; plan-push remains unranked and read-switch remains a late root. Ask what rank means to the author. Do not infer mastery or reorder the chain.
>
> Evidence status: no player evidence established. Next investigation is proposed only; no source repair or adoption authorized.

The existing [energy/planning experiment](../examples/crate-puzzle/game-design/experiments/energy-encourages-planning.md) remains proposed and unexecuted, with measures/method open. The [route-cost experiment](../examples/crate-puzzle/game-design/experiments/route-cost.md) also remains unexecuted; its analytical timing reference assumes correct decisions and cannot establish understanding or enjoyment.

If a designer separately requests coordinate rank 3, edit that field only. Preserve identity, body, prerequisites, plan-push null, and existing switch-chain ranks. Correcting placement still does not answer the feedback question. This guide itself authorizes no record edits.

## Practical questions and counterchecks

- **Causal discrimination:** which two plausible explanations would produce the same visible success? What changed condition could distinguish them without merely increasing motor difficulty?
- **Feedback access:** can the player connect this change to this action, or could delay, competing cues, camera position, or hidden state obscure it? Missing feedback description is a documentation question first, not proof of missing implementation.
- **Next use:** what nearby opportunity makes the newly learned relationship seem useful? Could a player understand it yet choose another goal?
- **Prior knowledge:** which part is new here, and which part assumes input/display/genre familiarity? Is apparent rapid learning actually prior knowledge?
- **Repetition:** does another attempt explore a changed prediction, apply a familiar tool, or merely repeat a consumed payload? Repetition alone establishes none of mastery, boredom, or grind.
- **Transfer:** what changed situation would still use the same relationship? Could assistance, memorized layout, or an unrelated strategy explain the apparent transfer?
- **Lost interest:** what behavior was actually observed, under what context? What supports burnout over completion, interruption, or access difficulty? Do not turn Cook's status icons into telemetry-derived mental states.
- **Scope check:** does the selected chain illuminate the candidate experience, or flatten a social/narrative/expressive aim into a skill acquisition requirement?

## Common mistakes

- Mapping inputs to nodes but omitting modeling. Name the usable understanding and how feedback could revise it.
- Treating simulation state as sufficient feedback. Identify what can actually be perceived; leave attention/interpretation unverified.
- Reading Cook's chain as the local AND prerequisite contract. Explain conceptual tool use before interpreting authored dependencies.
- Ranking to mastery or prerequisite depth. Preserve nullable authored placement; question conflicts without repairing them.
- Calling every bypass a teaching failure. Prior knowledge, assistance, or intentional discovery may matter; inspect rather than infer.
- Treating an introduced skill as mastered, or an unused skill as burned out. Intent and opportunity are not evidence.
- Calling all narrative arcs and all repeated actions loops at every scale. Identify repeatable system use and consumed payload separately.
- Removing arcs because loops are valuable. Cook describes mixtures; framework review does not authorize design adoption.

## Source and transfer limits

Both retained essays were read in full, including *Chemistry*'s conclusion and References And Notes and *Loops and Arcs*' market discussion and conclusion. They are designer models/arguments, not a validated instrument for scoring fun or inferring individual cognition. Their broad biological, evolutionary, market, and causal claims are not independently established by this guide. Cook's strong claims about one-time mastery pleasure and measurable burnout remain attributed claims, not universal acceptance criteria.

Essential local figures inspected: *Chemistry* Diagrams 4–10, 13–14, and the evocative-atom image in §5; all four *Loops and Arcs* figures. Diagram 4's compressed/uneven labels require the prose distinction above. The small Tetris overview supports network form, not a reliable transcription of every node. Thin connectors/labels in transparent loop/arc images have limited contrast; accompanying full prose supplies their meaning. No copied diagram or proprietary assessment instrument is reproduced here.

Real gaps remain: *Chemistry* Diagram 11 (Active Icon) returned 404 during capture; active status is explained in prose and shown in Diagram 10. Original 2007 edition was not independently compared. Workshop/Project Horseshoe slides, Tetris PDF, linked research, and other bibliography works were not read for this guide. Do not imply their coverage or framework completeness. All four *Loops and Arcs* images were recovered in the source pass; no missing-figure claim remains for that essay.

Local records support declared relationships and candidate intent only. Missing layouts prevent reachability/solution claims; unexecuted experiments prevent evidence claims. No aggregate fun/psychological score, new schema, mandatory category completion, automatic sweep, or inferred player capability follows from application. Guide remains a review draft, not catalog readiness.

## References

- Daniel Cook, **The Chemistry of Game Design** (2007; author repost 2021): [actual local article](../references/skill-atoms/captures/chemistry-of-game-design/article.md). Reviewed entire capture. Useful locators: §2 Player Model, Skill, Driven To Learn, Perceived Value; §3 Interaction Loops, This Thing We Call Play, Chaining Of Game Mechanics; §4 Status Of Atoms, Active Skills, Burnout; §5 Pre-existing Skills and Evocative Stimuli using Arcs; §6 Using Skill Chains; final References And Notes, Relationship of Skill Chains to MDA. Figure locators and limits listed above.
- Daniel Cook / Danc, **Loops and Arcs** (2012): [actual local article](../references/skill-atoms/captures/loops-and-arcs/article.md). Reviewed entire capture. Useful locators: Loops; Arcs (sequence of arcs and expanded loop); Mixing Loops and Arcs; The growth of arcs in games; Untangling loops and arcs in existing game forms; A brilliant future for loops; Conclusion. Four figures accompany the first three sections.
- [Capture index, edition/fidelity notes, exclusions, provenance, and rights](../references/skill-atoms/README.md). Captures are local offline reference only, not publication-cleared; copyright remains with rightsholders. Independent source notes are not substitutes for these full captures. Application procedure, review questions, and hypothetical interpretation are independently authored workbench adaptations.
