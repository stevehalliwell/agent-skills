# Evidence extracts — similarity/discovery

Bounded first pass, observed 2026-10-28. Public pages only. Provenance `primary` describes origin, not truth. Quotes below are source observations; interpretations and proposed game adaptations are separate. PDF extraction sometimes joins lines or splits words. Locations use printed pages/sections, not unstable extraction line numbers. No search snippet serves as evidence.

## S1 — Node Similarity

- URL: https://neo4j.com/docs/graph-data-science/current/algorithms/node-similarity/
- Provenance: primary; official Neo4j GDS documentation. Fetched readable content; introduction, configuration and glossary read. Page identifies documentation ref `2026.10`; mutable `/current/` URL.
- Exact extract, Introduction: “Two nodes are considered similar if they share many of the same neighbors.”
- Exact extract, Introduction: “For every node `n`, we collect the outgoing neighborhood `N(n)` of that node, that is, all nodes `m` such that there is a relationship from `n` to `m`.”
- Exact extract, Introduction: “Node Similarity has time complexity O(n³) and space complexity O(n²).” Superscripts rendered as Unicode here; wording unchanged.
- Exact extract, Introduction, paragraph about `topK`: “running time is unaffected - we still have to compute results before potentially throwing them away.”
- Exact extract, glossary, heterogeneous relationships allowed: “The algorithm treats all selected relationships similarly regardless of their type.”
- Exact extract, configuration, `upperDegreeCutoff`: “Inclusive upper bound on the node degree for a node to be considered in the comparisons.”
- Paraphrase: documented metrics include Jaccard, Overlap and Cosine; a relationship property can supply weights.
- Interpretation: do not silently flatten `introduces`, `requires`, `prerequisites` and `mechanics`. Build separate typed feature channels. GDS cost warning concerns its documented algorithm, not every possible sparse intersection implementation.
- Limits: no game-design or learning evidence. Excluding high-degree compared records is not the same as discounting a ubiquitous shared feature. Page glossary includes alternative trait definitions; cited definitions must not be read as proof that every trait is active. No dependency recommendation.

## S2 — PathSim

- URL: https://vldb.org/pvldb/vol4/p992-sun.pdf
- Provenance: primary; Sun et al., PVLDB 4(11), 2011, pp. 992–1003. Fetch returned local PDF-extraction pointer; extraction read through printed p. 998, including methods and experiment scope.
- Exact extract, abstract, p. 992: “a meta path is a path consisting of a sequence of relations defined between different object types”. Extraction hyphenation normalized only in this short extract.
- Exact extract, §2.3, p. 994: “As the relation of peer should be symmetric, we then confine PathSim merely on the symmetric meta paths.”
- Exact extract, §2.3, p. 995: “a very long meta path may be misleading.”
- Exact extract, §3.2, p. 996: “after concatenation, M could be dense, i.e. , the CandidateSet could be very large.”
- Exact extract, §4.2/Table 6, p. 998: “Venue NMI 0.6159 0.8198 0.8116” (columns RW, PRW, PathSim).
- Paraphrase: PathSim uses twice the connecting-path count divided by the sum of each object's self-path counts. §3.2 builds candidates through sparse two-step neighbor access in a partially materialized path matrix, rather than checking every target. §4 uses DBLP and Flickr, not design records or players.
- Interpretation: typed path templates separate “shares required skill” from “shares a mechanic through a required skill.” Sparse candidate discovery is useful even without adopting PathSim scores. Long paths and hubs can erase specificity.
- Counter-evidence: Table 6's venue-only clustering result favors PRW over PathSim. Authors' favorable aggregate result is not universal superiority. Normalized peer visibility is not a complete cure for ubiquitous intermediary features.
- Limits: PathSim's symmetric, same-type contract does not cover an arbitrary directed challenge-to-skill relation or mixed introduction/demand comparison. Paper does not validate proposed game queries, path lengths or weights.

## S3 — Weisfeiler–Lehman Graph Kernels

- Landing URL: https://jmlr2020.csail.mit.edu/papers/v12/shervashidze11a.html
- PDF URL: https://jmlr2020.csail.mit.edu/papers/volume12/shervashidze11a/shervashidze11a.pdf
- Provenance: primary; Shervashidze et al., JMLR 12, 2011, pp. 2539–2561. Landing abstract read; linked PDF extraction read through p. 2554. Both URLs remain separate discovery observations.
- Exact extract, §2, p. 2543: “augment the node labels by the sorted set of node labels of neighbouring nodes, and compress these augmented labels into new, short labels.” Algorithm 1 specifies a multiset, preserving multiplicity.
- Exact extract, §2, p. 2543: “either G and G′ are isomorphic, or the algorithm has not been able to determine that they are not isomorphic”. Mathematical primes rendered typographically.
- Exact extract, §2.1, p. 2544: “The runtime complexity of the 1-dimensional Weisfeiler-Lehman algorithm with h iterations is O(hm).” Mathematical spacing normalized.
- Exact extract, Theorem 7, p. 2549: “O(Nhm + N²hn)”. Mathematical typography normalized; this is the all-pairs kernel bound for N graphs, not just label refinement.
- Paraphrase: Algorithm 1 combines current label with sorted neighboring labels using an injective compression. §2.2 ties compressed labels to rooted subtree patterns; §3.2 counts matching labels across refinement depths. Definitions use undirected node-labelled graphs; footnote on p. 2541 says discrete edge-label extension is straightforward but omitted.
- Interpretation: small-depth, relation-aware signatures could bucket graph-wide dependency shapes without comparing every pair. This is a proposed directed/typed adaptation, not the tested method unchanged.
- Limits: exact signatures split after one extra/missing edge; identical signatures do not prove identical graphs, rules or meaning. O(hm) relies on stated bounded-label counting/radix-sort assumptions; ordinary string sorting need not achieve it. Full pairwise kernel matrices retain a quadratic term. No design-quality or comprehension evaluation.

