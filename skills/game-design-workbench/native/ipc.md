# Drive a running Explorer through local IPC

Use `gd viewer --help` for syntax and [shared controls](controls.md) for application behavior. IPC does not start Explorer, activate its OS window, emulate input, or write source. Only unified `view explorer` exposes a session. Windows uses a current-user local named pipe; macOS uses a private Unix socket with peer-UID checks. Same-user processes are inside the trust boundary. No TCP or Linux viewer transport.

## Inspect before changing anything

```sh
gd viewer list
gd viewer inspect --target SESSION --fields selection,source
gd viewer inspector --target SESSION
gd viewer nodes --target SESSION --projection actors --limit 16
gd viewer actions --target SESSION --projection actors
gd viewer controls --target SESSION --projection actors --form query
```

Use the exact session token from `list` or the viewer. Restart changes it; refresh does not. A session candidate can be stale. Pick by loaded bindings, not discovery order. `inspect` reads loaded state; `source` is the selected loaded Markdown body, not fresh disk, frontmatter, or every contributing declaration. `nodes` reads a prepared canvas, without preparing panes or rerunning queries. `visible` is not the same as onscreen.

## Guide the user in Explorer

```sh
gd viewer hint --target SESSION --projection skills --reference 'skill-atoms/move'
gd viewer inspect --target SESSION --fields agent_hint,selection
```

Replace the example reference with a loaded record. `hint` queues a suggestion without changing human selection or taking focus. `display_pending:true` does not mean it is already on screen. `inspect` distinguishes the queued hint from the displayed hint. The user chooses **Go** or **Dismiss**.

Offer an evidence row or inspector section instead of navigating directly:

```sh
gd viewer hint --target SESSION --projection state-charts --form chart.proposal --id PROPOSAL_KEY --expect SELECTION_TOKEN --value 'Check which participant proposes this state change.'
gd viewer hint --target SESSION --projection state-charts --form inspector --id form.chart --expect CONTROL_TOKEN --value 'Choose a noun and axis, then run the chart.'
```

Use the same form, key, and parent (`--reference` when required) as `select`. Inspector hints use the `presentation` form's `control_token`. Target a named section such as `inspector_selection`, or a form shown by `controls`: `form.chart`, `form.query`, `form.resource`, `form.structure`, or `form.rules`. The Rules target reveals the rule controls or the buttons that open them. Optional `--value` is plain guidance text, up to 255 UTF-8 bytes. **Go** checks freshness, validates the exact target, opens its pane and reveals it without writing source. A stale or unavailable target does not switch panes. **Dismiss** changes no investigation state.

Use `navigate` for authorized direct changes to application context. Supply a projection and one `--reference ALIAS/NAME` or `--id NATIVE_ID`; omit both to open the pane. Reference navigation can match multiple representations. Exact node navigation does not substitute a nearby or same-name ID. Neither bypasses filters or active-input guards. Use `select` for supported evidence rows and discover actions such as `view.fit` and `view.selection.focus` to frame evidence.

## Stage and run explicitly

```sh
gd viewer controls --target SESSION --projection actors --form query --control root_case --limit 16
gd viewer configure --target SESSION --projection actors --form query --control root_case --value 'ALIAS/RECORD#CASE' --expect TOKEN
gd viewer action --target SESSION --projection actors --action query.run --expect UPDATED_TOKEN
gd viewer result --target SESSION --projection actors --form query --limit 64
```

