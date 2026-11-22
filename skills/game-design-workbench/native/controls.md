# Shared investigation controls

[Explorer](explorer.md) and [local IPC](ipc.md) use the same application controls and native analyses. Availability depends on the pane, selection, and loaded scope. IPC does not mirror every desktop control.

## Know which state you are changing

- **Presentation:** projection, layout, camera, and visibility control what you see. Searching the shown-node list is separate from hiding nodes on the canvas. Switching views retains each pane's state and does not apply staged inputs.
- **Pending inputs:** choosing cases, axes, endpoints, filters, or assumptions in an analytical form stages inputs. **Run** uses them explicitly. **Display**, where offered, selects which applied variants to present. Failed Run keeps the last good result; check whether the displayed answer matches your pending inputs.
- **Evidence:** inspecting or paging retained rows does not rerun analysis. Read coverage and producer limits as well as page counts. The shown graph drives graphical layers; built-in structural questions use their explicit query scope instead. Hidden nodes are not necessarily absent from the loaded model.
- **Draft:** rank and prerequisite edits have their own history. Navigation and display resets do not adopt, discard, or write a draft.

## Reset, navigate, and refresh

| Control | Changes | Keeps |
| --- | --- | --- |
| **Reset view** | Current camera, layout, and display defaults | Current selection, query inputs/results, and draft |
| **Reset inputs** | Stages analytical defaults | Last good result and current display |
| **Clear** in an analysis | Releases that retained result; check its form for input-reset behavior | Other analyses and panes |
| **Reset all** | Resets selections, view state, pending inputs, retained analyses, and navigation in all panes | Active projection, loaded sources, draft and Undo/Redo |
| **Back / Forward** | Traverses selection/navigation history | Analytical inputs and draft edits |
| **Refresh sources** / `F5` | Reloads the same bindings on success | Last good snapshot on failed validation or remap |

Refresh is explicit and guarded. Active input, pending rank text or analysis, a modified draft, or retained Undo history can block it. Redo-only history requires explicit discard before refresh. Report or resolve draft changes first; refresh does not merge or force-overwrite. See [draft handoff](explorer.md#draft-editing-and-json-handoff).
