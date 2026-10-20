# Crate-and-switch example

67 records across 16 collections. Includes six skill atoms, three crate/push cases, two unexecuted investigations, and local `tools/game-design-workbench/` snapshot. Tutorial source inside that snapshot is documentation, not a second configured design.

## Game concept

[Concept and current working mode](game-design/concept.md). Separate [MDA aesthetic: deliberate tension](game-design/aesthetics/deliberate-tension.md).

## Start

For direct entry here, complete installed `tools/game-design-workbench/references/runtime.md` §Runtime, then load `tools/game-design-workbench/SKILL.md` for agent use. Run commands from this project root:

```sh
node tools/game-design-workbench/scripts/workbench.mjs capabilities --json
node tools/game-design-workbench/scripts/workbench.mjs validate --json
node tools/game-design-workbench/scripts/workbench.mjs query skill-atoms --fields name,rank,prerequisites --json
node tools/game-design-workbench/scripts/workbench.mjs query matrix --json
node tools/game-design-workbench/scripts/workbench.mjs query evidence --json
node tools/game-design-workbench/scripts/workbench.mjs resolve --term "  BOX " --json
```

Expected: coordinate rank 2; plan-push null; switch-chain ranks 3. Three distinct crate/push cases. Closed-exit walking unknown; wall activation not applicable. Evidence query lists two proposed-stage investigations, not completed tests.

## Try bounded requests

1. `Inspect crate/push cases`: compare conditions, consequences, feedback, and known/no-effect/unknown coverage.
2. `Review coordinate using skill-atoms`: inspect both prerequisite chains and authored rank, without claiming native rank analysis.
3. `Change coordinate rank to 3; preserve every other authored field/rank`: compare full source diff; only selected rank should change semantically.
4. `What would removing pushing affect?`: trace declared dependencies; retain uncertain consequences.
5. `Inspect route-cost experiment`: read local delay assumptions, embedded 8 s / 90% criterion, and analytical reference. No simulation or playtest has run.

## Investigations

`experiments/energy-encourages-planning` retains original hypothesis UUID and claim as a proposed playtest investigation. Method remains undesigned. It is not combined with timing model: human planning and assumed route delays ask different questions.

`experiments/route-cost` owns timing method and former route-time criterion, including original criterion UUID and authored null thresholds.
## Saved history

Earlier baselines and dated reviews predate consolidation and remain unchanged/stale. To review current source, follow installed `references/review.md` §Review and §Save and reopen; capture a new baseline. Bound concept/mode is captured automatically; select an actual committed intention only when one exists. Do not relabel old evidence.
