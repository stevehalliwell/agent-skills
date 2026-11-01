# Todos

## Promote accepted Attendant spike into maintained skill

Status: implementation under review; rollout not approved.

Native CLI/GUI spike-like accepted. Decision checks record agreed direction; delivery checks record completed work. Implementation authorized; rollout still requires explicit approval.

### Decisions

- [x] Complete and accept native CLI/GUI spike-like.
- [x] Determine final layout of Attendant native.
  - Product lives in ordinary `skills/attendant/native/` folder: `apps/cli/`, `apps/gui/`, `src/`, `tests/`, `tools/`, `vendor/`, root build entry points and ignored `build/` output.
  - Skill keeps `SKILL.md`, `references/` and thin launcher under `scripts/`. No build/runtime dependency on `exploration/` or `spikes/`.
  - Building on demand is acceptable: if native tool is not built, skill directs agent to a separate build reference, then agent returns to original task. Prebuilt distribution is not required.
- [x] Determine final feature set and what to prune from spike-like.
  - Keep native CLI and GUI as one product over shared core. CLI works without GUI; skill provides agent guidance and integration.
  - Keep all nine CLI commands, current GUI editing/navigation/preview/reference/feedback workflows, and agent reveal/inspection/feedback discovery/resolution.
  - Keep shared core, pinned dependencies, CJIT/Zig builds, focused native regression tests and reusable performance checks. Promote required RichMd bridge/patch code from spike into maintained source.
  - Prune JS/native equivalence checks and redundant spike-era test permutations. Native is no longer required to repeatedly prove parity with retired JS version. Keep enough direct native coverage to establish core correctness and protect important failure cases; review retained coverage before deleting tests.
  - Exclude historical SQLite comparisons, frozen comparison baselines, experiment-only generators, superseded demo hosts, reports and captured evidence from final product tree. Preserve history through Git and README links.
  - Git integration and renderer recovery remain outside this scope. Feature decisions do not complete pruning or release qualification.
- [x] Decide JS version's future: remove from maintained skill/product after native rollout. No supported fallback or separate JS home.
  - Document former JS version in final README with historical commit hash or tag. Current reference: `330136285cb1470ccecc00ca244894c9de2122a3` contains `skills/attendant/scripts/attendant.mjs`; tag name remains open if used.
  - Removal remains future work; current launcher stays unchanged until rollout approval. Update JS-dependent tests as part of removal.
- [x] Decide repository home: keep Attendant here under `skills/attendant/native/`, without submodule or nested repo.
  - Keep product slim. Revisit own repo if measured footprint or maintenance becomes a problem.

### Delivery

- [x] Move maintained CLI, GUI, shared core and required RichMd bridge/patch into agreed `skills/attendant/native/` layout. Update paths so builds/runtime no longer depend on `exploration/` or `spikes/`.
- [x] Prune historical experiment machinery, superseded demos and reports/evidence from final product tree. Keep history accessible through Git and documentation links.
- [x] Reduce tests to focused native coverage. Remove JS equivalence checks and redundant permutations; retain direct assertions for important behavior and failure cases, plus reusable performance checks.
- [x] Add build reference and native skill/launcher guidance. Missing native build directs agent to build reference, then back to original task. Keep active JS launcher unchanged until rollout approval.
- [x] Verify independent CJIT/Zig builds and focused CLI/GUI checks on disposable workspaces. Record results and remaining release limitations.
- [ ] Review/amend imported product and record user's planned macOS testing findings. Current implementation/builds are Windows-specific; macOS support/testing is not complete.
- [ ] Obtain explicit rollout approval before changing active launcher or removing JS.
- [ ] After approval, switch launcher to native, remove JS runtime and remaining JS-dependent tests, and link former JS version by commit hash or tag in final README. Verify skill workflow after switch.

### Checks

- Windows x64: independent CJIT/Zig CLI and API builds; Zig GUI dependency DLL; production/test GUI and model builds for both C99 compilers. Dependency revision/patch verification passed offline using copied local checkouts; no acquisition run.
- Direct native checks: 20 CLI groups, 92 API cases, 193 model assertions per compiler, 18 GUI/model groups including 16 real Win32/D3D windows. No JS comparison oracle. Results under `skills/attendant/native/build/{cli,api,gui}-check-results.json`; final GUI run `gui-check/run-WOrXrc`.
- Import proof: 38 checked source files byte-identical to snapshot; only CLI runtime edit is executable name in help. Stale GUI test-state/fault input guards and missing-DLL build guidance passed. Protected old viewer binaries/DLLs unchanged. Details: `skills/attendant/native/build/import-proof.json`.
- Synthetic process benchmark ran with rotating serial compiler order and reported variability. No representative performance or GUI-footprint claim. Existing unchanged JS runner's 7 tests still pass; these are transition checks, not imported native tests.
- Skill frontmatter/local links, Bash/Node syntax and full candidate whitespace check passed. Pinned upstream vendor formatting remains byte-preserved. Active JS launcher unchanged; rollout remains unapproved.
- Remaining qualification: authoritative-workspace/human rollout acceptance, real-project integrated loop, GUI startup/idle/interaction footprint, distribution/license audit and broader platform/failure limits. See `skills/attendant/native/README.md`; renderer draft recovery remains explicitly deferred, not reopened by cleanup.

### Historical context

Accepted runtime snapshot: `330136285cb1470ccecc00ca244894c9de2122a3`, already published on `feat/shared-markdown-gui`. Import selected files, not its commit sequence. Local import used planning snapshot `528f091cc11fc174780b3cc9448a4c7bca8c4c1a`; runtime/bridge sources are identical to published snapshot. Historical documents are not current delivery checklists.

- [Native exploration and decisions](https://github.com/stevehalliwell/agent-skills/blob/330136285cb1470ccecc00ca244894c9de2122a3/attendant-native-discussion.md)
- [GUI spike-like implementation plan](https://github.com/stevehalliwell/agent-skills/blob/330136285cb1470ccecc00ca244894c9de2122a3/exploration/attendant-native/GUI-PLAN.md)
- [Review findings, evidence and accepted deferrals](https://github.com/stevehalliwell/agent-skills/blob/330136285cb1470ccecc00ca244894c9de2122a3/exploration/attendant-native/GUI-REVIEW.md)

Source and documents also remain available locally with `git show <snapshot>:<path>`.
