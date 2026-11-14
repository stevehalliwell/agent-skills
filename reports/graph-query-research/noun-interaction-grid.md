# Noun classification and noun interaction grid

## Result

Distinguish actor, non-actor and unclassified nouns wherever UI presents noun identity. Add noun × noun interaction grid; retain existing noun × verb grid as Verb usage.

User chose **distinct declared cases** as noun-interaction cell count. Actor interaction map retains its separately agreed distinct-record weight; do not silently change it.

## Classification

- Use existing canonical noun `is_actor`: true means Actor, false means Non-actor, absent means Unclassified.
- Apply consistent text marker wherever noun appears: cards, grid headers, pickers, lists, inspection and noun-bearing evidence. Color may reinforce marker; never rely on color alone.
- Preserve authored names, references and typed identity keys. Presentation labels must not change selection identity or stored source data.
- Derived bindings refer to canonical noun classification. Unresolved noun references remain unresolved; never label them non-actor.
- Include all three classifications in noun browsing and existing explicit case/state queries. Do not guess classifications or change example data.

## Compact label polish

- Shared `display_name`: `name - A` for Actor, `name - N` for Non-actor, `name - U` for Unclassified. Canonical `name`, reference, ID and source stay unchanged. All existing consumers, including plain CLI/evidence labels, receive compact suffix.
- Actor label underlined; Non-actor bold; Unclassified regular. Derive style from explicit canonical metadata and preserved `is_noun`, never suffix parsing or axes. Unresolved references have no resolved noun record and retain unresolved wording, not U.
- Apply rich style to graph/compound query cards, noun/verb grid headers, plain matrix viewer, noun-picker items, shown-node browse buttons and selected noun headings. Mixed prose, native combo previews and CLI remain plain text with suffix. No inline markup in typed labels/JSON or new font asset. Bold uses one-unit glyph overstrike; underline uses one-unit stroke.
- Style only known noun span inside composed labels. Bounded prefix copy preserves complete UTF-8; measure sentinel cancels final-glyph spacing. Graph measurement includes decoration at raw anchors, not filtered pose. Retain clipping and existing card/grid geometry policies.
- Selection, pending/applied inputs, drafts/history, source association and explicit Run unchanged. Existing review viewer untouched. User accepts reviewed Mario Noun interactions label presentation; no every-view/font/platform acceptance inferred.
- Verification: warning-free CJIT CLI/core/plain-view/graph-view/adapter builds; core148/148, focused Node11/11. Covers A/N/U labels, canonical names/references, style from metadata despite normalized roles, unrelated/unresolved labels, bounded UTF-8 prefix handling, existing query semantics, Find, shared controls and both-example IPC. Both example source hashes unchanged. Deep/research not rerun for label polish.
- Artifacts: `native/build/noun-labels-work/` build logs, `core.log`, `checks.log`; separate `plain/` executable avoids overwriting graph build. User authorized separate updated Mario Noun interactions viewer, PID30996 / session `30996-550fe44eb69bb308`. No existing viewer closed or modified; only controls/actions inspected after launch. User reports presentation looks good and authorizes checkpoint. Acceptance covers reviewed labels only.

## Noun interactions

- Both axes show all loaded nouns, including isolates and unclassified nouns. Headers show classification.
- A cell counts distinct declared cases in bound interaction collections where both noun types explicitly participate. Count each case once per pair despite duplicate local bindings.
- Diagonal counts a case only when it declares at least two distinct local bindings of same noun type. Count once per case, regardless of binding-pair multiplicity. Single binding never creates self-interaction.
- Symmetric co-participation, not directional modification. No cross-case joins, prose inference or automatic mechanic/call expansion.
- Example: one interaction record containing three shared cases gives cell count **3**. Actor map still gives record weight **1** for same actor pair.
- Cell selection exposes contributing case names, source records and conditions. Retain case identity; do not flatten evidence into only a count.
- Unknown, incomplete or limited scope must be visible. Zero means no retained shared case, not proof that interaction is impossible.
- Preserve noun × verb view and its evidence under explicit Verb usage label. New grid must not require verb inputs merely to become available.

## Preserved behavior

Keep independent view selection, typed identities, source inspection, pending/applied inputs, drafts/Undo and last-good results. Never implicitly Run analytical queries. Do not close or replace review windows without coordination.

## Acceptance

- Same noun has same classification marker across implemented UI surfaces, including existing verb grid and new noun grid.
- Unclassified noun stays visible and never becomes false by default.
- Fixture with three shared cases in one record yields 3; repeated binding within a case does not inflate count.
- Separate cases containing only one noun each do not produce shared-case interaction.
- Selecting cell lists exactly counted cases with source and conditions.
- Existing verb usage and explicit noun state queries remain available.
- New grid is available from nouns plus interactions without verbs; both examples retain all loaded nouns.

## Implementation

