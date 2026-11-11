# Shared investigation controls

Status: joint interactive Mario IPC tour completed. Analytical forms, presentation controls, retained result readers and typed evidence selection implemented and CJIT-verified. Matrix selection continuity and readable Focus selection accepted in reviewed Mario presentations. Longer/fading CLI toast accepted in separate Mario review. Step 1, full live safety/usefulness acceptance and final every-view walkthrough remain open. CPU checks do not establish unperformed visual/pointer acceptance. Separate review viewers opened with authorization; none replaced.

## Shared application boundary

`gd_explorer_controls.{h,c}` owns four analytical forms and dispatches application actions. `gd_explorer_presentation.h` adds immediate presentation metadata and transitions.

- Query: 16 fields. Actor inventory, finite variants/coverage and Effect contributors.
- Structure: 15 fields. Canonical structural and similarity questions across eight views.
- Rules: 10 fields. Conditional outcomes/consistency and object bindings.
- Resource: four fields. FROM, TO, accepted-only and work ceiling.
- Presentation: 54 fields. View-specific layout, sort/direction, pause/filter, distance focus, graph layers/peers, relationship/MDA roots, roles/witness filters, Resource/Challenge witnesses, similarity entry/peer/canvas/page, hypothetical knowledge, shown support-path bypass nomination, result pages and binding-axis page.

Descriptions define names, types, capacities, choices and editability. Discovery returns stable keys, pending/applied values, bounds and source-slot pages. Native indices remain snapshot-local output, never write selectors. GUI and IPC share domain builders, native selectors and application transitions; no parallel evaluator.

Analytical configuration stages one field plus dependent resets atomically. Only explicit Run/Display consumes pending analysis/display inputs. Presentation configuration applies immediately; discovery reports apply policy. Neither path changes authoritative sources, draft/Undo, active pane or OS focus.

Presentation layout choices are additive: Skills `depth | rank | free`, Challenges `bands | free`, Relationships `native | free`. Native defaults remain. Skills remembers depth/rank basis across Free; Relationships preserves root/revealed scope without implicit expansion. Existing Matrix/MDA keys unchanged.

Actors presentation `actor_overview: interactions | usage` selects retained overview without applying staged Query inputs. Default interactions uses weighted Free graph and reports `layout: free`; layout control stays disabled because map is always Free. Usage supports `layout: native | free`: Native fixes actors on outer ring and concepts centrally; Free lets all actors/concepts move. Free usage enables shared Initial arrangement, Pause/Space, workspace bounds and render-only filter. Switching geometry never applies pending Query inputs and Native restores original cloud pose. Overview selection/source browsing and last-good failure behavior stay intact. Explicit case/state queries remain available for every noun classification. `result --projection actors --form actor-overview` returns retained nodes, counts and disclosures through byte paging; nonempty `--fields` is unsupported. [Extraction semantics and review boundary](../../../reports/graph-query-research/shared-force-layout-query.md).

**Noun interactions** uses projection `noun-interactions`, available from nouns + interactions without verbs. Existing `matrix` key remains **Verb usage**. Noun interactions presentation `layout: grid | free` switches table/default and all-noun Free graph atomically. Graph uses canonical nouns and retained undirected pairs, including self-loops; count labels/color/width use same case metric. `paused` is editable only in Free graph. Row/column and case selection survive switching; `links` evidence exposes native pair counts and selects same `noun-cases` evidence. Initial CLI `--mode noun-interactions --layout graph` starts Free; `columns` is unsupported for this projection. Presentation `noun_row` and `noun_column` accept exact canonical noun references; `noun_case_page` pages 64 native case slots for selected nonblank cell. `result --projection noun-interactions --form noun-grid` returns retained sparse case counts, noun classifications and scope/limit disclosures through byte paging; nonempty `--fields` is unsupported. `evidence --projection noun-interactions --form noun-cases` pages native case slots; only counted cases produce entries. Evidence includes original case identity, record, context, participant conditions and retained case selectors. Row/column selections, source inspection, fit/framing, reset and refresh preserve independent pane state; no analytical Run. [Count semantics and verification](../../../reports/graph-query-research/noun-interaction-grid.md).