## S4 — node2vec: representation counter-option

- Failed candidate: https://pmc.ncbi.nlm.nih.gov/articles/PMC5108654/
- Fetch/read failure: fetch tool reported “Page appears to be JavaScript-rendered (content loads dynamically)”. No passage or full-paper interpretation retained from this URL. No authentication, bypass or environment change attempted.
- Alternative read: https://arxiv.org/abs/1607.00653
- Provenance: primary; Grover and Leskovec, arXiv v1, 3 July 2016; abstract only, KDD 2016 paper metadata.
- Exact extract, abstract: “we learn a mapping of nodes to a low-dimensional space of features that maximizes the likelihood of preserving network neighborhoods of nodes.”
- Exact extract, abstract: “We define a flexible notion of a node's network neighborhood and design a biased random walk procedure”.
- Exact extract, abstract: “multi-label classification and link prediction in several real-world networks from diverse domains.”
- Interpretation: learned neighborhoods are a plausible alternative if explicit signatures miss useful relations. These tasks do not demonstrate explainable encounter discovery or teaching transfer.
- Limits: full paper not read. No retained evidence for training cost, seed behavior, typed-edge support or performance in this repository. Recommendation to record seeds/training parameters is a reproducibility requirement, not a measured source finding.

## Repository evidence index

All paths below are repository-relative. Only ranges actually read are cited. Records are authored examples, not empirical game/player observations.

- R1: `skills/game-design-workbench/native/gd_skill_unlocks.c:38–88`: existing root-specific query, closure and comparison loop; comparison uses equal field/reference for prerequisites/mechanics, retains source edge indices, no all-pairs discovery. `:90–111`: JSON candidate evidence/source lines and scope statement.
- R2: `skills/game-design-workbench/native/gd_core.c:356–390`: role-specific frontmatter consumption. Challenge branch accepts `GD_INTRODUCES` through `GD_NEXT`; skill branch accepts prerequisites/mechanics; unknown keys skipped.
- R3: `skills/game-design-workbench/native/gd.h:8–25`: operation bounds and field/role enums. Bounds explicitly not author-project limits.
- R4: `skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/skill-atoms/jump-stomp.md:7–14,20–34` and `fall-stomp.md:7–14,20–34`: shared above-impact prerequisite/rule, distinct landing/falling prerequisite and expression; falling rank null and teaching encounter unclaimed.
- R5: same example, `challenges/first-gap.md:7–21,24–32` and `challenges/final-gap.md:7–22,25–33`: shared jump/death-and-retry mechanics and gap noun; different skill roles/demands and acknowledged route/geometry limits.
- R6: same example, `skill-atoms/land-on-target.md:7–14,20–26`, `control-fall.md:7–12,18–32`: branching ancestor evidence; shared `stop-at-edge`, additional jump branch for landing.
- R7: `skills/game-design-workbench/examples/crate-puzzle/game-design/skill-atoms/coordinate.md:7–13`, `plan-push.md:7–12`, `route-switch.md:7–12`: two-input AND combination versus one-input skill chains; rank and mechanic differences.
- R8: same crate example, `challenges/combined-room.md:7–26,30–32`: introduce-and-require coordinate, explicit direct plan-push/route-switch demands, mechanics/nouns declared; intent not proof.
- R9: both examples' `skill-atoms/.schema.md:1–11`, `challenges/.schema.md:1–21`, `skill-atoms/.usage.md:1–11`, `challenges/.usage.md:1–3`: declared links, AND semantics, null rank and learning caution. Plumber schema files retain starting-contract caveat; crate schema files identify live schema.
- R10: `skills/game-design-workbench/native/README.md:1–467` read in two slices. Relevant sections: Reader boundary; Skill understanding, combinations and unlocks; Focused relationships; Resource graph preview. Documentation corroborates rooted-only comparison and partial consumption; no scalability test performed here.
- R11: plumber `mechanics/walker-stomp.md:20–43,75–80`: falling/top-contact rule and unknown rebound/geometry. Crate `mechanics/crate-pushing.md:26–68`: energy/free-destination cases and unknown geometry; shared topology alone cannot interpret rules.
