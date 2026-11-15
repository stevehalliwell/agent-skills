---
id: 01a16f86-6ff6-77ff-81eb-09e65f43dd5a
name: flower-shelf
created_at: 2026-10-24T20:07:23.894Z
desc: Optional elevated supply can grant ranged power to a tall hero.
tags: []
status: accepted
entry: false
next:
  - challenges/shell-lane
introduces:
  - skill-atoms/fire-spark
practises:
  - skill-atoms/bump-block
  - skill-atoms/read-power
mechanics:
  - mechanics/supply-release
  - mechanics/spark-upgrade
  - mechanics/spark-launch
nouns:
  - nouns/spark-flower
---
## Goal and context

Optional raised growth supply rewards landing and underside targeting. Tall hero can obtain flower; small hero obtains a cap instead.

## Assistance and prior knowledge

Ranged capability is an opportunity conditional on carried form and collection, not guaranteed by entering this challenge.

## Decision and assessment

Separate choosing whether to pursue supply from performing release and collection. Skipping this optional opportunity remains valid. A useful reward does not establish that player wants ranged combat, understands its benefit, compares constrained improvements or is optimizing a build.

Pursuit becomes a candidate goal-directed improvement only under an explicit player goal and relevant constraints. No such player goal, alternative improvement set or comparison criterion is established here; leave optimization unclassified. Detour cost, clock margin and player preference are also unknown, so do not infer a meaningful trade-off or calculation demand merely from reward availability.

If player chooses collection, underside targeting, access and pickup contact involve execution. Release and collection are different events. Growth supply releases a cap for small form or flower for tall/spark form; flower collection by tall form grants spark, while spark collection grants no further form upgrade. Arrival alone grants nothing. Live-shot capacity still gates launching a shot after upgrade; obtaining spark does not guarantee an immediately available shot.

Intent check: should this remain an optional capability opportunity rather than a required answer? Whether players recognize or value it needs evidence. No explicit quiz, numerical reaction window, substantial dexterity test or global optimum is established. Missing a chosen pickup leaves a local goal unmet without proving failed course traversal or explaining why it happened.

```decision-assessment
id: optional-supply-pursuit
task: Choose whether to pursue raised growth supply.
context: Skipping is valid; player goal, valuation, detour cost and alternative improvements are unknown. Useful capability alone does not establish optimization or meaningful trade-off.
evidence_status: authored-assessment
rule_cases:
  - "mechanics/supply-release#growth-small"
  - "mechanics/supply-release#growth-powered"
demands: []
```

```decision-assessment
id: chosen-supply-collection
task: Reach and collect supply after choosing to pursue it.
context: Release and collection are separate; tall flower collection grants spark, while other starting forms differ. Arrival is insufficient and shot capacity still gates launch; geometry and timing tolerances are unspecified.
evidence_status: authored-assessment
rule_cases:
  - "mechanics/supply-release#growth-small"
  - "mechanics/supply-release#growth-powered"
  - "mechanics/spark-upgrade#tall-collection"
  - "mechanics/spark-upgrade#spark-collection"
demands:
  - kind: execution
    intent: undecided
```

## Route

Rejoin shell-lane. The challenge graph cannot express that power-state gate in its plain next arrows.
