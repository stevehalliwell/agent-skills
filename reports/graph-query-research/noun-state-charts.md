# Noun state charts

## Result

Implemented read-only `state-chart` query and separate **State charts** explorer view with **Statechart** and **Grid** modes. Select one exact noun and one or two declared enum axes. Reuse native binding inventory, finite actor hypotheses and mapped rule evaluation; no second predicate parser, transition authoring store or evaluator.

- Initial CLI slice: one axis at a time. GUI follow-up: UML-style diagram as primary mode, Grid as companion. User selected native evaluation of joint combinations, not an independent-evidence-only grid.
- Return state vocabulary, independent local-binding hypotheses, selected-axis proposals, full source guards, other-participant effects and unknowns.
- Gameplay-role representation remains open. Do not infer enemy/ally/safety from physical state, noun origin or `is_actor`.
- Both modes inspect the same applied native result. No combined-state reachability, executable state-machine semantics, inferred initial/final states or invented hierarchy.

## Invocation

```sh
native=skills/game-design-workbench/native
example=skills/game-design-workbench/examples/little-plumber-guy-nes/game-design
"$native/build/state-chart-joint-cli/gd.exe" query \
  --question state-chart --projection actors \
  --nouns "nouns=$example/nouns" --axes "axes=$example/axes" \
  --interactions "interactions=$example/interactions" \
  --mechanics "mechanics=$example/mechanics" --verbs "verbs=$example/verbs" \
  --resources "resources=$example/resources" \
  --control-vars "control-vars=$example/control-vars" \
  --related "systems=$example/systems" \
  --object nouns/shellback --axis axes/ShellState --json
```

- Required: explicit nouns/axes and at least one mechanics/interactions binding; exact `--object` and applicable enum `--axis`.
- Optional: exact `--case`, `--participant` with case, repeated `--status`, repeated finite `--value`, `--work-limit`, `--with-axis ALIAS/AXIS`.
- `--with-axis` must name a distinct valid enum axis explicitly declared on the same noun. Evaluate its declared finite values jointly with primary-axis values on each selected local binding; never assign same-noun peers. Other actors/contact facts remain unknown.
- Default values: declared enum vocabulary. Open vocabulary stays open; explicit additional samples are hypotheses, not new authored states.
- No fixed assumptions, effect/result selectors or numeric state chart. Use `actor-states` for explicit scenarios/numeric samples.
- `query --list` advertises parameters, fields, bounds, view, IPC form/actions and retained-result family. GUI uses declared values/all root statuses; explicit `--value` samples and arbitrary `--status` lists remain CLI-only. Standard JSON/text, agent/piped defaults and exit 0/2 remain.

## Output contract

| Data field | Meaning |
| --- | --- |
| `axis_definition` | Exact source identity, type, unit, vocabulary and closed flag. No inferred initial or terminal state. |
| `companion_axis_definition` | Optional second exact enum definition; absent for one-axis queries. |
| `state_values` | Native definite/unknown/excluded/unfinished case and binding counts per primary value. With a companion, existential aggregation over its declared values; incomplete local-binding coverage remains unfinished. Counts are not runtime frequency. |
| `scenarios` | Independent case/local-binding/value hypothesis, with optional `companion_value` assigned to that same binding. Exact source/actor, applicability/context/requirements/exclusions, completeness, conflicts, opaque effects and cycles. |
| `scenarios[].transitions` | Selected participant state or explicit preservation proposals on either selected axis. Each transition retains exact `axis` and `from`. `proposal_index` refers to same scenario's `proposals`; `to` remains conditional on retained guards. |
| `scenarios[].proposals` | All retained native writes, including other axes/participants, numeric expressions, units, source lines and origin paths. No side-effect loss merely because chart selects one axis. |
| `scenarios[].activations` | Root/callee identities, native truth and exact invocation path/call text. Definition/placement rows are not distinct runtime events. |
| `cases` | Root and retained dependency declarations using existing Rules JSON: whole context, requirements/exclusions, unknowns, calls, side effects and resolution diagnostics. |
| `findings` | Whole loaded rule-snapshot diagnostics, not selected-object-only findings. |

`--fields` selects advertised top-level sections; selectors, coverage, limits and errors remain. Default includes all sections. Text gives transition summary; JSON retains full guard/call/side-effect detail.

No selected-axis proposal means `axis_result:"unspecified"`, not an inferred self-loop or preservation promise. False hypotheses remain explicit; unknown/unfinished hypotheses never become checked absence. Source completeness, finite-query completion and report retention are separate.

## Retention and ownership

