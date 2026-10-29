# Original design-question audit

## Result and scope

Current tools answer declared relationships and selected conditional-rule hypotheses. They do not yet answer shared-outcome, actor-state, gameplay-role or resource-conversion questions end to end. Better layout does not close those gaps.

- Inventory baseline: `8e6e5d7`, accepted focus/readability checkpoint. Later Grid and Resource-report continuations noted below; original nine CLI probes remain historical evidence, not rerun results.
- Question authority: [original feedback](../../.pi/visualiser-review-notes.md). Current ownership: [todo.md](../../todo.md), Reviews 02–12 and Slice 6.
- **Narrow answer:** stated operation returns the stated declared fact, with limits. Not acceptance of the broader design question.
- **Partial:** source representation, kernel or inspection exists; requested question/result still lacks coverage.
- **Missing:** requested semantic analysis/presentation is not implemented. Not necessarily missing source data.
- Visual support below comes from renderer/control inspection and prior review evidence. No new pointer test performed for this audit. CLI results below were run read-only.
- No schemas, game facts, metrics, source winners or new canonical identities chosen.

## Resource-report continuation

- After accepted Grid checkpoint `89b54e3`, read-only `gd resources` report exposes existing C99 extraction to agents. No production direction, composed outcomes, route eligibility or actor-state applicability inferred.
- Live retained counts: crate 30 nodes / 17 relations, 2 known deltas; plumber 522 nodes / 462 relations, 42 known and 30 unknown deltas. Counts describe retained declarations, not role diversity, economic importance or complete outcomes. Explicit unknown-condition/effect declarations remain on 15 crate and 161 plumber cases.
- Plumber coin supply report retains `this + 1[coin]` as checked delta +1; `this + 1[coin] - {control-vars/coins-for-life}` and capped `min(this + 1[life], {control-vars/life-capacity})` retain unknown/unrecognized deltas. No unconditional award or complete conversion inferred.
- Evidence: `native/build/checks-1d2e38ad01004a36a3bfc17edda2a42e/resources-{crate-puzzle,little-plumber-guy-nes}.json`. Reproduce through `native/test.ps1`; new Node resource tests check guards, deltas/caps, source identities, scoped calls, missing/unloaded roles, malformed input and explicit route unavailability. Full Resource semantics remain Review 02/09 work.

## Questions and current answers

### Q1 — What happens when A meets B, in particular states?

**Partial.**

- Visual: unified Interactions Grid/graph retains conditional cases and source inspection. Resource preview shows direct case/participant/axis declarations. No actor-pair/state explorer or automatic dispatch across all relevant rules.
- Agent: `rules` returns typed actors, predicates and effects. `rules --case ... --assume ... --context ...` evaluates a selected case under explicit hypotheses; returns provenance, unknowns and conditional writes.
- Evidence E1–E3 below demonstrates actual outputs. A successful case evaluation is not a complete scene outcome or automatic case selection.
- Owner: Review 01 foundations; Review 09 navigation; Review 10 question surface.

### Q2 — Which distinct causes converge on the same outcome, such as death?

**Partial representation; missing outcome-level query/view.**

- Agent can inspect actor-qualified `axes/Life=dead` writes and origins. No existing command groups all causes into shared outcome destinations with retained gates and actor identity.
- Visual Resource nodes represent case-local/shared facets and roles, not a general actor/state/value outcome graph. A `death-and-retry` mechanic record is not the same concept as a particular actor becoming dead.
- Generic reachability, shape families and shared-neighbor marks do not supply missing outcome identity. Do not merge unrelated instances merely because noun/axis/state text matches.
- Owner: Reviews 02/09 outcome derivation and identity; Review 10 reusable convergence query.

### Q3 — How does vulnerability differ by form and hazard? Are pits harder?

**Partial individual-case evidence; missing comparative measure/view.**

- E1 returns powered contact proposals; E2/E3 retain conditional death proposals for small walker contact and tall-form gap contact.
- No complete form-by-hazard comparison, explicit denominator or accepted meaning of “common,” “rare” or “hardest.” Native findings must not convert case counts into observed death rates.
- Unknown applicability remains unknown. Absent writes are not proof of unchanged state; separate contact directions cannot be collapsed.
- Owner: Reviews 09/10. Choosing a prevalence/vulnerability measure remains material refinement, not a drawing fix.

### Q4 — Does a shell change from enemy to ally/non-enemy while remaining hazardous?

**Partial state transitions; missing gameplay-role analysis.**

