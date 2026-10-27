# State predicates and unknown truth

Review 01 slice. Shared C99 compiles participant `requires` and case-local `skip_if` into derived Boolean programs. Raw body text stays authoritative. No JSON expression-tree authoring, gameplay runtime or source writes.

## Source

```gd-rule
case contact:
    Player:
        noun: nouns/hero
        requires: alive AND (tall OR spark) AND NOT protected
        skip_if: dead
        other_axes: ANY
```

- Parentheses group expressions. Precedence: `NOT`, then `AND`, then `OR`. Binary operators associate left; repeated `NOT` associates right.
- Logic keywords and `ANY` are ASCII case-insensitive. `&&`, `||`, `!` mean AND, OR, NOT. State/axis names remain exact and case-sensitive; no alias or case guessing.
- Resolve bare states through that participant's explicit noun axes. Use `AXIS.STATE` for ambiguous names; optional canonical `ALIAS/AXIS.STATE` selects a loaded definition explicitly. Same-type participants remain distinct local bindings.
- `ANY` is an explicit unrestricted predicate. Missing `requires` or `skip_if` stays `unspecified`, not ANY, false or an inherited actor-wide guard. Qualify a state literally named `ANY`, `AND`, `OR` or `NOT`.
- State atoms are literal spans, not quoted strings or function calls. Whitespace, parentheses and logical symbols delimit atoms. Named values containing those separators are outside this condition fragment; their axis declarations remain intact.
- Numeric comparisons containing `<`, `>` or `=` remain `unsupported`: raw condition retained, no compiled program or Boolean result inferred. Numeric expressions/units need shared quantity-expression work, not a second evaluator.
- `other_axes: ANY` explicitly leaves other dimensions unrestricted. Omission stays `unspecified`; other values produce a sourced error. Neither form changes axis values or proves reachable combinations. ANY here does not fill a missing `requires`.
- True `skip_if` means exclude its owning case when applicability analysis is implemented. It is not a positive requirement, actor-wide inheritance or another case's guard. This slice compiles the guard separately; it does not return combined case applicability.

## Headless facts

Run existing `gd rules` with explicit interaction/mechanic, noun and axis bindings. `--fields cases,findings` selects condition detail. See [rule reader](rules.md) for full invocation and exit behavior.

Each participant keeps raw `requires`, `skip_if`, `other_axes` and field lines, plus:

- `requires_condition`, `skip_if_condition`: `state`, source `line`, `bindings_complete`, derived postfix `program`.
- Condition states: `unspecified`, `parsed`, `invalid`, `unsupported`, `limited`. Parsed syntax is not an applicable-case verdict or proof that all state names resolve.
- Program tokens: `op` (`state`, `any`, `not`, `and`, `or`), original `text`, byte `offset` within raw field value. State operands also retain canonical target/state and resolution. Operators never mutate actors.
- `other_axes_mode`: `unspecified`, `any`, `invalid`. Preserve omitted versus authored scope without representing omission as false.

Coverage adds `condition_count`, `condition_token_count`, `condition_bindings_complete`. Readiness covers authored supported predicates and `other_axes` declarations in retained cases only. It does not cover absent rules, unspecified other dimensions, context, reachability, effects, composition or complete migration. Zero authored conditions does not imply an unrestricted design.

Ambiguous names, closed-missing values, incompatible types, malformed logic and limits produce sourced errors. Open/incomplete names remain undetermined with informational findings. Unsupported numeric predicates remain informational partial coverage, not false. Invalid/limited fields return no valid-looking partial program; earlier complete fields stay inspectable.

## Read-only C99 truth

`gd_rule_condition_value` evaluates one compiled predicate under explicit `GD_RulePredicateFact` inputs. Facts key snapshot participant index, axis-record index and exact named state. These are logical assumptions supplied by an analysis caller, not gameplay observations or proof of a realizable scene.

- Three values: `GD_RULE_TRUTH_UNKNOWN`, `GD_RULE_TRUTH_FALSE`, `GD_RULE_TRUTH_TRUE`. Compare enum values explicitly; result is not a C Boolean.
- Missing facts, conflicting facts for one key, unresolved state operands and unparsed conditions remain unknown. Same-noun facts for another participant do not match. Invalid fact counts return unknown without reading outside supplied storage.
- `ANY` is true. Strong three-valued logic preserves uncertainty:

```text
NOT unknown           = unknown
false AND unknown     = false
true AND unknown      = unknown
true OR unknown       = true
false OR unknown      = unknown
```

```c
GD_RulePredicateFact fact = {player_index, life_axis_index, "alive", GD_RULE_TRUTH_TRUE};
GD_RuleTruth value = gd_rule_condition_value(
    rules, &rules->participants[player_index].required_condition, &fact, 1);
```

A true expression does not establish case applicability: other participants, unspecified axes, ordered context and local exclusions still matter. No input facts supplied by CLI yet; CLI exposes declarations/programs, not fabricated truth values. No tick, state transition, effect execution, sequence or independent corroboration is inferred.

## Case-local early-outs

Omitted `skip_if` means no guard declared. Do not evaluate absence, invent a false predicate or inherit a guard from another case/actor type. This differs from an authored guard whose value is unknown. Omitted `requires`/context still remain unspecified.

`gd_rule_case_exclusions` checks only authored local guards under explicit predicate facts. Return `GD_EXCLUSION_ABSENT` for no guards, `GD_EXCLUSION_NOT_EXCLUDED` when all authored guards are false, `GD_EXCLUSION_EXCLUDED` when any is true, or `GD_EXCLUSION_UNKNOWN` when remaining guards are unknown/unexamined. A known true guard excludes its case even if another guard is unknown; it does not exclude other cases.

Result retains `guard_count`, `checked_count`, snapshot `excluded_by` participant index (`-1` without witness), charged `work` and `limited`. A witness may end the check early; this is not priority or an interaction winner. CLI exposes authored `skip_if_count` only; no scenario facts or fabricated truth supplied.

Each check reserves conservative token/byte-comparison upper bounds against a separate 16,777,216 evaluation-work budget. A guard that does not fit stays unexamined; no partial false result. Inputs remain limited to 256 facts and existing bounded case/program storage. No-guard result needs no predicate evaluation. This checks exclusions only: absent/false guards do not prove positive requirements, context, complete axis scope, valid scene, applicable effects or a consistent transition.

## Bounds and checks

- Per field: 256 input tokens, 64 nested groups. Per result: 65,536 compiled tokens. Operator storage/evaluation stacks bounded by 256. No recursion or allocation inside compilation/evaluation.
- All condition/effect state lookups share existing 16,777,216 charged comparison/traversal work units. Work exhaustion retains prior checked bindings and makes remaining lookups unavailable, never false/missing.
- Global token-cap failure rolls back the current field and leaves later authored conditions limited, not partially compiled. Per-field syntax/depth/token failure does not discard other complete fields.
- Evaluation accepts at most 256 facts and 256 program tokens. Matching scans those bounded facts; state spelling comparisons stop at the compiled atom's retained length. Contradictory facts for one key remain unknown; this does not validate an entire scene's axis combinations.
- Native checks cover all NOT/AND/OR truth combinations, missing/conflicting/cross-participant facts, ANY versus omission and independent skip programs. CLI checks cover precedence/grouping, keyword/symbol aliases, exact state spelling, raw offsets/Unicode, unknown/unsupported/error output, source immutability, shared condition/effect lookup-work exhaustion and exact/exceeded depth/token limits. No UX, physics or universal equivalence acceptance claimed.
