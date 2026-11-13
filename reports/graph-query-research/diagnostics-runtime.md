# Diagnostics runtime

## Result and budget

- Default per-project CPU adapter invocation now measures below 1 second on this Windows/CJIT host, including process startup and fixture loading. Build time separate.
- Checkpoint rerun: Mario 746.9179 / 768.9685 / 744.5855 ms across three direct invocations. Crate 222.2886 / 177.683 / 179.0149 ms.
- These are observed runs, not a hard latency guarantee or cold-cache/CI percentile claim. Combined two-example Node group, including source hashing, remains above 1 second.
- Comparative research and extended presentation/refresh integration checks are opt-in, not routine change/release gates. Skipped suites are explicit in JSON; default success does not imply deep/research success.

## Commands and coverage

```text
test-explorer-view.exe --ipc-helpers GAME-DESIGN-DIRECTORY
  Basic adapter/control/query/configuration/serialization, feedback and sidebar checks.
  deep_checks:false; layout_research:false; presentation_checks:0; layer_result:null.

test-explorer-view.exe --ipc-helpers GAME-DESIGN-DIRECTORY --deep-checks
  Adds extended presentation/layout/selection checks and refresh/remap integration.
  deep_checks:true; layout_research:false; full layer_result.

test-explorer-view.exe --ipc-helpers GAME-DESIGN-DIRECTORY --layout-research
  Adds unchanged comparative research and implies --deep-checks.
  deep_checks:true; layout_research:true.
```

- Both flags may be supplied in either order. Unknown, misplaced and duplicate flags fail with status 2 before execution. Success is JSON stdout; usage errors go to stderr.
- Node deep case requires `GD_DEEP_CHECKS=1`; research case requires `GD_LAYOUT_RESEARCH=1`. Test discovery alone runs neither optional case.
- Deep suite retains Resource 600-step finite/work-budget/non-overlap assertions and noun 4096-step two-seed convergence regression. Noun gates unchanged: linked centres≥128, tail RMS≤0.05, max step≤0.2, centroid drift≤1, no overlaps.
- Current research fixture has17 configurations ×4096 steps, samples0/120/600/4096,256-step motion window, weighted/unweighted Effects, workspace and render-filter measurements. Generic layout fixtures retain private legacy/Neighborhood comparisons; Effects now uses Neighborhood only, with weighted/unweighted × workspace off/on. Previous19-configuration/95-record measurements below predate removal of public Native seeding; current85-record research not rerun. [Shared-default change](resource-readability.md).
- Shared fixture loading and basic adapter setup remain necessary in default path. No production CLI/GUI/solver changes, threshold relaxation or timeout widening. Existing 90000 ms test timeout unchanged.

## Serialization change

- Shared complete-result reassembly requests 64 KiB pages instead of 1 KiB. Same production IPC handler, Base64 decoding, result-token continuity, exact next-offset checks and complete payload assertions.
- All retained cases in each requested result are still inspected together. Paging is still used when payload exceeds 64 KiB; this is not a production response cache or serializer rewrite.
- Deep suite retains separate 1 KiB first/next-page, offset and stale-result tests. Default no longer repeats large-result serialization at every 1 KiB boundary.
- Prior temporary profiling found Actor structure/source reassembly alone cost 6284 ms; each page serialized complete result again. Larger pages remove most repeated work without discarding complete-result assertions.

## Verification

- Warning-free CJIT explorer adapter build: `native/build/diagnostics-fast-work/`. No GUI launch, replacement, OS input or authored source writes.
- Checkpoint rerun: 10 tests pass, covering both default examples, both Find examples, malformed flags and both opt-in deep examples. Default source hashes unchanged: Crate 290.5574 ms / Mario 869.9649 ms; combined example group 1161.3085 ms. Separate malformed-flag test 172.5095 ms. Hashing is outside direct native timings above.
- Opt-in deep checks pass with source hashes unchanged: Crate 521.7012 ms / Mario 3493.1497 ms. Full serialized layer-result checks run only here/research; default asserts null result and zero presentation checks.
- Opt-in Crate research passes. Both explicit flag orders pass; all 95 research records exactly match previous `sidebar-sections-work/crate.log` baseline. Mario research not rerun.
- Find remains a separate test case; no all-tests-under-1-second claim. Broader compiler/platform matrix remains deferred.
- Final artifacts: `native/build/diagnostics-fast-work/`: `checkpoint-checks.log`, `checkpoint-wall-times.json`, three checkpoint JSON/log pairs per default example, prior build/default/deep/research logs and both flag-order results. Research deliberately not rerun for routine checkpoint.

## Comments

Historical research-enabled Mario run took 99.058 seconds. First research-only split left default Mario around 10 seconds: separate temporary probes measured configuration 7132 ms, presentation 2823 ms and refresh block 458 ms. Those phase probes used platform-dependent `clock()` semantics, were removed from source, and remain only in ignored `diagnostics-profile-work/` artifacts. They are not paired end-to-end benchmarks for final mode split.