Presentation `initial_arrangement: native | neighborhood` applies immediately to prepared Skills/Challenges/Relationships/Interactions/MDA/Resources/Actors/Effects Free graphs; Native is default. Unavailable in native bands/stable Relationships, columns/grid and similarity canvases; does not switch layout mode. Repositions nodes and resumes settling while preserving selection, scope and matching retained analysis. Main-canvas aspect guides compact graph-derived neighborhoods; unknown viewport uses 1:1. No selection/zoom/resize reseed. Arrangement changes and `view.rearrange` reject changes to retained keys/edges/groups/bands rather than hiding source refresh; failed candidate retains prior state. [Contract, limits and review boundary](../../../reports/graph-query-research/shared-force-layout-seeding.md).

Prepared Free graphs add opt-in presentation `workspace_bounds: true | false` (**Workspace bounds (trial)**), default false. Fixed area/spring-density/aspect-sized rectangle also fits prepared pose. Toggle changes no node geometry, pause, selection or query inputs; requests Fit. Post-contact containment exempts held nodes; release returns them inside during active settling. Compounds stay together. `F` fits workspace; selection, pan, zoom and viewport resize do not resize it. Re-enable/reseed or replacement geometry recomputes bounds. Unsupported in Native constrained layouts, Grid/columns, fixed Usage cloud and similarity canvases. Keep preference while inactive. Bounds can crowd edges and increase motion; no convergence/default-policy claim. [Trial contract and measurements](../../../reports/graph-query-research/shared-force-layout-workspace.md).

Explorer Free graphs add presentation `visual_filter: true | false` (**Visual position filter (One Euro)**), default true. Rendering/picking use separate filtered pose; raw solver positions/velocities and IPC node positions remain unchanged. Links, labels and selection overlays follow displayed nodes; compound stacks share translation, held nodes bypass lag. Raw-anchor text/badge measurement avoids feedback through collision footprints. Pause stops solver while filter catches up. Unsupported canvases keep existing geometry. Workspace axes now include 20% extra room after review. [Filter contract, measurements and review boundary](../../../reports/graph-query-research/shared-force-layout-visual-filter.md).

Actors/Effects add presentation `layout: native | free`; Native remains default. Geometry changes retain applied query, pending inputs, exact typed selection and contributor context without Run/Display. Native restores scene's built pose, including wrapped Effects rows. Free uses shared solver; stacked variant cards move as one spatial unit, with separate exposed-strip selection. Other ghost-styled cards remain independent. Pause/Space stops settling, not dragging. Explicit Run/Display/retarget prepares selected geometry before scene replacement; allocation failure retains last-good display. No query/ghost analytical layers or similarity-distance replacement added. [Implementation and review gate](../../../reports/graph-query-research/shared-force-layout-query.md).

### Actions

- `query.run`, `query.display`.
- `structure.run`, `structure.clear`.
- `rules.run`, `rules.clear`.
- `resource.run`, `resource.clear`.
- `view.fit`, `view.rearrange`, `view.expand`.
- `view.start.add`, `view.start.clear`, `view.analyze`, `view.compare`.
- `view.reset`, `view.find.clear`, `view.witness.fit`, `view.cell.fit`, `view.selection.focus`.

`view.cell.fit` / **Frame selected pair** frames retained endpoints in Verb usage and Noun interactions Grid/columns/Free. Blank cells keep endpoints without inferred links; inactive links remain hidden unless existing overlay is enabled. Layout changes frame retained selection; Grid return centres selected cell/header at readable zoom. Case/source inspection stays retained. `view.fit` / `F` still fits whole canvas. Ordinary projection switches keep independent selections; explicit **Find selected in…** follows source association without replacing destination selection or filters. [Selection contract and checks](../../../reports/graph-query-research/selection-continuity.md).

`view.selection.focus` / **Focus selection** is camera-only across unified views/modes. Include visible direct neighbors when readable; floor1.0 zoom, fallback to selected items or primary endpoint for oversized context. Hidden/unrepresented selections leave camera unchanged. Grid focuses cell/header; similarity focuses placed entry/selected peer, with no topology changes. No selection disables action. Existing busy/token guards apply; no Run, filter change, node move or sampler reset. Action returns `framing` (`status`, `selected_items`, `hidden_items`, `context_nodes`, `clipped`, `hidden_link`, `zoom`); `inspect --fields panes` includes last `focus_selection` report. `focus_changed:false` still means no projection/OS-focus switch, not no camera change. [Framing policy and checks](../../../reports/graph-query-research/focus-selection.md).

```sh
gd viewer action --target SESSION --projection mda --action view.selection.focus --expect PRESENTATION_TOKEN --json
```

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

## Sidebar sections

