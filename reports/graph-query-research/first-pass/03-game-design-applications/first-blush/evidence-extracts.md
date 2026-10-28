# Evidence extracts — game-design applications

Bounded orientation, observed 2026-10-28. Provenance labels describe source origin, not truth. Exact quotations below support only stated scope. Selected passages read, not every external paper in full. Repository inputs are authored fixtures, not player evidence.

## E1 — Solution graphlets reveal combinations, not just individual mechanics

- URL: https://centerforgamescience.org/files/assets/papers/chi2015_progression.pdf
- Source: Butler, Andersen, Smith, Gulwani and Popović, *Automatic Game Progression Design through Analysis of Solution Features*, CHI 2015. `primary`; implemented research system plus player experiment.
- Retrieval: public PDF successfully extracted by fetch tool; 10 pages. Read extracted pp. 1–8 and Discussion/Conclusion passages on p. 9. Extracted Markdown is tool-managed temporary material; quotations retained here.
- Location: p. 5, “Solution Features in Refraction”.
- Exact extract: “As the analogue to n-grams for procedures, we use graphlets, which are small connected non-isomorphic induced subgraphs of a larger graph”.
- Exact extract: “In each puzzle, we enforced the constraint that all potential solutions to the level share the same solution graph.”
- Exact extract: “This analysis could be extended to puzzles with multiple, significantly different solutions by considering which subset of features must show up and which optionally may be used to solve the problem, but we omitted this for sake of simplicity.”
- Paraphrase: system extracts small patterns of linked pieces from valid Refraction solutions, with an auxiliary fraction-value graph. Features distinguish combinations missed by checking component presence alone.
- Hypothesis: patterns of authored skill prerequisites can expose declared combinations for inspection. Those are not solution graphlets unless an actual solution representation exists.
- Limit: same-graph guarantee is a generator constraint, not a general result for games or this repository.

## E2 — Bare ordering is not a progression policy; empirical support has limits

- URL/provenance: same primary paper as E1.
- Location: p. 6, “Creating a Progression”.
- Exact extract: “A simple approach would be to traverse the graph of partially-ordered problems in an order consistent with a topological sorting of the graph.”
- Exact extract: “It does not allow for control over the rate at which complexity increases or control over the balance between when new concepts (1-grams) are introduced versus combinations of concepts (2-grams and 3-grams).”
- Location: p. 8, Evaluation.
- Exact extract: “With our current results, we cannot be sure the progression had any effect on player engagement at all.”
- Observation/paraphrase: study compared 1,221 automatic-progression players with 1,156 expert-progression players; median time 184 versus 199 seconds; no significant time-played difference reported. No random-progression baseline. Completed puzzles differed, but puzzles themselves differed between conditions. This is not proof of equivalence, mastery or educational benefit.
- Location: p. 8, “Difference Between Progressions”.
- Exact extract: “This is because, at that point in the progression, our system had run out of new concepts to introduce. Our cost model then tends to choose very similar levels over and over.”
- Location: p. 9, Discussion.
- Exact extract: “However, if the goal is merely to produce a good game with a reasonable level progression, it is unclear that this system would be beneficial, even for domains well-suited to this approach.”
- Interpretation: shortlist inspection aids before automatic sequencing; preserve designer control and costs of feature selection/generation. Paper supplies counter-evidence to treating topological sort as adequate progression advice.

## E3 — Mechanic paths used in generated tutorials

