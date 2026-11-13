# Contextual analysis entry points

First slice implemented; reviewed Mario binding flow accepted. Focused CJIT/CPU checks pass on both unchanged examples. User authorized commit and continuation. Acceptance covers reported binding flow, not unperformed outcome/every-view checks.

## Outcome

Expose relevant analysis from Selection without requiring knowledge of tool's original projection. Keep current canvas, camera and primary selection. Reuse existing rule/binding readers; no new evaluator or automatic analytical Run.

## Scope

- **Inspect bindings** for selected loaded noun, typed axis or exact local binding.
- **Inspect conditional outcomes** for exact declared case.
- Resolve contexts through typed retained selection/evidence, not label text or generic edge-key interpretation.
- A source record, pair or derived item may represent several cases or bindings. Present matching existing evidence for explicit choice; never pick first case, assume authored A, merge local identities or infer actor classification.
- Offer only supported context actions. Explain missing source/role/context when an otherwise relevant action cannot proceed. Non-rule selections need no invented mapping.
- Keep existing Resources entry point available. Current-pane tools use pane-owned rule state, not Resources pane's pending inputs/result.
- Existing rule reader works over loaded rule scope, independent of canvas filters/camera and finite Actor hypotheses. Show explicit requested/applied scope; absence, unknown conditions and work limits retain native meaning.

## Interaction

1. Select noun Hero in Noun interactions. Selection offers **Inspect bindings**.
2. Open current-pane tool. Show proposed typed context and existing pending/applied scope. Do not run reader or replace retained result.
3. Explicit context staging prepares binding mode with Hero as object. Selection does not continuously retarget tool when user inspects another node.
4. If existing pending inputs would change, preserve them and require explicit resolution before replacement. Default preserves pending inputs; opening/choosing context alone must not discard them. No replacement on allocation/validation failure.
5. **Run** uses staged inputs through existing reader. Retain last-good result on failure. Editing inputs retains applied evidence until successful Run.
6. Select result row to inspect exact declaration/case/local evidence within tool. Do not silently replace primary canvas selection, move camera, change layout or switch projection. Explicit ordinary source navigation remains a separate user action.

Conditional-outcome example:

- Exact declared case selected: offer that case for context staging, then existing assumption/context editors and explicit Run.
- Interaction record or pair with several cases: show matching declared cases first. Require exact case choice before staging outcome context. No record-wide result presented as one case.
- New case context must not silently inherit facts/local selectors from unrelated old case. Context replacement follows same pending-input protection.

## Technical boundaries

- `gd_explorer_rules_view.c` supports current-pane controls/results. Source navigation maps canonical reference, source path and ID into destination model. Resource node keys and marked-declaration counts remain Resources-only.
- Reuse `GD_ExplorerRuleState` pending/applied forms, `GD_QUERY_BINDINGS`, `GD_NAMED_OUTCOMES` and atomic candidate replacement. Keep per-pane ownership and existing reset/reload/draft rules.
- GUI and IPC share context discovery, staging validation, action availability and execution. Expose tools through existing action/control discovery, `rules` form and retained `bindings`/`rules` results on eligible panes; no separate CLI evaluator or new transport.
- Context mutation must reject stale selection/scope and ambiguous identities. Existing guarded request rules apply; do not retry timed-out mutations.
- Scope/reset/refresh handling must cover rule state outside Resources. Source inspection uses rule model's canonical identity; never interpret source indices in another pane's model directly.
- Preserve existing work ceilings and bounded source/evidence lists. Do not broaden paging, infer applicability or alter solver/query geometry.

## Acceptance

- Noun/axis/local-binding selection exposes correct bindings context in current pane. Exact case exposes conditional outcomes. Multi-case selection requires explicit choice; unresolved/unsupported context never falls back to unrelated source.
- Opening and staging preserve primary selection, camera, layout, other panes, drafts/history and analytical results. No implicit Run, query retarget or projection switch.
- Dirty pending inputs survive opening/context selection and cancellation. Successful explicit Run alone replaces retained result; failed candidates retain last-good state.
- Result evidence preserves exact cases, participants, guards, source paths, unknowns and native omissions. Ordinary source navigation stays explicit.
- GUI/IPC context selection and Run use same helpers. Cover stale tokens, invalid/ambiguous contexts, missing roles, allocation failures, independent panes and reset/reload behavior.
- Run focused CJIT/CPU checks on both unchanged examples. Coordinate separate visual review; no existing viewer mutation without authorization. Keep current Mario deep drift failure separate; no threshold/timeout relaxation.