- Source `mechanics/shell-travel` retains the underlying shellback noun through resting/sliding transitions. Typed cases cover walker defeat and harmful hero contact.
- Neither those states nor player harm establish an enemy/ally classification. No current report traces explicit gameplay-role changes or excludes transformed non-enemies from enemy–enemy richness counts.
- Preserve user distinction: origin, current role and ability to hurt player are separate. Do not infer allegiance from shell motion, noun origin or hazard status.
- Owner: Review 09 role contract/view; Review 10 role-aware comparisons. No new allegiance field adopted.

### Q5 — Where are interactions sparse, uniform or complex? What does a proposed change add?

**Partial structural inspection.**

- Visual: grid/graph, incident-role lenses, branch comparison, shapes and overview reveal authored structure. Matrix/connectivity CLI reports declared noun/verb cases and distinct active partners.
- These are not actor-pair/state interaction diversity, role-aware enemy comparisons or a before/after complexity measure. Matrix axes are nouns and verbs, not arbitrary actor pairs.
- Sparse/blank does not mean defect; richer structure does not mean better experience. No universal complexity/fun score selected.
- Owner: Reviews 08/10; Review 09 supplies state/role semantics where needed.

### Q6 — Which skills combine, and which familiar encounters change one ingredient?

**Narrow skill answer; partial encounter answer.**

- Skills UI/CLI expose explicit AND prerequisites, hypothetical knowledge, unlock relationships and sourced shared-link candidates. E4 verifies `fall-stomp` needs both supplied prerequisites; `jump-stomp` still lacks `land-on-target`.
- Scaffolding CLI returns authored introduction/practice/requirement checks and route witnesses. They are opportunities, not evidence of learning.
- Shared mechanic/prerequisite links do not identify a pedagogically meaningful changed encounter. Arrangement, timing, pressure, feedback and recovery remain incompletely represented/queried.
- Owner: Review 03 accepted bounded engine; Review 04 parked encounter work; Review 10 reusable comparisons. No automatic teaching claim from `first-walker`.

### Q7 — What foundations, mandatory connections and alternatives matter structurally?

**Partial human/agent parity.**

- Visual: bounded closure/AND, configured-start dominators, support/bypass paths and typed reachability layers exist. Scope, direction, source witnesses and computation limits matter.
- Agent: `skill-atoms`, `scaffolding` and `mda` expose their domain results. Existing CLI does not expose every generic graph-analysis layer across all six projections.
- Shared C99 APIs exist in `gd_graph_analysis.h`; an agent could compile a caller. That is not an existing CLI report, named query catalogue or automatic access to current viewer scope.
- Structural necessity is relative to configured starts and shown links. Not actual traffic, causality, teaching importance or physical feasibility.
- Owner: Review 05 broader discovery/mandatory-path umbrellas; Review 10 headless query surfaces.

### Q8 — Are resources existing nouns in systems, including environmental opportunities?

**Partial source model/inspection.**

- Resource metadata references canonical nouns; definitions retain ownership/lifetime prose and control links. Mechanics own effects and existing resource references. MDA provides separate declared mechanic/system links.
- Agent: `gd resources` now returns retained case/facet/role declarations, declared noun references and descriptive resource prose with provenance. This is not a resource-system participation/awareness/lifetime analysis.
- No finished resource-system participation/awareness/lifetime question report. Player-held pools do not exhaust environmental opportunity framing.
- Review 02 deliberately preserves existing frontmatter. Missing query coverage does not justify speculative system/axis mirrors or duplicate resource identities. Retain distinct roles/pools and unknown ownership.
- Owner: Review 02 contract/consumer work; Review 09 integrated presentation. Frontmatter conformance remains Review 12, not a parallel validator here.

### Q9 — How can one resource become another, through which rules and side inputs?

**Partial extraction; missing route delivery.**

- Preview retains direct conditions, enum/quantity writes, checked signed deltas, preservation and guarded explicit invocation links. `gd_resource_graph.h` explicitly excludes route/composed-outcome expansion and production direction.
- `rules --case` evaluates one simultaneous proposal with explicit mapped calls; it is not a sequence planner or resource-conversion route query.
- `gd resources` now exposes direct nodes, incidence relations and findings, with `routes.available: false` and incomplete coverage. Directed `--from ... --to ...` queries remain unimplemented; unavailable is not no path. Current preview/overview is incidence, not flow; layout cannot create production edges.
- Owner: Review 02 bounded headless graph/routes, then Review 09 presentation. Keep AND inputs, alternatives, pool identity, caps and unresolved conditions.

### Q10 — Which resources have few roles, and which connections are pivotal?

**Partial generic evidence; missing resource-specific answer.**

