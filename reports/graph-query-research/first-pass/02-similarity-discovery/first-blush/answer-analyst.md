# Similarity/discovery — provisional first pass

## Result

Prioritize explainable, graph-wide candidate discovery over an embedding-first approach. Four useful techniques: typed shared-feature postings; short typed paths; prerequisite-closure differences; small-depth structural signature buckets. No durable algorithm/dependency choice made. All proposed views below are hypotheses, not implemented or evaluated findings.

Source discovery/orientation only, not exhaustive evidence answer. Six unique candidate URLs; 11 observations including repeat reads and one failed fetch. Evidence passages, source limits and repository ranges: [evidence extracts](evidence-extracts.md). `S1–S4` and `R1–R11` refer to that file.

Question: which automatic relatedness/family queries could reveal useful structure across encounters, skills and mechanics without manually choosing every pair? Consumers: designers and agents; output must change visible graph and expose exact declarations/witness paths. Not in scope: teaching-transfer claims, production implementation, new schemas or dependency adoption.

## Existing boundary

- Current native skill query already finds shared exact prerequisite/mechanic references **for one selected root**, not graph-wide discovery. Code explicitly says “never all pairs” and preserves field/reference/source-line evidence [R1]. This lane concerns automatic candidate generation across roots, encounters and indirect paths, not rebuilding that root inspector.
- Skill prerequisites/mechanics and challenge skill roles/`next` are consumed. Challenge schema additionally declares mechanics/interactions/nouns/verbs/resources, but challenge loader does **not** consume those fields [R2,R9]. Supplying related folders does not fix skipped frontmatter fields [R10]. Do not promise mechanic-based encounter discovery from current native challenge edges.
- Native bounds: 4,096 records, 32,768 edges per operation, not author-data limits [R3]. No timings or app experiment reruns in this pass.
- Existing relationship exploration and source panels offer relevant interaction primitives, not an existing graph-wide similarity system [R10]. Resource preview remains partial. Rule-body semantics are not interchangeable with plain typed references [R11].

## 1. Typed shared-feature postings — strongest starting point

**Question.** Which records share specific declared ingredients, and what differs? Find candidates automatically across each record type, then allow pair inspection.

**Concrete hypothesis.** Discover plumber `jump-stomp` and `fall-stomp` in a common above-impact family: both declare prerequisite `stomp-walker` and mechanic `walker-stomp`. Show branch contrast: `land-on-target`/`jump-launch` versus `control-fall`/`solid-collision` [R4]. This is a source-supported illustrative match, not a computed ranking. Both atoms are proposed; visibility must retain that status.

**Method/evidence.** Build postings keyed by `(source record type, field, canonical target reference)`. Walk each posting to generate candidate pairs; deduplicate pairs, retain every evidence channel. Optionally show Jaccard or containment per channel; no scalar score required. S1 documents neighborhood metrics but warns that GDS treats selected relationship types alike. Typed postings are a proposed lightweight adaptation, not GDS selection. Exact-identity common neighbors differ from merely similar-shaped neighbors (technique 4).

**Data.** Skill links/source lines already available. Feature frequency, pair witnesses and per-channel set differences derivable. Equal unresolved mechanic reference strings can support “same authored token,” but not a verified common mechanic definition; resolve status separately [R1]. No semantic labels or player evidence needed for this limited claim.

**Visible result.** Family badges or selectable candidate bundles on graph; selecting bundle highlights both records and shared target nodes/edges, fades unrelated edges, and shows differing branches. Derived connector uses dashed style/legend, never masquerades as authored edge. Clicking each highlighted declaration opens source field/line; pair inspector follows discovery rather than drives it.

**Limits/countercheck.** A ubiquitous prerequisite can create a meaningless near-clique. Report feature prevalence and compare results with that feature hidden/downweighted. Frequency discount is an analysis policy, not an empirical measure of importance. Do not infer semantic equivalence, contrast, learning transfer or mastery. Empty/missing links do not form an “identical empty” family. Duplicate declarations should not inflate set overlap, but remain source evidence.

