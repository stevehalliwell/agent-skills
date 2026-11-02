# Shared investigation controls

Status: joint interactive Mario IPC tour completed; user requested checkpoint and three UX follow-ups in root `todo.md` Step 2. Analytical forms, presentation controls, retained result readers and typed evidence selection implemented and CJIT-verified. Tour demonstrated shared selection/hints, configure/Run/evidence loops and retained independent pane context. User reports Grid-to-Free-graph selection not visibly following, toast too brief and missing consistent readable selection framing. These remain open; CPU checks do not override observations. Step 1, full live safety/usefulness acceptance and final every-view walkthrough remain open. Separate review viewer launched at user request; no existing explorer replaced.

## Shared application boundary

`gd_explorer_controls.{h,c}` owns four analytical forms and dispatches application actions. `gd_explorer_presentation.h` adds immediate presentation metadata and transitions.

- Query: 16 fields. Actor inventory, finite variants/coverage and Effect contributors.
- Structure: 15 fields. Canonical structural and similarity questions across eight views.
- Rules: 10 fields. Conditional outcomes/consistency and object bindings.
- Resource: four fields. FROM, TO, accepted-only and work ceiling.
- Presentation: 49 fields. View-specific layout, sort/direction, pause/filter, distance focus, graph layers/peers, relationship/MDA roots, roles/witness filters, Resource/Challenge witnesses, similarity entry/peer/canvas/page, hypothetical knowledge, shown support-path bypass nomination, result pages and binding-axis page.

Descriptions define names, types, capacities, choices and editability. Discovery returns stable keys, pending/applied values, bounds and source-slot pages. Native indices remain snapshot-local output, never write selectors. GUI and IPC share domain builders, native selectors and application transitions; no parallel evaluator.

Analytical configuration stages one field plus dependent resets atomically. Only explicit Run/Display consumes pending analysis/display inputs. Presentation configuration applies immediately; discovery reports apply policy. Neither path changes authoritative sources, draft/Undo, active pane or OS focus.

Presentation layout choices are additive: Skills `depth | rank | free`, Challenges `bands | free`, Relationships `native | free`. Native defaults remain. Skills remembers depth/rank basis across Free; Relationships preserves root/revealed scope without implicit expansion. Existing Matrix/MDA keys unchanged.

Presentation `initial_arrangement: native | neighborhood` applies immediately to prepared Skills/Challenges/Relationships/Interactions/MDA/Resources Free graphs; Native is default. Unavailable in native bands/stable Relationships, columns/grid and similarity canvases; does not switch layout mode. Repositions nodes and resumes settling while preserving selection, scope and matching retained analysis. Main-canvas aspect guides compact graph-derived neighborhoods; unknown viewport uses 1:1. No selection/zoom/resize reseed. Arrangement changes and `view.rearrange` reject changes to retained keys/edges/groups/bands rather than hiding source refresh; failed candidate retains prior state. [Contract, limits and review boundary](../../../reports/graph-query-research/shared-force-layout-seeding.md).

### Actions

- `query.run`, `query.display`.
- `structure.run`, `structure.clear`.
- `rules.run`, `rules.clear`.
- `resource.run`, `resource.clear`.
- `view.fit`, `view.rearrange`, `view.expand`.
- `view.start.add`, `view.start.clear`, `view.analyze`, `view.compare`.
- `view.reset`, `view.find.clear`, `view.witness.fit`, `view.cell.fit`.

Discover current availability with `viewer actions`. `view.analyze` explicitly prepares shown-canvas layer/knowledge evidence; Query, Grid and similarity canvases do not masquerade as ordinary graph-layer caches. `view.compare` explicitly computes retained Actor/similarity pair evidence. Reads never run either action. Reset rejects dirty drafts or retained Undo and builds replacement before releasing previous pane state.

## IPC use

