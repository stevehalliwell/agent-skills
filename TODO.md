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
- [x] Bring up native Attendant features and automated acceptance on macOS arm64; review/amend imported product and record results. CLI/API, interactive GUI/model/input, rendering, feedback and reveal/inspection now run with Zig/CJIT. Windows backend retained; current Windows proof is cross-compilation/linking only.
  - GUI: use GLFW + OpenGL3, matching pinned upstream RichMd examples. User selected this over custom Cocoa/OpenGL3 and Cocoa/Metal. Explicit pinned GUI dependency acquisition authorized; no tool/package installation authorized.
  - Reuse c-ink's SDK/CJIT signing/build and bounded Unix-socket patterns where applicable. Do not copy its different generated cimgui bindings.
  - Cover CLI/shared core, source/feedback API, GUI model, rendering/input, reveal/inspection and existing acceptance checks. Do not count skipped Windows-specific assertions as macOS passes.
  - [x] Acquire pinned sources; build native LuaJIT generator and GLFW/OpenGL3 dependency library. Preserve Windows bridge compilation.
  - [x] Verify native Zig/CJIT C99 consumers: ABI, rendering/images, Retina P6 orientation/content, contaminated GL transfer state and terminal-render cleanup. This is bridge acceptance only.
  - [x] Port shared core/CLI file, path, hash, timestamp, publication and diagnostic operations; build/run CLI/API acceptance with both compilers.
  - [x] Keep macOS failed-bootstrap cleanup simple: retain already-published files and report written paths; remove temporary staging and empty owned directories. No locking/quarantine/recovery system. User requested simplest macOS behavior for this low-impact edge case. Preserve Windows handle-based cleanup.
  - [x] Port GUI host/model/input and reveal/inspection; run shared GUI/model suite and macOS transport checks. Use private same-UID Unix message pipes across login sessions, not desktop isolation or application authentication; no new dependency or worker thread.
  - [x] Review native demo on owned workspace copy. User confirmed GUI works and accepted completed port for commit. Review copy: `skills/attendant/native/build/manual-review-MZMSLp/`. Launcher rollout remains separately unapproved.
- [ ] Obtain explicit rollout approval before changing active launcher or removing JS.
- [ ] After approval, switch launcher to native, remove JS runtime and remaining JS-dependent tests, and link former JS version by commit hash or tag in final README. Verify skill workflow after switch.

### Checks