- Native inventory: at most 8,192 local bindings; 64 finite values per selected axis. Optional companion adds a bounded Cartesian set of local hypotheses, not reachability exploration.
- Report: at most 4,096 scenarios, 8,192 proposals and 4,096 activations. Retain whole scenario slices atomically; limits preserve earlier rows.
- Shared inventory/evaluation/retention budget: `0..67108864`. Construction/compilation and fixed aggregation retain existing separate bounds. No threshold or timeout changes.
- `coverage.state_chart` discloses retained counts, selected bindings, hypotheses over retained bindings, unretained hypotheses, bounds, storage/work retention flags and owned bytes. Inventory unexamined count remains a separate conservative upper bound, not an exact missing-match count.
- Report owns 2,298,400 bytes on checked Windows x64 build, plus existing coverage storage. Process/release one native actor child at a time. Other query kinds allocate no chart.
- Strings borrow model/rules/input lifetime; copied proposals/activations never borrow released child arrays. Query destruction releases all owned results. Invalid arguments/allocation failures return structured errors; empty valid scope and limit-marked prefixes remain distinct.
- Serializer reads retained results only; no allocation, query evaluation or implicit Run.

## Viewer contract

- Start `gd-view view explorer ... --mode state-charts` with explicit nouns/axes and mechanics/interactions bindings. **Data / projection > State charts** selects the same independent pane.
- **View > Mode:** Statechart (default) or Grid. Mode change preserves pending inputs, applied result, inspection, gold state/cell filter, evidence pages and separate cameras; never Run.
- **Analysis:** choose noun, primary enum axis and optional companion; optionally exact source#case/local participant and work ceiling. **Run state hypotheses** replaces the result only after successful native construction. Invalid selectors/allocation failures retain last-good result. No implicit default noun or analytical Run.
- **Statechart:** noun composite, separate axis regions, rounded state boxes and directed conditional state proposals. Brackets show native guard-match status or conditional/unfinished annotation, not literal UML execution guards. Source case names are not proven runtime events. Full guards remain in inspection. Explicit preservation uses internal-effect annotation, not an inferred self-loop. Open-vocabulary proposal targets remain source-backed and labelled `[proposal]`.
- Diagram pages show eight retained arrows; hypothesis/proposal/activation lists page 64 retained rows. Paging never changes native caps or evaluation. Gold arrows mark conditional/unknown/unfinished evidence; green guard matches do not guarantee outcomes.
- **Grid:** primary values × companion values, or one column without a companion. Cells count matching, unknown, excluded and unfinished local-binding hypotheses. Missing inventory work, source incompleteness and retained-result omissions remain separate. A matching rule does not prove compatibility, feasibility or reachability.
- Click state/cell to filter retained evidence (gold outline); inspect hypothesis/transition for blue selection, exact source/local identity, whole root declarations, all participant effects, origin lines, mapped activations/calls and unknowns. Inspection does not change query inputs or move camera. Mode changes keep inspected hypothesis visible as blue state/cell cues.
- Wheel zooms; right/middle drag pans; F frames selected state/transition footprints or cell, otherwise readable initial region. Fixed diagram/grid placement never feeds graph solver or rule evaluation. Reset view changes cameras only; reset inputs stages defaults without replacing applied result; Escape clears inspection only; preserve gold filter, result, camera and drafts. Explicit **Clear state/cell filter** clears gold without clearing blue inspection. Reset all restores empty State charts pane and retains draft/Undo/Redo.
- Refresh checks pending inputs before replacement. Remap selected values and exact source/case/local/value hypothesis against fresh result; reject missing selections atomically. Selection history uses retained-query generation, not stale native indices across Run/refresh.
- IPC: projection `state-charts`; analytical form `chart`; actions `chart.run`, `chart.clear`; presentation `state_chart_mode: statechart | grid`; retained `result --form query`. Configure exact strings with current control token, then explicit Run. Generic graph node navigation/analysis is not this renderer's API; `nodes` discloses unavailable generic catalogue, and pane inspection exposes chart context separately.

## Observed Shellback result

Fresh full-binding query: 40 local bindings × three states = 120 hypotheses; 75 retained proposals; 123 activations. No query/storage retention limit. Native source coverage remains incomplete, and context is not supplied: examples below have `applicability:"unknown"`.

| Source case / hypothesis | Retained selected-axis proposal | Other retained effect |
| --- | --- | --- |
| `mechanics/shell-travel#kick-resting`, Shell resting | State sliding | Launch details remain authored unknown effects. |
| `mechanics/shell-travel#descending-stop`, Shell sliding | State resting | Hero Motion rising. |
| `mechanics/shell-travel#walker-impact`, Shell sliding | Explicitly preserve sliding | Walker Life dead. |

Joint `ShellState × Life`:40 bindings × three primary values × two companion values;240 hypotheses,150 proposals,246 activations. Query complete within declared finite scope; source incomplete, no retention omissions. This does not prove dead/walking or other combinations reachable.

Two-shell response remains unresolved in full declarations. Existing states are not classified as enemy, ally or safe by this report.

## Checks

Initial CLI slice (historical):

- Warning-free CJIT CLI/core/graph builds in fresh `native/build/state-chart-{cli,core,view}/` outputs. No viewer launched or existing viewer changed.
- Core:149/149. New checks cover native proposals, same-noun bindings, shared work bound, empty/exhausted scope and all four allocation stages.
- Focused Node:20/20 across state-chart, state-coverage, actor/effect, query-capacity and structural-query suites. Includes exact actor-query proposal parity; full guards, exclusions, side effects and mapped dependencies; field selection/errors/text/JSON; 4,096-scenario storage boundary with two unretained hypotheses.
- Both shipped examples exercised read-only: Shellback/ShellState and Switch/Occupancy. Source SHA-256 snapshots unchanged. Missing Crate axis is not invented; that noun has no declared axis.
- Replay audit:28 successful native probes, unchanged authored hashes. Artifacts: `skills/game-design-workbench/native/build/state-chart-audit-1794805850/`; new `shellback-state-chart.json`. Manifest retains exact arguments and source/executable/script hashes.