```sh
gd viewer controls --target SESSION --projection actors --form query
gd viewer controls --target SESSION --projection actors --form query --control root_case --offset 0 --limit 16
gd viewer configure --target SESSION --projection actors --form query --control root_case --value 'SOURCE#CASE' --expect TOKEN
gd viewer action --target SESSION --projection actors --action query.run --expect UPDATED_TOKEN
gd viewer controls --target SESSION --projection matrix --form presentation
gd viewer configure --target SESSION --projection matrix --form presentation --control layout --value grid --expect TOKEN
gd viewer evidence --target SESSION --projection matrix --form cells --limit 16
gd viewer select --target SESSION --projection matrix --form cell --reference 'nouns/Hero' --id 'verbs/Jump' --expect SELECTION_TOKEN
gd viewer result --target SESSION --projection actors --form query --limit 64
gd viewer refresh --target SESSION
```

- `controls`/`actions` do not prepare panes, allocate forms or evaluate analysis.
- `configure` requires latest form `control_token`. Stale token, active human input, pending rank text, disabled field, bad scalar or allocation failure rejects without staging. Token is stale-state guard, not authorization.
- Scalars use exact source/case keys, scoped participant names, attached axes, applied variant values and canonical native IDs. Literal compound selectors retain native syntax and Run validation. Empty values clear only where native semantics allow. Escaped LF/CR/TAB are allowed only in `value`.
- Canonical suggestions use existing matching picker/result scope. `canonical_choices_ready:false` is not absence proof. Explicitly run advertised `choice_discovery_question` when needed; no read creates that context.
- Source-slot pages may be empty/filtered. Follow `next_offset`; pages are not frozen transactions.
- Request cap: 8192 encoded bytes. Reply cap: 1 MiB. Scalar identity cap: 4095 UTF-8 bytes. Choice offset: 0..8192; page: 1..64. Capacity failure never returns successful truncated output.
- Mutations retain explicit targeting, busy/rank guards and unknown-outcome/no-auto-retry rules. No source-draft edit, Undo, discard or force-overwrite endpoint.

## Retained evidence and results

`viewer evidence` lists page families. Supported pages: `links`, `cells`, `cases`, `diagnostics`, `starts`, `known`, `rule-rows`, `rule-checks`. Diagnostic reference filtering uses exact loaded reference. Link/cell/rule row handles require returned `selection_token`; native row offsets are not durable IDs.

`viewer select` supports exact `source`, `link`, Matrix `cell`/`case`, `diagnostic`, `rule` and `rule.check`. Matrix cells use noun+verb references or returned cell key, including blank grid coordinates. Rule transitions reuse native row/proposal marking and axis-page reset. Source selection returns from similarity to ordinary inspection. MDA link selection maps layout slot through native link key to declaring model edge.

`viewer result --projection VIEW --form FAMILY` serializes retained native JSON:

- `query`: bindings/axes, Actor finite variants/coverage, Effect concepts and currently selected target-labelled contributors.
- `structure`: canonical question, declaration gaps or similarity result, including opt-in native inputs/source sections.
- `rules`, `bindings`: conditional outcomes/consistency or object binding inventory.
- `resource`: retained route coverage, witnesses, guards, side declarations and native graph evidence.
- `mda`, `challenges`, `skills`: native association/witness, route/demand and hypothetical AND-knowledge results.
- `layers`: explicitly prepared shown-canvas nodes/links, shared memberships, starts/support/bypass, foundations, reachability parent arcs, shape signatures and overview memberships.
- `comparison`: explicitly computed Actor/similarity pair evidence.

Byte-page envelope: `encoding:"base64"`, `offset`, `bytes`, `total_bytes`, `data`, `next_offset`, `result_token`. `--offset` counts decoded bytes. `--limit` is 1..64 KiB, default 64 KiB. Concatenate decoded pages before UTF-8/JSON parsing; a page may split a UTF-8 sequence or JSON token. Send returned token with subsequent pages. Token binds source/session, family/field filter, applied forms, selection, known set, starts, focused scope and retained rejected diagnostics. Restart after stale error.

Capture uses bounded anonymous pipe and byte-only reader thread. Native serialization stays synchronous on owner thread. No disk files, stdout redirection, model reads on reader thread or hidden Run/comparison. Page storage cap: 65536 bytes. Total serialized ceiling: 64 MiB; overflow fails, never truncates successfully. Each page reserializes retained result to count exact total; page capacity is not a latency guarantee. Native producer omissions, work/storage limits and suffix truncation remain authoritative. Invalid outer layer caches serialize null, not stale nested evidence.

