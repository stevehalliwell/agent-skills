# macOS native port

Status: **working; user accepted UI and live IPC smoke test** on native arm64 macOS.

## Delivered

- Bash build entry points for CJIT/Zig; shared Windows/Mac source manifest, Apple SDK setup, CJIT ad-hoc signing and executable-relative dylib loading.
- Existing Apple/CJIT float-limit compatibility applied to domain compilation units.
- Raylib 6.0 official release, commit `dbc56a87da87d973a9c5baa4e7438a9d20121d28`. No c-ink audio/window patches or prebuilt binaries imported.
- Mac IPC reuses c-ink's extracted proto-framework pipe backend. Workbench retains JSON protocol, PID-plus-nonce sessions, exact targeting, bounds, deadlines and unknown mutation outcomes.
- Owner-thread `funopen` result capture enforces existing page/64 MiB limits without temporary files.
- [Build commands and platform contracts](../../skills/game-design-workbench/native/README.md#build); [dependency provenance](../../skills/game-design-workbench/native/THIRD-PARTY.md).

## Checks

- CJIT/Zig CLI, core and protocol fixtures build. Each compiler: core 136/136; protocol 90 cases, writer 3 checks, capture 115 checks.
- CJIT full native Node suite: 62 passed, zero skipped. Includes actual Mac transport and CPU-only production adapters for Crate/Mario; example hashes unchanged.
- Build help/failure contracts: 10 passed. Shared manifest retains original Windows source set.
- Raylib/UI dylib, table and graph viewer compile/link. CJIT signing verifies; graph dependency uses `@executable_path/libgd-ui.dylib`.
- User authorized Mario unified explorer launch after compile/headless checks. Startup succeeded; exact-session IPC inspection succeeded. User: “looks good to me”.
- Authorized live IPC demo: select `skill-atoms/jump`; switch Skills to Free; Fit; pause/resume; navigate to Interactions; switch to Grid; Fit. All commands succeeded. Final draft: zero modified records, zero Undo steps. Source hashes unchanged. User: “yep that did stuff”.
- Left viewer open in Interactions → Grid. Runtime process/session identity is transient; rediscover before later control. Never rebuild a running artifact in place.

Evidence in ignored `skills/game-design-workbench/native/build/mac/`:

- `core-cjit-final.log`, `core-zig-final.log`, `viewer-{cjit,zig}-final.json`, `native-node-final.log`, `build-contract-checks.json`.
- `ui-review-w7g4dk0f/{launch.json,stdout.log,stderr.log,inspect.json,ipc-before.json,ipc-demo.json}` retains launch and live command receipts. No binary/log evidence committed.

## Limits

- Smoke acceptance confirms basic Mac rendering and observed IPC operations, not every projection/control, keyboard/pointer behavior or visual quality.
- Earlier Actor/Effect default-scene/layout concerns remain open. Port does not resolve or accept those designs.
- Windows raylib 6.0 runtime remains unverified; rebuild matching SDK/UI dependencies. Earlier Windows visual evidence used 5.5.
- Mac experiment process supervision remains explicitly unsupported. No model executed by this port verification.
- No Linux viewer, packaging, notarization or distribution-signing claim.