Unified views share ordered collapsible sections: View & layout; Find selected in another view; Display & filters; Analysis; Browse shown nodes; Selection & evidence; Sources & changes; Diagnostics. Unsupported sections are omitted. Native domain subfolds remain inside relevant sections.

- Matching sections share open/closed state across views and restarts. Reset/reload does not clear fold preferences. Pane selections, cameras, filters, results and pending inputs stay independent.
- Projection switch, stale/errors, hidden-selection notices and unsaved-draft status stay visible. Folding does not Run/clear analysis, change CLI availability or stop owner-loop preparation/analysis.
- Defaults open: View & layout, Display & filters, Analysis, Selection & evidence. Other common sections start closed.
- Only eight common flags saved in per-user `game-design-workbench/sidebar-folds.ini`: `%LOCALAPPDATA%` on Windows, `~/Library/Application Support` on macOS, `$XDG_CONFIG_HOME` or `~/.config` on Linux. Path appears under Sources & changes. General ImGui INI persistence remains disabled; no analytical or record-specific inspection state saved.
- Settings read at launch; actual toggles save atomically. Missing file uses defaults; read/save failure reports warning and preserves session state. Running viewers do not live-sync; last save wins. No project/source settings writes.

[Contract and review boundary](../../../reports/graph-query-research/sidebar-sections.md).

## Feedback

- Independent human-owned hint window, default 360 × 160. No appearance focus. Go uses guarded navigation; Dismiss/close clears hint only. Stale exact hint disables Go.
- Passive latest-wins CLI toast,6 seconds total:200 ms fade-in,300 ms fade-out, no sliding. New activity refreshes duration from current opacity without entrance flicker. Background/text fade together; no input capture or appearance/OS focus. Reads, failures and human Go do not produce toast.
- Toast update: warning-free CJIT CLI/explorer/graph builds; focused integration6/6, both examples/source hashes unchanged. CPU feedback checks25 per example cover fade endpoints, refreshed deadlines/opacity, draw-vertex alpha, style restoration, expiry, passive flags and retained human focus/hint ownership. User accepted separate Mario animation review; no every-view acceptance inferred. Evidence: `native/build/toast-feedback-work/`. [Contract and limits](../../../reports/graph-query-research/cli-toast-feedback.md).

## Verification and remaining work

CJIT only. CLI, transport, core, graph and explorer helper builds pass. Native core: 128/128. Protocol: 90 cases plus three writer and 115 capture checks. Each example: 254 IPC, 2178 shared-control, 105 configuration-loop, 185 presentation/result and 10 feedback checks. Targeted Node: 21 passed, zero failed/skipped. Source hashes unchanged; owned helper allocations released. No native window/renderer launched. Evidence: ignored `native/build/control-work/{adapter-crate.json,adapter-plumber.json,protocol.json,core.log,node.log,build-*.log}`.

Current coverage includes analytical discovery/Run/Display/Clear, Resource last-good staging, cold-pane refresh, inactive ownership, presentation across nine views, byte pages/stale field filters, Matrix cells/cases/layout preservation, MDA link mapping, knowledge/start/diagnostic token invalidation, Rule row marking and similarity source-selection precedence. Capture checks cover binary pages, EOF, empty/failed callbacks, 64 MiB ceiling and warmed handle stability.

Completed review hardening: full known/start/focus/diagnostic/contributor token scope; shared-cache ownership across inactive analysis and active similarity; MDA slot/key/source mapping; recursive retained similarity source; ordinary selection precedence; typed Matrix selection preservation; invalid outer-cache nulls; native return contracts; diagnostic omission/capture metadata; fail-fast expensive serializer loops. Independent source review found no remaining blocker within these classes. No latency guarantee inferred.

Regression checks reassemble full JSON from 1 KiB pages and parse it in Node. Finite-Actor similarity source regression verifies no all-pair comparison section is evaluated/exported. Private diamond graph verifies exact bypass nomination through production dispatch.

Review boundaries:

- Full similarity feature-member data is readable through native `inputs`; exact linked source is selectable. Per-feature GUI member-page buttons are not individually mirrored as IPC controls. No human transcription needed to inspect those members.
- Bypass nomination follows current prepared shown-path cache, like native panel. Scope changes may require explicit `view.analyze` and fresh choice discovery.
- Full application-control parity remains a final inventory/review claim, not inferred from `presentation_controls:true`.
- Joint interactive usefulness/safety review and every-view/mode/option walkthrough remain open. CPU checks are not GUI or usability acceptance. Alternate compiler matrix remains Step 6 work.
