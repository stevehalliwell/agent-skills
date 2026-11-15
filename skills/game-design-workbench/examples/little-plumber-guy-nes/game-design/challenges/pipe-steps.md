---
id: 01a16f86-6ff6-797c-a4fa-ac77768563f3
name: pipe-steps
created_at: 2026-10-24T20:07:23.894Z
desc: Raised pipes practise landing and conceal an optional entrance.
tags: []
status: accepted
entry: false
next:
  - challenges/first-gap
  - challenges/bonus-entry
practises:
  - skill-atoms/jump
  - skill-atoms/land-on-target
introduces:
  - skill-atoms/use-pipe
mechanics:
  - mechanics/solid-collision
  - mechanics/pipe-transfer
nouns:
  - nouns/pipe
---
## Goal and context

Cross successive solid pipes of differing height. One later pipe declares a downward cellar entrance; earlier pipes are only obstacles.

## Assistance and prior knowledge

Wide tops offer landing practice. Entrance discovery and visual hint remain design questions.

## Decision and assessment

Keep three situations separate. Landing on a chosen pipe top is an execution task: arriving on that support satisfies the local landing goal. Missing that target does not by itself establish death, failed course traversal or a knowledge error; no physical layout or measured landing tolerance is supplied.

Entrance discovery is an opportunity, not a compulsory answer. The entrance exists in design, but its hint and what player knows remain unspecified. Missing documentation is not evidence that destination is hidden from player. No knowledge quiz or required search demand is established.

Choosing cellar entry is separate from discovering it and performing aligned Down input. Continuing overworld is valid. No destination preview, route-cost comparison or player preference establishes a best route or meaningful trade-off yet. Linked entrance case leaves alignment and instance predicates unencoded; route arrows do not resolve them.

Intent check: should pipe landing test execution, and what should communicate optional entrance? Should destination be known before commitment? These are unresolved design questions, not adopted changes. No reaction window, dexterity classification or player-error explanation follows from failed entry.

```decision-assessment
id: pipe-landing
task: Land on a chosen pipe top.
context: Local landing goal during traversal; missing chosen support does not alone establish death or course failure, and landing tolerances are unspecified.
evidence_status: authored-assessment
rule_cases: []
demands:
  - kind: execution
    intent: undecided
```

```decision-assessment
id: entrance-discovery
task: Discover optional cellar entrance.
context: Design declares one entrance; hint, player knowledge and discovery criterion remain unspecified, and discovery is not required for overworld continuation.
evidence_status: prediction
rule_cases:
  - "mechanics/pipe-transfer#entrance"
demands: []
```

```decision-assessment
id: cellar-route-choice
task: Choose cellar entry or overworld continuation.
context: Both routes are valid; destination information, relative costs and player preferences are unspecified. Aligned Down input belongs to entry performance, not proof of route preference.
evidence_status: authored-assessment
rule_cases:
  - "mechanics/pipe-transfer#entrance"
demands: []
```

## Route

Overworld continuation goes to first-gap. Choosing aligned down entry goes to bonus-entry. These arrows do not encode the input/alignment predicate.