- Parser/binding/math findings expose some incomplete declarations. Incident counts and configured-start necessary connections can support inspection of the shown projection.
- No finished resource-role diversity or condition-aware flow-pivotal report. A high-degree record is not necessarily pivotal; low role diversity is not automatically a flaw.
- Need the actual resource graph/query semantics from Q9 before claiming flow-specific importance.
- Owner: Reviews 02/08/09/10, as specified in current tasks.

### Q11 — Which authored mechanics connect to systems, dynamics and aesthetics?

**Narrow declared-link answer.**

- MDA UI and `mda --from ... --direction ...` return paths and missing-link findings over supplied MDA records. E5 exercises a forward query from `mechanics/enemy-contact`.
- These are authored links, not evidence that a player feels the stated aesthetic. Rule and MDA snapshots give mechanic references different meanings; source association is not canonical semantic equivalence.
- Owner: existing MDA capability; Review 10 may package named questions without strengthening claims.

### Q12 — Can incomplete designs grow without losing unresolved references?

**Partial, not uniform across consumers.**

- Rule/resource bindings retain missing/unloaded/ambiguous distinctions; structured findings expose source references. Existing consumers also have type/parse errors and partial-output policies.
- Review 08 explicitly requires referenced-but-undefined visual placeholders. Do not mark that complete merely because a parser reports a finding or another view retains an unresolved reference.
- No blanket repair or auto-created records. Attendant still owns generic schema validation; raw Markdown remains authoritative and human-editable.
- Owner: Review 08 consumer inventory/placeholders; Review 12 later metadata repair.

### Q13 — Can arbitrary associative dimensions be inspected in 2D?

**Missing requested feature projection.**

- Force layout, shown-link distance focus, exact local shape families and binned adjacency overview are different tools. None implements selected high-dimensional feature projection.
- UMAP/t-SNE/PCA/MDS remain unselected; no feature schema, weights, missing-value policy or dependency adopted.
- Owner: Review 11. Accepted MDA focus is not acceptance of this capability.

### Q14 — Can these tools help design a level of the requested quality?

**Design objective remains open; not a machine verdict.**

- Rules, authored MDA intent, skill opportunities and inspection support design reasoning. Current graph coverage does not establish coordinated arrangement/timing, instructional contrast or player experience.
- No level-quality score or guarantee. Follow-up work must connect questions to authored examples and explicit evidence, not increase graph richness for its own sake.
- Owner: Slice 6 example evolution, Review 04 encounter design when resumed, Reviews 09/10 semantic questions and framework-guided human review.

## Concrete agent evidence

Nine final CLI invocations completed with `ok=true`, exit 0. Inputs: shipped plumber example, read-only; before/after SHA-256 hashes unchanged. No game model, experiment, trial or viewer launched by this audit.

Evidence: `skills/game-design-workbench/native/build/question-audit-99f501cacdf2468e83127f12eb15f624/`. `metadata.json` retains exact argv, baseline commit, executable hash, input hashes and final exit/error results. Per-command JSON/stderr remain ignored build artifacts. Source definitions and code references below are durable evidence if artifacts are cleaned.

- **E1 — `powered-tall.json`:** `interactions/touch-walker-powered#powered-side-contact`, Player alive/tall/unprotected, Walker alive, explicit ordered side-contact fact. `applicability=true`; proposals include Form `small`, Protection `protected`, ProtectionTime `1` in declared axis unit, and Life preservation `alive`. Both `constraints_complete` and `values_complete` remain false; `limited=false`. No Walker result invented.
- **E2 — `small-walker.json`:** small/unprotected Hero and explicit side-contact fact. Declared Life `dead` and `this - 1[life]` writes remain conditional; applicability/requirements `unknown`. Supplying known state/contact does not fill opaque authored conditions or deduplication semantics.
- **E3 — `gap-tall.json`:** tall-form hypothesis and explicit lower-boundary contact. Declared Life `dead` and life debit remain conditional, applicability `unknown`; constraints/values incomplete. The authored rule ignores form/protection, but the tool does not prove a complete lethal transition from these facts alone.
- **E4 — `skills.json`:** root `skill-atoms/fall-stomp`, known `stomp-walker` and `control-fall`. `fall-stomp`: two supplied prerequisites, none missing, `all_declared_prerequisites_supplied`. `jump-stomp`: `land-on-target` missing. Known `control-fall` does not auto-mark its prerequisite known; hypothetical knowledge is not observed learning.
- **E5 — `mda.json`:** forward query from `mechanics/enemy-contact`; returned nine node/path entries. This is supplied-source trace evidence, not observed experience.
- `matrix.json`, `connectivity.json`, `scaffolding.json` and `rules.json` retain corresponding source/case/role declarations and domain checks. They do not add the missing query surfaces listed above.

Representative case invocation, from repository root:

