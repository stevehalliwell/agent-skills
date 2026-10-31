# Game Design Workbench — work ahead

Current task authority. Check off steps and their substeps as completed. Detailed contracts and earlier delivery evidence: [workbench specification and history](workbench-specification-and-history.md). Historical unchecked items are not current implementation status.

## Verification rule

**Until final review, use CJIT-only build checks.** Run relevant CJIT headless/native/CLI tests; compile UI without launching windows. Do not run Zig, alternate compilers or the full compile-mode matrix during intermediate steps. All supported compile modes/compilers are checked at Step 6, not after each slice.

No focus stealing, OS pointer/global-key input automation or unsolicited GUI launches. Preserve existing windows and drafts. Coordinate window-based checks with user at final review.

## Steps

- [ ] **Step 1 — Bidirectional CLI/viewer IPC**
  - [ ] Refine transport, session targeting, context payload, supported commands/events and security boundaries. None selected yet.
  - [ ] Let human point at GUI context and let agent read what human is looking at.
  - [ ] Let agent show referenced things and change displayed context through application controls, not mouse/key automation.
  - [ ] Preserve independent panes, exact identities, pending/applied state, drafts/Undo and atomic last-good reload. IPC does not authorize authoritative source writes.
  - [ ] Complete scoped CJIT-only checks; record supported behavior and remaining limits.
  - Inspect local reference projects before settling requirements/transport; no copying or adoption implied:
    - Attendant C99: `D:/Repos/agent-skills-attendant-native/exploration/attendant-native/apps/gui/README.md`, plus `inspect.{c,h}` / `reveal.{c,h}` there. Read human-selection ownership, dirty/revision context, explicit agent hints and focus-preserving inspection/navigation.
    - Ink player: `D:/Repos/c-ink/tools/story-player/README.md`, plus `ipc.c` / `protocol.c` there. Read shared-session discovery/targeting, bounded request/reply, GUI-owner dispatch, security and timeout/unknown-mutation handling.
  - Contract: [IPC requirements](workbench-specification-and-history.md#bidirectional-cliviewer-ipc).

- [ ] **Step 2 — Remaining UX improvements before renewed review**
  - [ ] Refine and repair Resource group separation/node overlap. Full view remains unaccepted; witness filtering does not fix it. Do not invent directed flow or missing data.
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
