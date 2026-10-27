# Conditional outcomes and bounded comparison

Shared C99 reads authoritative `gd-rule` text. Optional headless analysis combines predicates, explicit axis scope, ordered context and local exclusions, then expands simultaneous proposals. It does not execute gameplay or infer an initial scene.

## Inspect one case

Use normal [explicit collection bindings](rules.md#run), plus:

```text
--case interactions/touch-walker-powered#powered-side-contact
--assume Player:axes/Life=alive
--assume Player:axes/Form=tall
--assume Player:axes/Protection=unprotected
--assume Walker:axes/Life=alive
--context "Player axes/Contact.sideContacts Walker"
--fields analysis
```

- `--case ALIAS/NAME#CASE` selects one parsed case. Omit `#CASE` only when record has one case. No implicit selection.
- Repeat `--assume LOCAL:ALIAS/AXIS=VALUE` for current values. Require explicit noun/axis membership and valid type. Numeric text uses declared axis unit; no expression, coercion or starting value from configuration. Enum current-value input supplies selected value true and other declared values false; absent axes stay unknown. Open enum values do not close vocabulary.
- Repeat `--context "LEFT ALIAS/AXIS.STATE RIGHT"` for hypothetical ordered relation truth. Selected case's known bare operator also works. `--context-not` supplies explicit false. Reversed endpoints do not imply false or true. Context input requires declared enum operator; no geometry, contact or symmetry inferred.
- At most 64 value arguments and 64 context arguments. Enum expansion fits 256 facts. Contradictory facts, unknown locals, unsupported types and unowned axes fail, never select a winner. Query input work has separate 16,777,216 budget.
- Without assumptions, unknown quantities/predicates remain unknown. `ANY` stays explicit. Omitted guard creates no exclusion; it does not establish positive applicability. `unknown_conditions` narrows applicability to unknown unless another condition already establishes false.

`analysis.scenario` retains selected source, typed assumptions, context assumptions, three-valued applicability/context/requirements, exclusion witness/counts, expansion/cycle/work counts, limits, proposal provenance and write checks. These are supplied hypotheses, not observations. `constraints_complete` covers declared targets and known actor-axis effect scope; `values_complete` covers emitted values. Neither proves an exhaustive design, reachable scene or physical validity.

Each proposal retains root participant/noun, canonical axis, state or numeric value, expression, bounds, source record/case/line and expansion path. `conditional: true` means applicability or expansion scope remains uncertain. Finite invalid/out-of-bounds values remain visible; bounds never clamp, wrap or reject gameplay. Write checks use true/false/unknown conflict, not file order or priority. Unknown/omitted effects are not preservation.

## Explicit composition

```gd-rule
case contact:
    context: ANY
    Player:
        noun: nouns/hero
        requires: ANY
        causes: mechanics/power-loss#small with Victim=Player, Budget=Lives
    Lives:
        noun: nouns/lives
        requires: ANY
```

Invocation: `ALIAS/NAME[#CASE] with Callee=Caller, Other=OtherCaller`. Separate calls with semicolons. Reference without case selects all callee cases, still unordered.

- Every selected callee participant needs explicit map to a local caller. No label/type-based guessing. Known noun types must agree; unknown bindings remain incomplete rather than authorize a transformation.
- Shorthand `causes: mechanics/power-loss#small` inside participant binds only when every selected callee case has exactly one participant. Local spelling need not match.
- Expand iteratively. Active-path cycles, unavailable calls, work/depth limits and opaque effects remain incomplete. No implicit repeated application, later tick, automatic repair or sequential mutation.
- All direct/invoked quantity expressions read same immutable starting facts. Mapping changes participant identity, never values or time.
- Composition preserves every declared source/effect. Explicit reference agreement is not independent corroboration.

## Preservation

`preserve: Life, Form`, `preserve: ALL`, `preserve: OTHER` name constraints, not default effects.

- Named axis resolves through participant noun inventory. ALL preserves each declared applicable axis.
- Collect writes across whole expanded case before resolving OTHER. A component's OTHER must not contradict explicit writes in another component merely because of presentation order.
- Opaque/conditional/unexamined effects make affected OTHER promises conditional. Never claim preservation because a call was skipped by compilation limits.
- Named/ALL preservation may conflict with explicit writes. Retain both; no implicit override.
- Missing starting value leaves preserved value unknown. Duplicate declarations remain separate; duplicate inventory entries do not create artificial ambiguity.

## Compare declarations

Add `--analyze` to compile invocations and compare bounded known enum branches across supplied records/collections. May combine with `--case`; scenario stays separate. `--fields analysis` requires `--analyze` and/or `--case`. Default `gd rules` remains declaration inspection.

Comparison relations:

- `equivalent`: same scoped gates/proposals across fully checked closed enum product, complete effect scope and no unexamined numeric domain. Not a physics or gameplay proof.
- `redundant`: matching scoped declarations, but numeric domains or missing effect scope prevent complete equivalence claim. Structural numeric-program equality is not algebraic equivalence or proof arithmetic is valid for every input.
- `compatible_overlap`: established overlap without incompatible modeled writes; different/partial effects may coexist. Unmentioned effects stay unspecified.
- `conflict`: concrete incompatible simultaneous writes under retained hypothetical witness. Retain both writes and their source paths, even after scratch evaluation changes.
- `disjoint`: checked closed-domain gates do not overlap within established actor/context scope.
- `undetermined`: missing/ambiguous scope, open vocabulary, unsupported numeric reasoning, unexamined branches or limits prevent stronger result.

Unique noun types align participant variables hypothetically for comparison, not invocation bindings or observed actor instances. Multiple same-type locals remain undetermined; never guess which shell is which. Different ordered predicates/action links do not establish joint occurrence. No reversed-contact inference. Input statuses/dispositions remain attached to each source; default includes all supplied sources, not an accepted-only verdict.

Reports retain checked/overlap/matching/compatible/undetermined branches, unexamined known branches (`null` if product count unknown/overflowed), open-vocabulary/numeric/effect coverage, scope reason, shared derivation and witnesses. A valid conflict witness is existential; unexamined branches cannot erase it. Agreement over known values never closes an open vocabulary. Numeric continuum is not enumerated. Runtime unit/arithmetic errors remain uncheckable even when programs match. No universal mathematical solver, probability, quality/fun/learning score or automatic source merge.

## C99 ownership and budgets

`gd_rule_analysis.h` exposes analysis construction/destruction, case lookup, bounded scenario input parsing, pure evaluation and bounded comparison. Analysis allocator owns fixed storage; model/rules remain immutable and must outlive analysis. Evaluation borrows validated assumption arrays/text until reporting finishes. Destroy analysis before rules/model. Comparison copies conflict witnesses instead of publishing mutable scratch indices. No evaluator/source writer in JS, no projection dependency, no recursion.

Caller-provided evaluation storage is large; use caller allocator/arena rather than assuming a small thread stack. Current Windows/CJIT x64 `sizeof`: model 21,882,368 bytes; rules 15,086,744; controls 4,112,424; analysis 9,384,944 (includes owned scenario/scratch evaluations). Combined 50,466,480 bytes before retained rule text/allocator overhead. One evaluation is 1,951,368 bytes; scenario input is 8,208. Sizes depend on compiler/ABI, not a portable storage contract. Native checks use a 64 MiB arena; no broad peak-memory benchmark claimed.

- 8,192 calls; 32,768 bindings; 64 mapping entries per call.
- 16 expansion levels; 4,096 expansions; 8,192 simultaneous proposals.
- 256 predicate facts; 64 ordered-context facts; 128 retained write-check witnesses plus omission count.
- 4,096 case pairs; 128 declared enum branches per pair; 16 witness dimensions.
- 67,108,864 invocation-compilation work units; separate 67,108,864 comparison work units; 16,777,216 per evaluation.
- Conservative whole-evaluation reservation may stop comparison before nominal budget fills. Earlier checked portions remain; report later pairs/branches unexamined. Work counts are charged operations, not wall time or byte allocations.

Tests: native `rule_outcomes` plus `test/native-analysis.test.mjs` cover direct/composed agreement, whole-case OTHER, explicit preservation conflict, runtime errors, source/fact immutability, ordered context, contradiction/type/ownership failures, cycles, invocation exhaustion, branch relations and incomplete vocabularies. Manual UI, physical reachability, broad performance and universal equivalence remain untested/out of scope.
