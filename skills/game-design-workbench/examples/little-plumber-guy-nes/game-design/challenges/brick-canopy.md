---
id: 01a16f86-6ff6-79b9-b246-34d27e875eda
name: brick-canopy
created_at: 2026-10-24T20:07:23.894Z
desc: Mixed overhead blocks contrast small and tall responses.
tags: []
status: accepted
entry: false
next:
  - challenges/pipe-steps
practises:
  - skill-atoms/bump-block
  - skill-atoms/read-power
introduces:
  - skill-atoms/shorten-jump
mechanics:
  - mechanics/brick-response
  - mechanics/jump-cut
nouns:
  - nouns/brick
  - nouns/ceiling
---
## Goal and context

Cross mixed ordinary bricks and coin supplies over broad support. A concealed life supply sits near the canopy's outer edge. Small form sees bumps; powered form can open overhead space.

## Assistance and prior knowledge

Compare block responses; try shorter jumps without a lethal floor.

## Decision and assessment

Keep discovering block behavior separate from performing a chosen action. Small form preserves an ordinary brick on a valid rising underside hit; tall/spark form destroys it. Destruction removes support and is not pickup collection. These are declared state-dependent outcomes, not evidence player noticed or understood them.

Comparing responses offers a discovery opportunity. What player has already seen, notices or infers remains unknown; no required investigation or knowledge-test demand is established. Predicting bump versus break from a supplied form/contact state could isolate a knowledge question, but no such prompt or response criterion is implemented in this encounter. That quiz remains a proposal, not an additional gameplay task.

A chosen underside hit or shorter-jump attempt involves execution. Reaching that chosen contact can satisfy a local action goal; whether brick then bumps or breaks depends on form and conditions, not simply input correctness. Missing a target does not identify a knowledge error. Broad support and optional concealed reward do not establish a lethal traversal test or compulsory collection goal.

Intent check: should contrasting feedback invite discovery without requiring an explicit quiz? Should shorter-jump practice remain optional? Intended demand remains undecided. Visible feedback, exact geometry, timing tolerances and player learning need separate evidence; no reaction/dexterity label follows from jump input alone.

```decision-assessment
id: block-response-discovery
task: Explore ordinary brick responses across power forms.
context: Optional discovery opportunity; valid small underside hit preserves brick while tall or spark destroys it. Player prior knowledge and noticing are unknown; no implemented prediction prompt.
evidence_status: authored-assessment
rule_cases:
  - "mechanics/brick-response#small-bump"
  - "mechanics/brick-response#powered-break"
demands: []
```

```decision-assessment
id: chosen-canopy-action
task: Perform a chosen underside hit or shortened-jump attempt.
context: Local action goal over broad support; resulting brick response is state-dependent. Exact geometry and timing tolerances are unspecified, and missed contact does not diagnose knowledge.
evidence_status: authored-assessment
rule_cases:
  - "mechanics/brick-response#small-bump"
  - "mechanics/brick-response#powered-break"
  - "mechanics/jump-cut#ascending-release"
demands:
  - kind: execution
    intent: undecided
```

## Route

Hidden reward is optional. Continue to pipe-steps; exact instance coordinates are intentionally not a tile map.