- Initial macOS arm64 baseline: native Zig CLI build fails on Windows-only `bcrypt`/`shell32` link flags. CJIT CLI build fails on missing SDK include search (`assert.h`). Core/API/GUI also contain direct Win32 calls; suffix/link changes alone are insufficient.
- macOS JS transition checks: 7/7 pass after canonicalizing test fixture roots with `realpath` (`/var` aliases `/private/var`). JS runtime unchanged. Added missing direct `<limits.h>` include to native `base.h`; standalone Zig/CJIT checks compile/run native memory-region create/protect/release.
- macOS GUI bridge: native arm64 LuaJIT and `libcimgui_richmd.dylib` build without warnings on installed SDK14.5/Zig0.17.0. Native Zig/CJIT C99 consumers each pass four hidden rendering frames, ABI checks, live image creation/zero-image teardown, 1280×960 P6 screenshots with top/bottom marker checks, contaminated pack/unpack state restoration and injected terminal-render failure cleanup. Evidence: `skills/attendant/native/build/bridge-check-results.json`; report hashes test inputs, signed consumer binaries and screenshots and records compiler identity.
- macOS CLI/API: Zig and CJIT build/run; 20 CLI groups and 94 API cases pass on owned Unicode-path workspaces. Includes atomic save, stale-source rejection, feedback bootstrap/fault recovery, read-only rejection and UTC date range/epoch/timezone checks. Replaced Darwin `timegm` with bounded Gregorian conversion after pre-1900/errno failures. Current reports: `skills/attendant/native/build/{cli,api}-check-results.json`; runs `cli-check/run-YefKrM` and `api-check/run-a9Wobw`. Windows x64 CLI/API cross-link; no Windows runtime regression claim. JS transition tests remain 7/7. macOS failure checks assert retained files and reported paths after seed/comment publication and config conflicts, preservation of external edits, temporary-file cleanup and retry conflict without overwrite.
- macOS application: 20 GUI/model groups pass with 194 model assertions per compiler; covers production render/ABI/screenshots, diagnostics/navigation, literal-path undo isolation, editing/dirty-close/save guards, source/preview feedback, cross-compiler inspection/resolution and terminal-render cleanup. Maximum 4 MiB selection plus 4 MiB context survives delayed-reader backpressure while later model selection changes. Evidence: `skills/attendant/native/build/gui-check-results.json`, run `gui-check/run-r2qzz3`.
- macOS local control: 16 groups pass across both compilers; clean/dirty/stale/new-tab Go behavior, selection/caret/scroll/focus preservation, unrelated/ambiguous viewers, path/line/symlink rejection, fragmented/short/oversize/unknown messages, silent-client timeout/reconnect, unexpected discovery replies and unsafe/symlink endpoints. No source/projection writes. Evidence: `skills/attendant/native/build/control-check-results.json`, run `control-check/run-m8bJVa`.
- Independent GUI review found Windows reserve-without-commit regression and permissive discovery reply handling. Fixed both; shared zeroed-buffer helper commits Windows pages without changing arena reservation semantics. Windows CLI/API/model cross-link and production/test GUI C99 sources cross-compile to x64 COFF. No Windows runtime claim. Windows fault probes retain OS environment reads for in-process injection and original `_strtoui64` parsing. Follow-up review found no new actionable code defects within inspected scope.
- macOS CJIT GUI launches without loader environment: link-only dylib copy uses executable-relative install identity; runtime copy remains byte-identical to stack library. GUI manifests hash link copy. Both GUI suites explicitly unset `DYLD_LIBRARY_PATH`; installed `install_name_tool` cannot rewrite CJIT's executable format, so no executable patch is attempted.
- Independent bridge review found pixel-transfer state safety, LuaJIT stale-object reuse and test-evidence identity gaps. Fixed all three. Negative library probe without transfer normalization fails (`Cannot read screenshot buffer`, image not loaded); production library passes. LuaJIT now cleans only owned build outputs before native rebuild. Windows bridge cross-compiles to x64 COFF; no native Windows execution or full Windows regression claimed.
- Historical Windows x64 import qualification (before macOS edits): independent CJIT/Zig CLI and API builds; Zig GUI dependency DLL; production/test GUI and model builds for both C99 compilers. Dependency revision/patch verification passed offline using copied local checkouts; no acquisition run.
- Historical Windows native checks: 20 CLI groups, 92 API cases, 193 model assertions per compiler, 18 GUI/model groups including 16 real Win32/D3D windows. No JS comparison oracle. Results under `skills/attendant/native/build/{cli,api,gui}-check-results.json`; final GUI run `gui-check/run-WOrXrc`.
- Import proof: 38 checked source files byte-identical to snapshot; only CLI runtime edit is executable name in help. Stale GUI test-state/fault input guards and missing-DLL build guidance passed. Protected old viewer binaries/DLLs unchanged. Details: `skills/attendant/native/build/import-proof.json`.
- macOS synthetic process benchmark: 3 samples per compiler/command with rotating serial order; `skills/attendant/native/build/bench/run-l1tvZD/results.json`. Checks pass; small warm fixture only, not representative workload evidence.
- Historical synthetic process benchmark ran with rotating serial compiler order and reported variability. No representative performance or GUI-footprint claim. Existing unchanged JS runner's 7 tests still pass; these are transition checks, not imported native tests.
- Skill frontmatter/local links, Bash/Node syntax and full candidate whitespace check passed. Pinned upstream vendor formatting remains byte-preserved. Active JS launcher unchanged; rollout remains unapproved.
- Remaining qualification: authoritative-workspace/human rollout acceptance, real-project integrated loop, GUI startup/idle/interaction footprint, distribution/license audit and broader platform/failure limits. See `skills/attendant/native/README.md`; renderer draft recovery remains explicitly deferred, not reopened by cleanup.

### Historical context

Accepted runtime snapshot: `330136285cb1470ccecc00ca244894c9de2122a3`, already published on `feat/shared-markdown-gui`. Import selected files, not its commit sequence. Local import used planning snapshot `528f091cc11fc174780b3cc9448a4c7bca8c4c1a`; runtime/bridge sources are identical to published snapshot. Historical documents are not current delivery checklists.

- [Native exploration and decisions](https://github.com/stevehalliwell/agent-skills/blob/330136285cb1470ccecc00ca244894c9de2122a3/attendant-native-discussion.md)
- [GUI spike-like implementation plan](https://github.com/stevehalliwell/agent-skills/blob/330136285cb1470ccecc00ca244894c9de2122a3/exploration/attendant-native/GUI-PLAN.md)
- [Review findings, evidence and accepted deferrals](https://github.com/stevehalliwell/agent-skills/blob/330136285cb1470ccecc00ca244894c9de2122a3/exploration/attendant-native/GUI-REVIEW.md)

Source and documents also remain available locally with `git show <snapshot>:<path>`.
