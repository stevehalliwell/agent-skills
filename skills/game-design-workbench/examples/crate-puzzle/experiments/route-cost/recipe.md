# Crate route timing: baseline recipe

Status at source checkpoint: **prepared, not executed**. User authorized a narrow local checkpoint and one 1,000-trial baseline run in isolated checkout `D:/Repos/agent-skills-route-baseline`. Native supervision/result validation were checked with synthetic emitters. Actual trial results belong in owning experiment and retained result files after execution. No push, publication or control adoption authorized. Opening this file does not run anything.

First authorized baseline run is now recorded in [owning experiment results](../../game-design/experiments/route-cost.md#results). Code and frozen inputs remain unchanged after that run.

Owner: [route-cost](../../game-design/experiments/route-cost.md), id `01a15175-3c00-7e68-8f3b-b760984a9a2d`. Owning record links this prepared package and its agreed experiment-local clock boundary; no game rule is adopted.

## Question and scope

What is completion time for four walks followed by three successful pushes under seven independent uniform 0–1 s delays?

- Start clock before first delay. Include each delay and action duration. Stop after final push finishes. Boundary confirmed during preparation.
- Assume legal route and correct decisions. Do not simulate puzzle solving, map traversal, learning, tension, or observed players.
- Consume frozen resolved durations; never evaluate source formulas inside model.
- Subtract frozen push cost only for three successful pushes. Reject insufficient starting energy, negative cost, or initial energy beyond capacity as unsupported input. Do not invent blocked-push, recovery, restart, or exhaustion policy.
- Baseline only: `walk-speed` 2 m/s. No candidate search, winner, design verdict, or source write. Suggested speeds 3/4 in walkthrough remain future comparisons, not executed configurations.

## Files and dependencies

- [route_timing.c](route_timing.c): experiment-specific C99 program. Only declared input read and stdout/stderr writes; no shell, network, subprocess, live design reads, or canonical writes.
- [frozen-input.json](frozen-input.json): actual native resolver snapshot, typed definitions/units/formulas/dependencies/identities, agreed assumptions, local measures, criterion, seed and limits. Source paths are example-relative. Export artifact, not editable authority.
- [gd_experiment.h](../../../../native/gd_experiment.h) / [gd_experiment.c](../../../../native/gd_experiment.c): owned bounded input, PCG32, finite measurements, step/time/output accounting and event helpers. Depends on `gd.h`, not live `GD_Model` loading or evaluator code.
- [LibYAML](../../../../native/THIRD-PARTY.md#libyaml): pinned `2c891fc7a770e8ba2fec34fc6b545c672beb37e6`, MIT; existing source/header/config/license subset.
- [PCG minimal C](../../../../native/vendor/pcg-basic/PROVENANCE.md): pinned `bc39cd76ac3d541e618606bcc6e1e5ba5e5e6aa3`, Apache-2.0. Source/header/license unchanged. Reentrant generator only.
- [test-experiment.ps1](../../../../native/test-experiment.ps1): explicit engineering compilation and helper checks. Does **not** run route model. Requires installed PowerShell 7 and CJIT and/or Zig; no downloads or installations.

Freeze origin, from repository root:

```powershell
./skills/game-design-workbench/native/build/gd.exe controls `
  --control-vars control-vars=skills/game-design-workbench/examples/crate-puzzle/game-design/control-vars --json
```

Resolved values and definition metadata were copied without evaluating formulas or modifying canonical files. Retained dependencies cover all seven declared controls. No temporary overrides. Do not regenerate this historical input from later controls. A new investigation gets new reviewed inputs/code/recipe.

## Inputs and limits

```text
recipe_id: crate-route-cost-baseline-v1
model_id: crate-route-timing-v1
configuration_id: baseline-speed-2
random_generator: pcg32-xsh-rr-v1
master seed: 0x000000000000002a
trials: 1000
trace_trial: null
steps_per_trial: 1,000,000
wall_seconds: 30
output_bytes: 67,108,864 (64 MiB)
compile watchdog: 30 seconds per compiler process
```

Seed initializes PCG state; zero-based trial index selects stream. Uniform draws are `ldexp(draw, -32)` on [0,1), a discrete approximation to continuous uniform delays. Corresponding future configurations may reuse trial streams, not compiler-specific `rand()`. No cryptographic or statistical-significance claim.

Frozen game values: capacity 3 energy, initial 3 energy, push cost 1 energy, tile distance 1 m, walk speed 2 m/s, walk duration 0.5 s, push duration 1 s. These are resolver outputs, not copied model constants.

Four scoped measurements: `elapsed-time` number/s; `completion-time` number/s; `route-complete` boolean/unitless; `remaining-energy` integer/energy. Keys are canonical experiment-local names scoped by header experiment id; frozen definitions also retain `<experiment UUID>#<measure name>` identity. Interrupted evidence retains attempted elapsed time/energy; unfinished route completion time is null with explicit undefined reason. A fully simulated route whose result emission is interrupted may retain defined completion time but still belongs to incomplete execution.

Frozen criterion is completion time <= 8 s in at least 90% of valid attempted trials. Model never evaluates criterion. Agent interprets complete validated evidence separately; incomplete/timeout/error evidence cannot establish criterion satisfaction. Continuous analytical reference `1312/5040` is not a seeded trial result.

## Engineering checks: safe before checkpoint

From repository root:

```powershell
pwsh -NoProfile -File skills/game-design-workbench/native/test-experiment.ps1
```

Script compiles helper checks and route program using same sources for installed CJIT 1.2.0 and Zig 0.17.0. Only helper executables run. 100 input/random checks cover upstream six-draw golden vector (state 42, stream 54), repeatability/uniform mapping/bounded integers, strict JSON and typing, owned-input allocation failure, finite measurements, encoding, step/time/output limits, and frozen-input parsing. 20 result helper checks cover contract boundaries, both reader allocation failures, cleanup counts and missing completion. No route actions, trial times, expectation fractions, or experiment outcomes are computed by tests.

Build outputs/logs and exact compiler argument arrays: ignored `native/build/experiment-checks/`. Zig application/helper/vendor compilation uses C99 with `-Wall -Wextra -Werror`. Builds do not replace a running viewer. These checks do not establish model runtime/protocol behavior or workload performance.

## Checkpoint and execution gate

Follow [checkpoint policy](../../../../workflows/commands.md#experiment-checkpoints).

1. Review model/helper/vendor source, input values/definitions, recipe, pending Git diff/status including untracked files, dependency notices, and relevant checkpoint scope. Existing native directory contains substantial earlier uncommitted work; do not commit it by implication or include unrelated files.
2. Obtain explicit commit and execution approval and agree isolated execution location. These are approved for this baseline in current session; scope remains one frozen configuration, not a sweep.
3. Use ordinary Git to create reviewed checkpoint and isolated exact committed checkout. Do not overwrite current worktree, force-add ignored/private material, or substitute later controls. Verify clean relevant source and actual `HEAD` there.
4. Compile from that checkout with explicit script above. Record actual compiler version/path and build argument array from that execution, not preparation artifacts. Build failure is not a simulated outcome.
5. Only after approval, invoke compiled process with exact committed frozen input. Model interface (supervisor supplies these flags; do not invoke directly to bypass watchdog):

```text
route-timing --input <committed frozen-input.json>
  --checkpoint <actual checkpoint commit>
  --build <actual compiler invocation/argument array>
  --toolchain <actual compiler version>
```

`--checkpoint` records supplied provenance; program does not verify Git or grant permission. [Native supervisor](../../../../native/experiments.md#run-only-after-reviewed-checkpoint-and-execution-approval) requires ASCII CLI paths/metadata for this increment and supplies recorded input copy. Unicode JSON content is supported; Unicode Windows file paths/CLI metadata and non-Windows execution are not established.

6. Use [implemented native supervision command](../../../../native/experiments.md#run-only-after-reviewed-checkpoint-and-execution-approval), with compiled model/input/checkpoint/build/toolchain and new approved dated result directory outside design collections. It launches suspended into owned kill-on-close Windows job, arms independent wall watchdog before resume, captures raw stdout/stderr under combined cap, and validates full protocol. Child runs in recorded result directory with NUL stdin and immutable recorded input copy. Model also checks cooperative time/steps/stdout bytes. Process/job/watchdog is not a security sandbox.
7. Parse/validate all NDJSON before interpreting it: exact protocol/experiment/configuration, identities, declared type/unit definitions, unique trial indices, finite measurements, trace monotonicity, counts, final completion and process exit. Exit 0 alone is insufficient. A killed process or missing/malformed final record is incomplete evidence, never fabricated success. Preserve partial/error/limit records and logs.
8. Preserve actual checkpoint, frozen input, source/dependency versions, actual build/invocation, elapsed execution, effective limits and outputs. Interpret criterion only after validation. Record actual evidence through public Attendant only when authorized; canonical source writes are separate from execution.

[Supervision/result-reader implementation and synthetic checks](../../../../native/experiments.md) are now available. Use them after checkpoint/execution approval; do not bypass safeguards or treat synthetic fixtures as game trials. Result views remain later work. No lifecycle/checkpoint automation, expectation CLI, or schema migration is added.

## Output contract

Always NDJSON on stdout; diagnostics on stderr. Explicit `--help` prints text without starting trials. Unknown/duplicate/missing flags fail without prompts. Input errors emit structured `error`; no successful execution is implied.

```text
header: protocol/model/helper, actual checkpoint/build/toolchain, pinned dependencies, full typed frozen input
step: selected trace only; configuration_id, trial, increasing step, action/delay/state/elapsed measurement
trial: configuration_id, zero-based trial, status, step count, named typed measurements; undefined reasons if interrupted
completion: status, requested/attempted/completed trial counts, partial_evidence
```

Exit codes: 0 complete execution regardless of measurements; 2 input/runtime/I/O error; 3 cooperative limit. Supervisor termination is incomplete even if available measurements look plausible. Output/I/O failure may leave partial/malformed final bytes; preserve them and report failure, not a repaired result.

Trace selection is frozen `trace_trial`, null or one valid trial index. New trace generation changes prepared input and needs reviewed checkpoint/execution; opening a saved trace never executes model. Batch/sweep automation, aggregate summaries/percentiles and result views remain later work.

## Preparation evidence

100 input/random and 20 result helper checks passed under CJIT 1.2.0 and Zig 0.17.0. Those engineering builds compiled both route executables without executing them. Native supervision/result validation pass synthetic fixture checks under both compilers; [shared verification boundary](../../../../native/experiments.md#verification-boundary) records coverage. Engineering checks are not measured route outcomes. Subsequent authorized baseline execution is linked above; no winner or adoption follows.
