<!-- wrap-up:start -->
## Pi handoff

Status: Native Attendant product import complete on `feat/attendant-native-product`, based on `dev`; awaiting user review/amendments and planned macOS testing. Native rollout not approved.

Done:
- Imported self-contained CLI/GUI/shared core/build stack into `skills/attendant/native/`. Historical experiments/demo hosts/evidence excluded; runtime/bridge sources preserved apart from CLI help executable name.
- Replaced JS equivalence checks with focused native coverage; added build/command/feedback guidance. Existing JS runner remains default and unchanged.
- Updated `TODO.md`: agreed decisions and first five delivery steps checked; review/testing, explicit rollout approval and final JS retirement remain open.

Next:
- Review imported product and record user review/macOS testing findings in `TODO.md` before considering rollout.

Context:
- Attendant tasks not configured; root `TODO.md` owns current scope/progress. No code port or launcher switch authorized by wrap-up.
- Windows x64 validation passed: CJIT/Zig CLI/API/production/test GUI builds; 20 CLI groups, 92 API cases, 193 model assertions per compiler, 18 GUI/model groups including 16 real Win32/D3D windows. Seven unchanged JS transition tests also pass. Shell/Node syntax, skill frontmatter/local links, manifests, stale-input guards and full candidate whitespace checks passed.
- Current builds are Windows-specific: Win32/D3D11 GUI and Windows APIs in core. No macOS build/testing or portability implementation completed. User plans macOS testing; record failures without assuming native macOS support exists.
- Native build/run instructions live in build reference. No automatic acquisition/installs. Local checks copied existing pinned GUI checkouts and verified offline. Fresh checkout must explicitly acquire or supply these dependencies; generated output/evidence stays ignored under native `build/`.
- Historical runtime snapshot `330136285cb1470ccecc00ca244894c9de2122a3` is already published on `feat/shared-markdown-gui`; links use this commit. Local planning snapshot `528f091cc11fc174780b3cc9448a4c7bca8c4c1a` has identical runtime/bridge sources. Do not merge/cherry-pick full spike history into product branch.
- First-five completion is not release readiness. Human/real-project acceptance, GUI footprint, distribution/license audit and broader platform/failure qualification remain open. Renderer unsaved-draft loss explicitly accepted/deferred; Git integration out of scope. Do not reopen those as cleanup prerequisites.
- Old manual viewer binaries/DLLs and leftover spike/build artifacts remain untouched. Do not restart viewers, overwrite mapped binaries or disturb unsent work. Use fresh GUI variant for further builds when outputs may be mapped.

Files:
- [Task/checklist](../TODO.md)
- [Native product/readiness](../skills/attendant/native/README.md)
- [Build/setup reference](../skills/attendant/references/build.md)
- [GUI/IPC contract](../skills/attendant/native/apps/gui/README.md)
- [Skill/default runner](../skills/attendant/SKILL.md)
<!-- wrap-up:end -->