**Cost/confidence.** Proposed candidate emission work proportional to `sum_f choose(d_f,2)` before pair deduplication; hub postings can still approach quadratic output. Stream/limit output separately from work accounting; display omitted/work-limited state. A top-k output cap alone does not prevent expensive comparisons (S1). High confidence in evidence traceability; untested usefulness and implementation cost. No new external dependency inherently needed, but no “easy to add” claim.

## 2. Short typed paths — encounter discovery without duplicate direct links

**Question.** Which encounters are indirectly related through demands, skill prerequisites or mechanics? Keep “requires,” “practises,” and “introduces” meanings separate.

**Concrete hypothesis.** Discover `first-gap` and `final-gap` via shared `requires: jump`. Also expose that `land-on-target` is `tests_mastery` in first-gap but `requires` in final-gap [R5]. A strict same-role query and a mixed-role discovery query must return different labelled evidence. With additional reader support, both also share declared jump-launch/death-and-retry/gap ingredients; those fields are in records but not native challenge graph [R2,R5].

**Method/evidence.** Start with explicit templates such as `challenge —requires→ skill ←requires— challenge` and `challenge —requires→ skill —mechanics→ mechanic ←mechanics— skill ←requires— challenge`. These are traversal witnesses, not new authored links. S2 shows why relation sequences change similarity semantics and describes sparse path-based candidate access. Use its short-path idea without requiring PathSim scoring. If using PathSim itself, restrict to same-type symmetric round trips and retain normalization definition; asymmetric mixed-role witnesses are not automatically PathSim similarity.

**Data.** Challenge demands and skill mechanics already consumed; paths and inverses derivable. Direct challenge-mechanic/noun queries need reader work despite schema support. Required/practised paths do not substitute for actual route order or physical feasibility. No need for authors to repeat indirect mechanic links.

**Visible result.** Choose a named relation lens, not a pair; graph shows auto-discovered encounter groups/derived connectors. Expand connector to highlight complete typed path, field labels, intermediate skill/mechanic and endpoint statuses. Give direct declarations and indirect paths distinct legend entries. Alternative witnesses stay inspectable, with explicit truncation counts.

**Limits/countercheck.** Long paths broaden relatedness until almost everything connects; S2 warns very long paths can be misleading and candidates may become dense. Cap template length, not just displayed witnesses. Compare strict versus mixed skill-role lenses; do not blend them into opaque weights. Path multiplicity from duplicate declarations must not masquerade as stronger design evidence. PathSim normalization is not a guarantee against ubiquitous features. No claim of teaching order, successful completion or player transfer.

**Cost/confidence.** Index incoming/outgoing typed links, compose bounded templates, deduplicate endpoint candidates. Frontier/feature expansion can explode around hubs; avoid materializing every long-path pair. S2's large-network storage examples are not estimates for this repository. High confidence in typed witness semantics; medium confidence in which templates designers find useful. No selected engine.

## 3. Prerequisite-closure families plus deltas — reveal shared foundations

**Question.** Which alternatives share foundations several edges away, and what extra AND requirements distinguish them?

**Concrete hypothesis.** Automatically place jump-stomp/fall-stomp in a shared-foundation family, then show shared indirect `stop-at-edge`. Jump branch reaches it through `land-on-target`, while fall branch reaches it through `control-fall`; landing also requires `jump` [R4,R6]. Show a separate direct evidence lane for shared `stomp-walker`. This avoids demanding duplicate direct prerequisite links from authors.

Crate `coordinate` provides a caution: its AND set combines `plan-push` and `route-switch`, while each has its own prerequisite chain [R7]. A descendant/subset can look close to its foundation yet demand more, not be interchangeable. `combined-room` already declares these direct demands and introduces/requires coordinate together; do not count closure duplicates as independent support opportunities [R8].