- URL: https://arxiv.org/html/1807.04375v2
- Related candidate: https://arxiv.org/abs/1807.04375
- Source: Green et al., *AtDELFI: Automatically Designing Legible, Full Instructions For Games*, FDG 2018, arXiv v2. `primary`; demonstration/evaluation on eight GVG-AI games, not evidence of production adoption.
- Retrieval: abstract and selected full-HTML sections 4.1.3, 5.1.3, 5.4, 6 and 7 readable.
- Location: §4.1.3, Critical Paths, anchor `#S4.SS1.SSS3`.
- Exact extract: “We define critical paths as the series of mechanics starting from an explicit player input and ending with a winning or losing terminal state.”
- Location: §5.1.3, anchor `#S5.SS1.SSS3`.
- Exact extract: “Afterwards, it sorts these paths by length and picks the shortest path for every terminal mechanic. Finally, it iterates over every avatar sprite and picks the longest ‘shortest path’ for each terminal mechanic.”
- Location: §7 Conclusion, anchor `#S7`.
- Exact extract: “The RealPortals tutorial did not at all explain that the player can travel through the use of portals, or that using the portals to move around is required in every map of the game.”
- Exact extract: “This is because critical paths are built using just VGDL information in the current system, and the knowledge that one would have to use portals to win would only be obtained by an actual play-through of the game.”
- Paraphrase: authors show game-rule chains can select tutorial material, but explicit-rule topology misses map-specific necessities. None of their agents won RealPortals, leaving video evidence incomplete.
- Hypothesis: show candidate finish/hazard mechanic chains with unknown preconditions and source declarations. Do not call a shortest record path a necessary playable strategy.
- Scale limit: their implementation describes finding every possible avatar-to-terminal path before selection. Do not copy path enumeration into cyclic/larger author graphs. No repository performance inference.

## E4 — Economy graphs need executable semantics and objective constraints

- URL: https://arxiv.org/html/2404.18574v1
- Related candidate: https://arxiv.org/abs/2404.18574
- Source: Rupp et al., *GEEvo: Game Economy Generation and Balancing with Evolutionary Algorithms*, IEEE CEC 2024 / arXiv v1. `primary`; benchmark plus fictional mage/archer case study.
- Retrieval: abstract and selected full-HTML §§II, III-A, IV-B2 and V readable.
- Location: §III-A, anchor `#S3.SS1`.
- Exact extract: “A game economy can be considered as a directed graph which defines how resources of different types are generated and transitioned to other resource types.”
- Exact extract: “We only deal with fixed economic systems, where all resources come from the economy itself.”
- Exact extract: “Weights on the edges describe how many resources flow from one node to the adjacent one. Depending on the node type, the weights are absolute values or probabilities.”
- Location: §V Discussion and Limitations, anchor `#S5`.
- Exact extract: “However, there is no setting where all economy graphs could be balanced.”
- Exact extract: “While the probabilities for normal and critical hits still differ, it equalizes the damage for both hits and thereby mitigates the randomness.”
- Exact extract: “At the one hand, it fits the balancing criteria, at the other it might not be a desirable solution since now the intention of the economy design is obsolete.”
- Paraphrase: balance uses repeated simulations and explicit fitness objectives, not graph degree. Optimizing damage equality can remove intended critical-hit distinction. Authors also discuss unattainable targets and local optima.
- Hypothesis: inspect which rule declarations spend a shared resource; require explicit occurrence/timing/policy assumptions before simulating cumulative cost. No automatic tuning recommended here.

## E5 — Puzzle methods are domain-dependent

- URL: https://www.scss.tcd.ie/mads.haahr/papers/de-kegel-2020-transactions.pdf
- Source: De Kegel and Haahr, *Procedural Puzzle Generation: A Survey*, IEEE Transactions on Games 12(1), March 2020. `expert-secondary`; survey, not independent confirmation of each method.
- Retrieval: PDF extracted successfully, 20 pages. Only opening two pages read substantively; no claims based on unreviewed survey method sections.
- Location: printed p. 22, §I-B Review Methodology.
- Exact extract: “Work on procedural puzzle generation has mostly focused on specific puzzles, because creating a generator requires at least some knowledge of specific puzzle rules.”
- Interpretation: supports caution about generic puzzle graph advice. Not a substitute for original method papers. No new puzzle-generation recommendation drawn from this survey in first pass.

## E6 — Inaccessible lead

