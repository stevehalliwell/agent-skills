# Visual exploration — provisional shortlist

## Result

Prioritize four interaction patterns, not a new universal graph dashboard:

1. **Finding selection that changes graph state while retaining context.**
2. **Paired supporting/counterexample witness inspection.**
3. **Typed, explicit context expansion from a finding.**
4. **Small, separate exploration history with restorable analysis states.**

All game-design interactions below are **proposed adaptations/hypotheses**, not evaluated UI improvements. Existing native behavior is distinguished from proposed additions. This is bounded orientation/source discovery: 7 unique candidate URLs, 5 successfully read, 2 failed; no full capture campaign, implementation or player evidence. Sources and bounded passages: [evidence-extracts.md](evidence-extracts.md). Discovery trail and candidate observations remain separate.

## Frame and evidence bar

- Question: how can designers inspect graph findings, understand their supporting declarations and retain scope/assumption context without being overwhelmed?
- Consumers: parent synthesis and later human design review.
- Coverage: visible result selection; supporting/counterexample paths; focus/context and partial data; analysis history distinct from author edits.
- Out of scope: choosing analysis algorithms, graph drawing/layout survey, causal learning claims, database choice and production design.
- Mandatory AI-assisted guide read. Claims remain source reports, code observations, or labelled hypotheses. Established research is relevant; software docs carry version/read-date caveats.
- Source bar: original graph-exploration research and official software documentation. Product manuals demonstrate patterns, not comprehension benefits. No search summary used as substantive evidence.

## Preservation baseline from actual repository reads

Native `README.md` and C paths below are under `skills/game-design-workbench/native/`. Example paths are under `skills/game-design-workbench/examples/`; bare record filenames refer to the named example collection.

- Relationships already starts at one-hop incoming/outgoing scope, expands explicitly, retains boundary count and source lines; it is not whole-project impact analysis (`README.md:136-148`; `gd_focus.c:7-51`).
- Skills already supports hypothetical-known dots, chain rings and shared-link comparison evidence. Known selections never imply prerequisite knowledge or automatic learning (`README.md:217-225`; `gd_graph_view.c:439-482,678-688`).
- Challenges already switches with Skills/Relationships, preserves draft/history and highlights retained route witnesses; dirty refresh and invalid pending edits must stay guarded (`README.md:236-244,373-380`; `gd_graph_view.c:283-307,691-694`).
- Mode direction matters: Skills arrows mean prerequisite to dependent; Relationships arrows mean authored source to target (`README.md:142,189`). Findings must not silently reverse these semantics.
- No native schema interpreter. A loaded folder is not proof that every schema field is consumed. Challenge schema declares mechanics/interactions/nouns/verbs/resources, but challenge-role reader handles skill roles and `next` only (`gd_core.c:350-394`; crate `challenges/.schema.md:1-19`). Do not promise a challenge-to-mechanic context path without extending the reader contract.
- No source writes, new dependency requirement or discarded drafts implied. Native resource preview remains partial; this lane makes no claims for it.

## 1. Result brushing with explicit analysis-layer scope

**Problem.** A finding listed only in inspector prose leaves designers searching for the affected nodes. Filtering everything else away can hide why a result exists.

**Concrete interaction and visible state — hypothesis.** In plumber Skills mode, select `stomp-walker`; choose “Shared declaration candidates.” Graph marks `jump-stomp` and `fall-stomp` as candidates, and marks their shared `walker-stomp` declaration evidence in a supporting Relationships scope. Keep non-matching skills visible but subdued by default. Selecting a candidate highlights its two direct prerequisite links in Skills mode, not an invented candidate-to-candidate authored edge.

- `jump-stomp` is proposed, depends on `stomp-walker` AND `land-on-target`, and references `walker-stomp` (`examples/little-plumber-guy-nes/game-design/skill-atoms/jump-stomp.md:7-15`).
- `fall-stomp` is proposed, depends on `stomp-walker` AND `control-fall`, and references the same rule (`fall-stomp.md:7-15`).
- Both may also appear through shared prerequisite evidence. Show evidence types separately; repeated declarations are not extra independent support.
- With hypothetical known `{stomp-walker, land-on-target}`, visibly distinguish supplied assumptions from result eligibility: jump method has all direct prerequisites supplied, fall method still lacks `control-fall`. This is a derivation from declarations, not tested learning. Do not mark prerequisite ancestors known.