## Implementation

- `gd_explorer_rule_context.h` resolves typed canvas metadata. Noun pairs retain exact declared membership; Actor map associations reuse retained case provenance. Native activations/local bindings stay exact. Shared Effect concepts enumerate source cases from retained incoming proposal links, not first contributor. Verb cells expose contributing records' declared cases; UI states this is source association, not exact cell/case membership.
- Cross-family source mapping checks canonical reference, source path, ID and retained noun identity. Normalized projection roles do not become rule indices. MDA uses its current `mda.record`, not stale generic linked selection.
- GUI and IPC share `rules.bindings.open`, `rules.outcomes.open`, `rules.selection_context`, `rules.inputs.reset`, `rules.run` and retained `bindings`/`rules`. Context keys are closed, mode-specific choices. Rules tokens include typed selection, applied canvas Query scope, scene identity and context mode.
- Opening shows proposed context plus pending/applied scope. Staging does not Run. Dirty pending inputs block replacement; explicit input reset keeps retained result and permits fresh context. New contexts clear unrelated facts, local selectors and status filters. Native candidate failure keeps last-good state.
- Each pane owns rule workspace. Result rows inspect tool evidence only. Explicit source button maps into current pane; no Resource-node interpretation elsewhere. Escape also clears tool row/marks, not its pending/applied inputs or retained result.
- Call-local discovery snapshot: **4,108 bytes**, no allocation/evaluator. GUI/control pages batch source/case discovery; scene contributors require one retained-link scan. Pane rule owner: **118,880 bytes**, excluding native owned result. Existing evaluator/work ceilings unchanged.

## Verification

- Warning-free Windows CJIT Graph, ExplorerTest, CLI and core builds. Fresh output: `skills/game-design-workbench/native/build/contextual-analysis-work/`.
- Core: **148/148**. Focused Find/shared UX/production IPC: **9/9**, both examples' source hashes unchanged.
- Added `test_explorer_rule_context.h`: **396** Crate checks, **15,514** Mario checks. Covers independent pane staging, real noun/native local/activation contexts, exact pair/map/shared-Effect evidence, duplicate case/local rejection, missing roles, dirty pending protection, failed allocation/Run retention, raw geometry/camera/selection/query/draft preservation, guarded IPC discovery/staging/stale selection and cross-family result serialization.
- Same-snapshot rule remap candidate rebuild and dirty-input rejection checked outside Resources. Reset view retains workspace; Reset all clears all pane rule owners. No window, OS input or source writes.
- Artifacts: `contextual-analysis-{graph,adapter,cli,core-build,core,checks}.log`, `crate-context-checks.log`, `mario-context-checks.log` in fresh output.
- Authorized separate Mario viewer PID12740/session `12740-7534ac21ffface12` opened on Noun interactions Free. Selected Hero, paused layout and opened Inspect bindings; no inputs staged or analysis Run by agent. User confirmed staging Hero, explicit Run and result-row inspection behaved as described, then authorized commit/continuation. Existing viewers not closed/replaced; no OS input injected.
- Conditional-outcome visual review and broader pointer/every-view/restart/standalone/platform checks remain unperformed. CPU checks do not establish those outcomes.

## Exclusions

- Contextual Actor state queries, Resource witnesses, structural/similarity tools and global conditional-consistency entry points.
- Evidence-list paging redesign, similarity navigation redesign, automatic analysis, source writes and persisted investigation state.
- Every-view usefulness acceptance, restart/standalone checks and final compiler/platform matrix.

## Comments

Deep/research checks not rerun. Prior Mario deep presentation drift **1.819844 > 1** remains separate; no solver, threshold or timeout change. Viewer launch/setup and commit received separate explicit user authorization. Review viewer remains user-owned; do not change or close it without authorization.