Statechart/Grid follow-up:

- Follow-up builds: `native/build/state-chart-joint-{cli,core}/`, `native/build/state-chart-gui-tests/`; core149/149. Focused chart/coverage/query/capacity/structural Node23/23 includes two-axis native actor parity, same-noun peer isolation, wrong/undeclared axes, zero/partial work and unchanged 4,096-scenario cap with four omitted joint hypotheses.
- Dedicated source-read-only application checks45/45 on each shipped example exercise native joint cells, mode switching without allocation/Run, pending/applied separation, four allocation rollback stages, selection history, shared Focus/F without allocation/evaluation, partial-row Grid filter/framing, camera/input/selection resets, exact refresh remapping and Reset all. CPU checks do not establish rendered/pointer acceptance.
- Ordinary IPC regressions:Crate377/Mario393 checks; each3330 control checks,25 feedback checks,36 sidebar checks,123 configuration checks. Extended presentation/deep/research checks not run. Authorized dedicated viewer verified chart configure/Run/result, Statechart/Grid mode round trip with identical retained bytes, and stale-control rejection. Existing result tokens include presentation scope: reacquire after mode changes; token change does not mean query reran.
- Latest replay:29 read-only probes in `native/build/state-chart-joint-audit-1794817854/`; both example hashes unchanged. Joint report `shellback-joint-state-chart.json` supplements one-axis regression.
- First visible Statechart/Grid captures: `native/build/manual-review-captures/review-25316-a7e160483b8948c7a7194865852a5069.png` and `review-25316-056321f53d4d4dfea805ab73c8f0c8c1.png`. Found diagram caption spill, weak arrowheads and clipped form labels; fixed caption bounds, open arrowheads, above-input labels and collapsed scope/legend. Gold filter and blue inspection now use independent borders, including partial-axis filters across Grid cells.
- Updated warning-free production viewer: `native/build/state-chart-gui-review-final/gd-view.exe`. Caption containment, arrowheads and label fixes observed in earlier authorized viewer. Escape now preserves gold filter and clears blue inspection only; both45-check suites pass. Final visible viewer showed white client area, confirmed by user; visual/pointer acceptance blocked.
- Minimized/covered window interrupted first capture/live-mode attempt. Timed-out mutation was not retried automatically; resumed only after user restored window and approved continuation. Capture tool now has explicit `-BringToFront`, default off; never restores minimized windows or sends pointer/keyboard input.
- Alternate compiler/platform matrix remains final Step 6. No manual rendered-chart acceptance claimed. User authorized local checkpoint of implementation and review evidence; no push.

## White-window diagnosis

- No chart/render code changed to guess at cause. Raylib and UI DLL hashes match between earlier-rendered and final builds; imports use same DLL names. IPC remained responsive, retained240 hypotheses, and reported0 draft edits while final desktop window was white. User confirmed actual white window, not just bad capture.
- Authorized startup comparison:two sequential60-frame windows, normal exit0 each. Native framebuffer PNGs both show correct empty State charts UI. Artifacts: `native/build/state-chart-render-comparison-1794859600/`.
- Authorized applied-query comparison:600 frames per build, same30-second stall guard and unchanged native work limits. Baseline captured correct Grid framebuffer with240 hypotheses,150 proposals,246 activations; desktop focus request failed before second mode, so baseline comparison script reported failure. Process subsequently finished bounded capture; no force-kill.
- Remaining candidate run captured correct Statechart framebuffer, preserved query bytes across both modes, exited0 and retained unchanged example SHA-256. Desktop captures failed because Windows refused foreground focus. Overall command exits2; this is **not** desktop-render or pointer acceptance.
- Applied artifacts: `native/build/state-chart-stateful-comparison-1794860678/baseline-native.png`, `native/build/state-chart-stateful-candidate-1794860859/candidate-native.png`. Startup/native framebuffer success does not explain earlier white desktop window; cause unresolved.
- Reusable diagnosis: `scripts/compare-workbench-state-chart-rendering.mjs --help`. Explicit executable(s), client, new output and `--allow-windows` required. One or two600-frame windows; normal exit only,30-second guard leaves stalled viewer running. Capture failures remain explicit while native frame capture can complete. No pointer/keyboard automation, automatic mutation retries, source writes or threshold changes.
- Tool checks:syntax/help, missing authorization/input failure, candidate-only live run. Foreground-policy failures recorded rather than suppressed. Human interaction and continuous-window presentation remain unverified.
- User directed moving to next work. Further white-window diagnosis and visual/pointer acceptance are deferred, not resolved. No additional viewer launch authorized by that direction.