- URL: https://ojs.aaai.org/index.php/AIIDE/article/view/12477
- Candidate: Joris Dormans, *Simulating Mechanics to Study Emergence in Games*, AIIDE 2011; `primary` by origin, unverified content here.
- Fetch result: `Error - fetch failed`. No usable paper passage obtained.
- Search identified Machinations as relevant; search synthesis is discovery only, not evidence of effectiveness or adoption.
- Crawl4AI fallback skill read. Brief forbids environment changes; no local service started or tools installed. Leave lead unresolved; do not claim full-source review.

## Repository evidence and reader boundaries

Paths below relative to repository root. Line ranges refer only to files actually read.

- `skills/game-design-workbench/examples/crate-puzzle/game-design/challenges/combined-room.md:7–32`: introduces coordinate; requires plan-push, route-switch and coordinate; explicitly no physical map and no observed learning.
- `skills/game-design-workbench/examples/crate-puzzle/game-design/challenges/push-room.md:7–33`: introduces push; next links permit switch-practice or combined-room; assistance/observed learning unspecified.
- `skills/game-design-workbench/examples/crate-puzzle/game-design/skill-atoms/coordinate.md:8–13`: AND prerequisites plan-push/route-switch; shared crate-pushing mechanic.
- `skills/game-design-workbench/examples/crate-puzzle/game-design/skill-atoms/.usage.md:3–9`: author contract distinguishes understanding, contextual application, AND prerequisites, comparison suggestions and unobserved mastery.
- `skills/game-design-workbench/examples/crate-puzzle/game-design/challenges/.schema.md:3–15` and `.usage.md:3`: schema declares mechanics/interactions/nouns/verbs/resources plus route/skill roles. Native challenge loader does not therefore consume every one.
- `skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/challenges/finish.md:8–28`: move-right requirement, finish-approach practice, course-finish mechanic; terminal challenge is not campaign inference.
- `skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/interactions/finish-post-contact.md:18–39`: completion/clock/hazards in prose and unknown effects; alive requirement explicit; same-frame death/finish order unresolved.
- `skills/game-design-workbench/examples/crate-puzzle/game-design/resources/player-energy.md:9–16`: resource noun plus initial/capacity controls; per-player-per-challenge ownership; no regeneration/conversion defined.
- `skills/game-design-workbench/examples/crate-puzzle/game-design/resources/.schema.md:3–6` and `.usage.md:3`: pool configuration links declared, behavior owned elsewhere.
- `skills/game-design-workbench/examples/crate-puzzle/game-design/mechanics/crate-pushing.md:26–68`: successful push spends push-cost; insufficient-energy/wall cases preserve energy; geometry unencoded.
- `skills/game-design-workbench/examples/crate-puzzle/game-design/interactions/free-push.md:34–45`: spatial binding unknown; numeric guard/write explicit. Energy participant has no explicit pool binding here. Matching Energy label is not pool identity.
- `skills/game-design-workbench/native/gd_core.c:356–387`: consumes resource noun; challenge entry plus skill-role/next fields; skills prerequisites/mechanics; generic rule/case relationships separately. Resource initial_control/capacity_control and challenge mechanics are not consumed merely because schemas declare them.
- `skills/game-design-workbench/native/gd_routes.c:6–16,65–130`: bounded queue scratch; demand closure through prerequisite links; two-state prior-opportunity walk, not route enumeration.
- `skills/game-design-workbench/native/gd_resource_graph.c:31–68,90–131,157–185`: facets stay distinct absent resolved explicit binding; graph captures requirements/state/quantity writes and compiled invocations with unknown/limited counts. It is not a full gameplay transition graph.
- `skills/game-design-workbench/native/README.md:33–48,177–229,246–268,422–430`: resource preview incomplete; skill comparison rooted not graph-wide; route witnesses already implemented; record/relationship bounds explicit. Full README read in two tool calls.