**Method/evidence.** Cytoscape documents reciprocal chart/network selection and separate select/show filter actions (E2). Attribute mappings can change graph properties (E4). Neo4j documents dim-before-dismiss and filter toggles (E5). Adapt the interaction contract, not any product's complete interface or color scheme.

**Data.** Existing: statuses, prerequisite/mechanic links, current known selections, source lines and selected-root comparison rows. Derivable: candidate membership and direct missing-prerequisite membership. Missing implementation: candidate result overlay/linking; current renderer shows known dots and chain rings, not a graph-wide candidate overlay. No missing author data is required for this narrow case.

**Inspection/legend.** Active layer names query, root and supplied assumptions. Show `matches`, `context`, `outside revealed scope`, `unloaded/unresolved` separately; missing or omitted results must not look like non-matches. Toggles affect presentation, not underlying computation. “Show only matches” is separate and reversible; declare whether supporting context remains pinned. Click evidence to inspect each original field and line. Candidate membership must visibly alter graph, not merely add a label in details.

**Cannot establish/failures.** No automatic contrast, transfer, mastery, design merit or physical feasibility. Accepted shared understanding does not adopt proposed variants. Dimmed nodes may be mistaken for rejected records; layer legend must distinguish status from query membership. Shared references to unloaded definitions remain authored evidence, not validated rule equivalence. Typography/color design belongs to drawing lane.

**Scale/confidence.** Apply result masks to rendered records/edges; avoid recomputing comparisons on every hover. Current query caps: 16,777,216 charged work units and 128 comparison rows; partial results remain explicit (`README.md:221`). Larger graphs need server/index/result paging decisions, not a larger author-data cap. Cytoscape explicitly offers update-on-release for sluggish networks (E3). Confidence: high that patterns exist; medium adaptation fit; no native UX performance measurement.

## 2. Paired witness inspection: “why this finding” and “what challenges it”

**Problem.** One highlighted path may be mistaken for every path, or a shortest path mistaken for proof of opportunity coverage.

**Concrete interaction and visible state — hypothesis.** Select crate `combined-room` / `route-switch` opportunity check. Use two named controls:

- “Earlier support witness”: `move-demo → push-room → switch-practice → combined-room`.
- “Bypass witness”: `move-demo → push-room → combined-room`.

Toggle between them with other route links retained. Pin the alternative as comparison context if useful. The graph must highlight the selected witness nodes and `next` edges, with a separate marker for `switch-practice`'s earlier introduction. Do not draw `introduces` as a traversal edge. Display `some_routes`, not “player unprepared.”

**Repository grounding.** Crate `move-demo.md:8-15` starts route; `push-room.md:16-18` declares both branches; `switch-practice.md:8-18` introduces switch skills and continues; `combined-room.md:8-17` demands `route-switch`. `skill-atoms/route-switch.md:8-12` also requires `read-switch`. These authored records support the contrast, not measured player behavior. Native README already states the bypass/support example (`README.md:266`).

**Method/evidence.** Neo4j exposes path inspection but returns only first shortest path within 20 hops (E6): concrete counterexample to equating one drawn path with exhaustive explanation. GraphTrail highlights full action chains to explain how a result was obtained (E7); that is analysis provenance, not graph-support semantics. Native supported/unsupported opportunity witnesses, not generic shortest-path search, supply the right domain evidence here.

**Data.** Already available: check result, role declarations, prerequisite demand, source lines, supported/unsupported witnesses. Traversal derives coverage and retained witnesses. Missing author data for stronger claims: physical gates, actual routes, assistance and observations. Current route control already highlights one witness and exposes source-navigation buttons (`gd_graph_view.c:114-125,163-164,198-207,283-307`); proposed delta is paired contrast/pinning and visible support-role connection, not invention of witness analysis.

**Inspection/legend.** “One retained witness; not all routes” next to selector. Step selection highlights exact graph segment and original `next` declaration. Follow demand to skill source, then prerequisite source; show role and direction. Truncated witness visibly says suffix-only; omitted witness count remains visible. Incomplete route refs make negative claims unknown, not absent. Filtered-out proof elements need reveal/pin affordance before users inspect a path.

