# Actors, effects, and states

Use these questions to inspect declared participants, compare hypothetical states, or trace a proposed outcome back to its contributors. They share the [rule evaluator](outcomes.md), not a gameplay simulator. Use `gd query --list --json` for exact parameters and fields.

## Start from a participant or an outcome

```sh
gd query --question object-bindings --projection actors \
  --nouns 'nouns=game-design/nouns' --axes 'axes=game-design/axes' \
  --interactions 'interactions=game-design/interactions' --json
```

Supply relevant mechanics, resources, controls, and related records when the evidence refers to them. Use exact `ALIAS/NAME`, `ALIAS/RECORD#CASE`, and local participant keys from loaded evidence.

| Question | Inputs and evidence |
| --- | --- |
| `object-bindings` | Inventory local participants and their declared axes, types, domains, units, and constraints. Narrow by object, axis, case, or local participant. In Explorer, **Selection & evidence > Inspect bindings** can stage an exact context. |
| `actor-states` | Give `--case` and `--axis`. Defaults to authored A; use `--participant` for another binding. Uses declared enum values or repeated `--value` samples; numeric axes require samples in declared units. Other facts stay fixed or unknown. Compare whole cases, proposals, and exact shared/unique members, not just counts. |
| `state-coverage` | Give `--axis` with noun/axis and rule bindings. Optionally narrow to object, case, or exact local binding. Counts definite, undetermined, excluded, and unfinished hypotheses separately for bindings and deduplicated cases. Other bindings do not share the selected value. Use **Finite state coverage across bindings** in Actors. |
| `effect-contributors` (`effects`) | Filter by `--object`, enum `--axis`/`--value`, or effect family `--effect`; use `--to` for an exact concept ID instead of an axis/value/family selector. Inspect direct declarations, concrete mapped placements, ancestor guards, and side effects. Paths are not temporal sequences. |

**Actors** also offers an interaction map and usage cloud. The overview groups explicitly classified actors; its weights are not runtime counts. Explicit state and rule queries are available for nouns of any classification. `A`, `N`, and `U` denote actor, non-actor, and unclassified. These labels do not imply allegiance, safety, or global runtime identity. See [noun authoring](../templates/nouns/.usage.md).

Keep unknown guards, limited work, incomplete source, and omitted rows distinct from checked absence. The CLI supports more scoping options than some viewer forms. Actor status filters select roots; contributor filters check exact placement ancestry. Neither makes an unexamined variant checked.

## State charts

Use `gd query --question state-chart --projection actors` with the same explicit bindings and `--object ALIAS/NOUN --axis ALIAS/ENUM_AXIS`. Add `--with-axis ALIAS/ENUM_AXIS` to compare joint hypotheses on the **same local binding**. Optional exact case/local scope, explicit finite samples, statuses, and work limits are in the catalogue.

In **State charts**, select a noun and one or two enum axes under **Analysis**, then Run. **Statechart** shows conditional transition and preservation proposals in separate axis regions. **Grid** shows retained joint hypotheses; select a cell to filter evidence. Switching modes does not Run. For either mode, inspect whole guards, mapped calls, and other participants' effects. Missing selected-axis effects do not imply preservation. The chart does not infer initial/final states, state hierarchy, roles, or combined-state reachability. See [IPC state chart controls](ipc.md#read-evidence-and-results).
