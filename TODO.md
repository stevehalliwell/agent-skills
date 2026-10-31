# Todos

## Determine final shape of Attendant native

Status: planned; decisions complete, delivery pending.

Native CLI/GUI spike-like accepted. Decision checks record agreed direction; delivery checks record completed work. Refinement does not authorize implementation or rollout.

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

- [ ] Move maintained CLI, GUI, shared core and required RichMd bridge/patch into agreed `skills/attendant/native/` layout. Update paths so builds/runtime no longer depend on `exploration/` or `spikes/`.
- [ ] Prune historical experiment machinery, superseded demos and reports/evidence from final product tree. Keep history accessible through Git and documentation links.
- [ ] Reduce tests to focused native coverage. Remove JS equivalence checks and redundant permutations; retain direct assertions for important behavior and failure cases, plus reusable performance checks.
- [ ] Add build reference and native skill/launcher guidance. Missing native build directs agent to build reference, then back to original task. Keep active JS launcher unchanged until rollout approval.
- [ ] Verify independent CJIT/Zig builds and focused CLI/GUI checks on disposable workspaces. Record results and remaining release limitations.
- [ ] Obtain explicit rollout approval before changing active launcher or removing JS.
- [ ] After approval, switch launcher to native, remove JS runtime and remaining JS-dependent tests, and link former JS version by commit hash or tag in final README. Verify skill workflow after switch.

### Historical context

- [Native exploration and decisions](attendant-native-discussion.md)
- [GUI spike-like implementation plan](exploration/attendant-native/GUI-PLAN.md)
- [Review findings, evidence and accepted deferrals](exploration/attendant-native/GUI-REVIEW.md)
