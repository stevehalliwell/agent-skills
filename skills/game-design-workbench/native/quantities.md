# Numeric predicates and quantity proposals

Author readable rules. Shared C99 math handles controls and actor quantities; no JavaScript evaluator or gameplay runtime. Raw `gd-rule` text stays authoritative.

```gd-rule
case spend:
    context: ANY
    Actor:
        noun: nouns/player-energy
        requires: Amount >= {control-vars/push-cost}
        other_axes: ANY
        set:
            Amount: this - {control-vars/push-cost}
```

- Predicate LHS names one numeric axis through owning participant's explicit inventory. Bare technical name or canonical axis reference; no global guesses.
- Comparisons: `=`, `==`, `!=`, `<`, `<=`, `>`, `>=`. Group with existing AND/OR/NOT. Arithmetic parentheses/functions stay inside numeric atom; logical operators end that atom outside arithmetic groups.
- RHS uses existing bounded control formula grammar: numeric literals with `[unit]`, `{ALIAS/NAME}`, arithmetic and documented numeric functions. Dollar-prefixed references fail. No cross-actor variable lookup or general algebra solver.
- `this` is assignment-target starting value, not previous assignment's output. Every quantity query reads same immutable supplied facts. Predicate RHS cannot use `this`.
- `UNKNOWN` explicitly retains unknown result. It does not mean zero, unchanged, a sample or a random value. Omitted effects remain unspecified.
- Existing enum/boolean configuration stays typed: `becomes: {control-vars/initial-power}` resolves enum value through actor axes; `requires: {control-vars/clock-runs-in-cellar}` requires boolean. No boolean-to-number or enum-to-truth coercion. Configuration value is not automatic actor initialization.
- Missing actor values/control inputs remain unknown. Unloaded control collections preserve unresolved references; an explicitly supplied control collection with a missing reference produces a sourced error.
- Unit symbols/scales remain exact. Numeric operands do not silently convert units. Arithmetic uses binary64 rounding, not exact decimal/rational arithmetic. Integer consumer checks apply to represented operands/results in JSON-safe range. Reject fractional final values rather than convert them, nonfinite arithmetic, unsafe integer magnitudes and divide by zero. Reject literal text whose fractional part rounded to an integer; genuine fractional intermediates such as `0.5 * 2` remain supported. An integral floating-point result is not proof of exact authored decimal mathematics.
- Predicate thresholds may be fractional even for an integer axis. Axis bounds constrain proposals, not comparison thresholds.

## Inspection versus scenario

`gd rules` emits each assignment's raw expression/target plus derived `quantity`: syntax `state`, raw `text`, token count, binding readiness and `inspection`. Numeric condition tokens retain `compare`, canonical target, comparison and RHS quantity inspection. Enum/boolean references retain canonical control provenance.

Inspection has no actor starting values. A constant proposal can resolve; `this + 1[life]` cannot fabricate its starting value. `inspection.number` may retain a finite proposed value despite a numeric-type error. `bounds` distinguishes unknown, not declared, within authored bounds and outside authored bounds. Outside bounds is a finding, not an implicit clamp, wrap or gameplay rejection. Explicit authored min/clamp remains explicit math.

Case-level `unknown_conditions` and `unknown_effects` retain readable missing/unmodeled qualifiers with source lines. They do not create predicates, defaults or effects. Known qualitative motion, geometry, once-only constraints and reset details must not disappear merely because arithmetic exists.

Read-only `gd_rule_quantity_value` and `gd_rule_condition_value` consume explicit `GD_RulePredicateFact` assumptions keyed by snapshot participant/axis. Fact `value` is enum token or numeric literal in declared axis unit. TRUE asserts literal; false/unknown numeric facts alone do not establish a current quantity. Conflicting, missing, wrong-type or oversized numeric assumptions remain unknown. Facts do not prove realizable scenes or observed gameplay.

These primitives establish neither complete case applicability nor preservation, composed outcomes, consistent simultaneous writes or universal equivalence. Remaining Review 01 integration must retain that distinction.

## Ownership and bounds

- `gd_controls.c`: one numeric compiler, unit implementation and value evaluator. Existing control dependency solver remains owner of configuration evaluation. Mixed role loads search control partition of role-sorted model.
- `gd_rule_quantities.c`: scoped quantity programs, typed assumptions, proposal/type/unit/bounds checks and selected control dependencies. Result owns dependency snapshot, borrows model/source text; destroy with rule result. No source writes or internal allocation during pure queries.
- 4,096 expression bytes, 256 expression tokens, 128 expression-stack entries; existing unit limits apply.
- 16,384 rule quantity programs, 32,768 shared quantity tokens; 67,108,864 charged quantity work units, not a memory allocation or wall-time claim.
- Boolean field/query budget stays 16,777,216 charged work units. Numeric reservations cover bounded byte/unit operations and numeric fact parsing. A field/guard that does not fit remains unknown/unexamined.
- Malformed/limited RHS in a condition rolls back its whole Boolean field and its quantity programs; no `ANY OR malformed` success. Other complete fields remain. Charged work is not refunded. Invalid assignment retains raw declaration and no partial program.
- Numeric facts: at most 256 per call; numeric literal fewer than 128 bytes. No recursion, actor-state mutation, implicit downloads or experiment rerun.

Checks cover comparisons, arithmetic grouping, same-starting-value reads, same-type participants, missing/conflicting facts, typed control references, fractional thresholds, consumer constraints, explicit unknowns, source lines/raw offsets, whole-field rollback and source immutability. Linux execution and integrated UX remain untested.