**Cannot establish/failures.** An introduction is an authored opportunity, not successful teaching. A bypass can be intentional discovery, not an automatic defect. All graph routes are not physical traversals. Repeated visits and strict-before-arrival semantics matter. Highlighted suffix is not complete route; two witnesses do not enumerate all alternatives.

**Scale/confidence.** Native route analysis uses iterative traversal/two-state BFS, not route enumeration. Bounds: 32,768 checks, 128 retained witnesses, 256 trailing nodes/path; computed coverage survives witness omission (`README.md:256-268`). Simultaneous all-path overlays would defeat this economy. Retain bounded selected explanations beyond current graph limits; large rendering still needs separate review. Confidence: high data/renderer fit, medium interaction value until manual review.

## 3. Finding-led, typed expansion with boundary context

**Problem.** Whole-graph overview may be irrelevant for a precise question, while unqualified neighbor expansion mixes prerequisites, shared rules and route transitions.

**Concrete interaction and visible state — hypothesis.** From selected plumber `fall-stomp`, “Explain shared rule” opens Relationships rooted at `mechanics/walker-stomp` with explicitly loaded mechanic context. Visible scope includes `stomp-walker`, `jump-stomp`, `fall-stomp` and their consumed declarations to that rule. Expand `fall-stomp` along prerequisites only to reveal `control-fall` and `stomp-walker`; separately expand `jump-stomp` to `land-on-target`. Root and declared edge field remain visible. A breadcrumb records analysis intent; canvas boundary count distinguishes undisplayed or unresolved declarations.

This is targeted explanation of a discovered family, not a claim that users must choose every comparison pair. For corpus-wide discovery, sibling analysis lane can feed selected result set into this workflow.

**Method/evidence.** IBM's original 2009 abstract proposes search/context/expansion rather than mandatory overview, with legal-citation demonstration (E1). Neo4j documents selective relationship type/direction expansion and limits (E5). Cytoscape offers adjacency Add versus Replace, with repeated Apply expanding scope (E3). Full IBM paper could change automatic context-selection recommendations; only abstract read.

**Data.** Already: explicit reveal state, loaded identities, incoming/outgoing consumed declarations, field/source lines and boundary counts (`gd_focus.c:7-51`). Rule record must be explicitly loaded through existing bindings; no conventional-folder discovery. Narrow adaptation needs no new author links. Current expansion is all incoming/outgoing, not typed: typed controls/pinned proof subset are new behavior. Missing or unbound definitions cannot become invented placeholder identities.

**Inspection/legend.** Name root, allowed relationship fields, direction convention, number shown and number incident but not shown. Separate “outside revealed scope” from “not uniquely loaded” if adding detailed categories; current boundary count combines these. Selected edge reveals its field on graph and source line in panel. “Center here” resets scope; “Expand” grows it. Clearing analysis layer restores prior user exploration rather than editing sources.

**Cannot establish/failures.** Local context can conceal isolates and distant bridges. Degree-of-interest ranking risks hiding surprising low-degree records. Preserve explicit Show all/result overview route; do not make relevance algorithm silently authoritative. Raw authored prerequisite arrows reverse the Skills presentation convention. Rooted graph grouping is not a causal or progression explanation.

**Scale/confidence.** Current expansion scans edges and rebuilds visible layout; dense hubs can reveal many neighbors immediately. Typed field filters, explicit apply and shown/boundary counts are safer first slice than automatic global relevance. Larger corpora would benefit from adjacency indexes and bounded result pages; no timing measurements. Confidence: high baseline support; medium typed-expansion fit; low claim about optimal automatic relevance.

## 4. Bounded exploration history, separate from edit undo

**Problem.** Changing root, known assumptions, filter or witness can erase why current graph is shown. Edit undo must not become accidental analysis-navigation undo.

**Concrete interaction and visible state — hypothesis.** Keep small analysis-state trail for plumber comparison:

1. `root=stomp-walker; known={}; layer=shared declarations`.
2. `known={stomp-walker,land-on-target}; candidate=jump-stomp`.
3. Branch to `candidate=fall-stomp; still missing=control-fall`.

