# Focus selection

## Contract

Add **Focus selection** GUI button and shared `view.selection.focus` viewer action across unified projections/modes. Camera only; keep existing whole-canvas Fit/F, pair/witness actions and distance-focus layout separate.

User chose **readable selection with direct context**:

- Resolve current typed node, pair or link. Otherwise resolve inspected source to existing representations; never create selection, topology or query result.
- Include visible direct neighbors only, not connected component/transitive closure. Honor existing role, witness, scope and inactive-link visibility.
- Fit context at zoom 1.0–1.2 when possible. Zoom floor 1.0 keeps existing world-space labels at authored display size, including MDA's 16-unit names. This is not font/DPI or all-label readability proof.
- If context cannot fit above floor, try selected items alone. If selection itself is too large, center primary selected node/source endpoint at zoom 1.0. Distant endpoints/context may remain off-screen; disclose clipping.
- Grid focuses selected cell/header at readable scale; Cartesian axes are not graph neighbors. Blank cell remains blank.
- Similarity focuses selected placed entry and optional selected peer. No invented springs/neighbors or changes to distance embedding. Unplaced primary stays unplaced.
- Hidden/unrepresented selection leaves camera unchanged and reports why. Partial visible selection frames visible part and discloses hidden context. Source absence is not inferred.
- No selection disables action. Preserve geometry/velocity, render-filter state, selections, source/case evidence, pending/applied forms, filters, drafts/Undo and inactive-pane memory. No implicit Run, root change or OS focus change.

```sh
gd viewer action --target SESSION --projection mda --action view.selection.focus --expect TOKEN --json
```

Use existing presentation control token, busy guards and structured error contract. Successful action returns `framing` status/counts/clipping/zoom, including successful hidden/unrepresented no-op. Existing `inspect --fields panes` exposes last framing result. No new top-level command or flags.

## Acceptance

- MDA selection with tall direct neighborhood stays at zoom >=1.0, primary centered when needed; no whole-stack shrink.
- Small neighborhood fits with direct neighbors; second-hop nodes do not affect camera.
- Exact node/pair/link, Grid blank/header and similarity entry/peer framing use existing identities and coordinates.
- Hidden, absent and unplaced selections report explicit outcomes without revealing data or changing camera.
- GUI and IPC use same action. Camera operations leave raw layouts and domain state unchanged, including allocation-failure budgets and filtered-pose twins.

## Implementation

- `gd_explorer_focus_selection.h` resolves existing node/pair/link/source context and shares camera math with similarity. Node mask4096 bytes; Matrix source-case fallback adds4096 bytes. Work bounded by retained nodes, links and case-pair memberships. No allocation, graph expansion or evaluator call.
- Typed canvas context takes precedence over separately inspected source. Source-only Matrix cases resolve through exact retained cell membership; no implicit Find or new canonical equivalence.
- Optional viewer-owned render pose is borrowed only when source/identity/keys match. Camera uses shown positions; neither sampler nor raw layout changes. Inactive panes cannot borrow another pane's pose. IPC node positions stay raw.
- Successful framing cancels older queued Fit and suppresses next automatic distance-layout camera fit for that pane. Distance-layout settings and geometry remain unchanged by camera action. Similarity retains independent ordinary camera/pending Fit.
- Global GUI button and action catalogue share `view.selection.focus`; last report is labelled **Last focus**, not current completeness. Existing action guards and whole-view/cell/witness actions remain.

## Checks

- Warning-free CJIT CLI, explorer-adapter and graph builds. Core145/145; focused production/unified/standalone integration6/6. Both examples retain source hashes and release ownership.
- Production tests cover all nine unified projections, node/source-only framing, Matrix source cases, Grid/header/pair changes, hidden links and similarity entry/peer/unplaced behavior.
- Synthetic MDA chain proves one-hop context, excludes remote second hop, and reproduces former full-fit zoom below1.0. New focus keeps zoom >=1.0; oversized selected link centers source endpoint. Hidden/unrepresented selection preserves camera.
- Full raw-layout bytes and domain inputs/selections remain unchanged. Separate shown-pose and actual viewer-pose paths retain raw and sampler hashes; mismatched-pane pose ignored. Allocation failure armed during focus is not consumed. Explicit camera survives subsequent normal distance-layout auto-fit attempt.
- CPU checks do not prove pixels, real-pointer behavior or every-view readability. Final compiler matrix and broad live acceptance remain deferred.

## Live IPC review setup

Separate Mario MDA viewer opened with authorization: PID33064, session `33064-a0f0536a4b231f16`, artifacts/executable `native/build/focus-selection-work/`. Discovery found no running viewers before launch; none closed or replaced.

- Kept default **bands** layout. Selected discovered native ID `mechanics/enemy-contact`; no MDA query/root change.
- Whole-view Fit camera: `(640,1747)`, zoom0.221062. Focus selection camera: `(190,3120)`, zoom1.2.
- Framing reports `partial`: selected1, hidden0, direct context3, clipped=true, hidden_link=false. Selected card remains at readable scale instead of shrinking to include whole stack. Off-screen context is disclosed.
- Selection/source DTOs and full 44-entry identity/visibility catalogue identical before/after focus. CPU checks separately establish raw geometry and sampler invariance; catalogue equality is not a live coordinate/pixel check.
- Artifacts: `review-before.json`, `review-after.json`, `review-framing.json`, `review-configure.jsonl`, stdout/stderr logs. No OS input posted.

User accepted reviewed Mario MDA presentation and authorized checkpoint/continuation. Acceptance covers this presentation, not unperformed node/pair/link/Grid/similarity pointer checks or every-view readability. Broader review and final compiler matrix remain open.

## Comments

Review setup rejected `layout:native` because MDA key is `bands`; default already used bands, so no replacement configuration needed. Reference navigation performs Find without changing selection; initial Focus correctly rejected missing selection. Discovered exact node ID, selected it, then applied guarded Focus. One nodes read with unsupported reference argument was also rejected. No unknown/timeout mutation retried.
