# Compare and tune controls through experiments

For one test, use [experiment workflow](experiment.md). Read this extension only for requested candidate comparisons, interactive refinement or algorithmic search.

Agent owns question, experiment creation, result interpretation, and refinement. Native tools resolve control values and formulas; they do not extract expectations from prose or decide what makes a game better. No experiment schema change or general expectation CLI is required.

## Establish investigation

1. Read owning experiment, relevant rules, controls, and live project usage. Keep measures, criteria, assumptions, and results in that experiment; no separate measure collection.
2. State question and what evidence can answer it. Distinguish simulated outcomes from observed play. Preserve unknowns; do not invent player policy, feelings, probabilities, or a success criterion.
3. Choose smallest credible method: calculation, generated model, game instrumentation, or playtest. Do not build a simulator when direct calculation answers the question. Changing game code or running a playtest needs its own scope and permission.
4. Define adjustable literal controls, fixed inputs, and method limits. Start with one candidate when no search is requested. Formula outputs are derived, not independent tuning knobs. If a supplied knob is formula-defined, expose its upstream literals or explicitly discuss revising the formula.

## Prepare inputs and experiment

- Use `controls --control-vars ALIAS=FOLDER --json` for typed definitions, units, formulas, and dependencies. Use `controls export ... --set ALIAS/NAME=VALUE --as json` for a candidate's resolved values. Same resolver feeds baseline and candidates; never copy formulas into an experiment or ask LLM to calculate replacement config values.
- Freeze actual effective values and definitions with recipe/code, assumptions, source references, toolchain, and limits. Exported configuration is an experiment input artifact, not a second editable authority. Do not edit derived values by hand.
- Model consumes its frozen inputs, not live working design files. An experiment driver may resolve candidates from an isolated committed copy of control definitions, then pass each resolved snapshot to the model. Record every effective candidate.
- Define local measurements and their units, clock/event boundaries, modeled failures, and undefined outcomes. Preserve all attempted trials; incomplete routes must not disappear from a denominator. Do not silently treat undefined time as zero.
- Generated code implements only this investigation. Reuse [shared C99 frozen-input/PCG32/measurement/step/event helpers](../native/README.md#prepared-experiments). [Crate baseline](../examples/crate-puzzle/game-design/experiments/route-cost.md) supplies one completed worked example with source checkpoint, actual inputs and results; [native supervision/result validation](../native/experiments.md) were used for that bounded run. New trials still follow reviewed checkpoint/execution approval. Do not invent an installed runner or add a general experiment registry, expectation parser, optimizer service, or lifecycle wrapper.
- Inspect source and I/O scope before execution. Follow [checkpoint workflow](commands.md#experiment-checkpoints): prepare first, obtain required approval, checkpoint reviewed code/inputs, then execute exact committed state. Missing implementation/dependencies/approval means prepared or blocked, not executed.

## Inspect and refine

Report measurements first, then explain what they imply under stated assumptions. Preserve unsuccessful, incomplete, and invalid outcomes. A criterion comparison is not proof of player success probability or design quality.

**Interactive:** present baseline and candidate results, explain trade-offs, accept next tweak, resolve formulas again, and rerun under execution policy. New manually edited code/inputs need a new reviewed checkpoint; never mutate a historical snapshot. Do not require another approval for a candidate already covered by a committed, authorized setup.

**Algorithmic:** put search policy in this experiment's setup/code, not a new generic CLI. Declare objective/comparison, allowed literals, bounds or candidate set, trial/step/time budget, seed policy, and stopping condition. Ask only for material missing choices; never choose an objective such as fun or balance by implication. Candidate generation within committed policy does not need a new commit per candidate; manual changes to policy/inputs do. Record generated candidate values, formula outputs, measurements, and why search stopped. Do not discard inconvenient trials or silently expand bounds.

Compare candidates using controlled assumptions. Shared random streams can support comparison when model warrants them; label method and limits rather than claim statistical significance. A deterministic experiment may have no spread.

## Adopt or retain proposal

Keep candidate values temporary until adoption is authorized. Successful experiment execution does not authorize baseline changes.

For chosen literals, regenerate complete resolved candidate through common resolver. Use [control tuning workflow](tune-controls.md) to preview import, recheck binding/source/planned writes, and pass native items unchanged to public Attendant. Do not import a hand-edited mixture of old derived outputs and new literals. Retain recorded experiment snapshots/results unchanged after adoption.

Record effective controls, actual execution provenance, results, interpretation, limitations, and adopted or deferred decision in owning experiment through public Attendant workflow. Do not relabel a preparation, analytical prediction, or hypothetical output as an executed result.

Worked preparation: [crate-puzzle timing and tuning](../examples/crate-puzzle/experiment-tuning.md).
