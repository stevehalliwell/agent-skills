# Route timing: agent-led tuning preparation

Preparation walkthrough, not an executed experiment. Owner: [route-cost](game-design/experiments/route-cost.md). Inspectable C99 baseline model, frozen typed inputs, shared helpers and build checks now live in [prepared recipe](experiments/route-cost/recipe.md). Both compilers built model; only engineering helper checks ran. [Native supervision/result validation](../../native/experiments.md) now pass synthetic engineering checks; reviewed checkpoint/execution approval remains before trials. Canonical controls and experiment record remain unchanged.

## Question and source

Existing question: route completion time under assumed delays. Existing method: four walks, then three successful pushes; independent uniform 0–1 second delay per action. Assume route legality and correct decisions, not human puzzle solving.

Existing criterion: completion-time at most 8 s in at least 90% of valid attempted trials. [Baseline model](game-design/experiments/route-cost.md#results) has one completed run; candidate speeds remain unexecuted. Baseline analytical reference is 1312/5040, approximately 0.2603, for completion by 8 s under continuous delays. This is mathematics, not measured trial count or observed player performance.

For tuning, propose adjusting literal `walk-speed` while preserving `walk-duration` and `push-duration` formulas. Keep energy controls fixed. This investigates a timing trade-off, not whether faster movement improves game design.

## Resolve baseline and candidates

From example root, set `$gd` to explicitly built native executable. Use actual configured alias/path; shipped example uses `control-vars`.

```powershell
$gd = '<workbench>/native/build/gd.exe'
$folder = (Resolve-Path './game-design/control-vars').Path
$mapping = "control-vars=$folder"
& $gd controls --control-vars $mapping --json
& $gd controls export --control-vars $mapping --as json
& $gd controls export --control-vars $mapping --set control-vars/walk-speed=3 --as json
& $gd controls export --control-vars $mapping --set control-vars/walk-speed=4 --as json
```

Actual resolver outputs, verified without changing source:

```text
walk-speed 2 m/s: walk-duration 0.5 s; push-duration 1 s
walk-speed 3 m/s: walk-duration 0.3333333333333333 s;
                  push-duration 0.6666666666666666 s
walk-speed 4 m/s: walk-duration 0.25 s; push-duration 0.5 s
```

Baseline export:

```json
{
  "energy-capacity": 3,
  "initial-energy": 3,
  "push-cost": 1,
  "push-duration": 1,
  "tile-distance": 1,
  "walk-duration": 0.5,
  "walk-speed": 2
}
```

Without delays, four walks plus three pushes take 5 s, approximately 3.333333333333333 s, and 2.5 s for these respective candidates. These are calculations from resolved durations, not simulations. Only baseline speed 2 has a recorded trial fraction; speeds 3/4 remain calculations, not trials. No winning value or adoption has been established.

## Prepare smallest experiment

[Prepared baseline](experiments/route-cost/recipe.md) contains ordinary experiment-specific C99 model, recipe and frozen inputs. No `gd expectations`, automatic prose parser, or new schema fields.

Clock boundary confirmed for prepared model: start before first action delay; stop after final successful push finishes. Include all seven delays and action durations. Canonical source proposal remains unchanged; prepared snapshot supplies this agreed experiment-local boundary, not an adopted game rule.

Model reads frozen effective control values. For this prescribed legal route, add each action's resolved duration and its independent delay; report elapsed/completion time, route-complete, and remaining energy with local measure definitions. Broader energy-failure/recovery behavior needs explicit modeling before testing insufficient-energy variants. Do not guess it from a control value alone.

Prepare baseline first. Suggested finite comparison setup, not a claim of execution:

```text
adjustable literal: walk-speed
candidate values: 2, 3, 4 m/s
fixed: remaining source control definitions and route/delay assumptions
candidate resolution: existing gd controls --set, never copied formulas
trials: 1000 per candidate
random generator: planned shared PCG32 contract; verify helper before use
seed: explicit recorded value; use corresponding trial streams across candidates
limits: declared trial/step/time/output limits under checkpoint policy
measurements: actual times, completion states, energy, counts and spread
```

[Checkpoint and execution policy](../../workflows/commands.md#experiment-checkpoints) still applies. Prepared package covers baseline only; suggested comparison values above remain future work. Do not execute unreviewed model code, invent an installed runner, request a checkpoint before execution safeguards are ready, or claim an engineering test as an experiment trial.

## Refine from actual results

Interactive path: agent reports baseline/candidate measurements, explains timing trade-offs and model limits, then resolves the requested next candidate. No assumed winning value.

Optional algorithmic path: if requested, encode a finite ascending scan of the declared candidate list in this experiment's setup. An explicit objective could be the lowest candidate speed whose measured fraction within 8 s reaches 0.9. Stop at first qualifying candidate or exhausted list; report none if none qualifies. This is a proposed search policy, not an automatically chosen design objective or guarantee about players. Keep all valid attempted trials in denominator; incomplete evidence cannot establish criterion satisfaction.

Choose another search objective or bounds only from supplied direction. Search can reuse one committed, authorized setup; changing its policy manually requires a new checkpoint. Search does not write canonical controls.

## Adopt chosen candidate

After actual investigation and authorized choice, regenerate chosen candidate export through resolver. All formulas recompute together. Follow [value import/adoption](../../workflows/tune-controls.md), not manual edits to derived values. Only literal `walk-speed` needs adoption for this proposed tuning set; other definition metadata and historical results stay unchanged.

No adoption, simulation, player observation, or design verdict occurred while preparing this example.
