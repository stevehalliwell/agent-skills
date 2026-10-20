# Playcentric design and evidence

Pack: `playcentric` · version 1
Attribution: Tracy Fullerton, Game Design Workshop: A Playcentric Approach to Creating Innovative Games

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
- [Sources](#sources)

## Scope

Connect intended player experience with a testable question, prototype, observations, and resulting design choices.

## Inputs and query support

Roles: `aesthetics`, `experiments`.

Available recipes: `query evidence`; `query records --collection <mapped-alias>`.

## Working concepts

- **Experience goal:** Describe an intended player experience precisely enough to influence a design choice. Keep it separate from a feature list and from a claim that the experience has already occurred.
- **Testable uncertainty:** Choose a claim whose possible answers could change the next decision. State an outcome that would challenge the preferred interpretation, not only observations that would confirm it.
- **Question-shaped prototype:** A prototype should expose the uncertainty with an appropriate amount of implementation. A timing model, paper interaction, and playable scene answer different questions and omit different parts of experience.
- **Evidence and iteration:** Separate assumptions, analytical derivations, executed model output, and observed play. A resulting design choice is another step that needs explicit authority; evidence does not silently adopt an option.

## Apply to selected slice

1. Connect one supplied experience goal to a decision that remains open. Read relevant experiments, their hypothesis/expectation/observation sections and local measure definitions, plus any existing project decisions. Incomplete investigation is valid input for discussion.
2. Write the uncertain claim and a plausible rival explanation. Specify what observation or result could distinguish them. Keep player experience claims separate from numerical model predictions.
3. Propose the smallest suitable prototype or method. State what it includes, what it omits, what would be recorded, and what assumptions limit the answer. Proposal does not authorize execution or participant recruitment.
4. Inspect existing evidence provenance before interpreting it. Identify source, method, conditions, and whether the result is analytical, simulated, observed, or merely expected. Empty observations remain empty.
5. Return the decision-linked question, proposed method, and interpretation boundary. If evidence later exists, explain what it changes and what remains uncertain; follow user choice on retaining alternatives/rationale instead of automatically adopting a repair.

## Original prompts

1. What experience goal is concrete enough to guide a prototype choice?
2. Which uncertain claim matters most to the next decision?
3. What smallest prototype could expose that uncertainty without pretending to be the whole game?
4. What would be observed, measured, or asked, and what result could contradict the claim?
5. Which evidence comes from people, which from a model, and which remains a designer assumption?
6. How did the evidence change a decision, and which alternative/rationale should remain available for later reconsideration?

## Worked example

- **Authored facts:** The illustrative route has 4 walks and 3 pushes, with base time 5 s. A proposed model assumes independent uniform delays of 0–1 s per action. No trials or participant observations exist.
- **Analytical reference:** Under those assumptions, total time lies between 5 and 12 s and probability of at most 8 s is 1312/5040, about 0.2603. This is a derivation, not measured success or evidence about real player delay.
- **Interpretation boundary:** An executed model could later check implementation against the analytical reference. It still would not validate the delay assumptions or show that a deadline creates the intended experience.
- **Useful next question:** Is the next decision about arithmetic correctness, plausible player timing, or the experience of pressure? Each needs a different method; do not use one precise result to answer all three.

## Evidence and counterchecks

- Record contradictory outcomes and method limitations, not just a preferred result. An unexpected observation may challenge the model, the hypothesis, or the procedure.
- Separate measurement from expectation comparison and design judgment. Meeting a numeric threshold is not proof that the intended experience occurred.
- Later simulation execution requires its own explicit one-shot approval and checkpoint protocol. This slice neither runs simulations nor turns an experiment proposal into completed evidence.

## Common traps

- Do not treat a proposed method, analytical probability, or expectation as an executed result.
- Do not infer participant counts, motivation, emotion, or capability from a timing distribution.
- Do not let evidence storage imply adoption. Record uncertainty with the investigation; use existing project decisions only when wanted, not a mandatory archive.

## Missing data and evidence limits

Empty observations honestly mean no observations recorded. Discuss test design without fabricating participants, sample counts, measured outcomes, or completed iterations.

A proposed experiment is not execution evidence. Simulated predictions are not observed play. No automatic repairs, playtest execution, or simulation. The evidence query is an inventory; inspect linked record bodies for full method and evidence trail.

## Sources

- [Game Design Workshop — official site](https://www.gamedesignworkshop.com/)

Access checked 2026-10-19: Official Game Design Workshop overview inspected: experience goals, prototyping, playtesting, revision. Full book/exercises not reproduced.
