# Q1/Q3 — compare declared hazard responses

Bounded agent answer checked across cases. No new design issue inferred; existing rules unchanged.

Question: how do small/tall/spark form and enemy-contact protection affect authored walker side contact, gap death and clock expiry? These three situations are not a complete hazard inventory.

## Answer

- **Small, unprotected; walker side contact:** [walker-small](game-design/mechanics/enemy-contact.md) proposes `Hero.Life=dead` plus local Lives expression `this - 1[life]`.
- **Tall or spark, unprotected; walker side contact:** `walker-powered` proposes small form, protected state and configured protection time. Explicitly preserves local `Lives.Amount`. It does not emit a `Hero.Life` write/preservation; do not turn omitted effect into a checked survival result. Other simultaneous effects remain outside this comparison.
- **Protected, any of three forms; walker side contact:** `walker-protected` explicitly preserves Hero Life and Form. Starting Life is not supplied here, so preservation is not proof Hero is alive or becomes alive. Protection time is not frozen.
- **Gap lower-boundary crossing, all six Form/Protection combinations:** [gap-lethal](game-design/interactions/touch-gap-bottom.md) proposes Hero death and local Lives expression `this - 1[life]`. Protection does not suppress this declaration.
- **Clock-expiry occurrence, all six combinations:** [expiry](game-design/mechanics/active-clock.md) proposes Hero death and local Lives expression `this - 1[life]`. Remaining clock value/expiry occurrence is not supplied or inferred from the Form/Protection hypotheses.

These are conditional declarations under each source case's full guards and unknowns. No contact, falling, expiry, initial scene or runtime event has been established. No death frequency, difficulty or ranking of hazards follows. “Gap rule declares death across these states” does not mean “pits are harder.”

## How native output supports this

Read-only joint `state-chart` supplies Form and Protection to one exact local Hero binding at a time. Other participants, context and axis values stay unknown. Source case identities remain separate; matching noun/participant names never merge actors or inventory.

For each of six combinations, compare three distinct walker-case hypotheses:

- One has retained conditional proposals and `applicability: unknown`.
- Other two return `applicability: false` with no proposals under supplied enum values.
- This is a comparison of independently evaluated declarations, not automatic runtime dispatch or proof exactly one gameplay event occurs.

Gap and expiry each retain six conditional death hypotheses. All30 representative case/binding/state rows are checked. Remaining chart rows stay available; they are not presented as an exhaustive vulnerability matrix.

**Read side effects, not only arrows.** Selected axis is Form. Gap and expiry have no Form transition arrows, yet retained side-effect proposals contain Life death writes. Empty transitions mean no retained selected-axis proposal, not no effect, survival, a self-loop or preservation. Protection is the companion assumption, not a combined-state reachability graph.

Every proposal retains affected participant/noun, canonical axis, source case/path/line, conditional flag and activation path. Source cases retain whole context, requirements, unknown conditions/effects and other participants. Numeric Lives values remain unknown without initial quantity; do not add decrements across independent cases. Simultaneous lethal-event deduplication remains authored unknown, not permission to spend several lives.

## Reproduce

From repository root:

```powershell
node scripts/audit-workbench-design-questions.mjs `
  --executable skills/game-design-workbench/native/build/reference-similarity-cli/gd.exe `
  --output skills/game-design-workbench/native/build/question-workflow-NEW
```

Existing output refused. Audit delegates interpretation to native C; JavaScript selects native rows and asserts fixture expectations, not a second evaluator.

Q1/Q3 checkpoint result `../../native/build/question-workflow-q1-q3/` (later all-question replay adds structural/encounter checks):

- 33 successful native probes, including new `hero-form-protection-hazards.json`; prior32 probes remain successful.
- Hero chart:128 exact local bindings,768 hypotheses,918 retained proposals,804 activations; no retention/storage/work omission. Finite query complete; source incomplete and final outcomes unproven.
- Five representative source cases across six combinations:30 rows. Checks preserve exclusions, conditional death/power-loss/preservation and the empty-Form-arrow/Life-side-effect distinction.
- Native charged state coverage work60,622,956 within unchanged67,108,864 ceiling. This workload count is not a runtime or performance guarantee.
- Full authored game-design hashes unchanged; executable/source/script hashes and exact argv retained in `metadata.json`. Script syntax check and diff whitespace check pass. No engine/rule/schema changes, renderer launch, prototype, player study, commit or push.

## Remaining limits

Q1 still does not have an automatic scene-wide dispatcher or complete combined outcome. Q3 still lacks observed hazard encounters, exposures, deaths, a difficulty definition and player evidence. Existing source/native output is sufficient for this stated conditional comparison. No new score, rule repair or gameplay-role inference is justified.
