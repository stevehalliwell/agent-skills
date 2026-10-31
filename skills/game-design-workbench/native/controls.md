# Shared investigation controls — implementation inventory

Status: shared-form/action refactor, named action IPC, guarded source refresh and UI feedback implemented. Typed parameter configuration and full control/result coverage remain missing. Shared-investigation walkthrough is not ready.

## Implemented shared boundary

`gd_explorer_controls.{h,c}` owns 41 field descriptions across three form families:

- Query: Actor inventory/finite variants/coverage and Effect contributor display. Sixteen stored fields.
- Structure: canonical structural questions and similarity across eight views. Fifteen stored fields.
- Rules: Resource conditional outcomes/consistency and object bindings. Ten stored fields.

Each description defines field name, storage type/offset/capacity, label and primitive widget choices/height where applicable. GUI primitive editors, logical pending/applied equality and IPC form inspection use these descriptions. Native indices remain internal, snapshot-local values—not write selectors for a future IPC API.

Shared semantic transitions handle Actor question/root/participant/axis changes, structural question changes and rule inspection mode changes. Preserve dependent-input clearing from GUI behavior. Reject invalid native selector indices without partial transition. Empty query literal lists ignore unused parser-mode flags during logical comparison, matching prior behavior.

Six named application actions share availability and dispatch:

- `query.run`: replace Actor/Effect result through existing native builder.
- `query.display`: rebuild drawing from applied query, not pending analytical inputs.
- `structure.run`: run canonical structural/similarity question.
- `structure.clear`: allocate default inputs before releasing old result.
- `rules.run`: run conditional reader/binding inventory.
- `rules.clear`: clear rule/binding inspection only.

GUI buttons use the same action dispatch. Action code calls existing domain builders; no parallel evaluator. Domains retain result ownership, diagnostics and last-good failure behavior. Pending form creation is explicit and lazy. Reads do not allocate forms.

IPC inspection now reports applied rule form for successful object-binding queries too; previously only conditional results exposed applied inputs.

## IPC commands implemented

```sh
gd viewer actions --target SESSION --projection actors
gd viewer action --target SESSION --projection actors --action query.run
gd viewer action --target SESSION --projection actors --action query.display
gd viewer refresh --target SESSION
```

- `actions` reads shared action descriptions/availability for requested or active pane. No preparation, input creation or evaluation.
- `action` requires projection and exact action name. Execute one of six shared operations against that pane; do not activate an inactive pane or steal OS focus. Run consumes current pending analytical inputs explicitly. Success means action executed, not proof of complete coverage. No parameter-writing command yet.
- `refresh` reloads original collection bindings through same native replacement pipeline as GUI refresh. No project rebinding or disk writes. Block active text/pointer/navigation input, pending rank text, modified draft and retained Undo history—even when net draft diff is empty. Pending analysis/remap conflicts reject without losing last-good state. Successful reload increments generation and invalidates old exact-ID hints.
- All mutations retain explicit session targeting, bounded strict JSON and unknown-outcome/no-auto-retry rules. Reads remain non-consuming. Request allows no source-draft edit, Undo, discard or force-overwrite option.

## Remaining control inventory and IPC work

Existing shared native analysis panels already cover several views. Reuse them; do not rebuild their semantics in transport code.

