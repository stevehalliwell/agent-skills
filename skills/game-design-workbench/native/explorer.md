# Use the desktop Explorer

[Build the viewer](build.md), then run the returned executable. Example below uses `gd-view` as its name. Use the actual executable path and [source bindings](cli.md#supply-source-folders).

```sh
gd-view view explorer --mode matrix --layout grid \
  --nouns 'nouns=game-design/nouns' --verbs 'verbs=game-design/verbs' \
  --interactions 'interactions=game-design/interactions'
```

## Choose data, then presentation

**Data / projection** chooses which relationships to inspect. **Layout** or **Mode** chooses how to display those relationships. Missing bindings disable views with an explanation. Empty valid collections remain valid.

- **Verb usage** (`matrix`): Grid for all declared pairs, including inactive cases; Free graph or noun/verb columns for connected groups. In Grid, select a cell or header; arrows move selection and PageUp/PageDown cycle cases. Use the legend to distinguish active pairs from other declared cases.
- **Noun interactions**: Grid or Free graph of case co-participation. Select a pair to see counted cases. See [what the counts mean](cli.md#interaction-counts).
- **Skills**, **Challenges**, **Relationships**: prerequisite/rank views, declared route views, and raw linked context around a selected record. [Skills and routes](skill-routes.md).
- **Actors**, **Effects**, **State charts**: interaction and usage maps, conditional questions, contributor evidence, and finite state hypotheses. [Actors, effects, and states](actor-effects.md).
- **MDA** and **Resources**: trace design intent or inspect resource declarations and conditional witnesses. [MDA traces](cli.md#mda-traces) · [Resources](resources.md).

## Inspect, compare, and trace evidence

1. Select an item to inspect its source. Use **Find selected in another view** when the same source has another representation. `Ctrl+F` searches shown items, not filtered or unplaced data. Wheel zooms; right/middle drag pans. `F` frames selection or the view; **Focus selection** frames direct context when readable. Escape clears selection after dismissing text or popups.
2. Use **Selection & evidence** for cases, bindings, and conditional outcomes as offered. Use **Built-in structural / similarity questions** for native questions over explicit inputs. Use **Analysis layers** on supported graph views to explore shown links, prerequisites, shared neighbors, shapes, and paths. The adjacency/incidence overview counts shown links; brush a cell to highlight represented links, not to change analysis inputs.
3. Follow evidence to exact source rows. A selected definition may have multiple contributing cases or placements. Check **Diagnostics** for source and analysis findings. Rejected refresh leaves the last good view; distinguish new errors from evidence loaded earlier.

Free layouts can settle; Space pauses or resumes. Moving nodes is presentation only. Workspace bounds, island separation, and displayed-motion smoothing can improve readability but do not change relationships. Fixed grids, Statechart, and similarity maps do not settle. Graph layers are not available on every canvas.

See [shared controls](controls.md) for what Run, Reset, navigation, and refresh keep or change. Use [local IPC](ipc.md) for authorized agent interaction with a running Explorer, including [reading and patching inspector state](ipc.md#read-and-patch-the-active-inspector). Standalone `view matrix`, `view skill-atoms`, `view mda`, and `view resources` remain available; they do not expose Explorer IPC.

## Draft editing and JSON handoff

Only Skills edits ranks and prerequisites in memory. Select a skill or link, then edit rank or prerequisites in its inspector. Clear rank sets null. Cyclic links can remain as findings. Dragging a node does not edit rank. Undo/Redo in Explorer act on the draft, not source files; Ctrl+Z outside text editing undoes a draft action. Draft analysis and related route checks can differ from the headless CLI, which reads disk.

Finish or close to emit one JSON report, or supply `--output FILE.json` to create a new report file instead of writing to stdout. Existing files are not overwritten. Check `ok`, errors, `data.applied:false`, and each net change's source identity and `loaded`, `current_disk`, and `edited` values. A failed disk read or identity mismatch is not an authored null; check `current_disk_available`. Unrelated external changes are not proposed reversions.

Re-read current records before adopting anything through [Capture](../workflows/capture.md) and authorized Attendant edits. The report is neither a lock nor approval. Refresh only after dealing with the draft; do not expect it to overwrite or merge edits.
