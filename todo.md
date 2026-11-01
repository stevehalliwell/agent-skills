# Game Design Workbench — work ahead

Current task authority. Check off steps and their substeps as completed. Detailed contracts and earlier delivery evidence: [workbench specification and history](workbench-specification-and-history.md). Historical unchecked items are not current implementation status.

## Verification rule

**Until final review, use CJIT-only build checks.** Run relevant CJIT headless/native/CLI tests; compile UI without launching windows. Do not run Zig, alternate compilers or the full compile-mode matrix during intermediate steps. All supported compile modes/compilers are checked at Step 6, not after each slice.

No focus stealing, OS pointer/global-key input automation or unsolicited GUI launches. Preserve existing windows and drafts. Coordinate window-based checks with user at final review.

## Steps

- [ ] **Step 1 — Bidirectional CLI/viewer IPC**
  - [x] Refine first bounded slice: Windows current-user-only named pipes, versioned JSON, GUI-owner dispatch, PID/nonce session identity, explicit targeting on ambiguity, non-consuming inspection, separate queued hints/direct source-Find navigation and explicit bounds. No network listener or automatic mutation retries. Local reference implementations inspected.
  - [ ] Let human point at GUI context and let agent read what human is looking at.
  - [ ] Let agent drive shared investigation through IPC: discover available controls/options, change views/display modes/selections, fill inspector values and explicitly apply/run analysis. Human must not transcribe agent instructions into fields. Inspection/reveal alone does not meet this requirement.
  - [ ] Inventory every investigation control/action across all views; identify identical/similar behavior and refactor duplication into shared controls/actions before adding parallel IPC handlers. GUI and IPC reuse control descriptions, valid options, validation and application paths where semantics match; retain domain-specific differences. Reduce code and per-view CLI plumbing, not merely hide copies behind wrappers.
  - [x] First shared refactor: Actor/Effect, structural/similarity and rule/binding forms share descriptions/defaults/equality/primitive editors/inspection, selector transitions and application actions. Expose action discovery/execution plus guarded source refresh over IPC. CJIT-only core/protocol/CPU/Node checks pass; no full parameter-control or walkthrough readiness claimed. [Inventory, evidence and remaining work](skills/game-design-workbench/native/controls.md).
  - [x] Add typed discovery/configuration for Query, Structure and Rules forms: exact/scoped scalar keys, native text syntax, pending/applied values, stale-input tokens, guarded atomic staging and explicit Run/Display. CPU loops cover finite Actors, Resource bindings and discovered similarity features/map; no live acceptance claimed.
  - [x] Add Resource witness form and shared Run/Clear: stage FROM/TO, accepted-only and work ceiling; retain labelled applied witnesses/highlights and selection/filter on pending edits or failed Run. Protect cold pending forms on refresh; reload cold applied analysis; never apply staged Actor/Effect inputs during first canvas preparation. CJIT-only checks pass; live review remains open.
  - [x] Prepare joint IPC control/result review build: 49 presentation fields, shared display actions, typed link/cell/case/rule selection, native retained result byte pages, complete byte reassembly, diagnostic loss metadata and review hardening. CJIT core 128/128; targeted Node 21 passed/0 skipped; both examples pass private-pane loops. Existing GUI untouched; no live acceptance claimed.
  - [ ] Complete final application-control parity review through shared controls/actions. Source-change path: normal authorized disk edits, then explicit GUI refresh; no IPC skill draft edits/Undo or speculative changes in user buffers. Refresh must not silently discard active edits, dirty drafts or retained Undo history. Preserve human-input ownership, pending/applied distinctions, existing drafts/Undo and last-good failure behavior.
  - [x] After shared-form/action refactor, move hint into separate ImGui window; no inspector child/reserved area. Preserve separate hint ownership, guarded Go/Dismiss and no focus on appearance. CPU UI-frame checks pass; live placement/readability acceptance remains deferred.
  - [x] Add passive toast for successful CLI navigation/action/refresh; no input capture/focus, no notice for human Go. Timing/coverage and CPU checks documented in control inventory. Live acceptance remains deferred.
  - [x] Demonstrate agent-driven configure/run/inspect loops without manual field transcription: joint Mario tour covered Skills foundations, Matrix cell/case inspection, finite Actor Form variants and Resource witnesses. Separate hint/Go and inactive-pane reads demonstrated; draft remained clean. User found tour useful and supplied three follow-up UX items below. Full every-view/mode/option usefulness and destructive safety checks remain open.
  - [ ] Preserve independent panes, exact identities, pending/applied state, drafts/Undo and atomic last-good reload. IPC does not authorize authoritative source writes.
  - [x] Implement refined continuation: exact native-node select/frame for direct navigation/human Go, read-only paged node discovery and opt-in selected loaded source body. Reject hidden/missing/ambiguous/stale targets; no fabricated IDs, source fallback or pending query application.
  - [x] Complete IPC CJIT-only checks; record supported behavior and remaining limits. Native 128/128; targeted Node 18 passed/0 skipped; 215 CPU-only adapter checks per example; graph/explorer test compiled without windows. Evidence and limits in IPC contract.
  - [x] Initial user-requested joint Mario smoke review: human selection/source read; separate hint and human Go; direct exact death-result selection/frame/source without OS focus steal; independent Actors selection restored; Dismiss retained selection/camera. Draft clean. User confirmed observed behavior; no broader usefulness acceptance inferred.
  - Implemented: `gd viewer list|inspect|nodes|hint|navigate`, queued hint Go/Dismiss, pane/query/draft/source context, guarded source-Find/projection navigation and exact native-node selection/framing. Four analytical form families plus presentation now configure through IPC. Added retained native result byte pages, evidence/diagnostic pages and typed link/cell/case/rule selection. Serializer fail-fast/recursive-read hardening and production-dispatch byte reassembly/inspection checks pass. Joint interactive Mario tour completed; user feedback captured in Step 2. Step 1 and full application-control parity/usefulness acceptance remain open. Tour does not establish every-control or destructive safety acceptance. Also retain live active-input/pending-edit guards, draft/reload interactions and broader targeting/usefulness review. Unchecked human-context items above remain open for that review. No unsolicited GUI launch or acceptance inferred from CPU/headless checks.
  - Inspect local reference projects before settling requirements/transport; no copying or adoption implied:
    - Attendant C99: `D:/Repos/agent-skills-attendant-native/exploration/attendant-native/apps/gui/README.md`, plus `inspect.{c,h}` / `reveal.{c,h}` there. Read human-selection ownership, dirty/revision context, explicit agent hints and focus-preserving inspection/navigation.
    - Ink player: `D:/Repos/c-ink/tools/story-player/README.md`, plus `ipc.c` / `protocol.c` there. Read shared-session discovery/targeting, bounded request/reply, GUI-owner dispatch, security and timeout/unknown-mutation handling.
  - Contract: [IPC requirements](workbench-specification-and-history.md#bidirectional-cliviewer-ipc).

- [ ] **Step 2 — Remaining UX improvements before renewed review**
  - [ ] **Expand shared force layouts — refinement in progress.** Add Free graph to Skills (retain node band annotation), Challenges (directed seed), Relationships, Actors and Effects; preserve all current layouts. Implement shared footprint-aware node/label separation first, then bounded edge-crossing reduction as separate delivery step, including Resources. MDA Free graph and grouped Interactions Free graph are visual references. Audit/refactor existing shared solver rather than introduce per-view solvers. Preserve typed identities, meaningful groups, selection, applied analysis and GUI/IPC parity. All nodes, including data roots, may move in Free graph; existing constrained modes retain root/band behavior. Shared Initial arrangement choice: Native (default) / Neighborhood; changing seed repositions nodes and restarts settling without resetting selection, scope or analysis. [Requirements, source audit, delivery slices and acceptance](reports/graph-query-research/shared-force-layout-plan.md).
  - [ ] Refine and repair Resource group separation/node overlap. Full view remains unaccepted; witness filtering does not fix it. Do not invent directed flow or missing data.
  - [ ] **Preserve and follow selection across presentation/analysis views — needs refinement.** User reports selecting brick × bump in Interactions Grid, then switching to Free graph leaves nothing visibly selected. Preserve corresponding selection when changing layout/mode; apply consistent selection following through app so sources can be traced across analysis views. Exact cross-view mapping and interaction with independent remembered panes remain TBD; do not treat existing CPU selection checks as live acceptance.
  - [ ] **Make CLI toast easier to notice — needs refinement.** User saw passive toast but found it too brief. Consider longer visibility and/or animate in/out. Duration and animation remain TBD.
  - [ ] **General focus-selection button and CLI action — needs refinement.** Provide consistent selection framing across views/modes. User reports MDA still frames entire vertical stack, making text unreadable. Frame selected node(s) and their links without zooming out so far that text becomes unreadable. Exact framing/readability policy remains TBD; existing whole-view/cell/witness fitting and distance-focus layout do not establish this behavior.
  - [ ] **Consistent search/filter/scope language — needs refinement.** Distinguish sidebar list search, canvas display filters and analytical scope across projections/modes. Similar-looking controls must disclose their different reach; exact wording and placement TBD.
  - [ ] **Consistent pending/applied/display behavior — needs refinement.** Reconcile GUI/IPC presentation application and immediate neighborhood/MDA exploration versus staged analytical questions. Make pending inputs, applied scope and drawn results clear. Exact application policy TBD; preserve last-good results and human-input ownership.
  - [ ] **Consistent action availability and Reset/Clear semantics — needs refinement.** Align enabled/disabled feedback and reasons across GUI/IPC. Distinguish resetting inputs, clearing results and resetting current view; account for Actor inventory return and draft/Undo guards. Exact behavior TBD; do not discard human work.
  - [ ] **Contextual analysis entry points — needs refinement.** Make relevant analyses discoverable from selected item rather than requiring knowledge of which projection hosts tool, especially rule/binding inspection currently under Resources. Placement and supported entry points TBD; no duplicate evaluator implied.
  - [ ] **Consistent evidence-list paging — needs refinement.** Align counts, paging, truncation notices and source-navigation affordances across result/member lists. Distinguish GUI list limits from available native evidence. Exact list conventions TBD.
  - [ ] **Clear similarity navigation — needs refinement.** Clarify current canvas, selection and return destination when switching between similarity and ordinary source inspection. Reconcile Fit/comparison/paging affordances and remaining GUI/IPC navigation differences. Exact interaction contract TBD.
  - [ ] Resolve further scoped usability blockers found through shared GUI/agent context. Record unresolved decisions rather than invent behavior.
  - [ ] Check changes with CJIT only. Do not reopen accepted bounded work without evidence.
  - Contract: [readability requirements](workbench-specification-and-history.md#review-07--readable-graphs-and-usable-controls).

- [ ] **Step 3 — Assess choices, dilemmas, calculations and related distinctions**
  - [ ] Define distinctions needed for options, choices, calculations, puzzles, dilemmas, etc. “Player choice” alone is not sufficient design justification.
  - [ ] Assess what current data/tooling can classify or detect; identify evidence, missing capabilities and limits.
  - [ ] Report supported distinctions and remaining questions before UX review. No taxonomy, automatic classifier or implementation selected by this task.
  - Context: [pre-review capability assessment](workbench-specification-and-history.md#next-integrated-review-checkpoint--current-feedback-coverage).

- [ ] **Step 4 — Human UX review and Actors/shared-death walkthrough**
  - [ ] Explain what actor-state analysis does after next few improvements.
  - [ ] Demonstrate how to see many actions leading to shared concept of death. Distinguish all-contributor discovery from causes for a specific target/result; preserve participants, guards, source evidence and unknowns. No contributor count or usefulness acceptance assumed.
  - [ ] Review navigation, controls, source inspection and actual usefulness across both examples, using IPC context.
  - [ ] Review MDA/Resources, graph/path readability, dense adjacency/incidence explanation, focus transitions, camera options and cursor-offset picking.
  - [ ] Complete user-owned draft interaction checks: rank/prerequisite edits, Undo, dirty refresh, failed reload/output retention and successful handoff.
  - [ ] Account for review notes; record acceptance or explicit objections. Correctness/rendered captures alone do not establish usability.
  - Sources: [integrated notes](reports/graph-query-research/integrated-review-notes.md), [visualiser observations](.pi/visualiser-review-notes.md), [current UX contract/evidence](reports/graph-query-research/human-inspection-ux.md).

- [ ] **Step 5 — Revisit all fourteen original Mario-question groups**
  - [ ] Use [original 14-question audit](reports/graph-query-research/original-question-audit.md) as question inventory, not as proof questions are answered.
  - [ ] Revisit **all fourteen original question groups**, especially partial/unavailable answers. Determine whether latest tools, IPC and improvements now give better answers.
  - [ ] Review actual visual answers and agent-usable CLI results together. Retain conditions, provenance, unknowns and remaining tool/data gaps.
  - [ ] Update answer/support assessment and record user acceptance or remaining objections for each group. Do not close review from catalogue entries or automated tests alone.

- [ ] **Step 6 — Final review and complete validation matrix**
  - [ ] Rebuild and check **all supported compile modes/compilers**, including CJIT headless/plain-view/graph-view/explorer tests and configured alternate C99 compiler paths. Record any unsupported or unavailable check; do not imply a pass.
  - [ ] Run full relevant native/CLI/Node integrations and agreed window-based checks. Coordinate windows with user; do not steal focus or manipulate existing review app/drafts.
  - [ ] Recheck record conformance if later changes affect schemas/data; retain completed strict-validation evidence. Do not reopen completed Review 12 solely because historical checkbox remains unchecked.
  - [ ] Review entire committed branch and history: code/tests, instructions/guides, templates/examples, reports, provenance/rights and accidental private/unrelated content.
  - [ ] Resolve blockers or record accepted limits; retain separate human usefulness and interaction acceptance.
  - [ ] Obtain explicit approval before push, publication or history rewrite. Local checkpoint approval is not publication approval.
  - Contract: [full final gate](workbench-specification-and-history.md#final-gate--review-committed-branch-assets-before-pushcompletion).

## Deferred — not current implementation steps

- [ ] Better default font, remaining layout/path refinements and manual node pinning; refine when scheduled.
- [ ] Review expression/category overlap after practical use; avoid duplicate facts and parallel definitions.
- [ ] Control-variable JSON storage migration, explicitly deferred.
- [ ] Remaining framework-guide human review; preserve provisional/source-limited coverage and distinct experience/MDA lenses.
- [ ] Optional graph-backend investigation, simulation batch/sweep/replay/native result views and further experiment cases only when requested. No backend/dependency adoption or experiment rerun implied.
- [ ] Reconcile historical verification gaps with current tests before final review; do not rerun or reimplement every historical unchecked item blindly.

[Earlier contracts and deferred detail](workbench-specification-and-history.md#work-plan) remain preserved. No scope discarded by tracker split.

## Working constraints

- Markdown records authoritative; public Attendant owns managed metadata. No root task collection or parallel tracker.
- Preserve types, units, direction, AND semantics, conditions and incomplete scope. No invented gameplay facts, runtime actor merging, pools or applicability/execution claims from association.
- No unrelated migrations, frozen-evidence edits, experiment reruns, dependencies or publication by implication. Commit only with user approval.

## Comments

- Context-first UX committed in `b1c08d9`; deferred walkthrough/IPC priority in `db1922a`. Human acceptance remains open.
- Earlier validation includes native 128/128 plus CLI, Node 58 passed/1 skipped and compile-only Zig UI checks. Those historical checks do not override current CJIT-only rule before Step 6. Earlier owned-window evidence predates focus prohibition.
