# Contextual reflective design questions

Pack: `schell` · version 1
Attribution: Jesse Schell, The Art of Game Design: A Book of Lenses; Game Design: A Deck of Lenses, Tenth Anniversary Edition

Provisional application guide; pending human review. Uses retained English deck text, not a full-book reading. Card concepts are attributed below; selection, procedure, and review artifacts are workbench adaptations.

## Contents

- [Purpose and unit of analysis](#purpose-and-unit-of-analysis)
- [What the retained deck provides](#what-the-retained-deck-provides)
- [Choose cards by the unresolved question](#choose-cards-by-the-unresolved-question)
- [Inputs and interpretation](#inputs-and-interpretation)
- [Apply to a selected design slice](#apply-to-a-selected-design-slice)
- [Worked example: interpreting a failed push](#worked-example-interpreting-a-failed-push)
- [Review artifact and later evidence](#review-artifact-and-later-evidence)
- [Counterchecks and transfer limits](#counterchecks-and-transfer-limits)
- [References](#references)

## Purpose and unit of analysis

Use Schell's lenses to question a specific design situation from a deliberately chosen perspective. A lens is a description and a set of questions, not a rule recognizer, causal model, checklist score, or guarantee of improvement. The useful result is a clearer assumption, consequence, alternative, or evidence gap tied to a design decision.

**Unit of analysis:** one stated question about a player moment, rule arrangement, sequence, or production decision, viewed through selected cards. Name the scale: feedback after one action differs from engagement across a session. The deck includes production and business topics as well as play; do not turn every card into a mechanic review.

Use this guide when a discussion has become stuck on one interpretation, an attractive change has unclear costs, or a desired experience lacks an explanation of how current rules support it. Do not use it to certify enjoyment, diagnose players, prove balance, or inspect every possible design concern. If the unresolved issue is only a broken record reference, inspect that directly rather than dressing it as a lens finding.

## What the retained deck provides

The [local table](../references/schell/deck-table.md) contains 116 English cards and 451 questions. The [supplied JSON](../references/schell/deck-en.json) is the exact-text source, including description layout and ordered question arrays. Table use notes are independent commentary; they are not Schell's descriptions. Use a note to find a candidate, then read that card's actual description and questions before applying it.

Identify a card by **internal index plus exact title**, as used in the table. Indices belong to this retained edition, not every book or physical deck. Fractional indices remain fractional; internal index 113 corresponds to the infinity-marked card in raw `name`. Do not renumber cards or use a JSON array offset as a card ID.

Source structure matters:

- **Description and questions:** state the perspective and what it asks the designer to consider. A title alone is not enough to reconstruct the lens or answer it.
- **Suits:** Designer, Player, Experience, Process, and Game group cards in the supplied data. Membership can overlap; these are navigation aids, not required workbench categories or five mandatory review passes.
- **Cross-cutting perspectives:** several cards can question the same moment without asking the same thing. Emotion concerns intended and current feelings; Feedback concerns communication in the present situation; Punishment concerns penalties, purpose, fairness, and alternatives. Answers can create tension rather than converge on a single best rule.
- **Iteration:** supplied `strings` instructions describe trying a game, examining problems, changing it, and trying again. They present lenses as aids to new questions and perspectives alongside playtesting. The app's random selection, favorites, and Divination are navigation/ideation features, not evidence that a drawn card contains the correct answer. This guide selects by relevance rather than requiring random draws or a deck sweep.

Card 9, **The Elemental Tetrad**, explicitly names Aesthetics, Technology, Mechanics, and Story, asking about their use and reinforcement around a theme. These are not the workbench's 14 record roles. Its use of “Aesthetics” must not be silently equated with an individual MDA aesthetic or the whole concept/player story. The short card does not supply a full theory of those relationships; do not invent missing book definitions.

## Choose cards by the unresolved question

The following are **workbench selection routes**, not official deck categories, a complete catalog, or a sequence to execute. Read only entries relevant to the current question. A clear user selection needs no reconfirmation. If a broad request could target materially different decisions, resolve that scope before reading unrelated cards.

- **What are we trying to resolve?** Index **14 — The Problem Statement** questions the underlying problem, assumptions, suitability of a game, and how resolution would be recognized. **2 — Essential Experience** shifts attention from a feature to the experience it should capture. Use these when purpose is disputed; do not demand a fixed destination in game-explorer mode.
- **Whose experience are we assuming?** **19 — The Player** examines preferences and expectations; **1 — Emotion** separates desired feelings from feelings during current play. Neither turns a hypothetical player story or designer preference into audience research. **110 — Transformation** asks about beneficial and harmful change, not only immediate enjoyment.
- **Do actions create worthwhile agency?** **39 — Meaningful Choices** concerns meaningful options, choice quantity, and dominant strategies. **59 — Control** concerns expectations, interface use, perceived influence, and power. **79 — Freedom** distinguishes available freedom from felt freedom and possible overload. More actions need not improve any of these.
- **What can the player understand now?** **63 — Feedback** examines needed/wanted information, feelings, and the current goal. **55 — Visible Progress** asks what counts as progress and what can be seen. Correctly exposing a value does not establish that its meaning or contribution to progress is understood.
- **What does a setback cost and contribute?** **47 — Punishment** questions penalties, their purpose and perceived fairness, alternatives, and relation to rewards. Consider time, lost state, and replay effort where actually supported, not only numeric deductions. Its claims about meaning and pride are propositions to examine, not established player outcomes.
- **How does involvement change over time?** **69 — Interest Curve** asks about a hypothesized engagement shape and comparison with playtesters; **21 — Flow** asks about focus, goals, distraction, challenge, and skill change. A drawn curve or designer assertion is not observation. Neither selection silently invokes psychological Flow or GameFlow as another framework review.
- **Do parts support the same purpose?** **9 — The Elemental Tetrad** offers a cross-element perspective; **92 — Inner Contradiction** questions subsystem purposes and conflicts with the whole. **53 — Balance** returns to a broad felt judgment. A vague answer to Balance is a starting question, not a numerical balance verdict or proof that every component is justified.
- **What would distinguish competing explanations?** **103 — Playtesting** asks about purpose, people, setting, observations, and evidence collection. Use it to turn a specific uncertainty into a possible investigation; applying the card does not execute or authorize a playtest.

Start with the smallest useful selection, often one card. Add another only when it exposes a different consequence or challenges an interpretation that matters. Explain each addition; agreement among several overlapping cards is not independent corroboration. If no retained card fits, state the gap rather than inventing an attributed lens.

## Inputs and interpretation

Roles: `mechanics`, `aesthetics`, `challenges`, `experiments`; related records only as needed.

Inspection topics: selected card questions, intended experience and player context, relevant rules and relationships, retained rationale, and investigation evidence.

Read the live concept/mode and source owners for the selected slice. Keep overall experience and hypothetical player story distinct from individual MDA aesthetics. Read relevant accepted rules, proposed/rejected alternatives, actual observations, and unresolved conditions. Follow named `control-vars` when a cost or timing value matters; do not repeat a guessed value as a rule.

For an explicit emotional intention, use [shared emotional-experience framing](../workflows/emotional-experience.md) and only useful entries from the [working vocabulary](../references/emotional-experience/emotions.md). Ask what kind of feeling is meant, whose it is, what it concerns, and what gives it meaning—not merely whether the outcome is positive. Satisfaction, relief, and fiero can follow the same success with different interpretations; a helper's naches and a learner's triumph are different people's experiences. Preserve plain language, supplied meanings, overlaps, and tight transitions without forcing a named emotion or another framework review.

A card can reveal that essential input is missing. Mark an answer **unknown** when there is no observed feeling, actual layout, intended audience evidence, or specified interface. An accepted dynamic can still be a prediction; an accepted experiment can still be proposed and unexecuted. No new schemas, required lens tags, scores, or framework-specific collections. Existing experiments own hypotheses, measures, methods, observations, and interpretations.

## Apply to a selected design slice

1. **Bound the question.** Name the moment, decision, relevant intent or exploration lead, and what is outside scope. Done: the review can change or clarify a concrete discussion, not merely produce more commentary.
2. **Select and read cards.** Record retained edition, indices, exact titles, and why each perspective matters. Read descriptions and questions, not only the table's use notes. Done: each selected card has an explicit connection to the bounded question; no unrelated lens sweep.
3. **Establish source facts.** Trace rules, lifecycle, current constraints, and retained rejection rationale. Separate authored intention, observed evidence, predictions, and missing inputs. Done: an answer cannot silently become a new rule or invented player reaction.
4. **Answer the relevant card questions.** Tie each answer to a source, observation, or explicitly labeled interpretation. State why any question is outside scope or unanswered; do not fill it with a plausible story. Done: reader can distinguish the card's question from the agent's answer and challenge the answer's basis.
5. **Compare and countercheck.** Identify the assumption doing the work, a conditional consequence, and the strongest contrary interpretation. If a contrast helps, change one meaningful feature and trace costs elsewhere. Done: alternatives remain unadopted and do not evade prior rejection rationale by being renamed.
6. **Return the bounded artifact.** Report source state, selected cards, findings, unknowns, counterchecks, and a useful next question. Done: no lens score, unrequested record edits, saved report, or executed experiment. Save only when requested; follow the existing review workflow if saving is authorized.

Stop when the selected question is clarified or missing facts prevent a defensible answer. State the missing input rather than expanding into other frameworks or treating more cards as a cure for absent evidence.

## Worked example: interpreting a failed push

**Question:** Could a player understand why a push failed and revise a plan without the interface prescribing the solution?

**Selected cards:** **63 — Feedback**, **47 — Punishment**, **1 — Emotion**. Feedback examines what this moment communicates; Punishment checks whether the failure itself carries a penalty; Emotion checks the difference between desired commitment/satisfaction and unobserved reactions. These are three perspectives within the requested Schell review, not three new framework reviews.

**Authored facts from the tutorial, not play observations:**

- [Concept](../examples/crate-puzzle/game-design/concept.md), “Intended experience,” “Minute to minute,” and “Constraints and risks”: `game-explorer`; candidate direction is thoughtful commitment, understandable changes, and successful replanning. Blocked-action feedback and restart discovery remain unresolved. Player story is hypothetical; no physical room geometry or playable build is supplied.
- [Crate pushing](../examples/crate-puzzle/game-design/mechanics/crate-pushing.md): a successful push needs a free destination and sufficient energy, moves one cell, and spends `push-cost`. Wall-blocked and insufficient-energy pushes move nothing and cost nothing. Pulling is unavailable.
- [Restart](../examples/crate-puzzle/game-design/mechanics/challenge-restart.md): accepted rule reconstructs the challenge's starting arrangement. Sparse record links do not prove the rule is unavailable. Exact restart presentation and any unspecified resource-reset behavior must not be filled in.
- [Rejected pulling proposal](../examples/crate-puzzle/game-design/verbs/pull.md), “Disposition and rationale”: rejection preserves irreversible-push planning. Reopening that choice would need explicit direction, not an attractive lens answer.
- [Careful positioning](../examples/crate-puzzle/game-design/dynamics/careful-positioning.md): finite pushes encouraging consideration is a prediction, not an observation. [Planning investigation](../examples/crate-puzzle/game-design/experiments/energy-encourages-planning.md) remains proposed, with no defined measures or executed results.

**Card-grounded answers and unknowns:**

- **63 — Feedback:** the current goal may require distinguishing a blocked destination from insufficient energy; otherwise a player could revise the wrong part of a plan. This is an agent interpretation of two authored failure conditions. What players actually notice or want to know is unknown. Documentation does not specify the signal, so do not report a proven missing signal in a playable build.
- **47 — Punishment:** a refused push has no authored energy or movement penalty. Do not claim it consumes energy or forces restart. A successful but strategically damaging push is a different event: it commits state and spends energy. Restart permits another attempt, but actual replay effort depends on absent geometry and presentation. Fairness, frustration, and pride are unobserved; resource cost alone cannot answer those questions.
- **1 — Emotion:** the concept supplies desired thoughtful commitment and satisfaction; current player emotions are unknown. Predicting confusion or frustration does not establish either. A candidate change should be evaluated against the desired experience, not the assumption that every uncomfortable moment is bad.

**Hypothetical contrast, not an adopted mechanic:** compare a generic refusal signal with a signal distinguishing blocked space from insufficient energy, while leaving push conditions and costs unchanged. Neither presentation is claimed to exist now. Naming the failed condition need not identify the correct future push; predicting an entire solution is a separate, more intrusive proposal.

**Countercheck:** more explanation may improve causal legibility yet distract, overexpose a puzzle relation, or fail to address a mistaken goal. Conversely, lack of a documented signal may be a documentation gap rather than a design problem. Preserve both explanations. Adding pulling would not be a feedback-only change and would directly reopen the rejected commitment decision.

## Review artifact and later evidence

A useful result can remain in chat. If saving is requested, use the existing review contract rather than creating lens-specific records. Example artifact, independently authored from the selected cards and tutorial facts:

> **FAILED-PUSH-LEGIBILITY — unresolved documentation question / experience hypothesis.** Scope: interpreting one wall-blocked or insufficient-energy push. Cards: Tenth Anniversary Edition, 63 Feedback, 47 Punishment, 1 Emotion.
>
> **Fact:** both failed conditions leave position and energy unchanged; current records do not specify how their causes are communicated.
>
> **Interpretation:** a cause-specific signal could support revising the relevant part of a plan without changing the rule. This is not demonstrated behavior or an adopted interface.
>
> **Unknowns:** current presentation, actual room, player interpretation, emotional response, and effect on discovery. No observed harm or defect severity established.
>
> **Contrary case:** existing presentation may already distinguish causes, or additional explanation may reveal too much. Do not infer a penalty from failure or confuse replay after a successful bad push with cost of a refused push.
>
> **Next question:** with a specified room and two defined signal treatments, can players explain the failed condition and choose a relevant revision, and what understanding do they report? No result predicted as fact; no test executed.

If a later investigation is authorized, card **103 — Playtesting** helps refine this last question into participant/context choices, observations, and collection method. Hold rules, task, and relevant presentation comparable so a difference is not silently attributed to an unrelated change. Define how evidence distinguishes explanations before treating it as support. Correct retries alone do not prove understanding; faster completion alone does not prove satisfaction. Participants, measures, comparison order, and criteria still need definition. Do not overwrite the existing energy/planning investigation with this separate question without authorization.

## Counterchecks and transfer limits

- **Source questions versus original guidance:** card titles and question text belong to the retained source. Selection routes, done conditions, example interpretations, and artifact labels above are workbench adaptations, not an official Schell method or new card set.
- **Prompt versus finding:** asking about fairness, meaningful choices, or emotions does not establish a defect or a player state. Explain the local mechanism and evidence gap; do not score how many lenses a design passes.
- **Same name, different framework:** Flow and Balance cards are reflective prompts, not validated psychological instruments or quantitative solvers. Do not silently load other framework guides, import their models, or treat shared vocabulary as equivalent evidence.
- **Theme versus coherence proof:** the Tetrad and Inner Contradiction cards can expose a suspected tension. A missing story record or an unusual subsystem does not by itself prove incoherence or require new content.
- **Capability and experience limits:** no card answer establishes reachability, learning, mastery, audience fit, or enjoyment from source links alone. Actual geometry, implementation, and observation remain separate inputs.
- **Broad source claims:** descriptions sometimes make general claims about human attention, punishment, or control. Treat these as design propositions needing contextual counterchecks, not universal empirical laws. No tested effect size or validated scoring instrument is supplied by this deck.
- **User authority:** alternative generation does not adopt a design, change goals, reopen rejected rules, or authorize implementation. Preserve unknowns and deliberate exclusions. In explorer mode, a promising lead is not automatically a new project destination.

Coverage: application guidance draws on the retained instructions and 17 selected cards: 1, 2, 9, 14, 19, 21, 39, 47, 53, 55, 59, 63, 69, 79, 92, 103, and 110. The full 116-card table supports further selection, but every card has not received an individual worked application here. Book content, other editions/languages, artwork, and cross-edition numbering were not reviewed. This expansion is not full-book coverage or a completed human review.

## References

- Jesse Schell, [*Game Design: A Deck of Lenses*, displayed Tenth Anniversary Edition — raw supplied English JSON](../references/schell/deck-en.json). Exact locators: `LensList` entries selected by `index` and `title`, with `description`, ordered `questionlist`, and `suitlist`; instructional `strings` keys `p1s1`–`p1s3`, `p2s1`, `p2s2p1`–`p2s2p3`, `p2s3p1`–`p2s3p3`, and `p3s1`. Read selected entries, not a presumed array offset. Source basis is user-supplied text, not a new web crawl or book capture.
- [Complete English card reference table](../references/schell/deck-table.md): all 116 indices/titles, independent functional notes, descriptions, and 451 questions. Use raw JSON where exact whitespace or question-array boundaries matter. The source table is separate from this original application guide.
- [Source identity, earlier browser checks, supplied-data verification, and rights](../references/schell/README.md). Earlier inspection dated 2026-10-21; supplied JSON preserved locally and mapped to the earlier inventory. Local Git retention approved; source content remains local-only/not-publication-cleared. No new acquisition or redistribution clearance implied.
