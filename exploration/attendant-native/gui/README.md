# Shared Markdown workspace

Windows C99 host. Zig-built DLL owns cimgui/ImGui/RichMd/Win32/D3D11. Native CLI remains GUI/C++-independent.

Current UI: collection/root-file navigation, reusable document tabs, full source selection/copy, body preview, typed references, visible unresolved references and local Markdown links. Source editor remains read-only at this checkpoint. Editing/Send surface follows; document-state save/reload/conflict/comment operations already have headless acceptance.

## Build and run

Requires existing CJIT or Zig and explicitly prepared/built [GUI stack](../../../spikes/cimgui-richmd-c99/README.md). No automatic installs/downloads. Build and acceptance use Node; GUI runtime does not.

From repo root:

```bash
bash exploration/attendant-native/build-gui.sh cjit
bash exploration/attendant-native/build-gui.sh zig
exploration/attendant-native/build/gui/attendant-gui-zig.exe \
  --project "$PWD/exploration/attendant-native/fixtures/project" \
  --assets "$PWD/spikes/cimgui-richmd-c99/build/deps/richmd/assets" \
  --file records/tasks/first.md
```

Keep copied `cimgui_richmd.dll` beside executable. Runtime assets remain external. Launcher/installed skill unchanged.

`--help` lists arguments. `--frames 1..6000 --shot FILE.ppm` runs bounded real GUI and records D3D buffer readback; screenshot needs frame bound. Missing project/assets and invalid arguments fail without writes.

## Acceptance

```bash
bash exploration/attendant-native/build-gui.sh cjit --check
bash exploration/attendant-native/build-gui.sh zig --check
node --test exploration/attendant-native/gui/tools.test.mjs
node exploration/attendant-native/gui/check.mjs
```

- `check.mjs`: unique generated copies only; Unicode/space paths, actual preview pixels, real reference-button mouse input, navigation/tab reuse, source inventory unchanged and no DB preparation.
- Test-only input driver absent from production build. Headless model requires generated ownership marker. Both compilers run 42 always-on checks: navigation, missing refs, dirty retention, Unicode/CRLF Send, save, external clean reload, external dirty conflict, discard/cancel and missing-file recovery.
- Build manifests bind C/build inputs, generated stack headers/dependency manifest, actual DLL and native executables. Guards reject stale source/substituted DLL before GUI execution. Acceptance replaces stale success with partial failure report.
- Local evidence: `build/gui-check-results.json`, owned runs under `build/gui-check/`, source/toolchain manifests under `build/gui/`. Retained checkpoint: [evidence/windows-x64.json](evidence/windows-x64.json).
- Real window/D3D validation requires interactive Windows desktop. Fixed 1440×1000 requested window; hardware/WARP choice not recorded. No long-running/device-loss/minimize, wider glyph/path, performance or release qualification yet.

## Model and ownership

`workspace.c` owns tab buffers and navigation snapshots; core API results remain borrowed. Refresh never changes active selection or dirty source. Explicit navigation changes active tab; duplicate canonical paths reuse existing buffer. Clean external changes reload; dirty changes stay in memory and require explicit save/discard/cancel. Save verifies exact previously read hash even when metadata polling misses a change.

Bounds: 16 tabs, 4 MiB UTF-8 per document, 4096 navigation files, 16 MiB navigation arena. Project/source API uses existing native capacities. Preview hides recognized leading front matter; full source remains unchanged. Source selection tracks UTF-8 byte endpoints; model converts to saved full-file Unicode code-point offsets for core verification.

Preview text copying is not original-source range proof. Preview Send remains unavailable until exact mapping is verified.

## Comments

Test conditions never rely on C `assert`: current Zig `-O3` implicitly defines `NDEBUG`. No explicit assertion-removal or fast-math build option is added. Historical spike/performance evidence remains tied to its recorded binaries; no new performance claim here.