- `gd_noun_grid.{h,c}` builds immutable sparse symmetric pairs from original interaction collections in rule snapshot. Owns bounded pair/hash storage; borrows rules. Retains all loaded noun axes even when pair-work budget is zero.
- Bounds: 4,096 noun records, 32,768 retained pairs, 64 × 1,024 × 1,024 construction work units. Allocation failure rejects candidate. Capacity/work exhaustion retains disclosed incomplete prefix; evidence reproduces retained counts.
- Explorer adds ninth projection `noun-interactions`, titled **Noun interactions**. No new snapshot family or evaluator. Existing `matrix` key stays compatible and displays **Verb usage**.
- Count grids share linear blue-to-teal fill and fixed min/max gradient legend above grid. Includes Noun interactions, Verb usage, standalone matrix and adjacency/incidence insets. Normalize over all retained populated cells, independently of pan/zoom; blanks stay neutral. Empty sets show no populated cells; equal counts use solid midpoint. Preserve numeric labels, accepted-case marker and selection outlines.
- Grid uses readable initial camera, sparse drawing, pan/zoom, fit, cell/header selection, arrows and PageUp/Down. Sidebar row/column pickers use canonical noun references. Case evidence pages cover 64 matching retained cases; show source, context and participant conditions. Selected source remains inspectable.
- GUI/IPC share immediate `layout: grid | free`, `noun_row`, `noun_column`, `noun_case_page` controls. Grid remains default. Free graph reuses retained noun pairs: every noun including isolates, undirected links with explicit self-loops, case-count labels, brighter blue-to-teal strokes and 2..8 screen-pixel widths sharing full-retained-graph linear normalization, draggable classified cards and shared bounded solver. Matching actual-count line legend; dark count badges/grid fills stay unchanged. [Link-count contract](link-count-encoding.md). Pause applies only in Free. Pair/noun selection and source/case evidence survive atomic switching; failed allocation retains old layout. No browse-filter or structural-query controls, second evaluator or implicit Run.
- Noun Free graph spring targets include 282 layout units outside card footprints: minimum rest span 330 units for 48-unit-high cards, matching MDA's rendered minimum spring scale. This is a soft target, not guaranteed pair distance. Shared Neighborhood Free preparation reserves matching room. Shared hard collision gap remains 8 units; other views retain existing spring defaults. Same solver and bounds; no new per-frame allocations or count/evidence changes.
- CLI `--mode noun-interactions --layout grid|graph` selects initial mode; `columns` is rejected. Graph IPC `links` evidence exposes case count and leads to same counted `noun-cases` records.
- Retained IPC `result --projection noun-interactions --form noun-grid` supports whole-result byte paging; rejects nonempty `--fields`. `evidence --projection noun-interactions --form noun-cases` pages matching counted cases and retains native case identities. [Current paging contract](evidence-list-paging.md).
- Canonical noun records retain separate display labels, including after normalized snapshot loading. Existing name/reference fields and typed selection keys remain unchanged. Labels use bounded retained-text storage, not a second actor dataset.

## Checks

- CJIT native core: **142/142**. Covers classification identity/normalized roles, three cases versus one record, diagonal pairs, isolates, repeated bindings, no cross-case joins, source-role filtering, pair capacity, both constructor failures and evidence/count agreement at every work-budget prefix.
- Native Node: **62 passed, zero failed, one Mac-only skip**. CPU production explorer adapters pass both examples, including ninth-view discovery, retained result/evidence, canonical selectors, framing/rearrangement and selected-pair refresh. Source hashes and owned-allocation checks remain clean.
- CLI, headless protocol fixture, basic viewer, graph explorer and CPU explorer adapter build with CJIT. Final compiler matrix remains Step 6; no alternate-compiler claim.
- Evidence: `skills/game-design-workbench/native/build/noun-grid-work/build-final.log`, `native-node-final.log`, `fixtures-final/noun-grid.json` and fresh executables in same directory.
- Spacing/drift fix: CJIT core **142/142**, focused CLI + both-example CPU adapters **5/5**. Added spring-gap response/validation and seed-spacing tests. Both examples pass 4,096-step Native/Neighborhood probes with finite coordinates, no final card overlaps and bounded final 256-step motion/drift.
  - Mario Native: minimum linked-center distance **61.104 before / 159.328 after**, in layout units; tail RMS **0.628261 before / 0.015804 after**, in units/step; centroid drift **33.966577 before / 0.514894 after**, in units per 256 steps.
  - Mario Neighborhood: minimum linked centers **212.075 units**, tail RMS **0.001575 units/step**, centroid drift **0.100815 units per 256 steps**. Native still reports bounded collision work/movement limit; measurements are not universal convergence proof or visual acceptance.
  - Evidence: `native/build/noun-spacing-work/baseline.log`, `mario-final.log`, `crate-final.log`, `build-final.log`, `adapter-final.log` under skill root. CLI, core, CPU adapter and graph explorer build with CJIT. New viewer not launched; existing windows unchanged. Broader Resources-based Free graph review captured separately in `todo.md`.
- Free-graph follow-up: CJIT core **142/142**, focused CLI + both-example CPU viewer checks **5/5**. Covers all nouns/isolates, exact pair topology, explicit self-loop routes, both layout-candidate allocation failures, case selection round trips, link/evidence selection and Free-mode selected-pair refresh. CLI/basic domain, CPU adapter and graph explorer builds pass. Evidence: `native/build/noun-graph-work/build.log` and `adapter.log` under skill root. Build not launched; visual review pending.
- Count-formatting follow-up: CJIT core **142/142**, both-example production CPU adapters **3/3**, basic viewer and graph explorer builds pass. Tests cover empty/uniform ranges, min/max/midpoint, clamping, integer extremes and palette endpoints. Evidence: `native/build/count-heatmap-work/build.log` and `adapter.log` under skill root. Count-formatting build not yet launched; existing review windows untouched. Human visual/usefulness acceptance remains pending.

## Comments

- Actor map still weights distinct interaction records. New grid counts cases intentionally; do not compare counts as same metric.
