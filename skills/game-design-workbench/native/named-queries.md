# Native delegated questions

Five built-in names reuse existing native algorithms and report writers. No new evaluator, recipe storage, gameplay taxonomy or query language.

```text
gd query --list --json
gd query --question NAME --projection PROJECTION SOURCE_BINDINGS... PARAMETERS... --json
```

## Operations

- `conditional-outcomes`, projection `rules`: explicit `--case ALIAS/NAME#CASE`; optional repeated `--assume LOCAL:ALIAS/AXIS=VALUE`, `--context "LEFT ALIAS/AXIS.STATE RIGHT"`, `--context-not "LEFT ALIAS/AXIS.STATE RIGHT"`; optional `--work-limit 0..16777216`.
- `conditional-consistency`, projection `rules`: compare supplied rule cases with fixed native work/branch/pair limits. No case, assumptions, endpoints or work override.
- `skill-unlocks`, projection `skills`: optional repeated `--known ALIAS/NAME` and selected `--from ALIAS/NAME`. Resolve active Skills draft, not baseline model. Fixed native work limit.
- `resource-declarations`, projection `resources`: whole existing Resource declaration graph. No endpoints, assumptions, accepted-only flag or work override.
- `resource-routes`, projection `resources`: required `--to NODE_ID_OR_REFERENCE`; optional `--from NODE_ID_OR_REFERENCE`, `--accepted-only`, `--work-limit 0..16777216`. Blank FROM means upstream search.

Bindings remain explicit. Skills needs `--skill-atoms`. Rules/Resources need `--mechanics` and/or `--interactions`; supply authored nouns, axes, Resources and controls as needed. Missing definitions remain sourced gaps, not invented bindings. Wrong operation/projection pairs and unsupported graph/status/display filters fail explicitly.

## Semantics and scope

- Whole supplied native reader context, not camera, graph filter or visible nodes. Skills uses current draft revision.
- Outcomes reuse three-valued whole guards, early-outs, simultaneous same-starting-value proposals, exact local targets/maps and whole-expansion preservation. No conflict winner or execution claim.
- Consistency aligns uniquely typed local roles hypothetically. Existential conflict and bounded equivalence do not establish shared runtime actors or gameplay occurrence. Keep native witness, vocabulary, numeric and unfinished-branch flags.
- Knowledge means explicit assumptions only. Do not expand known set transitively. Prerequisites remain AND. Empty prerequisites do not mean learned; chains/combinations/shared links do not prove acquisition, contrast or mastery.
- Resource requirements enable without consumption. Checked debit/credit needs explicit pool+axis joins. Preserve full ancestor guards, mapped placements, OR structure and unresolved frontiers. Conditional witnesses are not feasible plans, temporal sequences or final outcomes.
- Route eligibility retains native accepted/proposed and defined-interaction policy; accepted-only narrows it. Other delegates retain their existing status semantics, not a generic status filter.

## Output

JSON keeps native report intact:

```text
data.question
data.projection
data.parameters
data.native_result.data
data.native_result.coverage
coverage
```

Native result remains authoritative for typed identities, exact source lines/conditions, findings, checked work and omissions. Wrapper coverage describes delegation/owned bytes; it is not a replacement completeness verdict.

`--fields` prunes only `data.native_result.data`; envelope parameters/coverage remain. Supported fields appear in catalogue. Text mode calls existing native text writer after named-question/parameter heading. Existing CLI paths remain available; tests compare complete nested native data against those paths.

## Bounds and ownership

- At most 64 raw literal facts, 64 context triples and 64 known-skill assumptions. Native enum expansion still has 256-fact cap; native context cap remains 64.
- Outcome evaluation: 16,777,216 work units. Input parsing/compiled source preparation have separate existing native limits.
- Consistency: 67,108,864 work units, 4,096 retained pairs, 128 branches per comparison and 16 witness facts. Report unchecked pairs/branches separately.
- Skills: 16,777,216 work units and 128 shared-link comparison rows plus omission count.
- Resource route search: 16,777,216 work units, 64 witnesses, 64 steps. Selector work is separate. Upstream graph construction gaps remain native graph coverage, not completed-search proof.
- Declaration delegate borrows graph; no additional search/work ceiling.
- `gd_named_build()` owns result/scratch through project allocator. Rule delegates allocate private mutable `GD_RuleAnalysis` workspace, copying compiled tables only; never evaluate/compare into `project->calls`.
- Model/rules/graph and request strings outlive result. Skill draft edit invalidates retained skill result. Writer rejects stale draft revision/model before any output; rebuild before reading borrowed model.
- Legacy assumption parser combines source lookup and diagnostics. Single-threaded adapter forwards diagnostics and restores every source diagnostic slot when diagnostic model is separate. Viewer captures/restores same channel around candidate construction. Source records/rules unchanged; no concurrent parser/observer contract added.
- Candidate failure frees only candidate; caller retains prior result. Destroy result before project/draft storage.

## Viewer

Resources contains independent **Object bindings / rule checks [native]** section with conditional outcome, consistency and object-binding modes. No axis/finite-variant requirement. Same named adapter supplies scenario/comparison, proposals, conflict pairs and native witnesses. Inputs are literal lines; `!` prefix marks negative context triple. Pending edits do not change applied result. Rule proposals/pairs mark original case declarations in purple, including native proposal source-case ancestry. These outlines do not identify distinct mapped placements or assert Resource flow/applicability. Inspector retains raw source/call/map access and reports hidden/unretained declarations.

Object-binding mode delegates existing `object-bindings --projection actors` kernel, with all object/case/local/axis/status/work selectors. Native binding rows preserve distinct local identities, declared axis types/domains/units/bounds and full source. Selected row outlines original case declaration. No extra evaluator or geometry. Axis details use 16-reference pages; compact case card shows at most 64 participants with explicit omission count and full source body available.

Rule/binding inspection owns separate pending/applied buffers and private workspace. Run replaces atomically. Clear releases inspection. Refresh rebuilds from request strings only when native reader fingerprint/bindings remain unchanged; otherwise clear inspection before accepting changed context. Transient row inspection clears on refresh. This conservative guard is not a new durable actor identity scheme.

Skills retains existing explicit-knowledge/chains/combinations controls. Resources retains declaration/conditional-witness controls and adds native route work ceiling. Pending work ceiling keeps applied witnesses visible; failed run retains them; refresh requires pending ceiling applied first. Ordinary geometry/camera and Resource layout policy unchanged. Similarity canvas remains separate; explicit source-inspection action returns to ordinary canvas.

## Verification boundary

Full CJIT/Zig native and CLI, Node/public Attendant integrations, strict both-example metadata validation and bounded window/render checks pass. Native 127/127 per compiler; Node retains one skipped desktop-report integration. Extended window harness exercises all three inspector modes, candidate failure, reload/reset and cleanup on private copies, without OS input. Real-pointer/manual usefulness remains human gate. [Validation evidence](../../../reports/graph-query-research/validation-checkpoint.md); native reader/report checks do not replace Attendant validation or all-14-question usefulness review.