- Discover forms, exact scalar keys, bounds, pending/applied values, and disabled reasons through `controls` and `actions`. Examples include `query`, `structure`, `rules`, `resource`, `chart`, and `presentation`. Follow choice `next_offset`, even on empty filtered pages. `canonical_choices_ready:false` may need the advertised discovery question, not a guessed key.
- `actions` reports the action's `form` and `requires_control_token`. Read `control_token` inside the matching `forms[]` entry from `controls`, not at the response root. After `configure`, use the updated token from its matching `forms[]` entry or read controls again. A token checks freshness, not authority. Configuration stages analysis but does not Run; presentation controls apply immediately. Actions against inactive panes need not activate them.
- On timeout/disconnect, mutation outcome may be unknown. Do not retry automatically. Inspect first. Refresh is explicit and [guarded](controls.md#reset-navigate-and-refresh). IPC cannot edit, Undo, discard, or adopt draft changes; `draft.redo.discard` clears only Redo history with the current presentation token.

## Read evidence and results

`viewer evidence --projection VIEW --form FAMILY` pages matching retained rows. Examples: `cells`, `cases`, `noun-cases`, `actor-cases`, `rule-rows`, `chart-cells`, `chart-hypotheses`. Follow `next_offset`. `source_slot` is not the page position; use exact advertised key/reference and `selection_token` from the same evidence scope for `viewer select`. Page counts are not native producer completeness.

`viewer result --projection VIEW --form FAMILY` reads retained JSON, not a new query. Examples: `query`, `structure`, `rules`, `bindings`, `resource`, `skills`, `challenges`, `mda`, `layers`, `comparison`, `noun-grid`, `actor-overview`. Pass the returned `result_token` as `--expect` for subsequent pages. `offset`, `bytes`, and `total_bytes` count decoded bytes; `--limit` sets page size in KiB (1–64, default 64). Concatenate decoded base64 pages before UTF-8/JSON parsing. A page can split a character. Restart retrieval on stale tokens. Total serialized output is capped at 64 MiB; pages are not a latency guarantee.

For **State charts**, discover the `chart` form on `state-charts`; stage exact `object`, `axis`, and optional `companion_axis`, then invoke `chart.run`. Presentation `state_chart_mode` switches between `statechart` and `grid` without Run. Use `chart.cell` and `chart.hypothesis` selection forms with keys from the corresponding evidence pages; cell selection filters, hypothesis selection inspects. `result --form query` reads full applied chart evidence. Explicit finite samples and root status filters remain CLI features.

### Reveal an exact case behind a link

Open `noun-interactions` or `actors`. Use `evidence --form links` and `select --form link` to select the pair or Actor association. Then read its cases:

```sh
gd viewer evidence --target SESSION --projection noun-interactions --form noun-cases
gd viewer select --target SESSION --projection noun-interactions --form noun.case --id CASE_KEY --expect SELECTION_TOKEN
```

For `actors`, use `actor-cases` and `actor.case`. Use the row's `key` and that response's `selection_token`; do not pass a case name, line, slot, or reference filter. Selection moves the inspector to the matching page, opens the case details on the active pane, and scrolls to the row. It does not run analysis, replace the selected link, or switch an inactive pane. Reread evidence when selection changes.

### Inspect nested case participants and Similarity members

For a selected noun pair or Actor association, `case-index` returns case keys and participant counts without embedding participant rows. Page through exact local participants, then reveal one in the active pane:

```sh
gd viewer evidence --target SESSION --projection actors --form case-index
gd viewer evidence --target SESSION --projection actors --form case-participants --reference CASE_KEY
gd viewer select --target SESSION --projection actors --form case.participant --reference CASE_KEY --id PARTICIPANT_KEY --expect SELECTION_TOKEN
```

After selecting a Similarity entry, read `similarity-features` for feature IDs, values, and member counts. Use a feature with `kind: "set"`:

```sh
gd viewer evidence --target SESSION --projection mda --form similarity-members --reference FEATURE_ID
gd viewer select --target SESSION --projection mda --form similarity.member --reference FEATURE_ID --id MEMBER_KEY --expect SELECTION_TOKEN
```

Use `similarity.member-source` instead to open the member's loaded source in the ordinary canvas, when available. `visible_page` reports a nested GUI cursor when one exists. Follow `next_offset`: nested replies can return fewer rows than requested to stay within transport limits. Use child keys with the exact parent and `selection_token` from the child reply.

### Reveal a proposal, activation, or contributor step

On `state-charts`, first select a hypothesis. Read `chart-proposals` or `chart-activations` and use the row key and token with `select --form chart.proposal` or `chart.activation`. Activation reveal expands the row. For Effects, select a result, read `effect-paths`, then read `effect-path-steps --reference PATH_KEY`. Use `select --form effect.path-step --reference PATH_KEY --id STEP_KEY --expect SELECTION_TOKEN`. Use `hint` in place of `select` to offer any of these reveals to the user.

## Set camera and inspector state

Read the `presentation` form and use its `control_token`. Set the canvas camera in one operation:

```sh
gd viewer configure --target SESSION --projection noun-interactions --form presentation --control camera_pose --value '125.5,-64,0.8' --expect TOKEN
```

`camera_pose` is `world_x,world_y,zoom`, not pixels or pointer input. X/Y are bounded to ±1,000,000; zoom to 0.00001–4. The control applies to the current canvas mode, including Similarity and the active State chart mode. It cancels pending fit for that canvas. Camera movement does not invalidate evidence tokens.

`inspector_open` shows or hides the inspector. `inspector_width` is the shared preferred width: 0 uses automatic width, otherwise 310 pixels through window width minus 256. Read `inspector_pixels` for effective width (0 when hidden). Set named `inspector_*` folds such as `inspector_selection` to `true` or `false`. Use `controls` for the full list. `inspector_scroll` sets vertical scroll in pixels up to `inspector_scroll_max`. To scroll to an available section by name instead of pixels, use `select --form inspector --id inspector_selection --expect CONTROL_TOKEN` against the active pane. State charts expose hypothesis, transition-label, proposal, and activation page controls. IPC updates the running window without saving preferences or taking OS focus.

### Read and patch the active inspector

```sh
gd viewer inspector --target SESSION
gd viewer inspector --target SESSION --offset NEXT_OFFSET --expect INSPECTOR_TOKEN
gd viewer inspector-patch --target SESSION --expect INSPECTOR_TOKEN --value '{"width":500,"folds":{"SECTION_KEY":true}}'
```

`inspector` returns the active projection, shell geometry, and a paged list of rendered foldouts, child scroll regions, and evidence pagers. Use each item's `label`, `section`, and `depth` for context; use its opaque `key` for writes. Do not save keys across sessions. Follow `next_offset` with the same `inspector_token` to avoid mixing snapshots. `inspector` defaults to 64 items per page; set `--limit` to 1–64. `limited:true` means the survey hit its 2,048-item cap; `editable:false` means the widget ID is ambiguous. A closed inspector returns no items. Open a parent and read again to discover children. No hidden panels are evaluated.

Patch only the fields to change. `open` is a boolean; `width` is the preferred width in pixels (0 for automatic); `scroll` is the inspector's vertical offset in pixels. `folds` maps fold keys to booleans. `scrolls` maps child scroll keys to vertical pixel offsets. `pages` maps pager keys to **zero-based** page indices; send one page change per request, with no other fields, because paging can replace nested widgets. Check `max` before writing. A successful page request is queued for the next draw; read again to confirm the page. Read again after geometry or fold changes to discover the resulting scope and scroll bounds. If `viewer.inspector_unavailable` follows a change, wait for the next frame and read again. If the inspector is hidden, show it in a separate patch before changing its contents.

Raw IPC accepts a JSON object rather than an escaped string: `{"protocol":1,"command":"inspector-patch","expect":"INSPECTOR_TOKEN","patch":{"open":true}}`. Non-page patches are validated together before any write. Unknown fields or keys, out-of-range values, stale tokens, and active input or pending rank block the patch. Content changes can clamp scroll on the next draw. Patches do not run analysis, edit drafts, or write source. For analytical inputs and other named pane controls, use `controls` and `configure`.

### Move a graph node

Read `nodes` for exact ID, `movable`, and `layout_position`. Read `controls --form presentation` for the current `control_token`:

```sh
gd viewer move --target SESSION --projection resources --id NATIVE_ID --value '125.5,-64' --expect TOKEN
```

Coordinates are native layout units, not pixels. The command returns the applied position: ranked layouts preserve their rank axis, and grouped layouts may move related nodes. Settling may move nodes again; this is not a pin. Move does not select a node, edit source, or emulate a drag. Grids, Similarity maps, State charts, and native Actor usage clouds do not support manual placement.

## Coverage limits

Case participant reveal currently covers noun pairs and Actor overview associations. Some Rules and Structure drill-downs still lack exact row selection. Inspector snapshots cover rendered sections, foldouts, child scroll regions, and evidence pagers, not transient popup state or arbitrary desktop widget actions. In-progress pointer gestures are not mirrored. State reads are snapshots, not subscriptions. Draft editing, Undo/Redo, and finishing a change report remain desktop operations.

Transport caps: 8 KiB encoded requests, 1 MiB replies, 4,095-byte UTF-8 native IDs, and 259-byte UTF-8 source navigation references. Commands return JSON on stdout and diagnostics on stderr; `viewer --help` is text. Check `query --list --json` for advertised IPC coverage. `ipc_available:false` does not imply all guarded viewer controls are unavailable; do not assume full parameter or evidence parity.
