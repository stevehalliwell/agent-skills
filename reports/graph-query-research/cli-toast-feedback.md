# CLI toast visibility

## Contract

User chose **6 seconds with subtle fade**:

- Total6 seconds: 200 ms fade-in, steady display, 300 ms fade-out. No slide or position change.
- Latest successful CLI activity replaces text and refreshes deadline. Fade from current opacity, never restart from transparent while visible.
- Keep existing passive top-left window, no input/navigation capture or appearance/OS focus. Preserve independent hint/Go/Dismiss behavior.
- Reads, failures and human Go remain silent. No source/query/selection/layout changes; no new command, preference or protocol field.

## Acceptance

- First toast alpha0 at0 ms,0.5 at100 ms,1 at200 ms;1 at5700 ms,0.5 at5850 ms,0 at6000 ms.
- Replacement during steady display remains opaque and extends duration. Replacement during fade-out starts from current opacity and returns smoothly to opaque.
- Background and text fade together; restore ImGui style after draw.
- CPU ImGui frames retain passive flags, human focus and hint ownership; toast remains active after3 seconds and expires after6 seconds without refresh.

## Implementation and verification

- `gd_explorer_viewer.h`: analytical opacity envelope with existing ImGui clock; retain current alpha on replacement. No per-frame allocation, source/query action or new input handling.
- Multiply text style and explicit background opacity together; balance style push/pop even when window contents skipped.
- Warning-free CJIT CLI/explorer-adapter/graph builds in `native/build/toast-feedback-work/`. Existing viewer binaries/windows untouched.
- Focused integration6/6; both examples/source hashes unchanged. CPU feedback checks increased10 to25 per example: exact fade endpoints, steady/fading replacement, deadline refresh, draw-vertex alpha, style restoration, passive flags, human focus, hint ownership and expiry. No native window/renderer or OS input used.
- Artifacts: `build.log`, `adapter.log`, `adapter-first.log`, `crate-probe.json`, `crate-probe.log`.

## Live review setup

User authorized separate Mario viewer. Discovery found no existing viewers; none closed or replaced. Opened `native/build/toast-feedback-work/gd-view.exe`, PID12744, session `12744-e7fe89b4da69109f`, MDA bands, enemy-contact selected and readable Focus applied. No OS input posted.

Triggered two Focus actions2 seconds apart to refresh passive CLI feedback, then waited7 seconds after refresh. Viewer left open. Artifacts: `review-pid.txt`, `review-session.txt`, `review-focus.json`, `review-configure.jsonl`, stdout/stderr logs.

User accepted reviewed animation and authorized checkpoint. Acceptance covers this presentation, not unperformed every-view checks. CPU timing/draw-data checks and elapsed wall time alone do not establish perceived noticeability/readability.

## Comments

Launch helper initially searched discovery for a separate PID field, but discovery returns session keys only. Matched launched PID prefix to exact session; no relaunch or mutation retry needed.

Initial Mario integration run exceeded existing90-second adapter timeout. Same build passed rerun in84.66 seconds; no timeout or assertion widened. Timing headroom remains limited; no performance improvement claimed.
