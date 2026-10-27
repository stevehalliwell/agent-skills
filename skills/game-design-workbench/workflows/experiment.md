# Generate and run one experiment

Turn one design question into a small test, run it when authorized, and record what happened. Deliver an experiment, not an experiment framework.

## Workflow

1. **Read the question and relevant design.** Read owning experiment, linked rules, controls, and live collection schema/usage. Reuse an existing investigation when it asks the same question. Separate simulated behavior from observed play. Done: one question and its source facts are explicit.
2. **Choose the smallest useful test.** Use calculation when sufficient; generate a small C99 model when repeated trials or state changes are needed. State assumptions, measured outputs with units/boundaries, supplied criterion, and unsupported cases. Ask only for missing choices that materially change the answer. Do not invent player behavior or a design objective. Done: another reader can tell what this test can and cannot establish.
3. **Make concrete files.** Save experiment-specific code and frozen inputs beside existing example/project artifacts. Resolve named controls through the shared native resolver; retain effective definitions and values rather than copying formula evaluation into the model. Set finite trial, step, time and output bounds; record seed for random trials. Reuse existing helpers where useful. Done: experiment record links actual program, input and short reproduction instructions, not a proposed future implementation.
4. **Check, then run within authority.** Inspect source/I/O and compile the test. Follow existing [checkpoint/execution policy](commands.md#experiment-checkpoints); reuse available supervision rather than build another runner. Preparation requests stop here with a clear unexecuted status. For authorized execution, run the exact recorded code/inputs and retain raw measurements, logs and exit/completion state. If blocked, name the missing approval or dependency; do not expand into infrastructure work. Done: either actual evidence exists or the precise execution blocker is recorded.
5. **Record results and interpretation.** Check output identities, types, counts, completion and finite values before interpreting. Preserve failures, incomplete attempts and undefined values. Write actual counts/measurements, criterion comparison, limitations and raw-output links into owning experiment. Keep predictions and observed results separate. Done: reader can trace each reported result to a real run; unexecuted tests have no claimed results.

Keep experiment-specific details in experiment record/artifacts. Keep this workflow reusable. One baseline is enough unless comparison or search was requested. No new registry, schema migration, generic protocol, optimizer or result viewer just to answer one question. Do not adopt control changes, commit or publish by implication.

## Worked example: crate route timing

Read [experiment record](../examples/crate-puzzle/game-design/experiments/route-cost.md) first. It links [generated program](../examples/crate-puzzle/experiments/route-cost/route_timing.c), [frozen inputs](../examples/crate-puzzle/experiments/route-cost/frozen-input.json) and [reproduction recipe](../examples/crate-puzzle/experiments/route-cost/recipe.md).

- Question: how long do four walks then three successful pushes take under seven assumed uniform 0–1 s delays?
- Baseline: resolved walk duration 0.5 s and push duration 1 s; 1,000 trials with seed `0x000000000000002a`.
- Clock: before first delay through completion of final push; include all delays and action durations.
- Outputs: elapsed time, completion time, route completion and remaining energy. Compare actual valid attempted trials with supplied 8 s / 90% criterion only after execution.
- Result: one authorized baseline run completed; owning record links raw measurements, exact checkpoint/build, summary and criterion comparison. Analytical prediction remains separate from measured result. No claim about human planning, fun or puzzle solvability.

This is the target artifact shape, not a required seven-action model for other questions. Reuse existing example code when it answers the request; otherwise generate only the investigation-specific model.

For requested candidate comparisons, interactive refinement or bounded search, continue with [control tuning](experiment-tuning.md). Do not start that larger workflow for a single baseline test.