Canonical structural scope and shown-canvas layer scope differ. Neither proves gameplay importance, causation, learning, simultaneous acquisition, execution order or final outcomes. Source-body inspection reads loaded Markdown, not unsaved text or fresh disk.

## Resource pending/applied contract

Stage all four Resource inputs while retaining last-good witnesses, applied endpoint/status/work labels, selected witness, highlights and display filters. Invalid input or failed Run/allocation preserves previous result and pending inputs. Only successful Run replaces witnesses. Explicit Clear resets Resource query/witnesses and defaults, retaining witness-only display setting and other forms. Snapshot exposes `resource_query.pending`, `applied`, `pending_differs`; applied is null before successful Run. Flat compatibility fields still describe pending inputs.

## Source refresh

Source changes remain disk-first authorized edits, followed by explicit guarded refresh. No IPC draft mutation or implicit export/discard.

Refresh blocks active input, pending rank text, modified draft, retained Undo history, pending analytical inputs and remap conflicts. Zero-net-diff Undo still blocks. Cold configured forms preflight before replacement allocation. Cold applied analysis rebuilds even without base canvas. First Actor/Effect canvas uses default applied scope while retaining staged inputs. Failed replacement preserves previous project/results; success advances generation and invalidates exact-ID hints.

## Feedback

- Independent human-owned hint window, default 360 × 160. No appearance focus. Go uses guarded navigation; Dismiss/close clears hint only. Stale exact hint disables Go.
- Passive latest-wins 2.5-second CLI toast. No input capture or OS focus. Reads, failures and human Go do not produce toast.

## Verification and remaining work

CJIT only. CLI, transport, core, graph and explorer helper builds pass. Native core: 128/128. Protocol: 90 cases plus three writer and 115 capture checks. Each example: 254 IPC, 2178 shared-control, 105 configuration-loop, 185 presentation/result and 10 feedback checks. Targeted Node: 21 passed, zero failed/skipped. Source hashes unchanged; owned helper allocations released. No native window/renderer launched. Evidence: ignored `native/build/control-work/{adapter-crate.json,adapter-plumber.json,protocol.json,core.log,node.log,build-*.log}`.

Current coverage includes analytical discovery/Run/Display/Clear, Resource last-good staging, cold-pane refresh, inactive ownership, presentation across eight views, byte pages/stale field filters, Matrix cells/cases/layout preservation, MDA link mapping, knowledge/start/diagnostic token invalidation, Rule row marking and similarity source-selection precedence. Capture checks cover binary pages, EOF, empty/failed callbacks, 64 MiB ceiling and warmed handle stability.

Completed review hardening: full known/start/focus/diagnostic/contributor token scope; shared-cache ownership across inactive analysis and active similarity; MDA slot/key/source mapping; recursive retained similarity source; ordinary selection precedence; typed Matrix selection preservation; invalid outer-cache nulls; native return contracts; diagnostic omission/capture metadata; fail-fast expensive serializer loops. Independent source review found no remaining blocker within these classes. No latency guarantee inferred.

Regression checks reassemble full JSON from 1 KiB pages and parse it in Node. Finite-Actor similarity source regression verifies no all-pair comparison section is evaluated/exported. Private diamond graph verifies exact bypass nomination through production dispatch.

Review boundaries:

- Full similarity feature-member data is readable through native `inputs`; exact linked source is selectable. Per-feature GUI member-page buttons are not individually mirrored as IPC controls. No human transcription needed to inspect those members.
- Bypass nomination follows current prepared shown-path cache, like native panel. Scope changes may require explicit `view.analyze` and fresh choice discovery.
- Full application-control parity remains a final inventory/review claim, not inferred from `presentation_controls:true`.
- Joint interactive usefulness/safety review and every-view/mode/option walkthrough remain open. CPU checks are not GUI or usability acceptance. Alternate compiler matrix remains Step 6 work.
