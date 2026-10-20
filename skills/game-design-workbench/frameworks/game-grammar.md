# Game grammar and meaningful play

Pack: `game-grammar` · version 1
Attribution: Related meaningful-play concepts: Katie Salen Tekinbaş and Eric Zimmerman, Rules of Play. Noun–verb case matrix: workbench modeling technique, not an attributed book prescription.

## Contents

- [Scope](#scope)
- [Inputs and query support](#inputs-and-query-support)
- [Working concepts](#working-concepts)
- [Apply to selected slice](#apply-to-selected-slice)
- [Original prompts](#original-prompts)
- [Worked example](#worked-example)
- [Evidence and counterchecks](#evidence-and-counterchecks)
- [Common traps](#common-traps)
- [Missing data and evidence limits](#missing-data-and-evidence-limits)
- [References](#references)

## Scope

Inspect a selected noun/action pair, its conditional cases, consequences, feedback, and relevance to later play.

## Inputs and query support

Roles: `nouns`, `verbs`, `interactions`, `mechanics`, `challenges`.

Inspection topics: interaction cases and relevant source records.

## Working concepts

- **Identity and action:** Nouns identify things; verbs identify actions. Keep target identity separate from contextual objects. A wall can constrain pushing a crate without becoming the target of that push.
- **Conditional case:** An interaction states when a particular action/target combination has a particular outcome. Conditions, consequences, and feedback belong together so a reader can explain why this case differs from another.
- **Discernible consequence:** A player needs some way to perceive what an action did or why it did nothing. Feedback can explain a defined no-effect outcome; absence of movement alone does not establish adequate feedback.
- **Integrated consequence:** An outcome matters through its relationship to later choices and state. A local animation may be perceptible while its strategic consequence remains unclear. These are different review questions.

## Apply to selected slice

1. Select one noun/action pair or a small supplied set. Resolve canonical terms and aliases first. Record exactly which target and action are under review; do not expand the full Cartesian product.
2. Read every declared case for that pair. Extract its conditions, changed state, unchanged state, and feedback. Keep explicit unknown, excluded, not-applicable, and defined-no-effect cases distinct.
3. Compare conditions for overlap and gaps. Ask what supplied rule chooses between overlapping descriptions. Mark the issue unresolved if the source does not establish precedence; do not invent an execution order.
4. Trace each consequence into one later decision or challenge. Separate what the rule changes from what presentation communicates. Link shared behavior to its owning mechanic instead of duplicating it in every case.
5. Return a compact case comparison and source-specific questions. If a new case seems useful, describe it as a proposal. Review does not authorize creating the case, changing feedback, or declaring remaining pairs invalid.

## Original prompts

1. For this action on this target, which conditions choose the outcome? Can more than one case apply?
2. Can the player perceive what changed, including a defined no-effect outcome?
3. What later choice or state makes that consequence matter?
4. Does a contextual noun participate in the condition without being the action target?
5. When two descriptions disagree, which source owns the rule and which should only link to it?
6. Is an absent case unknown, deliberately excluded, or genuinely not applicable? What supplied fact establishes that distinction?

## Worked example

- **Authored facts:** Crate/push has free, wall-blocked, and low-energy cases. Wall/activate is explicitly not applicable. Closed-exit/walk is unknown rather than a declared prohibition.
- **Interpretation:** Wall-blocked pushing has a defined no-movement outcome and feedback. It is not an empty design cell. Review can still ask whether that feedback explains the relevant condition.
- **Counterreading:** Two blocked cases could be relevant when both wall and energy conditions hold. Their existence alone does not establish which feedback wins; inspect case prose before claiming ambiguity.
- **Useful next question:** Does supplied source describe a joint-condition outcome or precedence? If not, retain that uncertainty without converting it into an automatic rule repair.

## Evidence and counterchecks

- Ask a player to explain an observed outcome only when a playtest is authorized. A correct guess after repeated attempts differs from clear feedback on the first relevant action.
- Compare perceived local feedback with understanding of later consequences. Recognizing that a crate moved does not prove understanding of an irreversible placement.
- Treat case coverage as authored coverage, not state reachability. A declared combination may never occur in a particular level; proving that requires a different method.

## Common traps

- One matrix cell can contain several meaningful cases. Collapsing them into one sentence hides conditions and may contradict explicit no-effect behavior.
- Do not infer missing rules from familiar genre behavior. Unknown closed-exit walking stays unknown until supplied or adopted.
- Do not copy shared spending rules into noun identity or verb descriptions. Keep one authoritative owner and link from cases.

## Missing data and evidence limits

Keep blank cells unknown. Preserve explicit excluded/not_applicable cases and defined no-effect cases. Do not infer exclusions from genre conventions.

Matrix rows expand only explicitly declared applicable nouns. Missing Cartesian pairs are not synthesized or condemned. Shared mechanics own common rules; cases own their conditions/consequences/feedback. No automatic reachability or exhaustive coverage proof.

## References

- [Rules of Play — MIT Press](https://mitpress.mit.edu/9780262240451/rules-of-play/)
- [Meaningful Play teaching excerpt](https://sites.cc.gatech.edu/classes/AY2006/cs4455_spring/documents/sz-unit11.pdf)

Access checked 2026-10-19: MIT Press request returned 403. Search-linked teaching excerpt supports discernible/integrated outcomes; PDF text extraction was incomplete. No full-book review claimed.
