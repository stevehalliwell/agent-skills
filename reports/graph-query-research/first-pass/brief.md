# Graph-query research — delegated first pass

## Question and use

Which graph analyses could reveal useful, non-obvious structure in our game-design records, and how could users inspect those findings visually?

User approved five independent research lanes. Produce orientation and source discovery, not an exhaustive evidence answer or production implementation. Parent synthesizes a shortlist for later review. No deeper capture campaign without approval.

## Context

- Repository: `D:/Repos/agent-skills-game-design-workbench`.
- Task authority: `todo.md`, Research useful design-graph queries. Encounter discovery/similarity is a research subitem, not a manual pair-comparison implementation.
- Consumers: humans and agents exploring/designing games. Humans should not need to manually compare every pair. Graph traversal should derive relationships without authors repeating links.
- Existing records: skill prerequisite AND sets and mechanics; challenge introduction/practice/requirement/mastery-intent links and routes; mechanics, interactions, nouns, verbs, resources and actor/state rules.
- Existing native tools: C99 CLI and desktop graph explorers. Inspect relevant `skills/game-design-workbench/native/README.md`, collection schemas/usages and actual code before claiming a capability already exists or is easy to add. Current graph bounds include 4,096 records / 32,768 edges; research larger-scale trade-offs too, but do not impose those bounds on author data.
- Ground examples in `skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/` and `skills/game-design-workbench/examples/crate-puzzle/game-design/`. These are authored examples, not measured player-learning evidence or reverse-engineered game facts.
- Skill work is accepted. Resource graph preview remains partial. Do not treat `.pi/handoff.md` as current authority.

## Five coverage lenses

1. Structure/dependencies: bottlenecks, shared foundations, convergence, alternatives, clusters/isolates and pivotal connections.
2. Similarity/discovery: graph-wide relatedness, structural similarity, families and unusual variations; explainable findings rather than manual pair selection.
3. Game-design applications: documented applications to progression, puzzles, encounters and mechanic combinations; distinguish demonstrated uses from adaptations.
4. Visual exploration/interpretation: interaction with findings through selection, filters, overlays, supporting paths, legends and overview/detail.
5. Perceptual graph drawing/comprehension: drawing itself; layout, spatial grouping, routing, crossings, direction, labels, visual encodings and human comprehension.

Work only your assigned lens. Note overlap for synthesis instead of taking over sibling lanes.

## Evidence and bounds

- Public sources only. Prefer original research, authoritative technical documentation and documented applications. Vendor/community claims remain labelled claims. Neo4j/GDS is a reference, not a chosen dependency.
- Not a historical/current-market inquiry. Established techniques remain relevant; record software version/date when an implementation claim is version-sensitive.
- Selected guide: `C:/Users/steve/.pi/agent/skills/question-research/references/ai-assisted-research.md` — mandatory because AI performs discovery and interpretation. Read it before research. Other question-research branch guides do not apply unless a concrete new need arises; do not broaden the inquiry silently.
- Enable/use public-web search/fetch tools. Retain source discovery trail and bounded extracts supporting material claims. Never cite a search snippet as if the full source was read. Mark inaccessible sources and unverified leads.
- First-pass budget: at most 8 unique candidate source URLs per lane; repeated observations remain separate CSV rows. Aim for 3–5 promising capabilities/techniques, including limitations or an alternative that challenges the recommendation. Soft timebox about 20–30 minutes; report gaps rather than widening into a survey.
- Keep observation, paraphrase, inference, hypothesis and recommendation distinct. No inferred learning, causality, mastery or design quality from topology. A source's measured result may depend on graph size/task/population; preserve those limits.
- Structural similarity may support exploration without producing automatic design advice. No required similarity score or 2D projection. Do not mistake attractive drawings for demonstrated comprehension.

## Modification authority

Read repository inputs. Write only inside your assigned lane folder. User explicitly requested result folders, so file deliverables are required from the outset. No shared-file edits, app code, schema changes, record migration, source example edits, commits, pushes, browser UI automation, experiment reruns or additional agents. Do not install tools or change the environment. Use available research capabilities; report blockers.

## Required lane files

Create these under your assigned folder:

- `first-blush/answer-analyst.md`: provisional report with 3–5 strongest findings/techniques, sources, limitations and prioritized follow-up questions. Drawing lane uses comprehension problems/drawing techniques rather than inventing analysis metrics.
- `first-blush/evidence-extracts.md`: bounded source passages and stable locations for material claims; source URL, provenance label, exact extract, interpretation and limitations. Record fetch/read failures explicitly.
- `discovery/D-001/source-candidates.csv`: one row per observation, CSV header `url,title,source_type,evidence_label,why_retain`. Labels: `primary`, `expert-secondary`, `reported-secondary`, `community-report`, `discovery-signal`. Labels describe provenance, not truth. Preserve duplicate observations; do not exceed 8 unique candidate URLs.
- `discovery-journal.md`: append-only chronological searches, source trails, rejected leads and reasons for subsequent searches. Initialize before discovery; do not reconstruct a fictional journal afterward.

Do not create a full research register, source-map or capture queue in this first pass. Parent decides whether deeper retained-evidence work is warranted.

## Finding contract

For each promising capability/technique, include:

1. Question or comprehension problem addressed.
2. Concrete useful finding/visual change grounded in an actual plumber/crate record or graph. Label proposed adaptations as hypotheses, not observed results.
3. Method, source evidence and applicability limits.
4. Required data: already available, derivable by traversal, or genuinely missing. Distinguish schema-declared links from what current native readers consume.
5. Visible result and how users inspect supporting paths/source declarations. Analysis must visibly affect the graph, not only inspector text.
6. What it cannot establish; failure modes and counter-evidence.
7. Rough implementation/scale constraints and confidence. No invented performance measurements.

Cite only repository line ranges actually read. Do not invent source passages, source accessibility or empirical support. End with checks performed, unresolved gaps and whether a deeper pass could change the shortlist.

## Report back to parent

Parent pane: `w2:p1`. Stay in your own tab; never focus/close/control unrelated panes.

When files are ready, load `C:/Users/steve/.pi/agent/skills/herdr/SKILL.md`, verify `HERDR_ENV=1`, and use current CLI discovery as needed. Send one concise completion message, without `--wait`, to parent:

`herdr agent prompt w2:p1 "Research ready: <lane>. Report: <project-relative answer-analyst.md path>. Main findings: <short summary>. Limits: <short caveat>."`

If parent is blocked or the send fails, do not answer its dialog or retry blindly. Show a Herdr notification with lane/report path and leave your final response with the same information. Report blockers the same way if research cannot proceed. Do not wait for parent before finishing; leave your pane available for follow-up.
