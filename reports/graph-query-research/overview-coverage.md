# Noun, Actor and Effect overview coverage

Implemented; user accepts reviewed presentation and authorizes local checkpoint. Separate Mario review window: PID43872 / session `43872-5373b6fa50aaa5cb`, executable `native/build/overview-coverage-work/gd-view.exe`. Opened Noun interactions Free, paused and framed. No existing viewer replaced or closed. Acceptance covers reviewed presentation, not unperformed every-view, restart, standalone or platform checks.

## Contract

- Noun interactions Free: **Noun-pair links**. Count each retained sparse noun pair once, regardless of declared-case weight. Keep all nouns, including isolates. Self-pair contributes once. Full Grid hides overview and retains expansion preference.
- Actor interaction/usage maps: **Actor map links**. Count each shown pair, noun-usage or own-axis link once. Existing interaction-record weights, concept sizes and evidence stay unchanged. No inferred actors or synthetic links for empty maps.
- Actor/Effect query scenes, Native and Free: **Query associations**. Count each shown scene association, including context/ghost associations. Not unique cases, execution order, causality or final outcomes.
- Use symmetric incidence for these canvases. Mirror across-bin cells without doubling total links. Within-bin diagonal entries need not be self-links.
- Reuse default-open floating panel, collapse/reopen header, independent blue selected-node bin cue and gold cell brush. Collapse retains brush and graph highlights; selection does not reopen panel. F frames primary selection plus actual contributing endpoints. Escape / Clear selection clears inspection without moving camera or applying pending inputs.
- Query node selection may already retarget contributor context. Successful Run/Display/retarget/map changes clear transient slot-based brush, including identical-looking replacement topology. Keep expansion preference. Failed candidates retain scene, brush and cache. Native/Free geometry remains separate from analytical Run.
- Grid/similarity canvases remain excluded. No generic shared-neighbor, path, foundation, reachability or shape layers added to Noun/Actor/Effect canvases.

## Implementation

- `gd_graph_analyze_overview()` copies bounded retained topology into existing view-owned cache. O(nodes + links); no sorting, pair discovery, query evaluator, solver call or new allocation. Bounds remain 4096 nodes / 32768 links. Explicit `overview_only` cache mode prevents accidental reuse as full graph analysis.
- Overview counts use existing shared binning/brush code. Labels identify metric rather than borrowing noun case counts or Actor record weights.
- Shared overlay draws only actual selected links/endpoints, using displayed pose for drawing. Camera framing remains camera-only; raw solver/IPC coordinates stay unchanged.
- Noun source inspection resolves layout edge key to retained noun pair, then existing declared-case membership. Actor/Effect sources use existing typed scene-edge helpers. No Resource/model-edge fallback.
- Query edge selection stores scene-edge slot, not metadata source key. Shared framing now respects that distinction.
- Sidebar source evidence keeps 64-link cap, scoped ImGui IDs and existing case paging. Out-of-range overview bins no longer index sidebar identity ranges.
- IPC `view.analyze` prepares overview-only cache without Run. `result --form layers` supports `nodes,links,overview` only on these canvases; omitted fields default to those three. Reject unrelated fields. Omit unevaluated neighbor/shared counts. Overview reports `metric`, `link_count`, `selected_count`; inspect reports overview availability independently of generic layers.
- Windows cache size: 10,519,584 bytes, including 77,992-byte overview. Added cache-mode flag changes owner size by 8 bytes after alignment. No extra owner allocation.

## Checks

Artifacts: `skills/game-design-workbench/native/build/overview-coverage-work/`.

- Warning-free CJIT production graph, CPU adapter and core builds.
- Core: 148/148. Snapshot checks cover parallel links, self-pair, isolate, hidden identity, symmetric counts, unchanged-pose cache reuse, full-analysis mode transition and empty scope.
- Focused Node: 9/9 across current-view Find, shared UX and production IPC on both examples. Source hashes unchanged.
- New shared-UX checks exercise eight scenarios per example: Noun Free; Actor interaction map; Actor usage Native/Free; stacked Actor query Native/Free; Effect Native/Free. Check structural totals, empty Actor map, no generic layers/work, unchanged raw geometry/camera/pending/applied inputs, blue bin lookup, retained collapsed brush, F/clear, query-edge framing, failed replacement retention and successful replacement invalidation. Exercise actual IPC overview byte page and rejection of unrelated `shared` field.
- Deliberate deep presentation run: Crate passes; Mario fails existing noun Neighborhood centroid-drift check: **1.819844**, limit **1**, over final 256 of 4096 steps. Same failure and all reported geometry values reproduced with prior `overview-node-cue-work/test-explorer-view.exe`. No solver/threshold/timeout change made. Mario deep run stops before later presentation assertions; do not claim full deep pass.
- Live review IPC confirms Noun Free overview: 26 identities / 26 bins / 59 Noun-pair links; full result decoded as JSON, complete=true. Discovered session/control token before guarded Pause and F; both mutations succeeded. No query Run.
- User reports review looks good and requests commit. No individual pointer checklist or every-mode acceptance inferred. Layout research, restart/standalone acceptance and final compiler/platform matrix not run. No general GPU/font/readability or convergence claim.

## Comments

- Prior visibility/node-cue reports describe original six-view checkpoint. This report extends coverage without changing their reviewed expansion/brush policy.