- Shell/display (`gd_explorer_view.c`): projection selection, view-specific layout choices, sort/direction, settling, Fit/Rearrange, distance focus/auto-fit, Find, Browse and Reset. Refresh now shares guarded native operation. Draft/report actions remain human-only. Layout validity and other guarded transitions need shared application operations, not raw struct writes.
- Skills/Challenges/Relationships (`gd_graph_view.c`, `gd_graph_analysis_ui.c`, `gd_graph_reachability_ui.c`, `gd_graph_analysis_ui.h`): human-only rank/prerequisite draft editing; remaining IPC scope includes relationship root/expansion, known-skill assumptions; shared neighbors/lens/peer, paths/starts/witnesses/bypass, foundations, reachability, shapes and overview controls. Preserve typed identity and AND semantics.
- Matrix (`gd_matrix_grid_view.h`): graph/columns/grid, noun/verb headers, cells and case selection. Cell/edge inspection must not be approximated as node selection.
- MDA (`gd_mda_view.c`, unified shell): root, traversal direction, role/query/witness filters, witness selection and typed source inspection. Association remains distinct from causal flow.
- Resources (`gd_resource_view.c`): incidence role filters, route endpoints, accepted scope, work limit, witness selection and witness-only display. Retain native identities and incidence versus directed-witness distinction.
- Actor/Effect form pickers (`gd_explorer_query_view.c`, `gd_explorer_loaded_pickers.h`): exact source cases, local participants, attached typed axes, finite samples and assumptions, primary/comparison variants and exact contributor results. Extract selector discovery/resolution for GUI and IPC without exposing raw indices.
- Structural/rule pickers (`gd_explorer_structure_pickers.h`, `gd_explorer_rules_view.c`): source/native selectors, starts, relation/status scope, feature weights, comparison entries, binding filters and fixed assumptions. Existing shared form descriptions cover storage, not all semantic choices/constraints.
- Similarity (`gd_explorer_similarity_view.c`): ordinary/similarity canvas toggle, first/peer entry, feature/member comparisons, component tables, pagination and camera Fit. Preserve missing/unplaced state and independent ordinary camera.
- Evidence/diagnostics: complete result/inspection data needed to explain displayed analysis, selected edges/cells/evidence, paging and source actions. Existing IPC exports forms/selection, not full result tables.

Next IPC layer must discover control types/valid choices/current and pending/applied state; resolve exact identities; validate requested changes; execute shared actions; return visible state and explicit failures. Agent must configure and run investigations without human field transcription. Preserve active human input, pending edits, independent panes, draft/Undo and last-good results. No authoritative source writes or OS focus/input automation.

## Source-change boundary

User chose disk-first source edits, followed by explicit GUI refresh. Do not expose skill rank/prerequisite draft mutations or Undo through IPC. Agent uses normal authorized file-editing/CLI workflow for authored changes; application-control IPC changes analytical/display inputs and requests reload. Do not speculate inside user's draft buffers.

Refresh means request reload now, not silently discard active edits, dirty drafts or retained Undo history. Existing guards report conflicts; failed reload retains last-good snapshots/results. IPC does not bypass source-edit authorization or implicitly finish/export user draft.

## UI feedback — implemented after shared-form refactor

- Hint uses separate movable/resizable ImGui window, default 360 × 160 at canvas bottom-left. No inspector child/reserved space. Appear only when hint exists; no focus on appearance. Go uses existing guarded navigation; Dismiss/close clears displayed and queued hint only. Disable Go for stale exact-ID hint.
- Passive 2.5-second toast explains successful CLI navigation, analysis action or source reload. No input capture, navigation focus or focus on appearance. Latest notice replaces prior notice. No notice for reads, queued hints, failures or human Go. No-op view-only navigation does not show notice.
- CPU ImGui frames verify independent window, bounds, focus retention, passive flags, expiry and disappearance. Live placement/readability remains for final review.

## Checks

CJIT-only graph/explorer compile and CPU-only production helper checks; no windows launched or existing explorer replaced. Native core: 128/128. Protocol: 59 cases plus 3 writer checks. Both examples: 248 IPC checks, 556 shared-control checks and 10 CPU ImGui feedback checks each. Targeted Node: 20 passed, 0 skipped. Example source hashes unchanged; zero owned native helper allocations remain.

Checks cover all field offsets/capacities/types/names, every field's logical equality behavior, empty/nonempty literal modes, valid/invalid selector transitions, question/mode resets, lazy/read-only form access, every named action, action availability, allocation failures and last-good retention, applied binding-form inspection and pending-versus-applied query failure.

Checks also cover strict named-action/refresh envelopes, requested versus active pane ownership, successful/failing query actions, source reload allocation failure, pending-analysis conflict, successful atomic reload, dirty/Undo guards, hint invalidation and UI frames without native window or renderer.

Evidence: ignored `native/build/control-work/{adapter-crate.json,adapter-plumber.json,protocol.json,core.log,node.log,build-cli.log,build-transport.log,build-graph.log,build-explorer.log}`. All tested binaries built in this directory with CJIT. Live rendering and complete control/result coverage remain unverified/unimplemented; no walkthrough readiness or usability acceptance claimed.