**Method/evidence.** Compute ancestor sets per skill using cycle-aware traversal; emit postings keyed by ancestor, with direct/indirect distance and at least one sourced witness. Compare sets with shared versus extra requirements kept separate. This is a proposed extension of the actual native rooted closure [R1], supported conceptually by typed bounded traversal (S2), not an external proof of game utility. Full dependency closure differs from the fixed-length templates in technique 2 and needs explicit cycle handling.

**Data.** Resolved prerequisite links and source declarations already available; closure, shortest witnesses, ancestor prevalence and dependency deltas derivable. AND sets remain AND sets. Null rank is not missing prerequisite data, and rank is not part of structural distance [R9]. Incomplete or cyclic ancestry requires qualified/unknown results, not independent-root assumptions.

**Visible result.** Shared ancestors highlighted as common trunk with two coloured branch paths and extra requirement markers. A “foundation overlap” lens groups candidates graph-wide; selecting a group changes graph emphasis. Inspect paths back to exact prerequisite declarations, not only flattened ancestor lists. Distinguish shared ancestry from ancestral containment so parent/descendant pairs do not dominate alternative-method discovery.

**Limits/countercheck.** Very common roots swamp discriminative ancestors. Compare direct-only versus expanded results, show depth and prevalence, optionally suppress ancestor/descendant candidate pairs while keeping them visible as support. Missing loaded context makes negative differences provisional; no unobserved prerequisite is inferred. Structural unlock relation does not establish acquisition order, automatic knowledge or teaching transfer.

**Cost/confidence.** Repeated traversal is roughly `O(V(V+E))` in worst case; materialized closure may require `O(V²)` space before postings. Sparse closures/bitsets/cache choice requires later measurement. Current native query work budget must not silently become an author-data ceiling. High confidence in available prerequisite evidence; medium confidence in discriminative value at scale. No persistent closure selected.

## 4. Small-depth structural signatures — shape families and reviewable exceptions

**Question.** Which records occupy similar structural roles even without common target identities? Which records differ from a common shape by one branch?

**Concrete hypothesis.** Across crate skills, `plan-push` and `route-switch` each have one declared prerequisite and one mechanic; `coordinate` has two prerequisites and one mechanic [R7]. A role-labelled, one-hop signature lens could bucket the former and flag coordinate as a different combination shape. It must **not** say planning equals switch routing. At greater depth the chains may split; result depends on chosen projection and labels.

**Method/evidence.** WL-inspired refinement: initialize by record role; combine current label with sorted multisets keyed by incoming/outgoing field plus neighbor label. Bucket identical signatures for small depths. Keep typed identity-based view (1) alongside role/shape-only view. Source S3 supports iterative neighborhood label refinement and rooted-pattern interpretation; original definitions are undirected and node-labelled. Directed/typed adaptation and use as local node families are hypotheses, not the paper's tested game application. Simple exact buckets are structural families, not community detection. Low-frequency signatures are “unusual in supplied projection,” not anomaly scores.

**Data.** Current prerequisite/mechanic projection sufficient for stated crate example. Stable initial labels, field directions, status policy and depth are explicit analysis inputs. Semantic actor/state/rule features would need a separately agreed interpretation; shared noun counts cannot equate free-destination/energy rules with top-contact/falling rules [R11]. Missing data needs an explicit incomplete marker, not same label as known empty.

**Visible result.** Colour/outline buckets with legend and depth switch; identify singleton/rare shapes without relocating everything into a mandatory 2D embedding. Select bucket to highlight each rooted neighborhood and corresponding relation roles. Show first differing branch and source lines; retain signature dictionary, not opaque hash alone.

**Limits/countercheck.** One extra edge can split a meaningful family; label too coarsely and unrelated skills collapse. WL cannot distinguish all graphs (S3). Rare patterns may reflect incomplete authoring or small sample, not design problems. Do not present similarity clusters as equal semantics, originality, quality or player understanding. Directed edge adaptation requires validation, especially cycles and duplicate edges.

