# Node click and drag safety

Status: implemented; bounded Effects and outline review accepted. MDA focus-click check unverified; user chose to move forward without it. Decision-review checkpoint `ed81fd2` precedes this slice.

## Contract

- Canvas click selects/inspects without moving nodes/camera, rebuilding contributor context, reseeding layout or running analysis.
- Press temporarily holds picked node at its displayed position. Keep raw solver geometry separate from render smoothing.
- Drag starts only when pointer displacement exceeds 4 screen pixels from press. Exactly 4 pixels remains a click; threshold does not depend on camera zoom or MDA spacing.
- Preserve pickup offset. Once drag starts, returning inside threshold remains a drag until release.
- Preserve constrained bands and compound-stack movement. Fixed Grid/similarity positions remain fixed; no new dragging feature there.
- Release ends temporary hold, not selection. No persistent pin. Other unheld nodes may keep settling; released node can resume normal unpaused settling.
- Preserve independent panes, pending/applied inputs, retained results, drafts and Undo/Redo. No new IPC fields or implicit Run.
- Explicit Find, navigation, framing and layout actions retain their behavior. This contract changes canvas node clicks, not those commands.

## Causes and changes

- Effects canvas clicks called contributor retargeting, replacing scene and requesting framing. Click now keeps retained scene; **Result / contributor context** picker and **Show this result's contributor context** button perform explicit retargeting.
- Distance focus followed changed selection and could animate/refit graph. Canvas press now reuses existing pose guard and interrupts active focus transition. Holding during return pauses it; release resumes return before regular solver runs. Toggle focus off/on to refocus explicitly.
- Shared pickup offset lacked a screen-space gesture threshold. Added bounded gesture state to `GD_Layout`; Graph, MDA, Resource, Noun and Actor/Effect input paths use it. Existing standalone Graph/MDA/Resource paths share threshold.
- Shared explorer canvas dispatcher clears released holds across routes. Overview interception also releases MDA hold. No per-click allocation.

## Content-aware bounds and focus timing

- User found focused nodes outside old outline and requested content-aware bounds. Outline now encloses current displayed node centers plus measured raw card/label/badge footprints, with 48 display-world units of padding normalized for MDA spacing.
- Include shown off-screen nodes; exclude hidden nodes. Follow focus, dragging, return and render smoothing. Same outline helper serves unified and standalone Free viewers.
- Keep fixed prepared solver container separate. Outline resizing performs no solver/node/camera writes and does not change settling. Empty/invalid envelopes leave output untouched and draw no outline.
- New transition ignores previous frame's elapsed time. That time predates transition and can include costly preparation; it must not consume new 220 ms animation. Shared core animation duration remains unchanged.
- Move **Focus layout by shown-link distance** checkbox directly below Reset view in **View & layout**. Reset view still disables it. Select a visible node before enabling; canvas clicks do not refocus.

## Verification

- Focused CJIT CLI, graph viewer, core and explorer-adapter builds: warning-free.
- Native core: 148/148 checks passed, including screen-space threshold boundary, jitter, sticky drag, invalid coordinates, constrained drag, compound geometry and render-pose isolation.
- New explicit `--pointer-helpers` diagnostic drives actual shared explorer canvas dispatcher through raylib 6 automation events. Hidden owned window; no OS mouse input, review viewer, source writes or user-settings writes.
- Crate: 385 checks, 22 gesture cases. Two Actor-map modes have no nodes because sources contain no declared actor classification; no inferred actors added.
- Mario: 413 checks, 24 gesture cases. Native/Free paths, supported distance-focus paths and Noun Free island variant exercised. Fixed Grid modes are not draggable-node cases.
- Checks cover press, sub-threshold jitter, release, threshold crossing, camera/fit stability, retained Effects scene/contributor identity, pending query inputs, zero click allocations and smoothed displayed holds. Native/Free cases use 2.0/0.5 camera zoom; distance-focus cases use 1.0. Free solver advances 12 times while held; raw and displayed picked node stay fixed.
- Additional checks cover slow first-frame timing, held return pause/resume and content-envelope padding, hidden nodes, displayed centers, raw measured footprints, empty/invalid atomic output and unchanged solver workspace.
- Regression demonstrated: temporarily reintroduced old Effects click-retarget callback. Diagnostic failed on first Native Effects press, view 7/pass 0. Removed callback again; both examples pass.
- Focused Node tests: 20 passed, 3 expected skips. Includes CLI/query regression checks, Find, shared UX, production IPC helpers, explicit pointer checks and unchanged source hashes for both examples. Skips: opt-in deep checks, opt-in layout research and unprovided transport fixture.
- Human acceptance covers Effects Free checks and content-aware outline with 48-unit margin. MDA focus-click and initial-animation checks remain unverified; user chose to move forward without another pointer check. Broader standalone interaction/restart acceptance and final compiler/platform matrix remain open. No convergence claim or broader UX acceptance inferred.

## Live review

- User authorized separate Mario viewer. PID36264; session `36264-0c8d50dbb16a79e8`; executable `native/build/node-pointer-view/gd-view.exe`. Existing viewers left untouched.
- User accepted Effects Free paused click/drag check: inspection changes without graph/camera movement; intentional drag works without pickup snap.
- User accepted unpaused Effects Free hold: pressed card stays put while other nodes may settle. Release ends hold; no persistent pin.
- MDA feedback: initial animation unclear, focused nodes outside bounds, setting hard to find after Reset view. Updated outline, first-frame timing and checkbox placement. User confirmed outline follows content, requested more room, then accepted doubled 48-unit margin.
- Final authorized review: PID33788, session `33788-2aac85fbf6e8ac60`, executable `native/build/node-content-bounds-room-view/gd-view.exe`. Staged MDA Free with focus off; selected `mechanics/active-clock` through guarded source inspection without camera movement.
- User authorized restarting preceding PID34836 after clean-draft check. Read-only inspection timed out; process had exited before any close command. Launched final build only after confirming old process absent. Final viewer also exited after review; latest discovery contains no running viewers. No agent close or forced kill performed.
- User did not check MDA clicks with distance focus enabled and asked to assume success and move forward. Keep check explicitly unverified; do not count as acceptance. Initial-animation visibility also lacks explicit human acceptance. No blanket view/mode/platform acceptance inferred.

## Reproduce

From repository root, using prepared raylib 6 SDK:

```powershell
pwsh -NoProfile -File skills/game-design-workbench/native/build.ps1 `
  -ExplorerTest -RaylibDirectory C:/tools/raylib `
  -OutputDirectory skills/game-design-workbench/native/build/node-content-bounds-work

$env:GD_VIEWER_ADAPTER_EXE = (Resolve-Path skills/game-design-workbench/native/build/node-content-bounds-work/test-explorer-view.exe).Path
$env:GD_POINTER_CHECKS = '1'
node --test --test-name-pattern 'node click-versus-drag' skills/game-design-workbench/test/native-viewer.test.mjs
```

`GD_POINTER_CHECKS=1` is explicit hidden-window opt-in. Default diagnostics remain no-window. Real-pointer review must use a separately authorized viewer.