```powershell
$native = 'skills/game-design-workbench/native/build/gd.exe'
$game = 'skills/game-design-workbench/examples/little-plumber-guy-nes/game-design'
$inputs = foreach ($role in @('interactions','mechanics','nouns','verbs','axes','resources','control-vars')) {
    "--$role"; "$role=$game/$role"
}
& $native rules @inputs --related "systems=$game/systems" `
    --case interactions/touch-walker-powered#powered-side-contact `
    --assume Player:axes/Life=alive --assume Player:axes/Form=tall `
    --assume Player:axes/Protection=unprotected --assume Walker:axes/Life=alive `
    --context 'Player axes/Contact.sideContacts Walker' --fields analysis --json
```

Inspect `analysis.scenario` completeness, conditional writes and origin/via provenance. Do not read only the proposed state values.

## Original example observations retain ownership

The eleven numbered Mario observations in original feedback remain user-supplied reference observations, not independently verified facts or authorization to fill the fictional example. Their original comparison is a coverage inventory, not a fresh exhaustive record audit here.

1. First-screen idle danger/safe apron: Slice 6 arrangement; Q14.
2. Combined first enemy/block lesson: Review 04 and Slice 6; Q6/Q14.
3. Mushroom return timing: Slice 6; Q6/Q14.
4. Enemy rebound between pipes: Slice 6 interaction design; Q4/Q5.
5. Hidden block/life mushroom/generalization: Slice 6; Q1/Q6.
6. Enemy falling and all-form pit death: Slice 6 staging plus Q3.
7. Descending contact distinct from intentional jumping: accepted skill distinction; staging/teaching still Q6.
8. Incidental shell kick/hit chain and ally framing: Q4/Q6; Reviews 04/09 and Slice 6.
9. Vertical camera lock/above-viewport travel: Slice 6 authored behavior, not a graph-layout fix.
10. Destruction pleasure/treasure: Slice 6 and MDA intent/evidence distinction, Q11/Q14.
11. Star/contact-defeat sequence: optional design extension in Slice 6; Q1/Q6. Do not equate enemy-damage protection with general invulnerability.

Other retained authoring questions: separate run verb/air-control breakpoints; proximity-triggered activation; pipe/enemy timing; no-fall-damage and hidden-block coverage; shell-jump/coin/life account. Own them under Slice 6 and relevant Reviews 02/04/09. No thresholds, simulation geometry, new skill atoms or rules inferred by this audit.

## Delivery consequences

- Keep accepted focus/readability checkpoint separate from semantic-query completion. Animation, optional camera auto-fit and manual pins remain future tasks.
- Review 07 matrix grid/zoom/font work remains open. Resource root-oriented layout requires genuinely directed data, not current incidence.
- Review 02 is no longer waiting for Review 03 refinement: that bounded slice was accepted. Resource routes/composed outcomes still need delivery against the existing contract.
- Review 08 must assess actual unresolved-reference display per consumer. Review 09 owns actor/state/outcome/role presentation; do not substitute graph family labels.
- Review 10 needs named result/CLI contracts. Reusing a C99 kernel in a renderer does not alone expose a headless answer.
- This audit completes the support/ownership inventory only. It does not complete those tasks or establish usefulness of dense overview, new query metrics or all original design questions.

## Code and source anchors

- CLI availability/flags: `native/gd_cli.c`, `help()` and operation dispatch.
- Typed proposals/limits/provenance: `native/gd_rule_analysis.h/.c`, `gd_rule_evaluate`, `gd_write_rule_analysis`.
- Preview limitations/identity: `native/gd_resource_graph.h/.c`; no general shared actor/state/value destination nodes.
- Generic visual analysis interfaces: `native/gd_graph_analysis.h`, `gd_graph_analysis_ui.c`, `gd_explorer_view.c`.
- Explicit skill assumptions/candidates: `native/gd_skill_unlocks.c`; scaffolding: `native/gd_routes.c`; MDA paths: `native/gd_mda.c`.
- Plumber source: `game-design/interactions/touch-walker-{small,powered}.md`, `touch-gap-bottom.md`, `mechanics/shell-travel.md`, `axes/{Life,Form,Protection}.md`, `resources/retry-budget.md`. Paths relative to `skills/game-design-workbench/examples/little-plumber-guy-nes/`; native paths relative to the skill.

## Comments

- First skill probe incorrectly supplied `--challenges` to `skill-atoms`; rejected as documented. Corrected to the allowed skill-only binding. Rejected JSON retained as `skills-invalid-arguments.json`; final `skills.json` and metadata record the successful invocation. Not a product defect.
- Hypothetical actor labels and snapshot offsets identify supplied query participants, not discovered global runtime instances. Do not persist offsets as cross-snapshot identities.
