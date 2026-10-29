# Native game-design tools

C99 domain tools over supplied Markdown folders. Source analysis stays read-only; skill graph supports transient rank/prerequisite edits and JSON handoff, never authoritative Markdown writes. Same `GD_Model` feeds matrix/connectivity and skill prerequisite/rank CLI results and views, plus challenge/route opportunity analysis shared with switchable Skills/Challenges exploration, focused exploration of already-consumed relationships, and MDA CLI/explorer queries. Shared graphics-free C99 layout handles transient positioning/sorting; cimgui supplies graph controls and source panel. No Node, agent, display, raylib, or `.attendant/` needed for headless commands.

Sections: [Build](#build), [Unified explorer](#unified-explorer), [Resource declaration report](#resource-declaration-report), [Commands](#commands), [Matrix meanings](#matrix-meanings), [View](#view), [Dynamic graph](#dynamic-graph), [Focused relationships](#focused-relationships), [Graph analysis layers](#graph-analysis-layers), [Diagnostics browser](#diagnostics-browser), [Skill prerequisites and ranks](#skill-prerequisites-and-ranks), [Challenge route opportunities](#challenge-route-opportunities), [MDA declared traces and questions](#mda-declared-traces-and-questions), [Draft editing and JSON handoff](#draft-editing-and-json-handoff), [Prepared experiments](#prepared-experiments), [Reader boundary](#reader-boundary), [Checks and dependencies](#checks-and-dependencies).

## Build

Windows development build uses installed CJIT 1.2.0. Run from repository root:

```powershell
./skills/game-design-workbench/native/build.ps1
```

Optional view needs an explicitly supplied raylib **5.5 win64** SDK containing `lib/raylib.dll`:

```powershell
./skills/game-design-workbench/native/build.ps1 -View -RaylibDirectory C:/Dependencies/raylib-5.5_win64_mingw-w64
```

Build never downloads dependencies or builds them implicitly. Outputs default to ignored `native/build/`. Use explicit `-OutputDirectory PATH` for a separate build without replacing a running explorer or interrupting its draft; prepared UI library still defaults to `native/build/gd-ui.dll`. Launcher `native/gd.ps1` chooses headless or view executable; it never builds automatically. Direct executable invocation works too.

CJIT application code is unoptimized. Windows Zig 0.17.0 `zig cc -std=c99` headless build also checked; no macOS/Linux execution claim. Equivalent command, from this directory:

```sh
zig cc -std=c99 -DHAVE_CONFIG_H=1 -DYAML_DECLARE_STATIC=1 \
  -I vendor/libyaml/include -I vendor/libyaml/src \
  gd_core.c gd_output.c gd_layout.c gd_matrix_grid.c gd_layout_routes.c gd_layout_focus.c gd_focus.c gd_graph_analysis.c gd_graph_paths.c gd_graph_closure.c gd_graph_shapes.c gd_graph_overview.c gd_diagnostics.c gd_skills.c gd_skill_unlocks.c gd_draft.c gd_routes.c gd_mda.c gd_controls.c gd_control_import.c gd_rules.c gd_rule_axes.c gd_rule_logic.c gd_rule_quantities.c gd_rule_analysis.c gd_resource_graph.c gd_resource_output.c gd_explorer.c gd_cli.c vendor/libyaml/src/api.c \
  vendor/libyaml/src/parser.c vendor/libyaml/src/reader.c \
  vendor/libyaml/src/scanner.c -lshell32 -o build/gd-zig.exe
```

## Unified explorer

`view explorer` opens Skills, Challenges, Relationships, Interactions, MDA and Resources in one window. Needs prepared `-Graph` build. Working-branch command compatibility is not required. No new dependency, registry reader, schema change or automatic folder discovery.

```powershell
$native = 'skills/game-design-workbench/native'
$game = 'skills/game-design-workbench/examples/little-plumber-guy-nes/game-design'
./skills/game-design-workbench/native/build.ps1 -Graph -RaylibDirectory C:/Dependencies/raylib-5.5_win64_mingw-w64
$inputs = foreach ($role in @('skill-atoms', 'challenges', 'nouns', 'verbs', 'interactions',
    'mechanics', 'systems', 'dynamics', 'aesthetics', 'axes', 'resources', 'control-vars')) {
    "--$role"; "$role=$game/$role"
}
& "$native/build/gd-view.exe" view explorer @inputs
```

- **Data / projection** selects existing projection. **Layout** selects its supported presentation independently. New arbitrary data/view combinations remain out of scope. Unavailable projections explain missing explicit role bindings; empty valid folders remain valid.
- **Interactions → Layout → Grid** shows noun rows, verb columns and all declared cells, including inactive pairs. Free graph and Noun / verb columns remain choices. `--mode matrix --layout grid|graph|columns` sets initial Interactions layout; default remains Free graph. Grid starts at readable scale; `F` fits full extent. Wheel zooms cells/labels around pointer; right/middle drag pans. Click cells or axis headers; arrows select with native held-repeat, PageUp/Down cycles cases. Sidebar reuses conditional-case/source inspector. Blank cells imply no rule, exclusion or defect.
- Grid uses sparse cell drawing plus viewport-culling and level-of-detail labels/lines; never allocates or scans dense Cartesian product. All 4,096 axis records supported, without ImGui's 512-column cap. Below 8 screen-pixel text size, labels disappear with explicit zoom notice; subpixel cells merge pixels, not density counts or inferred clusters. Axes pan with canvas; hover identifies cells when headers leave viewport. Graph analysis/focus/overview layers are unavailable in Grid, explicitly disclosed; settings remain for graph layouts. Find highlights/frames source-associated cells/headers, including inactive cases. Reload remaps selected axes/case identity and re-resolves cells, including blank-to-declared and declared-to-blank changes.
- `gd_matrix_grid.h/.c` keeps graphics-free ordering, sparse lookup, hit/range geometry and clamped navigation. Each pane reserves 32,784 bytes presentation state outside typed snapshots/drafts; total six-pane reservation 196,704 bytes. Rendering/input lives in `gd_matrix_grid_view.h`, included only by unified shell. Existing standalone table shares cell lookup; its fixed-size/scroll presentation remains separate, not a compatibility requirement.
- Ordinary switching restores each view's own selection, camera, positions, layout, filters and analysis controls. No automatic selection following, re-rooting or filtering. Inactive views do not settle.
- **Find selected in** dropdown or canvas right-click explicitly opens another view. Qualified source identity bridges independent snapshots; record/node offsets never do. Case-local Resource facets map to their case source; resolved shared-pool facets map to resource source, not first contributing case. Unresolved shared identities do not acquire a fabricated source.
- Find highlights all visible source-associated representations and frames them together. Keep destination filters; report hidden matches. No visible matches leaves camera unchanged. No representation does not prove source absence or semantic equivalence. Interaction cases can match several matrix cells; case identity remains inspectable.
- **RESET CURRENT VIEW TO DEFAULTS** clears selection/Find, filters, hypothetical knowledge, configured analysis starts and layer settings; restores initial layout/order/direction/settling and fits shown graph. Relationships restores its default first-skill neighborhood, or first loaded record when no skills exist; MDA restores all-node/both-direction query. Shared neighbors defaults on; other analysis layers off. Other views, source data, drafts and undo remain intact. Invalid pending rank text blocks record selection, leaving and resetting view rather than discarding input.
- Shared navigation, browse/source inspector, diagnostics, analysis controls, Fit, Refresh, Undo and finish/report actions include Resource projection. Resource incidence still does not implement routes/composed outcomes.
- **Focus layout by shown-link distance** grades placement within preserved bands/columns; free layouts use 2D rings. Shortest shown-link distance is undirected for placement only, not flow or similarity. No hard hop cutoff.
- Directed views keep automatic **data roots** fixed: shown nodes with zero incoming shown arrows. This is projection-local, not proof of global parent absence or a configured path start. No synthetic roots for cycles. Directionless matrix/Resource incidence has no roots. Root-containing components retain roots while unpinned portions move together; other disconnected components move intact. Free-layout root-box repair translates unpinned primary geometry together, avoiding collapsed rings. Manual pinning is future work.
- Visible-node selection supplies **focus anchor**, not root membership. Source-only/edge inspection does not choose an anchor. Focus is static: select nodes, pan/zoom or Fit all. Turning it off restores regular positions/velocities/camera and normal dragging/settling. Initial framing covers anchor/direct neighbors only; camera never filters analysis scope. Each view retains mode; Reset clears it. Atomic reload prepares replacement scratch before swap.
- `gd_layout_focus.h/.c` owns bounded lazy pose/BFS scratch outside models/drafts: 630,832 bytes per activated view. Exact active scope revision caches pose; no repeated clearance search. Tests cover 128 independent distance/indegree oracle projections, fixed roots, both band/column orientations, rigid unpinned portions, ring-separation repair, restoration/cache and scratch-allocation rollback. One Windows CJIT run: 4,096 nodes/32,768 links, 422 ms prepare and 53 ms / 120 cached poses; directed free layout with 2,048 fixed roots, 3 ms prepare. Not frame-time guarantees.

Skills use node-to-node connections; rounded component backgrounds are omitted. MDA curves and Resource straight lines retain existing semantics/renderers. Graph windows request raylib 4× MSAA; realized driver sample count and zoomed-out readability still need visual review.
- `F`: fit; `F5`: refresh; Ctrl+Z: undo when not editing text; Space: pause/resume movable layout. Right/middle drag pans; right click without drag opens Find menu. Control/overview interaction stops node drag.
- Refresh prepares all typed snapshots and every visited replacement layout before swapping. Failed load, allocation or disappearing query/focus root retains last good project. Qualified identities remap selections, relationship centers, skill assumptions and analysis starts; Resource starts use case/participant/axis/pool identity, not record indices. Lost starts are disclosed. Modified/invalid draft blocks refresh.
- Skill draft lives across every view. Finish/close returns existing JSON proposal report, or writes explicit new `--output FILE.json`; no Markdown writes. Without skill data, successful close emits no draft. `--format` affects error output, not draft JSON. `--mode skills|challenges|relationships|matrix|mda|resources` selects initial projection; default first available. `--frames 1..600` and `--screenshot FILE.png` provide bounded rendering checks.

Implementation retains existing typed readers in at most four domain snapshots, one skill draft, six lazy view layouts and one active analysis scratch. No model/draft/reload struct enlargement. `gd_explorer.c` owns project lifecycle/source identity; `gd_explorer_view.c` owns window and independent pane state. Existing renderer/source adapters avoid a second graph evaluator.

```powershell
./skills/game-design-workbench/native/test-explorer.ps1 -RaylibDirectory C:/Dependencies/raylib-5.5_win64_mingw-w64
```

Checks use private copies of both examples, an owned bounded window and app-local state transitions. Verify six projections plus Grid, 4,095-column access, balanced 2,048×2,048 axes and empty axes, case/blank inspection, Grid switching/reset/reload, independent switching, multi-match Find/hidden counts, reset/draft isolation, qualified relationship centers, rank-validated selection, stale-edge recovery after edits/Undo, identity remapping and atomic reload rollback. Force raylib warning to stderr while stdout remains valid JSON. Allocation failures are injected; no OS input posted. Retain JSON/logs/screenshots and native source hashes in fresh ignored `build/explorer-checks-*`; original/private example hashes must stay unchanged. These checks do **not** establish real-pointer acceptance, layout readability or design usefulness. Review against [integrated notes](../../../reports/graph-query-research/integrated-review-notes.md); drawing/AA/jitter/selection-layout work and original-question audit remain open.

## Resource declaration report

`gd resources` exposes existing C99 direct-declaration graph without display, raylib or agent runtime. This is a bounded Review 02 report slice, **not directed resource-flow/routes delivery**.

```powershell
$native = 'skills/game-design-workbench/native'
$game = 'skills/game-design-workbench/examples/little-plumber-guy-nes/game-design'
& "$native/build/gd.exe" resources --interactions "interactions=$game/interactions" `
    --mechanics "mechanics=$game/mechanics" --nouns "nouns=$game/nouns" `
    --axes "axes=$game/axes" --resources "resources=$game/resources" `
    --control-vars "control-vars=$game/control-vars" --json
```

- Require explicit `--interactions` and/or `--mechanics`. Optional `--nouns`, `--verbs`, `--axes`, `--resources`, `--control-vars`, `--systems` and `--related`. No discovery or source writes. Same reader, extraction and guarded call binding used by Resource views.
- JSON `data`: `nodes`, `relations`, `routes`, `findings`; `--fields nodes,relations` selects sections. Standard `GD_AGENT=1`, piped JSON, explicit `--json`/`--format json|text`, error envelope and exit codes. Text prints scope summary plus JSON objects for retained nodes/relations; conditions/provenance remain available in either format.
- Nodes retain case-local facets, explicit shared pool/axis facets, resource roles and supported unresolved placeholders. Opaque percent-escaped IDs derive from qualified source/case/participant/axis or explicit pool identity, never transient indices or machine paths. IDs survive unrelated source-index changes and project relocation. Unresolved axis occurrences stay distinct by field kind/local declaration position; no guessed axis identity or global actor merge. Duplicate consumed identities remain loader failures.
- Case nodes retain whole raw context/participant guards and unknown declarations. Relations retain source/status/line, raw predicates/effects/expressions, typed target resolution, checked signed deltas, conditional preservation and case-specific call maps. Facet provenance is representative, not ownership. Role nodes retain declared noun and descriptive prose without converting it into constraints. Predicates are not consumed resources; incident rows are not independent AND requirements. Deltas are not final bounds-checked outcomes. Capped/unknown expressions retain unknown delta, not a promised award.
- All supplied statuses/dispositions remain inspectable; no route eligibility implied. `ok: true` means report construction succeeded. `coverage.complete` stays **false**: routes, composed outcomes and applicability evaluation are unavailable. `data.routes.available: false` is **not** an empty/no-path result. `--from`/`--to`, route filtering and scenario options are not supported here; use `rules --case` for explicit conditional evaluation. Missing/unloaded scope remains distinct where readers support it; malformed syntax/types and invalid closed vocabularies remain errors.
- Limits: 4,096 retained nodes, 32,768 relations, 67,108,864 construction work. Coverage reports retained counts, graph limits, unexamined cases/roles/invocation bindings/preservation, unknown facets/cases and typed-binding gaps. Serializer uses 16,384 bytes local endpoint-map scratch, no model/draft enlargement or dense allocation. Viewer does not yet display these opaque IDs or resource route witnesses.

## Resource graph preview

Standalone preview command below is still available; retaining it is not a compatibility requirement. Prefer [unified explorer](#unified-explorer) for shared controls, filtering, source inspection and refresh.

Review 02 is in progress. `gd view resources` shows direct declarations, explicit pool bindings and compiled invocation links from shared C99 graph. It does not implement resource routes, composed outcomes, source navigation, filtering or refresh. A line is not proof of a flow; omitted rules and unmodeled conditions remain unknown. Close and reopen if sources change. Unbound resource roles remain separate; the preview does not infer identity from matching names.

```powershell
$native = 'skills/game-design-workbench/native'
$game = 'skills/game-design-workbench/examples/crate-puzzle/game-design'
& "$native/build/gd-view.exe" view resources `
  --interactions "interactions=$game/interactions" --mechanics "mechanics=$game/mechanics" `
  --nouns "nouns=$game/nouns" --axes "axes=$game/axes" `
  --control-vars "control-vars=$game/control-vars" --resources "resources=$game/resources"
```

Select a node to inspect its source and incident declarations. Drag nodes; right-drag pans; wheel zooms; `F` fits; Space starts/stops settling. Preview is read-only; no manual UX acceptance claimed.

## Prepared experiments

Shared `gd_experiment.h/.c` provides bounded typed frozen-JSON input, reentrant PCG32, finite measurements, JSON encoding, monotonic clock/step/output accounting and explicit terminal status. No live control loading/formula evaluation, generic runner/expectation engine, checkpoint automation, or authoritative writes. Strict JSON only; LibYAML decodes UTF-8 after lexical gate and JSON surrogate-pair normalization. Caller-owned allocator; input cap 1 MiB, 4,096 nodes, depth 32. Integer fields require integer lexemes within ±(2^53−1); numbers must be finite and not underflow to zero. New PCG dependency carries unchanged source/header and Apache-2.0 license/provenance.

[Crate route-cost recipe](../examples/crate-puzzle/experiments/route-cost/recipe.md) contains inspectable baseline C99 model and actual resolved input snapshot. Clock includes seven delays and action durations, from before first delay through final push completion. Insufficient-energy variants are unsupported, not silently modeled. Canonical controls/experiment record remain unchanged.

```powershell
pwsh -NoProfile -File skills/game-design-workbench/native/test-experiment.ps1
```

From repository root, explicitly builds using installed CJIT/Zig and runs **engineering helper checks only**. Model is compiled, never launched by script. Outputs/logs/argument arrays stay in ignored `native/build/experiment-checks/`; no running viewer is replaced. 100 helper checks passed with CJIT 1.2.0 and Zig 0.17.0, including upstream PCG golden draws. [Crate baseline results](../examples/crate-puzzle/game-design/experiments/route-cost.md#results) now retain one complete validated run under Windows/Zig. No broader workload or cross-platform performance claim. [Explicit native supervision and result validation](experiments.md) now provide owned Windows jobs, independent watchdog, raw bounded capture and strict read-only NDJSON checks. `test-experiment-supervision.ps1` exercises synthetic emitters only, including parent/descendant hangs and pipe floods. Result views remain later work. New trials follow reviewed checkpoint/execution policy. Opening prepared inputs, recipe or saved results never executes model code.

## Commands

Every operation requires explicit role aliases and folders. No conventional-folder fallback, registry reader, or schema interpreter. Examples use shipped fixture paths from repository root:

```powershell
$collections = @(
  '--nouns', 'nouns=skills/game-design-workbench/examples/crate-puzzle/game-design/nouns',
  '--verbs', 'verbs=skills/game-design-workbench/examples/crate-puzzle/game-design/verbs',
  '--interactions', 'interactions=skills/game-design-workbench/examples/crate-puzzle/game-design/interactions'
)
./skills/game-design-workbench/native/build/gd.exe matrix @collections --json
./skills/game-design-workbench/native/build/gd.exe connectivity @collections --json
./skills/game-design-workbench/native/build/gd.exe matrix @collections --json --fields cells
./skills/game-design-workbench/native/build/gd-view.exe view matrix @collections `
  --related mechanics=skills/game-design-workbench/examples/crate-puzzle/game-design/mechanics
```

Repeat `--related ALIAS=FOLDER` to load linked rule/context records for navigation. Omitted related folders leave references visible but not loaded; this is not a missing-record verdict. Missing, ambiguous, or wrong-role matrix endpoints produce operation errors and partial data.

`--format json|text` overrides automatic format. Pipes, `GD_AGENT=1`, and nonempty `CI` default to JSON. `--fields` selects top-level `data` keys only: matrix `nouns,verbs,cases,cells,components`; connectivity `nouns,verbs`; skill-atoms `skills,links,components,findings`. Selecting `findings` also includes `findings_omitted`. Envelope/coverage/errors remain. `--help` and `--version` need no folders.

- Exit `0`: operation completed, including empty folders, unresolved authored coverage, and completed skill analyses with findings. Finding severity does not change exit status; no finding-threshold flag exists yet.
- Exit `2`: argument, loading, resource, output, or view failure. Headless failures retain partial JSON with `ok:false` and `coverage.complete:false` when model/output are available; diagnostics go to stderr. Early argument-decoding/allocation or broken-output failures may have stderr only.
- Matrix views emit no result. Skill view returns one JSON draft-change report on finish/close, or writes it to explicit `--output FILE.json` instead. Skill view rejects `--format text`; analysis commands retain text support. Runtime/capture failures use stderr and exit `2`; current-disk read failures retain proposed edits in partial report with `ok:false` and exit `2`.
- JSON `schema_version:1`: `data`, `coverage`, `errors`, and `errors_omitted`. Analysis results also include navigation `breadcrumbs`; draft reports use source paths directly. Paths use UTF-8; diagnostic `line:0` means location unavailable.
- `coverage.complete` concerns supplied source loading/computation only. It does not certify schema validity, complete linked context, exhaustive conditions, or gameplay truth.

## Matrix meanings

- One sparse cell per declared applicable noun/verb pair. Multiple cases stay separate, with source, body, status, coverage, and labelled relationships.
- `context_nouns` never create matrix membership. Duplicate target links within one case do not inflate membership.
- Active pair means at least one `status: accepted`, `coverage: defined` case. Distinct active partner count differs from case count.
- Proposed/inactive cases, unknown coverage, exclusions, and non-applicability remain visible without increasing active partner counts. Zero-degree nouns/verbs remain visible.
- Missing status/coverage stays null, not accepted/defined. Blank cell means no declared case, not a defect or implicit no-effect rule. Defined coverage does not prove exhaustive conditions.
- Conditions/consequences remain authored prose. Tools do not infer executable rules, emotional effects, or quality scores.

## View

Select cells with click/arrows. Held arrows repeat using raylib's native repeat events. Wheel pans rows; Shift+wheel pans columns. PageUp/PageDown (also held-repeat) or Previous/Next cycles cases. Right panel shows original Markdown and declared links; click uniquely loaded links for focused navigation, then Back to case. Wheel scrolls details; `C` copies shown source path. This is path navigation, not launching an external editor.

Source content is checked about once per second. Changes show stale banner; `F5` explicitly loads a fresh model. Failed refresh retains previous snapshot and failure banner. View never writes records. No automatic refresh or repair.

Bounded visible smoke capture:

```powershell
./skills/game-design-workbench/native/build/gd-view.exe view matrix @collections `
  --frames 3 --screenshot skills/game-design-workbench/native/build/matrix.png
```

`--frames` accepts `1..600`; screenshot requires bounded frames and `.png` destination. Capture overwrites that explicit output file. Screenshots support Windows Unicode paths; font glyph coverage is limited by raylib's default font. Live stale/F5/failure/recovery behavior is checked by the desktop workflow below. Broader mouse/keyboard and clipboard review remains.

## Dynamic graph

Build graph UI dependencies explicitly, using installed Zig 0.17.0 and the same raylib 5.5 Windows SDK. Only vendored dependencies compile as C++; application/layout code remains C99. No implicit download or dependency rebuild:

```powershell
./skills/game-design-workbench/native/build-ui.ps1 -RaylibDirectory C:/Dependencies/raylib-5.5_win64_mingw-w64
./skills/game-design-workbench/native/build.ps1 -Graph -RaylibDirectory C:/Dependencies/raylib-5.5_win64_mingw-w64
./skills/game-design-workbench/native/build/gd-view.exe view matrix @collections --layout graph
```

`-Graph` includes matrix table/graph and skill graph views and links prepared `gd-ui.dll`. Plain `-View` retains raylib-only table build; headless build needs neither graphics dependency. Close running view before rebuilding locked DLL/executable. Original prototype sources were paired with raylib 6.0; workbench rebuilds cimgui/rlImGui against retained **5.5** headers/import library and tests that matching runtime. No prototype prebuilt DLL is reused.

- **Force-directed:** nodes settle live. Drag holds a node; release resumes settling. Pause freezes settling but still permits dragging.
- **Noun / verb columns:** stable arrangement within connected groups. Start directly with `--layout columns` or switch in cimgui panel.
- **Name / active partners:** sort column positions or force-layout initial order. Changing sort/layout rearranges transient positions; it does not write records or lock force positions to a sort order.
- **Fit / Rearrange:** fit current positions or reset chosen arrangement. Right/middle drag pans; wheel zooms around pointer. Select nodes or edges for cimgui source/case details.
- **Other-pair overlay:** checkbox or `--show-other-pairs` reveals dashed inactive/unknown/excluded/not-applicable pairs. Overlay neither merges groups nor adds force springs. Active pair with several cases still has one spring/edge.
- **Refresh sources / F5:** explicit fresh load; failed load retains previous graph and stale notice. Successful load recomputes groups/layout, retaining selected endpoint/node identity when available. Nothing is persisted, including ImGui window settings.

Core `components` groups accepted, defined matrix pairs only. Every axis node belongs to one component, including isolated singleton nodes. Each JSON group has `anchor`, `members`, and distinct `active_pair_count`; context/mechanic links never join matrix groups. Example: two linked groups plus six singletons, eight components total. Grouping is declared structure, not inferred meaning or a quality score.

`gd_layout.h/.c` accepts labelled nodes, edges, caller-provided groups/column sides/layers, and sort weights, not game records, statuses, prerequisites, or ranks. `gd_skills.c` owns skill analysis and its layout adapter. Directed presentation locks ordering axis; domain code supplies layers. Four deterministic barycentre sweeps order within bands; label/card dimensions reserve disjoint lanes. Native renderer measures 18-pixel labels after window initialization; headless fallback counts UTF-8 codepoints, not bytes. Directed settling omits generic edge springs so shared parents cannot collapse packed lanes. MDA supplies 220 × 96 card bounds and retains its card/Bezier renderer. One fixed display step evaluates at most 32 repulsion samples/node plus active edges; larger groups use fixed, evenly spread reciprocal partners. Identical positions no longer receive tick-dependent sample forces; this reduces jitter without changing spring/drag constants. Sampled repulsion remains approximate, not a collision guarantee. No all-pairs force loop or convergence wait. Renderer runs at most four fixed steps/frame; pause/columns skip steps. Display physics is approximate layout, not game simulation or community detection.

Repeatable six-view rendering probes: `pwsh -NoProfile -File scripts/capture-workbench-graph-review.ps1 -RaylibDirectory PATH` from repository root. Requires prepared SDK/UI/executable; no downloads, prompts or OS input. Default captures five layer states for plumber; `-Example crate-puzzle` changes fixture. Each run uses fresh ignored directory, JSON metadata, unchanged-example/source checks and empty-draft assertions. Non-default layers use PRIVATE source copies with seeded flags/selections; they are rendering evidence, not real-control/input-isolation acceptance. Use PowerShell `Get-Help ./scripts/capture-workbench-graph-review.ps1` for contract.

Bounded graph capture works with existing `--frames`/`--screenshot` options. Physical pointer/clipboard behavior and large-project rendering still need review. Live external-edit/F5/failure/recovery paths are checked by the desktop workflow below; bounded captures and graphics-free solver/source-change paths are also checked.

## Focused relationships

**Explore relationships here** opens a selected graph node, conditional case, or linked source record in the same window. Skill/progression explorer also offers **Explore → Relationships**. Matrix graph offers **Return to matrix graph**.

- Initial scope: root plus one-hop incoming/outgoing uniquely loaded neighbors. **Expand this record one hop (in + out)** adds another explicit neighborhood. **Center relationships here** starts again from selected record.
- Stable radial placement is transient, not progression or causal structure. Square marks root. Labels show names/collection aliases; source panel retains exact qualified identities. Drag, pan, zoom, Fit, Rearrange, and sort remain available.
- Arrows follow authored source → target, including prerequisite declarations. This differs from Skills mode's prerequisite → dependent arrows. Select a link for field/source line; selected link shows field label on canvas. Raw duplicate/self links remain separate declarations, even when drawn on the same segment.
- Source panel lists outgoing and incoming consumed declarations. Click loaded references to inspect source; expansion remains explicit. Browse lists all loaded records in this mode.
- All statuses and revealed isolates remain. Boundary count covers raw declarations incident to visible nodes but not drawn: endpoints outside revealed scope or not uniquely loaded. Missing optional folders are not a missing-record verdict. No invented target nodes.
- Only fields already consumed by the active domain loader appear. Loading a folder via `--related` does not inspect its schema or every frontmatter field. This is not a complete project relationship/impact report, reference-hygiene check, or causal/learning verdict.
- Skill drafts, undo history, and report destination survive mode switches. Relationships mode is read-only. Draft edits/undo rebuild visible declarations; previously revealed isolates remain until recenter/reload.
- F5 retains root identity but resets revealed scope to one hop. Missing/ambiguous root or failed load retains last-good snapshot and error; leave Relationships or center another loaded record before accepting root removal. Dirty drafts still block reload.

Start directly from a loaded record, from skill folder:

```powershell
$game='examples/crate-puzzle/game-design'
./native/build/gd-view.exe view skill-atoms `
  --skill-atoms "skill-atoms=$game/skill-atoms" `
  --challenges "challenges=$game/challenges" `
  --related "mechanics=$game/mechanics" `
  --mode relationships --from mechanics/crate-pushing
```

`--from` must identify a unique loaded record; omission starts at first loaded skill, or first other loaded record when skills are empty. No automatic collection discovery. Shared graphics-free `gd_focus.c` owns reveal state/layout adapter; generic layout solver remains unaware of record semantics. No native record writes or standalone generic query command.

## Graph analysis layers

**Shared neighbors layer** works in matrix graphs, Skills, Challenges, Relationships, MDA and resource preview. Toggle it independently of source/draft/knowledge/witness state. Default on; settings remain transient.

- Scope is current view's shown nodes and links. MDA role/reference/query/path filters and matrix inactive-pair overlay affect scope. Pan, zoom, dragging and off-screen clipping do not. No hidden whole-source graph supplies extra matches.
- **Neighbor lens** selects **Any incidence** (default), **Same incident role**, or **Changed incident role**. Incident role is projection link kind plus endpoint side. Same-role needs one equal role pairing; changed-role needs one unequal pairing. Alternative declarations can satisfy both lenses, so these are not disjoint partitions. Resource preview cycles lens by clicking its label.
- `S` badge counts discovered peers sharing a neighbor under selected lens. Duplicate/parallel links do not inflate shared-neighbor counts. Evidence uses a qualifying role pairing, not merely the first link. Inspect view-specific evidence, not a semantic-equivalence score.
- Select a node for other-neighbor/incident-row counts and discovered peers. Open **Shared-neighbor evidence** to choose a peer and inspect one shared-node witness, its prevalence and two source links. Thick purple underlays mark existing witness links; they do not add a new edge or change arrows. Resource preview uses **Next retained match** and its own sidebar instead of cimgui.
- **Compare shared / unique branches** adds an optional one-hop overlay for selected A and discovered peer B. Default off; surrounding graph remains. Purple marks qualifying shared neighbors, blue A-only neighbors, amber B-only neighbors, gray shared identities excluded by lens. Compare identities before roles: an excluded common identity is not unique to either side. `S` badges retain graph-wide meaning.
- Branch links retain every shown declaration, including parallel alternatives. Classify each incident role; white marks mixed link classes. A/B labels identify pair anchors. Direct links/self-declarations remain incidences, so pair anchors can themselves be neighbors. Branch counts include such identities; existing **other neighbors** excludes self. Counts/classes describe source topology, not required or missing gameplay ingredients.
- Matrix links remain derived case-pair cells. Skill links remain prerequisite-to-dependent arrows. Challenge links remain declared `next`. MDA retains authored fields/directions. Resource relations remain typed incidence, not production/consumption direction or proof of flow. View-specific source details preserve these distinctions.
- No traffic, importance, learning, transfer, gameplay equivalence or balance claim. No prerequisite inference or source/rank mutation. Incident-role lenses do not compare whole conditions/behavior. Longer semantic path joins and structural-shape families remain pending.

Graphics-free `gd_graph_analysis.h/.c` owns caller-allocated scratch/result, containing layout indices only. No model/string ownership. Exact topology/relation-tag/mask/lens comparison caches results across camera/position changes. At most 65,536 incident occurrences are sorted/deduplicated into an inverted neighbor index; joins retain at most 16,384 unique pairs and charge at most 16,777,216 work units across candidate-pair visits, role-search iterations and hash probes. These compute bounds are separate from cimgui's 128 displayed peers. Exhaustion exposes `PARTIAL` and lower-bound `S`/shared counts; absence of retained match is not proof of no match. Other-neighbor/incident counts remain exact for shown projection. Evidence retains one deterministic example, not all alternative links. Kernel is callable without graphics; no new headless CLI command in this slice.

`gd_graph_compare(analysis, a, b)` returns cached ordered-pair classes/counts and per-layout-link flags. Visits bounded node/feature/link arrays with binary role lookups, `O(nodes + features + links * log(features + 1))`; no allocation or global discovery-work mutation. Index is complete before bounded pair joins, so selected-pair counts remain exact for shown projection even when global discovery is partial. No whole-source completeness claim. Cache invalidates with projection/lens, not camera; results borrow analysis-owned storage until next compare/analyze call. Shared `gd_graph_analysis_draw.h` keeps cross-view colors/labels consistent; source colors and arrows remain above analysis underlays.

### Directed drawing and source paths

Skills, Challenges and MDA retain existing depth/rank/entry-hop/role layers, cycle evidence and unplaced shelves. Barycentre ordering is bounded, deterministic geometric arrangement, not inferred source direction or a global crossing optimum.

- Native Skills use node-to-node links, existing deterministic in-band order, additional cross-axis spacing and bounded placement clearance for bypassed intermediate nodes. Two clearance passes allow at most 16,777,216 edge checks; not a complete obstacle/crossing solver. Same-band Skills links use rounded bows capped inside band clearance; self-links use bounded curved loops. This prevents collinear cycle links from implying incidence at intervening nodes without squared corridors or moving bands. Not a universal obstacle solver. Source directions and semantic bands remain unchanged. Challenges/Relationships retain clipped polyline ports, adjacent-band gaps and side corridors for long/back links. Self links remain explicit loops. Parallel/opposite declarations get distinct transient lane ordinals; every source edge remains available.
- Native Relationships uses live endpoint/parallel geometry without assigning semantic layers. Matrix graph keeps derived case-pair meaning. Resources keeps existing incidence renderer; MDA keeps its curved card links.
- Native base lines, arrowheads, branch/path/foundation underlays, selected field labels, Fit and hit testing all use same routed geometry. Selection follows rendered segments, not obsolete endpoint chords.
- Label/card dimensions are presentation only. No authored rank, status, reference, source arrow or AND relation changes. Camera/positions do not invalidate analysis.
- This is not a complete obstacle solver. Large parallel bundles, stacked analysis badges, manually displaced nodes and dense feedback regions need integrated reading review. Rearrange restores packed lanes. No comprehension or performance improvement claimed.

### Dense adjacency / incidence overview

Default-off **Adjacency / incidence overview** adds bounded fixed-screen inset across six views. All shown identities remain; contiguous projection-index bins compress to at most 32×32 cells. No semantic clustering or universal automatic size threshold. Camera, dragging and within-layer geometry order do not change bins.

- Directed rows follow **shown arrows**, not authored direction when adapter reverses presentation. Matrix/Resources use symmetric incidence, explicitly not flow. Across-bin mirrors do not double `input link rows`; same-bin contributions count once. Diagonal bins need not mean self-links.
- Brightness uses logarithmic bands of projected link-row counts, not resource quantity, strength, traffic or flow width. Parallel projected rows count separately; derived matrix rows remain derived, not invented declarations.
- Click inset cell or use **Row bin**, **Column bin**, **Inspect bin cell**. Actual contributing graph links/endpoints get gold underlays/outlines; unrelated members of aggregate bins do not. Side panel shows identity ranges/examples, exact count, typed counts and source provenance. Text caps 64 source links; brush flags/counts remain complete.
- Inset consumes its pointer/wheel/pan events; underlying node dragging stops when entering inset. Scope/mode changes clear cell selection. Layer toggle remains independent of graph controls and other analysis layers. Empty scope remains empty, never synthetic traffic.
- Resources cycles **Layers** through **Foundations**, **Typed paths**, **Shapes**, **Overview**. Overview also offers row/column cycling, **Next occupied cell**, **Clear cell** and arrow-key cell movement. Source examples retain up to four actual contributing rows.

Counts scan all shown input rows, independent of truncated shared-pair discovery. Missing/filtered declarations stay outside scope. Integrated active-cell/input-isolation review remains pending; no claimed readability/performance benefit. Full view-owned analysis scratch is 10,519,576 bytes; overview is 77,992 bytes, outside model/draft/reload snapshots.

### Structural shape families

Default-off **Structural shape families layer** adds two exact typed endpoint-neighborhood refinement rounds across all six views. Initial classes are projected source roles (resource case/facet/role kinds in Resources), not canonical identities, rank, status or labels.

- `shape <ID>/<size>` shows transient family and full membership count. Gold outer outline highlights selected family; source colors/arrows remain. IDs may change with projection/reload.
- **Discovered shape families** lists repeated buckets before node drill-down; choose representative for ordinary source navigation. **Shape signature and members** shows role tag, previous-round typed features and member references. Text retains 32 signature features/128 members; exact comparison uses full tuples.
- Duplicate declarations do not inflate signatures. Distinct neighbors of same class retain multiset multiplicity. Directed endpoint roles remain distinct; matrix/resource endpoint roles do not imply flow.
- Same family is not semantic equivalence, canonical identity or graph isomorphism. Rare shape is not a defect. Missing declarations can change families; results describe shown projection only.
- Work exhaustion suppresses family claims, never exposes intermediate refinement as proof. Shape work/cache is independent of shared-neighbor discovery; complete shape results can survive partial pair discovery. No unchecked hash equality, embeddings or quality score.

Resources **Shapes** tab keeps independent toggle; **Next repeated family** selects representative. Member labels retain snapshot-local `#node` keys because different case/participant facets can share axis labels. Integrated interaction/usefulness review remains pending.

### Typed multi-hop connections

Default-off **Typed multi-hop layer** applies across all six views. Select one projected relation kind; directed views offer shown-arrow or reverse traversal. Matrix/Resources use same-kind undirected incidence, explicitly not flow. Resources cycles **Layers** header between foundations/typed paths and uses click-cycling kind/peer/target controls.

- `T<n>` counts other shown origins reaching this identity; `+` means lower bound. Strict closure excludes origin itself. Auto-discovered peers share reached canonical identities, not equivalent meaning.
- Purple: shared reached identity; blue: A-reached only; amber: B-reached only. Entire reachable branch marks are ordinary topology, not AND necessity, learning or optional ingredients.
- Expand **Typed reachability evidence** to choose target and inspect one BFS witness from each origin. Text retains last 64 links; Resources shows last four. Full lengths/branch marks remain available. Reverse traversal is labeled; original source arrows never change.
- Source examples retain projection keys/roles and view-specific provenance. Filters define scope; camera does not. Unresolved/filtered declarations are not silently traversed. Same-kind traversal does not mix MDA fields or infer resource production routes.
- Foundation and typed traversal use independent caller-owned caches. Projection/role/orientation changes clear derived peer/target nominations; camera changes preserve them. Completed local rows remain usable when global discovery is partial. No new CLI syntax or source writes.

### Prerequisite foundations and AND branches

Default-off **Prerequisite foundations layer** analyzes shown Skills and shown skill-prerequisite links in Relationships. Other projections explain unavailability; Resources has **Foundations** tab. No hidden skill graph is analyzed behind another view.

- `F<n>` counts other shown skills whose strict prerequisite closure reaches this node. Discover reused foundations before choosing a pair. `+` means work-limited lower bound.
- `AND<n>` marks at least two distinct shown direct prerequisites. All remain required; shared ancestry never makes branches alternatives. Duplicate declarations do not inflate counts. `AND<n>+` is incomplete local count.
- `?` marks unresolved, wrong-role or outside-projection prerequisites, including inherited boundaries. `cycle` marks a directed cyclic dependency, not a grounded foundation or invented knowledge.
- Select a skill: auto-discovered **Shared-foundation peers** compare full strict closures. Purple: shared; blue: A-required only; amber: B-required only. Branch marks include entire shown dependency branches, not one path. Root itself is excluded even in cycles.
- Expand **Prerequisite-closure evidence** for required identities and first source per projected link; up to 64 dependency/link text rows, without truncating graph flags/counts. Relationships retains raw duplicate declarations. Existing skill-combination and route-opportunity controls remain separate; no automatic learning or second route engine.
- Completed local rows can be compared when global closure discovery is partial. An unfinished row cannot prove absent dependencies/peers. Camera/drag positions do not change scope. Draft/undo/reload recompute through existing projection lifecycle.

Storage: one view-owned 2,826,320-byte cache, including 2,097,152-byte bitset table at 4,096-node capacity. Not model/draft/snapshot storage. Chain capacity check uses exactly 16,777,216 queue/arc work units; one selected-source traversal alone is cheaper but cannot supply graph-wide reuse counts. Pair selection uses at most 4,096 × 64 word intersections. No benchmark or product-benefit claim.

### Mandatory paths and necessary connections

Optional **Mandatory paths layer**, default off, across all six graph views. Resource preview uses **Paths** tab; **Context** retains neighbor/branch controls. Layers stay independent.

- Analyze current shown projection. Challenges default to all shown authored entries. Disable **Use authored challenge entries** to choose explicit starts. Other views require **Add selection as analysis start**. Multiple starts supported; inspection never re-roots. No starts means unavailable, not inferred zero-indegree roots.
- Virtual start is analysis-only, disclosed in controls, not inserted into authored graph. White triangles mark actual analysis starts. Native Skills, Challenges and Relationships keep separate transient start sets. Successful reload remaps record keys by qualified reference unique in both snapshots; report missing/ambiguous starts instead of guessing. Failed reload retains old snapshot/start state.
- Directed scope: challenge `next`, shown prerequisite-to-dependent skill arcs, authored Relationships arrows, authored MDA arrows. MDA query direction does not reverse source arrows or select analysis starts. **Skill path domination is not AND prerequisite necessity**: every declared prerequisite remains required; one path does not establish mastery or an alternative method.
- Matrix/resource scope: undirected connections through shown derived cells/typed incidences. Necessary connections do not establish directional resource flow, composed outcomes, physical routes or ingredient requirements. Resource routes remain incomplete.
- `D<number>` counts other reachable nodes whose every path from starts includes this node; self excluded. `D-` means unreachable; `D?` means unavailable. Directed `J<number>` counts distinct reachable incoming neighbors when at least two exist; join/loop candidate, not proven reconvergence. Parallel declarations/self loops do not inflate `J`.
- Select a node: `!` marks its strict mandatory ancestors; pink underlays show one supporting path. Support links are **not all mandatory**. Keep original alternatives, arrows, colors and dashed source styles. Inspect **Mandatory-path evidence** for ancestor chain, support links and view-specific provenance. Derived matrix links remain explicitly derived. Resource sidebar shows last four support links; cimgui retains last 64. Whole path/ancestor counts and graph markers are not truncated to display limits.
- Results concern paths from starts to each reachable target, not paths to a chosen exit. Preserve cycles; require dominator fixed point. Unreachable nodes are not assigned zero-dominance proofs. Missing declarations, incomplete references and filtered paths can change wider conclusions; local shown-graph results do not certify source completeness. No traffic, importance, teaching or balance claim.

**Compare support / bypass**, default off, nominates nonmandatory nodes on selected DFS support path, nearest target first. cimgui lists first 64 candidates; Resource **Next support-path candidate** cycles all candidates. No candidate on this path does not establish no alternatives elsewhere. Witness traversal follows projected link order; no shortest-support-path claim.

- Cyan underlays show another path to same target avoiding nominated `C`, under identical fixed starts/scope. Shared segments retain both cyan outer and pink inner bands beneath source styles. `C` means avoidable on these shown paths, not an optional prerequisite, ingredient or gameplay method.
- `F` marks witness fork; `R` marks rejoin after `C`. Branch segments are internally vertex-disjoint between F/R. Cyclic graphs can order common nodes differently; require R to follow F on both paths. Separate starts use disclosed virtual fork, never an authored fork marker. This inspects two concrete witnesses, not every reconvergence.
- Expand **Mandatory-path evidence** for both paths and separate source declarations. Full edge markers/counts remain exact; link text retains last 64 links in cimgui or last four per path in Resources. Candidate nomination resets with target/analysis recomputation; camera changes preserve it.

Graphics-free `gd_graph_paths.c` reuses cached topology and its own DFS forest for support paths. No allocation or duplicate route-opportunity BFS. `gd_graph_paths(analysis, roots, undirected, work_limit)` requires prior `gd_graph_analyze`; work limit is 0–16,777,216 charged DFS arcs, node visits, predecessor scans and parent walks. Fixed-size indexing and selection explanations remain separately bounded by node/link arrays. Exhaustion makes proofs unavailable; never expose intermediate dominators as final. Complete local results remain available when shared-neighbor discovery alone is partial. Cache includes normalized starts, direction and limit; camera changes reuse it. Selection result borrows analysis-owned storage until another selection/path recomputation or projection invalidation. `gd_graph_path_bypass(analysis, selected, candidate, work_limit)` accepts a support-path candidate (`-1` uses current nomination). Separate bounded vertex-avoiding BFS reuses scratch without mutating original DFS parents, dominators or discovery work. Cost `O(nodes + links)`; no allocation or duplicate two-state route-opportunity engine. Query cache includes ordered target/candidate and its own limit; budget exhaustion exposes no partial witness. No new headless CLI command. Broader reconvergence discovery/manual acceptance remain pending.

## Diagnostics browser

Shared **Diagnostics** panel in matrix graph, Skills/Challenges/Relationships explorer, and MDA explorer. Plain raylib matrix table retains existing error display; initial load failures remain CLI errors, not an apparently empty graph.

- Browse existing domain findings/questions, loaded operation errors, and latest rejected-reload errors. No new schema reader, validation rules, repair, severity ranking, or design verdict.
- Search code, message, qualified record, field, referenced target, or source path. Search is plain substring, ASCII-case-insensitive; non-ASCII bytes match exactly. Source bodies are not searched.
- Filter by kind or **Selected record only**. No selected record means no matches under that filter. Filters never change analysis, MDA query, route witnesses, or drafts.
- Each entry keeps available code/message, field/reference, source path/line, and loaded record identity. Line 0 means unavailable. Omitted counts remain explicit; no matches is not completeness evidence.
- **Inspect loaded record** navigates source. Rejected-input entries say **Inspect last-good loaded record**: failed candidate bytes never replace current model. Inspect collapses browser to reveal source; filters remain. **Copy retained source path** copies path only, never opens editor or writes source.
- Error paths navigate only on an exact, unique loaded-path match. Unmapped, ambiguous, or shortened paths never invent record identity. Findings without a valid record index remain visible without navigation.
- Failed reload opens browser and retains all available producer errors before rejected model is freed. Latest failure replaces previous failure; successful reload clears rejected errors. No error history or persistent store. Capture allocation failure is explicit; previous good graph/draft remains.
- Retained reload snapshot owns one allocator-backed block, at most 128 entries. Code capped at 128 bytes; message/path at 4095 bytes each. UTF-8 shortening is marked. Producer omission counters remain distinct. No borrowed rejected-model strings survive destruction.

Graphics-free `gd_diagnostics.c/.h` owns metadata projection, filtering, and bounded error capture. Shared C99 `gd_diagnostics_ui.c/.h` renders existing facts through cimgui; no new generic CLI inspection/validation command. Mouse-driven filter/navigation/copy acceptance remains final-gate.

## Skill prerequisites and ranks

```powershell
$skillAtoms = 'skill-atoms=skills/game-design-workbench/examples/crate-puzzle/game-design/skill-atoms'
./skills/game-design-workbench/native/build/gd.exe skill-atoms --skill-atoms $skillAtoms --json
./skills/game-design-workbench/native/build/gd.exe skill-atoms --skill-atoms $skillAtoms --json --fields skills,findings
./skills/game-design-workbench/native/build/gd-view.exe view skill-atoms --skill-atoms $skillAtoms
./skills/game-design-workbench/native/build/gd-view.exe view skill-atoms --skill-atoms $skillAtoms --layout rank --direction right
```

- Exactly one explicit `--skill-atoms ALIAS=FOLDER`; optional repeated `--related` for source navigation. Alias must match authored prerequisite references. No matrix collections needed. Graph build is required for skill views.
- Include all declared skills/statuses, including proposed/rejected and isolated skills. Prerequisite arrays are AND sets. Preserve raw relationships; deduplicate resolved prerequisite pairs for analysis/edges. Mechanic/context links do not create prerequisites or join groups. `components` means weak prerequisite groups with `link_count`, not active matrix groups.
- Arrows point prerequisite → dependent. Default `--layout prerequisites` uses top-to-bottom chain ordering. `--direction right` rotates to left-to-right. Settling/dragging can move nodes across branches, not along ordering axis; this constraint holds while paused too. Sort name or declared link count; pause, fit, source/link inspection, clipboard path, and explicit refresh work as in matrix graph.
- `authored_rank` remains null or exact signed integer. `prerequisite_depth` is longest complete prerequisite path from a declared root, not rank. `minimum_rank` is maximum prerequisite display rank when any finite anchor exists; `display_rank` is maximum of that minimum and authored rank. Same-rank prerequisites are valid. Unranked ancestry without a finite rank anchor remains null, not invented rank zero.
- Rank layout orders finite display-rank bands, then prerequisite order within a band. Compress numeric gaps; pixel distance does not measure rank difference. Unranked prefixes precede finite bands; unknown ancestry follows in separate bands. Source ranks never change with geometry, layout, sorting, or refresh.
- Cycles retain every link and show red conflict markers/arrows, including self-loops. Cycle members share a conflict band; known links entering/leaving a cycle still preserve relative order. `layout_layer` describes relative prerequisite bands only, not current pixel position or compressed rank-view band. It is not authored rank or a claim that cyclic prerequisites are solvable.
- Missing, ambiguous, or wrong-role prerequisites remain raw references plus sourced findings. Do not turn these nodes into independent roots. Affected nodes/descendants have `placement_complete:false`; absolute depth, derived ranks, and unassessable `rank_conflict` stay null. Known relative links still order correctly. Descendants of cycles are marked incomplete, not incorrectly labelled cyclic themselves.
- `data.findings` exposes deterministic rank conflicts, cycles, unresolved targets, and unknown downstream placement with record/path/field. `coverage.complete` concerns successful source loading; separate `coverage.placement_complete` reports whether all supplied prerequisite ancestry can be placed. Both remain scope-limited, not proof of learning or schema validity. Findings are capped at 128 with explicit `findings_omitted`; all per-node flags remain available.

Example: six skills, five distinct prerequisite links, one group. `coordinate` retains authored rank 2, prerequisite depth 3, minimum/display rank 3, and `skill.rank_conflict`. `plan-push` retains null authored rank with display rank 1. Late root `read-switch` keeps rank 3 and depth 0. CLI source analysis and viewer draft analysis consume the same computation. Declared challenge/route opportunities use the headless command below; observed learning judgments remain outside native computation.

### Skill understanding, combinations and unlocks

```powershell
$game = 'skills/game-design-workbench/examples/little-plumber-guy-nes/game-design'
./skills/game-design-workbench/native/build/gd.exe skill-atoms `
  --skill-atoms "skill-atoms=$game/skill-atoms" --from skill-atoms/stomp-walker `
  --known skill-atoms/stomp-walker --known skill-atoms/land-on-target --json --fields exploration
./skills/game-design-workbench/native/build/gd-view.exe view skill-atoms `
  --skill-atoms "skill-atoms=$game/skill-atoms" --from skill-atoms/stomp-walker `
  --known skill-atoms/stomp-walker --known skill-atoms/land-on-target
```

`exploration` is additive to skill CLI data and supports existing `--fields` selection. Repeated `--known` takes at most 64 qualified references. `--from` selects one loaded skill; missing, wrong-role or ambiguous selectors fail with exit 2. With no root, report all eligibility rows but do not compute comparison candidates or rooted chains. Text and JSON describe the same assumptions. Empty input returns an empty exploration.

- `no_declared_prerequisites`: no prerequisite links, not already learned.
- `needs_more`: some direct prerequisites are not explicitly supplied.
- `all_declared_prerequisites_supplied`: every declared AND prerequisite is supplied, not observed mastery or automatic learning.
- `unresolved`: prerequisite ancestry is incomplete or cyclic. Supplied counts cannot establish eligibility.

Selecting an atom as known does **not** add its prerequisites. Newly unlocked atoms do not become known. `known_assumption`, direct `missing` references and counts stay separate from eligibility. Status is preserved in ordinary skill records; this query includes all statuses and does not adopt proposed/rejected content. Rooted ancestor/descendant flags describe structural chains, not an executable acquisition plan.

Comparison candidates share exact prerequisite or mechanic references with the selected root. Each evidence row retains shared reference, field and both source lines. Unloaded shared mechanic references are authored comparison evidence, not proof that definitions exist. Repeated authored links may produce repeated evidence. No similarity score, inferred contrast, learning transfer, new taxonomy or OR prerequisite groups.

Shared caller-owned `gd_skill_unlocks.h/.c` computes CLI/viewer results without source/rank mutation. Limits: 16,777,216 charged query work units; 128 comparison evidence rows. Exploration `complete:false` reports exhausted work or omitted evidence independently of source-loading coverage. Existing 4,096-record / 32,768-link bounds apply. No all-pairs query or recursion.

Existing Skills explorer adds hypothetical-known checkboxes, eligibility/missing counts, navigable comparison candidates and chain markers. Teal dot marks supplied assumptions; gold ring marks selected skill's ancestor/descendant chains. Draft edits/undo recompute results; mode switches retain assumptions. Successful reload remaps assumptions by canonical reference and drops removed skills; unsafe reload retains last-good state. Closing emits only existing draft-change report, not learning state. Font, density and presentation usefulness need human review.

Example: existing `stomp-walker` now describes shared above-impact understanding. Proposed `jump-stomp` combines it with `land-on-target`; proposed `fall-stomp` combines it with `control-fall`. Both reference the same walker-stomp rule. First-walker offers the jump combination; no challenge is claimed to teach controlled falling. New falling atoms retain null rank. Game rules remain unchanged.

### Switch Skills / Challenges in one explorer

```powershell
./skills/game-design-workbench/native/build/gd-view.exe view skill-atoms `
  --skill-atoms $skillAtoms `
  --challenges challenges=skills/game-design-workbench/examples/crate-puzzle/game-design/challenges `
  --mode challenges
```

`--challenges` is optional for existing skill-only use; no folder discovery. `--mode skills|challenges|relationships` chooses initial mode (default skills). Use **Explore** selector in same window:

- **Skills:** existing prerequisite/rank layouts and draft edits. Direct challenge-role links and indirect prerequisite demands navigate to relevant challenge.
- **Challenges:** authored `next` graph, entry markers, raw skill roles, shared opportunity checks, supported/unsupported witnesses, source/status/field lines. Skill navigation switches back to prerequisite view. Browse/filter current mode or select a finding to navigate.
- Stable bands show shortest declared entry-hop distance, not rank, physical reachability, or teaching order. Challenges do not force-settle; lateral dragging remains transient. Cycles, duplicates, statuses, and unreachable shelf remain. Only resolved `next` links can be drawn; raw unresolved references stay in source/findings. Role links are not invented route edges.
- Route checks use current in-memory skill prerequisites, with same `gd_analyze_routes` as headless `scaffolding`. Draft edits/undo recompute both analyses. Findings/unknowns/omission limits remain explicit. Gold route highlight covers retained witness suffix only when truncated.
- Switching preserves draft/history, selected source per mode, and report destination; transient placement rearranges and camera fits. Invalid pending rank text blocks navigation rather than disappearing. Challenge routes stay read-only. Finish from either mode emits same skill-change proposal; dirty refresh guard applies in both. Failed reload retains last-good model/query/view.

Manual final-gate checks: edit skill prerequisite/rank, switch to Challenges, inspect recomputed demand/witness, follow skill back, verify draft and Ctrl+Z survive; test invalid rank text blocks switch, finish in Challenges mode, and dirty/rejected/valid refresh. No agent pointer/global-keyboard automation. Code tests cover alternating layouts with unchanged draft history, prerequisite propagation/undo, raw route cycles/duplicates/unreachable nodes, and exact layout capacities; they do not establish mouse-interaction acceptance.

## Challenge route opportunities

```powershell
./skills/game-design-workbench/native/build/gd.exe scaffolding `
  --skill-atoms skill-atoms=skills/game-design-workbench/examples/crate-puzzle/game-design/skill-atoms `
  --challenges challenges=skills/game-design-workbench/examples/crate-puzzle/game-design/challenges `
  --json --fields checks,witnesses,findings
```

Exactly one explicit skills/challenges role each; optional repeatable `--related` context. Headless command has no GUI, matrix-role mixing, conventional aliases, schema interpreter, source writes, or rank-based route inference. Its analysis also feeds the combined explorer above. All statuses and raw duplicate declarations remain visible.

- `entry: true` marks start challenges. `next` links are alternative declared transitions. Missing/null/empty lists declare no links; terminal challenges do not certify full physical-game reachability. Consumed `entry` requires actual true/false boolean, including explicit `!!bool`; strings, null, numbers, yes/no, aliases, and duplicate consumed fields fail.
- Only earlier `introduces`/`practises` count as support opportunities. `requires`/`tests_mastery` are demands, not teaching. Expand demands through transitive AND prerequisites; retain one check per challenge/skill plus direct/indirect flags and one originating demand. Original role edges preserve all demands and duplicate declarations.
- Prior means strictly before arrival at challenge. Analyze all finite entry-to-visit walk prefixes, including revisits. A later supported revisit does not erase unsupported first arrival. Introduction/demand overlap produces informational `scaffolding.single_shot`; it does not mandate earlier teaching or prove inability.
- Per-skill prior coverage: `all_routes`, `some_routes`, `absent`, or `unknown`. Crate example has 4 challenges, 1 entry, 10 demand/prerequisite checks: combined-room planning support absent, switch support on some routes, and coordinate introduced/demanded together. Rank conflict remains a separate skill finding.
- Missing entries in nonempty graph, unresolved/wrong-role `next`, or unreachable demand challenge prevent full opportunity claims. Broken route refs conservatively make every check unknown, even in otherwise intact fragments. `supported_route`/`unsupported_route` mean a witness exists in resolved graph; false is not proof of absence when coverage is unknown. Challenge `reachable` is null when negative reachability cannot be established.
- `coverage.complete` means loading/computation completed, not full design/scaffolding validity. `routes_complete` covers declared entry/next structure; `requirements_complete` also exposes broken skill-role references and incomplete/cyclic prerequisite ancestry. Check coverage concerns named skill only; it is not overall readiness. Unencoded gates/conditions, physical traversal, player understanding, and actual mastery remain unknown.

JSON `data`: `challenges`, `skills`, `checks`, `witnesses`, `findings`. Record summaries hold IDs/statuses/source paths and raw relationship references/lines. Checks hold stable result index, challenge/skill, originating demand/line, direct role flags, prerequisite flag, current introduction, coverage, and witness-existence flags. Witnesses refer to check index and record-reference paths. Field selection preserves envelope/errors; selecting witnesses/findings includes respective omitted count.

Example bypass for combined-room route-switch check: `["challenges/move-demo", "challenges/push-room", "challenges/combined-room"]`; supported path also passes `switch-practice`. These are declared graph witnesses, not observed player routes. Same scope is explicit in output. Opportunity findings are `info`; broken domain references retain error findings, not automatic repair. Completed analyses/findings exit 0; argument/load/allocation/check-limit/output failures exit 2 with partial JSON where possible. No threshold flag.

`gd_routes.c` returns allocator-owned `GD_Routes`, borrowing model identities until result destruction; keep model immutable during that lifetime. Per-challenge iterative prerequisite traversal and per-skill two-state BFS avoid path enumeration, recursion, and a persistent transitive closure. Core demand/reachability work is bounded by `(challenges + demanded skills) * (records + relationships)`; each retained witness also traverses at most twice record count to reconstruct its path. At most 32,768 checks; exhaustion marks partial result/error rather than dropping demands silently. At most 128 witnesses; overflow increments `witnesses_omitted`. Paths retain at most 256 trailing nodes, with `prefix_truncated` and original `full_length`; never present a suffix as a complete route. Finding/witness caps do not change computed check coverage. No new dependencies or persistent projection.

## Control values and exports

Shared C99 resolver, export set selection, and read-only import planning are implemented. [Tuning workflow](../workflows/tune-controls.md) applies native-validated values through public Attendant; no new source writer, lifecycle wrapper, or JavaScript evaluator. Inspection defaults to all statuses; exports default to accepted controls. Explicit filters can select drafts without changing their status. Export does not itself adopt design decisions.

```powershell
$controls = @('--control-vars', 'control-vars=./examples/crate-puzzle/game-design/control-vars')
./native/build/gd.exe controls @controls --json
./native/build/gd.exe controls @controls --from control-vars/push-duration --set control-vars/walk-speed=4 --json
./native/build/gd.exe controls export @controls --as json
./native/build/gd.exe controls export @controls --as csv
./native/build/gd.exe controls export @controls --as ini
./native/build/gd.exe controls export @controls --status all --exclude push-cost --as json
./native/build/gd.exe controls export @controls --include walk-duration --include push-duration --as json
./native/build/gd.exe controls export @controls --status accepted --status proposed --exclude push-cost --as csv
./native/build/gd.exe controls import @controls --input ./game-tweaks.json --json
./native/build/gd.exe controls import @controls --input ./game-tweaks.csv --input-format csv --fields items
```

Run from skill folder. No collection defaults. Optional `--related` loads context, not additional control definitions. `--from` selects one control and required dependencies. Unrelated unspecified controls do not block that focused resolution. `--fields controls` keeps inspection envelope/errors. Explicit `--format text` selects readable inspection.

- Selection: repeat `--status STATUS`, `--include NAME`, and `--exclude NAME`; `--status all` must stand alone among status filters. Names may be bare stable keys or qualified references. Exclusions win. Explicit includes or `--from` bypass the implicit accepted-only export default unless an explicit status filter is supplied; explicit status filters intersect included names. Unknown/ambiguous names fail. At most 64 entries per filter kind, subject to CLI argument bound.
- Name filters select exported keys, not dependency availability. A named derived export may read upstream controls without exporting them. In status-filtered/accepted-default mode, dependencies must also satisfy status policy; report an excluded-status dependency rather than silently promoting it. Inspection retains dependency records and marks output roots with `requested`.
- Types: `integer`, `number` (binary64), `boolean`, `enum`. Preserve `0`, `false`, empty allowed enum strings, and exact allowed-value spelling. No string/numeric/boolean coercion.
- Exactly one value authority: typed `value`, or nonempty `expression` with `value: null`. Unknown/unspecified values cannot produce resolved exports.
- Numeric controls require explicit unit text; quoted `"1"` means dimensionless. Literal values/overrides carry declared units. Formula constants are dimensionless unless annotated, e.g. `0.2[s]`.
- Formula grammar: decimal/scientific constants, `{ALIAS/NAME}`, unary `+ -`, binary `+ - * /`, parentheses; normal precedence. Functions: `min(a,b)`, `max(a,b)`, `clamp(value,lower,upper)`, `abs`, `floor`, `ceil`, `round`. Round ties away from zero. Dollar-prefixed references are rejected with `controls.reference_syntax`; migrate live formulas to braces without `$`. Frozen experiment evidence keeps its historical bytes. No assignments, comparisons, arbitrary calls, or code evaluation.
- Unit grammar: case-sensitive ASCII symbols, `*`, `/`, signed integer powers. `m/s` equals `m*s^-1`; `ms` is not `s`. No implicit scale registry/conversions. Explicit conversion example: `{config/duration-ms} * 0.001[s/ms]`. Unit parentheses are not supported.
- Addition/subtraction/min/max/clamp require equal canonical units; multiplication/division combine units. Rounding/absolute value preserve units. Bounds apply in declared units. Invalid bounds, zero division, nonfinite results, and underflow to zero fail. Finite nonzero subnormal values are supported.
- Integers stay within `[-9007199254740991,9007199254740991]`; fractional literal text/results fail without implicit truncation. Explicit rounding functions may produce integers. Integer formulas also reject intermediate arithmetic outside that safe range. Arithmetic uses binary64, not arbitrary-precision rational math.
- `--set ALIAS/NAME=VALUE` is temporary. Numeric text and lowercase booleans are parsed strictly; enum values are raw argument text, not JSON-quoted strings. Override uses existing type/unit/constraints; never modifies baseline or formula. Overriding a formula temporarily replaces its computation, so unused original dependencies are not required.
- Missing/ambiguous/non-control references, dependency cycles, invalid operands, and limit/allocation failures have structured source-backed errors. Cycle diagnostics distinguish unresolved dependency regions without claiming every blocked node belongs to the cycle itself.

Exports use the same result, sorted by stable control name. JSON is an object with typed scalar values. CSV has `key,type,value,unit` columns with conventional quoting; formula-like enum text stays unchanged and warns on stderr. Never auto-open CSV. INI dialect has one `[controls]` section, numeric/boolean literals, and JSON-quoted/escaped enum strings. These are resolved artifacts, not formula definitions. Export stdout is raw selected format; `--json`, `--format`, and `--fields` do not apply to exports. Failed resolution returns exit 2 and JSON error envelope, never partial CSV/INI/game JSON.

`gd_controls.c` owns allocator-backed results borrowing immutable model metadata and override enum strings. Bounded shunting-yard compilation and iterative dependency resolution avoid recursion/code execution. Limits: 4,096 bytes and 256 bytecode tokens per formula, 128 parser/evaluation stack entries, 32,768 total tokens; 256-byte units, 16 canonical terms, 32-byte symbols, exponent magnitude 32; 64 enum choices per control. CLI accepts at most 64 overrides, also subject to existing 128-argument Windows bound. Errors never silently truncate. Usual record/file/text limits still apply.

`controls import` reads an explicit UTF-8 file (at most 1 MiB); `--input-format json|csv|ini` defaults JSON. Same status/name filters choose admitted keys. `--fields changes,derived,items` selects large sections while preserving ready/applied flags, digest, coverage, and errors. No `--set` during import. Return a read-only update plan, never source edits. All invalid batches have `ready:false` and empty `items`; successful no-op batches have an empty item array and need no Attendant update.

JSON supports a flat object of typed scalar values, full JSON escapes/surrogate pairs, and no duplicate keys, nulls, arrays, or nested objects. CSV requires the exact four-column export header; quoted multiline enum text is supported. INI accepts one `[controls]` section, blank lines, whole-line `;`/`#` comments, and the documented literal/quoted-string dialect, not arbitrary INI. Unknown/out-of-scope keys, duplicate keys, changed CSV types/units, invalid enum choices, unsafe integers, and constraint failures block the batch.

Compare edited values with a fresh current export, without old baseline snapshots. Update literals only. Preserve omitted controls and all definition metadata. Unchanged derived inputs are not direct tweaks; a changed derived input must match recomputed candidate output or the whole batch fails. Never replace a formula or infer upstream changes. Validate status-admitted declared dependents of changed literals even when excluded from export, so narrow imports cannot silently break affected formulas/bounds. Reverse dependency scanning is bounded at 32,768 known references.

`gd_control_import.c` owns bounded flat-format parsing, typed merge planning, and affected-scope validation; it borrows immutable model metadata and owns current/candidate results plus decoded input text. It never adds another authority. Recheck source and planned values before authorized public-Attendant application. Public Attendant replacements are individually atomic, not a cross-file transaction; failures can leave partial replacement and must not trigger silent retry/rollback.

## Rule declarations — Review 01 in progress

`gd rules --interactions ALIAS=FOLDER --mechanics ALIAS=FOLDER` reads authoritative `gd-rule` body blocks. Supply either or both source options. Optional `--axes`, `--nouns`, `--verbs`, `--control-vars`, `--related` use ordinary explicit bindings. JSON/text, pipe/agent defaults, `--fields records,cases,axes,actors,findings` and exit 0/2 follow existing CLI conventions. No new runtime dependency or source writer.

[Rule reader contract and limits](rules.md) covers declaration scope; [typed axes and explicit applicability](axes.md) covers `--axes`, actor references and readiness flags. Native type/unit/bound validation and noun/axis binding resolution work. Both examples now have axis collections/contracts and explicit noun inventories. State effects and quantity targets now bind through declared actor axes; ambiguous/type-incompatible/closed-missing names get sourced diagnostics. Open or incomplete names remain undetermined. [State predicate logic](conditions.md) now compiles grouped AND/OR/NOT and explicit ANY, retaining omitted/unknown scope and independent local skips. Read-only C99 evaluation consumes explicit predicate facts; CLI declaration inspection does not invent truth. Optional `--case` with explicit assumptions returns three-valued conditional applicability. [Ordered context](context.md) binds explicit local endpoints and rule `context_axes` vocabulary without actor-state leakage or scene inference. [Numeric guards and quantity primitives](quantities.md) now share existing control math, typed lookups, immutable starting facts, consumer constraints and diagnostic bounds. [Conditional outcomes and bounded comparisons](outcomes.md) now support `--case`, typed `--assume`, ordered `--context`/`--context-not`, explicit mapped calls, simultaneous writes and whole-case OTHER. `--analyze` compares bounded declared enum products; `--fields analysis` selects results. Both examples fully migrated: 85 rule records / 180 cases. Unknown geometry, timing, event identity, numeric domains and design gaps remain explicit. A successful declaration scan does not validate game rules.

## MDA declared traces and questions

Graph-enabled build above supplies both interfaces. From repository root:

```powershell
$mda = @(
  '--mechanics', 'mechanics=skills/game-design-workbench/examples/crate-puzzle/game-design/mechanics',
  '--systems', 'systems=skills/game-design-workbench/examples/crate-puzzle/game-design/systems',
  '--dynamics', 'dynamics=skills/game-design-workbench/examples/crate-puzzle/game-design/dynamics',
  '--aesthetics', 'aesthetics=skills/game-design-workbench/examples/crate-puzzle/game-design/aesthetics'
)
./skills/game-design-workbench/native/build/gd.exe mda @mda --json
./skills/game-design-workbench/native/build/gd.exe mda @mda `
  --from mechanics/crate-pushing --direction forward --json --fields nodes,links,paths
./skills/game-design-workbench/native/build/gd-view.exe view mda @mda `
  --from mechanics/crate-pushing --direction forward
```

Require exactly one mechanics, dynamics, and aesthetics mapping. Systems mapping is optional; repeatable `--related` loads source context. Aliases need not match role names. No inferred folders, status filtering, source writes, or framework sweep. Systems are constructed arrangements, not dynamics or a mandatory mechanics/dynamics bridge. Aesthetic records remain local aspects, not the whole player story.

### Trace and output contract

- Core authored references: `systems.mechanics`, `dynamics.mechanics`, `dynamics.systems`, `aesthetics.dynamics`. Mechanic nouns/verbs/control-vars/resources and dynamic/aesthetic experiments remain raw source context, not invented core edges.
- `--from ALIAS/NAME` selects root; absent root selects all supplied MDA records. Missing/ambiguous/non-MDA root fails, never silently selects all.
- `--direction forward` traverses reverse references toward aesthetics; `backward` follows authored references toward basis; default `both` explores connected associations in either direction. Traversal never flips stored or drawn arrows.
- JSON data keys: `nodes`, `links`, `paths`, `gaps`, `findings`; `--fields` selects keys while retaining coverage/errors. Text and pipe/agent defaults follow other commands. Nodes retain IDs, status, source path, raw relationships, basis state, and aesthetic-trace state. Node basis/trace facts concern whole supplied graph; query selects returned records, not permission to ignore supporting context.
- Links preserve raw duplicates, source, field, reference, target/resolution, source line/path, endpoint statuses, and endpoint-in-scope flags. Include selected and boundary declarations, not only a chosen witness. Isolates remain nodes. A boundary endpoint need not appear in selected `nodes`.
- Rooted paths give one shortest witness per reached node, not every possible chain. `steps[].link` names stable link index; `reversed` means traversal opposed authored reference. Unrooted query returns empty `paths`. Retain at most 64 trailing nodes per witness with original `full_length` and `prefix_truncated`; do not present suffix as whole path. All alternative declarations remain in `links`.

Crate fixture: **9 nodes, 6 core declarations, 3 isolated-mechanic trace questions**. Forward from crate-pushing reaches **4 nodes**. Shortest witness is crate-pushing, careful-positioning, deliberate-tension; separate path through puzzle-rules remains in links. Backward from deliberate-tension reaches 6 nodes, including three mechanics via puzzle-rules. These are declared associations, not causal or observed experiential claims.

### Questions, unknowns, and failures

`basis` describes immediate core references, not transitive mechanical grounding or evidence: `not_applicable` for mechanics; otherwise `none_declared`, `resolved`, or `incomplete`. `aesthetic_trace`: `declared_trace`, `none_declared`, or `unknown`. Positive trace names one example aesthetic, not guaranteed emotion or complete alternative coverage.

No declared basis/path yields informational review question, not automatic defect. Unresolved core references conservatively make every negative aesthetic-trace claim unknown across supplied graph. Unconfigured alias means `unloaded`, not proven missing record. Mapped missing, ambiguous, and wrong-role targets remain separate findings; unresolved mapped target under failed load is `unavailable`, not a proven missing record; invalid links never become traversable edges. Loaded related record does not acquire systems role merely because its alias is `systems`. Ancillary context alone does not poison core trace completeness.

`coverage.complete` concerns operation/load completion, not design validity. `query_references_complete` concerns selected source declarations; `graph_references_complete` concerns whole supplied core graph. Completed analysis with informational/broken-reference findings exits 0. Argument/load/allocation/output failures exit 2 with explicit errors and partial parsed records where possible. Partial load prevents negative trace claims. At most 128 findings; `findings_omitted` is explicit and does not change node states or gaps.

`gd_mda.c` owns shared analysis/query/path computation and graphics-free layout adapter. Allocator-owned `GD_Mda` borrows immutable model text/identities. Core indexing, multi-source aesthetic reachability, and each focused BFS take `O(records + relationships)` work without recursion/path enumeration or persistent closure. Path serialization reconstructs at most 64 nodes per selected record; raw relationship JSON uses existing bounded record/edge scans. No whole-command or large-project rendering-performance claim.

### Read-only explorer

Role bands default; free graph also available. Settling starts paused. Click node/edge to inspect; source panel shows exact Markdown body, identity/status/path, raw references, scoped findings, and shortest witness. Witness buttons and loaded-reference targets navigate source selection. Use selected record as query root, switch direction, or Show all. Gold highlights selected witness; teal retains other queried links. Display-only role/reference/query/witness filters never alter analysis. Text witness retains bounded suffix; graph highlights full parent witness.

Drag changes layout only. Role-band ordering axis remains locked. Right-drag pans; wheel zooms; **F** fits; **F5** or Refresh reloads. Disk polling marks stale without replacing snapshot. Failed reload retains last-good model/query/layout; missing query root rejects reload until root cleared. Close returns no change report or stdout data. This view has no source editor, draft, autosave, output-file option, or authoritative writer.

Manual review: select nodes/edges/findings, follow source/witness buttons, change root/direction, toggle filters/layout/settling, pan/zoom/drag, and inspect boundary/unloaded context. Code tests/screenshots/owned-window F5 do not establish mouse-interaction acceptance. Do not automate user's pointer or global keyboard input.

## Draft editing and JSON handoff

Select a skill node/edge. Edit rank with Enter, Apply, or field deactivation; Clear sets null. Invalid ranks are rejected without changing draft. Prerequisite checkboxes add/remove connections between loaded skills, including self/cyclic links that remain visible as findings. Filter by reference; existing unresolved/wrong-role prerequisites can be removed. Body, metadata, mechanic links, and descendants' authored values stay unchanged. Node dragging remains layout-only.

- Gold `*` and modified count identify directly edited records. Compare rank and prerequisite set against loaded baseline; returning values to baseline clears modified state. Derived movement alone is not an authored edit.
- Ctrl+Z outside text inputs, or Undo button, reverses draft edits to loaded disk sync point. Focused text input keeps its own text undo. Maximum 4,096 edit actions; refuse further edits instead of evicting baseline history. Full undo restores original duplicate declarations/order. No redo, autosave, or crash recovery.
- Disk polling/stale notice is separate. Modified drafts block F5/Refresh; undo to baseline before refreshing, or finish/report first. Clean successful refresh establishes a new disk sync point. No one-click discard/reload or automatic merge.
- Finish button or window close emits one JSON report. Escape does not close skill view. Rank text is checked before finish; invalid input keeps session open. Report output failure keeps draft/window available; choose a new report path and retry. Bounded empty-session output failures exit `2` rather than hang.
- Default stdout is report data only; diagnostics use stderr. Optional `--output FILE.json`, or report-path field in viewer, routes report to a UTF-8 file instead. Destination must end in `.json`; exclusive creation rejects existing files, including source aliases/hard links. No silent overwrite. An empty report has `data.changes: []`.

```powershell
./skills/game-design-workbench/native/build/gd-view.exe view skill-atoms --skill-atoms $skillAtoms --layout rank --json
./skills/game-design-workbench/native/build/gd-view.exe view skill-atoms --skill-atoms $skillAtoms --layout rank --output changes.json
```

Report shape (abbreviated):

```json
{
  "schema_version": 1,
  "ok": true,
  "data": {
    "applied": false,
    "changes": [{
      "reference": "skill-atoms/coordinate",
      "id": "record-id",
      "source_path": ".../coordinate.md",
      "current_disk_available": true,
      "fields": {
        "rank": {"loaded": 2, "current_disk": 5, "edited": 4},
        "prerequisites": {
          "loaded": ["skill-atoms/plan-push", "skill-atoms/route-switch"],
          "current_disk": ["skill-atoms/plan-push", "skill-atoms/route-switch"],
          "edited": ["skill-atoms/plan-push"]
        }
      }
    }],
    "affected": [{"reference": "skill-atoms/coordinate", "authored_rank": 4, "minimum_rank": 1, "display_rank": 4, "placement_complete": true, "cycle": false, "rank_conflict": false}],
    "findings": [],
    "findings_omitted": 0
  },
  "coverage": {"complete": true, "operation": "skill-atoms.draft-changes", "disk_changed_since_load": true},
  "errors": [],
  "errors_omitted": 0
}
```

Only net user-edited fields appear under `changes`; unrelated external changes never become proposed reversions. `affected` contains changed derived skill states, including unchanged authored descendant ranks; findings describe in-memory draft. Current disk is freshly loaded at finish, matched by reference/path/ID. Failed source loading conservatively makes current values unavailable; missing/moved/changed identity is an explicit error, never automatic retargeting. `current_disk_available:false` distinguishes unreadable current values from real null rank or empty prerequisites. Errors retain loaded/edited values in partial report. Disk-change flag is null when current snapshot cannot load.

Report is a proposal, not a transaction/lock, source mutation, or approval to apply. Agent inspects differences, rechecks current source and authority, then uses Attendant or precise authorized source edits. No native source serializer/patch writer. Headless CLI continues reporting disk state, not an unreported GUI draft.

`gd_draft.c` owns bounded skill edits, undo, and report delivery; strings borrow immutable baseline until draft is destroyed. Shared recomputation clears/rebuilds analysis/components after edits. Report reads one additional model at finish; no persistent projection/database or generic CRUD added.

## Reader boundary

Immediate, non-hidden `.md` files only; no recursive collection discovery. Identity is supplied `alias/name`, using `name` or filename fallback, never UUID. Names/aliases are ASCII technical keys, at most 128 bytes. Preserve optional `id`, `desc`, `status`, body, and case `coverage`. Consume `nouns`, `verb`, `context_nouns`, `mechanics`, `control-vars`, `resources`, and `verbs` links on cases/related records. Skill records additionally consume `prerequisites`, `mechanics`, and `rank`. Challenge records consume `entry`, `introduces`, `practises`, `requires`, `tests_mastery`, and `next` for the route consumer, plus context arrays `mechanics`, `interactions`, `nouns`, `verbs`, and `resources`. Resource and related-context records consume scalar `noun`, `initial_control`, and `capacity_control` references. These context/resource edges retain duplicates, field types and source lines; they do not become skill demands, route transitions, quantities or executable dependencies. MDA roles consume only role-specific fields listed in [MDA contract](#trace-and-output-contract), including context-only `experiments`. No other field becomes a native contract.

`gd_index_links(model,index)` builds typed incoming/outgoing adjacency over raw `GD_Edge` indices in declaration order. Caller allocates 288 KiB scratch and rebuilds after changing snapshots or edges, including draft edits/undo/reload. Index does not enlarge each baseline/draft/reload model. Missing/ambiguous targets remain outgoing declarations only; no placeholder identity, inferred edge or semantic cycle interpretation. Existing Focused Relationships projection shows newly consumed edges through its current raw-edge traversal; index is available for subsequent graph-wide analyses.

LibYAML parses frontmatter events. Consumed text/links require text/null or scalar-reference arrays as appropriate; no silent numeric/boolean coercion. Skill rank requires null or an integer scalar with signed decimal spelling in `[-9007199254740991,9007199254740991]`. Numeric strings, floats, aliases, other numeric spellings, and out-of-range ranks fail explicitly; no rounding. Duplicate consumed fields, escaped NUL, aliases in consumed fields, and top-level YAML merges fail explicitly. Other metadata is skipped within depth/event bounds; this is not whole-record validation. Markdown body is UTF-8 without NUL. Source changes during load require explicit retry. Content digest is a change hint, not a cryptographic integrity check or atomic multi-file snapshot.

Operation bounds: 16 collections; 65,536 visited directory entries; 4,096 records; 32,768 relationships and case memberships; 128 retained errors; 4,095 UTF-8 path bytes; 1 MiB/file; 64 KiB/frontmatter; 16 MiB retained text; metadata depth 32 and 65,536 YAML events/file. Limits fail explicitly, not truncate silently. Filesystem/third-party allocations remain outside caller-owned model allocation.

Attendant retains collection discovery, schemas, CRUD, search/query, generic validation, reference hygiene, and projection lifecycle. No native `validate`, `inspect`, generic query, repair, or data projection. Reassess overlap before extending consumed fields; a C99 Attendant port/shared foundation is separate future work, not implied scope.

## Checks and dependencies

```powershell
./skills/game-design-workbench/native/test.ps1
./skills/game-design-workbench/native/test.ps1 -View -RaylibDirectory C:/Dependencies/raylib-5.5_win64_mingw-w64
./skills/game-design-workbench/native/test.ps1 -Graph -RaylibDirectory C:/Dependencies/raylib-5.5_win64_mingw-w64
```

Native checks use copied MIT [base/test headers](vendor/proto-headers/README.md), caller-backed allocation, temporary fixtures, and direct domain APIs. Native domain/layout/draft/route/MDA/control/rule/graph checks: 89/89 under Windows CJIT; changed units also pass strict Zig C99 compilation. Shared-neighbor checks compare 128 projected graphs/masks against an independent adjacency oracle, retain duplicate/self-link evidence without inflated counts, verify inactive/hidden scope and camera-only cache reuse, and exercise pair/record/link capacity plus empty results. Incident-role results/witnesses match independent role-set oracle across 384 graph/mask/lens cases; explicit parallel alternatives satisfy both lenses without inflated counts. Role-only and lens-only changes invalidate cache. A no-match role-search workload exhausts work budget explicitly rather than silently returning a complete empty result. Selected-pair node/link classes match independent role-set oracle across same 384 projection/lens cases, including reversed-side counts. Dedicated checks cover unique versus excluded common identities, mixed direct links, hidden/invalid/self pairs, camera cache, projection invalidation, all 65,536 distinct incident roles and exact local comparison under both discovery limits. Rooted path checks compare 1,536 graph/mask/start/direction cases with independent vertex-removal reachability oracle, including cycles, multiple/hidden starts, inactive edges and support-path direction/provenance. Dedicated checks cover no starts, declared/explicit controls, zero/32-step and pre-fixed-point budget exhaustion/recovery, cache invalidation/camera reuse, 4,096-node chain/64-link suffix, 32,768-link projection despite partial global discovery, full root capacity and unique/missing/ambiguous reload remapping. Paired-path checks cover every nominated candidate in the same 1,536 oracle cases: avoided vertex, edge flags/direction/scope, fixed starts, disjoint witness branches, valid F/R ordering, cache and unchanged dominator/discovery work. Explicit cases cover reversed common-node order in cycles, synthetic multi-entry forks, hidden starts, invalid candidates, zero/one/32-step exhaustion/recovery, 4,096-node witnesses with rejoin outside retained suffix and complete local bypass despite truncated global discovery. Overview checks compare every cell/count/first-source/brush flag across 64 independent masked/active/directed/incidence projections; test parallel kinds, self-links versus within-bin entries, symmetric totals, 4,096-node/32,768-row capacity, empty scope, camera cache, mode reset and exact counts despite partial pair discovery. Shape checks compare 128 small projections against independent boolean-adjacency, insertion-sorted-token and pairwise-interning oracle. Dedicated checks cover relabeling/permutation partitions, direction/role/multiplicity changes, duplicate declarations, 4,096 isolates, all 65,536 distinct incident features, explicit work exhaustion/recovery, camera cache, empty projection and exact shape groups despite truncated pair discovery. Settling regression compares a 64-node cycle over 4,096 display steps: last-120-step RMS reduced from 0.342987 to 0.025314 and maximum from 0.982166 to 0.073985 world units/step. Tick-phase invariance is checked; no full-convergence or universal layout-quality claim. Directed drawing checks verify deterministic crossing reduction on three initial inversions, long-label/card separation through 120 settling steps, unchanged layer/key/link identities, both orientations, unplaced shelf, long/parallel/opposite/self/back geometry, finite points, route-segment hit distances and reserved-label/third-node avoidance in fixture. Existing late-root check now verifies routed false incidence rather than requiring different root columns. Prerequisite-closure checks compare 384 typed/direction/mask projections with independent boolean-matrix transitive-closure oracle, including direct AND deduplication, strict/self-cycle handling, boundary propagation, users, pair/link classes and unchanged discovery work. All completed oracle pair/target combinations additionally check typed BFS witness reachability, direction, scope, suffix continuity and unchanged discovery work. Typed-control checks cover independent foundation/reachability caches, default/retained relation, reverse/incidence modes, peer reset, camera/projection invalidation, empty kind/projection and zero-capacity witness suffix. Dedicated checks cover 4,096-node chain, exact work boundary, partial global/exact local rows, 32,768 duplicate links, hidden/missing projected declarations, unresolved/wrong-role sources, equivalent Skills/Relationships orientation, empty scope and cache/camera invalidation. Diagnostics checks cover metadata/kind/selection filtering, unknown/ambiguous source identities, independent finding arrays, owned error lifetime, UTF-8 shortening, shortened-path non-navigation, omission/overflow bounds, 384 retained items, empty scope, and allocation failure. Focused checks cover explicit one-hop/recenter, incoming/outgoing raw direction, duplicates/self links, statuses, unknown/outside boundaries, revealed isolates, draft edit/undo propagation, empty scope, and 4,096-node/32,768-link capacity. Staged graph suite checks direct root, unknown root failure, Unicode captures, and unchanged proposal. Private-copy relationship F5 checks cover changed scope, malformed source and removed root retaining snapshot, and restore/recovery; mouse-driven expansion/mode acceptance remains final-gate. Combined explorer checks alternate all three modes' layouts while preserving draft history, recompute transitive demands, retain route cycles/duplicate arrows/unreachable nodes, and cover exact 4,096-node/32,768-link layout capacity. Full graph-enabled CLI/capture suite and 5/5 public-Attendant authoring/adoption tests pass; pointer-driven mode switching remains final-gate manual acceptance. MDA adds custom aliases, statuses/isolates, source directions, optional systems, duplicate declarations, alternative/boundary links, context separation, strict fields, partial ambiguity preservation, 64 random graphs/2,304 focused queries against independent shortest-path closure oracle, exact 4,096-node/32,768-link limits, 64-node path suffixes, finding omission, and allocation failure. Shared API JSON matches CLI output exactly. Draft checks cover exact ranks, prerequisite changes/cycles, descendant preservation, undo/history/relationship capacity, three-way JSON handoff, identity/unavailable-disk errors, empty reports, and exclusive-output failure. CLI checks cover matrix/connectivity, cases/dispositions, skill prerequisites/ranks/findings, route opportunity classifications and concrete bypass/practice witnesses, cycles/missing targets, empty results, field selection, text, Windows Unicode/space paths, and structured failure. Core tests cover branches/merges, null/negative/zero/exact-limit ranks, late roots, cycle membership versus affected descendants, maximum-capacity SCC/topological analysis, and constrained dragging/settling in both layouts/directions. Route tests additionally cover duplicate roles/all statuses, transitive demand closure, wrong-role/missing references, missing entries, unreachable fragments, typed fields, 96 small graphs compared with independent product-graph transitive-closure oracle, 32,768-check boundary/overflow, witness/path/finding caps, and result/scratch allocation failures. Optional visible checks capture matrix table/force/columns, skill prerequisite/rank layouts in both directions, and cyclic/incomplete/empty graphs. Graph checks consume explicitly prepared UI dependency; they never compile it implicitly. Fixtures stay in ignored build directory; no test writes shipped game facts.

Live Windows desktop refresh check, using an already-built graph-enabled executable:

```powershell
powershell.exe -NoProfile -File ./skills/game-design-workbench/native/test-refresh.ps1
powershell.exe -NoProfile -File ./skills/game-design-workbench/native/test-refresh.ps1 -Mode mda
```

Desktop helper uses Windows PowerShell 5.1/.NET Framework. Default runs matrix table and skill rank graph against private example copies; `-Mode mda` runs MDA stale/reload checks. Captures before edit, stale detection, explicit F5 refresh, invalid input, failed refresh retaining last-good state, source restoration, and successful recovery. Restores edited bytes in `finally`, verifies SHA-256 against shipped source, and closes only owned child windows. Saves 14 client-area PNGs plus JSON capture manifest under ignored `native/build/refresh-<unique>/`; does not capture entire desktop or touch clipboard. `-Mode matrix|skills|both|mda` selects scope (seven captures per selected view); `-SettleMilliseconds 1500..10000` adjusts wait between stages. Posted F5 messages exercise the live window event path, not physical keyboard hardware. Script checks process/file restoration; inspect PNGs to verify actual UI states—no OCR or automatic UI-text assertions. Matrix defined→unknown and skill authored rank 2→4 changes, stale snapshots, retained state on invalid input, and restored baseline were inspected successfully. MDA's seven captures were inspected: aesthetic dynamics list becomes empty, graph changes from 6 to 5 links/3 to 9 scoped questions; numeric relationship item fails reload without replacing last-good state; restore/F5 returns 6 links/3 questions. All private bytes restored and shipped source SHA-256 unchanged.

Agent checks use code tests, bounded screenshots, and private source edits under running viewer only. No pointer movement, clicking, typing, or global keyboard injection. `test-refresh.ps1` retains targeted F5 messages to owned window; draft interaction modes were removed. It arranges a visible owned window; do not run desktop checks without allowing for that window.

### Manual draft checks

User runs at step end. Use a private example copy for external disk edits. Viewer itself never writes input records.

1. Select `coordinate`; change rank 2→4. Check modified marker, conflict clearing, descendant authored values unchanged.
2. Add self-prerequisite. Check cycle finding and cycle arrow; no automatic repair.
3. Click graph to leave text input, then Ctrl+Z twice. Check connection removed, authored rank 2 restored, modified count/history zero, original conflict returned. Undo button must also work after selecting an edge removed by undo.
4. Reapply edits. Change private disk rank to 5 externally. Check stale flag remains separate from modified flag; F5/Refresh must not discard draft.
5. Finish/close. Parse one JSON report: rank loaded 2/current disk 5/edited 4; net prerequisite edit included; `applied:false`. No authoritative input changes. Unmodified session returns empty changes.
6. Start another session with `--output` pointing at an existing `.json` file. Make an edit; finish. Check error preserves draft/window and existing file bytes. Choose a new Unicode/space `.json` path and retry; check file JSON, empty stdout, successful close.

Interaction acceptance remains pending this user pass; earlier injected-input probes were not reliable enough to replace it. Optional `GD_NATIVE_DRAFT_REPORT` with `GD_ATTENDANT_SKILL` and `GD_NATIVE_EXE` runs actual-report application through public Attendant on another private copy in `test/authoring.test.mjs`.

Dependency pins, subsets, licenses, and SDK hash: [third-party notices](THIRD-PARTY.md). Binaries/SDK cache are not tracked. No package installation required by tests beyond installed CJIT and optional supplied graphics SDK.