**Cost/confidence.** S3 gives O(hm) refinement under its sorting assumptions; this does not mean arbitrary strings or all-pairs kernel matrices are linear. Bucket signatures without forming full pairwise matrices; dictionary/sorting/memory costs remain. Bounded-depth whole-graph refinement should be measured before adoption. Medium confidence as secondary lens; lower confidence in useful outlier interpretation.

## Complex representation counter-option

S4's node2vec abstract describes low-dimensional neighborhood representations from biased random walks, evaluated on classification/link prediction. That is a plausible later candidate generator if explicit methods miss relations, not evidence for encounter discovery, semantic equivalence or teaching transfer. Learned similarity would still need a separately reconstructed typed witness overlay; vector proximity alone fails the inspection contract. No 2D projection required.

Only abstract read; PMC full text failed. No supported training-time, typed-edge or reproducibility claims retained. A deeper comparison must record seeds, walk/training parameters, dimensions, projection, software version and graph snapshot, then check stability and typed witness coverage. Added training and explanation machinery currently lacks demonstrated benefit over lightweight lenses. This challenges, but does not rule out, the shortlist.

## Cross-cutting controls

- Preserve qualified identities, status, source lines, snapshot and supplied collection scope. Do not merge similarly named records across examples; any cross-game semantic mapping is genuinely missing data.
- Separate absent/null/empty authored link fields, skipped native fields, unloaded collections, unresolved references, and incomplete computation where parser metadata allows it. Current missing/null/empty arrays often all mean no declarations [R10]; finer authoring-state distinction may need additional metadata, not invented certainty. Empty features are no comparison evidence.
- Typed role/channel weights are query policies, not evidence strength. Default to separate lenses, not unsupported numeric weights. If tested, retain frequency formula, normalization, exact query version, threshold, tie order and status policy. Sort deterministic outputs by canonical identity; stable signature compression must not depend on filesystem iteration order.
- Show hub prevalence and sensitivity. Excluding high-degree **source candidates** differs from discounting a popular **target feature**. Never call either cure complete.
- Bound compute and witness output independently. Omitted/truncated witnesses, unresolved scope and no-match findings remain visible. No negative claim of “unrelated” when graph coverage is incomplete.
- Analysis must highlight/group/filter graph with legend and inspectable source paths. Drawing/comprehension efficacy belongs to sibling visual lanes; no evidence here that these overlays improve comprehension.

## Prioritized follow-up

1. Human-review a small graph-wide output for techniques 1–2: which discovered families/deltas are genuinely useful? Include ubiquitous-root, empty, unresolved, proposed and rejected cases. No automatic design recommendation.
2. Agree first relation lenses and reader boundary: skill-only discovery already has consumed data; direct mechanic-based challenge similarity needs additional field consumption. This can change scope/cost materially.
3. Compare direct shared targets, expanded ancestors and role signatures on same retained snapshot. Count hub-driven candidates and omitted evidence; test differences before adding weights.
4. Only if these miss important examples, authorize deeper embedding/kernel comparison with fixed seeds/configuration and witness inspection criteria. No algorithm decision from source popularity.

## Checks, gaps and handoff

- Read shared brief/assignment completely before discovery; selected AI-assisted guide read. Append-only journal and observation CSV retained. Six unique candidate URLs below cap of 8; duplicate read observations preserved.
- Read actual records, schemas/usages, native README and relevant code; citations indexed only to read ranges. Source text passages retained with stable sections/pages; no snippet promoted to evidence. Citation-end ranges checked against file lengths.
- One failed PMC fetch recorded. Public arXiv abstract used only within abstract's scope. No code, schema, shared record, environment or app modifications; no agents launched, browser automation, benchmarks, experiments, commits or pushes.
- No graph-wide results computed; illustrative matches derive from inspected declarations. Sparse candidate costs are analytic estimates, not measured runtime. No player-learning evidence.
- Unresolved: useful relation lenses/weights, incomplete-data representation, large-graph candidate density, structural-family usefulness, and visual comprehension. Deeper pass could reorder techniques 3–4 or justify an embedding comparison; techniques 1–2 remain provisional, not adopted.