Selecting a trail entry restores query root/assumptions/filter/witness and corresponding graph overlay; hovering entry previews affected nodes/links without changing source draft. Label entries with source snapshot identity and draft revision. Old-snapshot entries remain historical or require explicit re-evaluation; never present them as current validated findings.

**Method/evidence.** GraphTrail uses visible meaningful-action trails and highlights the chain that produced a view (E7). Its original qualitative studies suggest useful recall/interpretation, but ten HCI participants and archaeology deployment are not game-design validation (E8). Pivots were hard to learn; chart history imposed management cost. Prefer small state trail/bookmarks over reproducing infinite chart canvas.

**Data.** Existing transient query inputs and source identity can describe states. Genuinely missing product capability: analysis-history store/model and snapshot/revision policy. No new author metadata or gameplay telemetry needed. Do not claim current native undo stores analysis history: it records skill edits. Successful reload currently remaps known identities, resets witnesses and revealed scope (`gd_graph_view.c:720-734`); a history proposal must define invalidation rather than bypass this behavior.

**Inspection/legend.** Mark `current`, `historical snapshot`, `re-evaluated`, `partial`. Each entry exposes query parameters and links to supporting declarations. “Back in exploration” is distinct from Undo; no source edits replay. Keep modified markers, stale banners and dirty refresh guards. Switching mode retains source selection/draft/report destination under existing contract.

**Cannot establish/failures.** Action history explains how analyst arrived at result, not whether result is correct or why game design is good. Persistent history adds memory/storage, invalid identities and stale interpretation risks. Full click logging adds noise; branch canvas adds navigation cost. Restoring assumptions must never imply learned state or persist it as author data.

**Scale/confidence.** Use bounded meaningful states, stable references and parameters rather than copying entire model/layout per event. Capacity and persistence policy require later review; do not invent a limit now. Defer durable cross-session replay until snapshot/draft semantics agreed. Confidence: medium evidence for general recall support; low-to-medium native value compared with smaller overlay/path improvements.

## Prioritized follow-up questions

1. Human task check: can designer identify shared rule and differing prerequisite from candidate overlay, without mistaking proposed status or hypothetical knowledge for accepted mastery?
2. Can designer explain `some_routes` using support and bypass graph witnesses, and identify omitted/truncated/unknown evidence correctly?
3. Does typed expansion preserve useful unexpected context, or would selection-only overview suffice at shipped fixture density?
4. Is analysis-history loss a real recurring problem? If not, defer history and keep existing mode/selection state. If yes, decide snapshot/draft invalidation before durable storage.
5. Retrieve full 2009 context paper in deeper pass; inspect automatic relevance assumptions and evidence, then compare local versus global tasks. Do not infer empirical comprehension from abstract's large-database demonstration.

## Checks, gaps and synthesis handoff

- Read shared brief/assignment before discovery; guide and artifact formats read. Append-only searches and repeated source observations retained.
- Read native README completely, plus actual focused reveal, reader, route UI, known overlays and refresh code. Grounded examples in actual plumber/crate records and relevant live schemas/usages.
- No tests/builds/experiments rerun; no browser automation, installations, code changes, extra agents or shared-file edits. Existing native acceptance claims are not new acceptance results.
- Checked candidate CSV structure/unique URL budget, artifact presence and cited repository line ranges. Source failures retained. No full research register/source-map/capture queue created.
- Gaps: no observed game-designer tasks; no comparative comprehension study of proposed UI; full context paper inaccessible through selected mirror; software docs partially version-moving; no benchmark at/above native 4,096-record/32,768-edge limits. These bounds describe current reader, not allowed author-data size.
- Deeper pass could change ordering of local expansion versus overview, and whether history earns its cost. Current strongest low-risk shortlist is **visible result selection plus inspectable paired witnesses**, preserving hypothetical-state and partial-coverage semantics.
- Sibling overlap: analysis lanes own how findings are computed; drawing lane owns encoding legibility, routing, layout and perceptual tests. This lane supplies workflow states and evidence affordances, not optimized drawing claims.

## Comments

- Neo4j/Bloom and Cytoscape are references only, not selected dependencies.
- GraphTrail source distinguishes aggregate exploration from node-link reporting; do not treat attractive graph appearance as demonstrated comprehension.
