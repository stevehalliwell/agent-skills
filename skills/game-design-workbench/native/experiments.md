# Explicit experiment supervision and result validation

C99 tools now provide bounded Windows process capture and graphics-free NDJSON validation. [Crate baseline](../examples/crate-puzzle/game-design/experiments/route-cost.md#results) now retains one complete validated 1,000-trial run from an isolated committed checkout. Synthetic infrastructure checks remain separate from game trials.

No automatic execution, compilation, Git/checkpoint/approval automation, registry, optimizer, expectation evaluation, source adoption, or result view. [Checkpoint policy](../workflows/commands.md#experiment-checkpoints) remains agent/human responsibility. Supplying a commit string records provenance; it does not prove committed state or grant execution permission.

## Build and engineering checks

From repository root, with explicitly installed tools:

```powershell
pwsh -NoProfile -File skills/game-design-workbench/native/test-experiment.ps1 -Compiler Both
pwsh -NoProfile -File skills/game-design-workbench/native/test-experiment-supervision.ps1 -Compiler Both
```

Choose `CJIT` or `Zig` when only one compiler is installed. No downloads, packages, Node, display, raylib or Attendant runtime. First script compiles route model but runs only input/random and result helper checks. Second script builds tools and runs synthetic emitters/validators only. No route model linkage in emitter or helper checks. Artifacts stay under ignored `native/build/experiment-checks/` and `native/build/synthetic-<id>/`; existing viewers are untouched.

Build outputs include:

```text
gd-experiment-run-cjit.exe       gd-experiment-run-zig.exe
gd-experiment-result-cjit.exe    gd-experiment-result-zig.exe
route-timing-cjit.exe            route-timing-zig.exe
```

Exact build argument arrays, compiler paths/versions and helper check counts: `native/build/experiment-checks/engineering-builds.json`. These are rebuildable engineering artifacts, not experiment evidence or permission to run route model.

## Validate frozen input without executing model

```powershell
$tools = './skills/game-design-workbench/native/build/experiment-checks'
& "$tools/gd-experiment-result-zig.exe" --check-input '<frozen-input.json>'
```

JSON summary reports `input_valid`, trial/measure/node counts and effective wall/output limits. This checks shared execution contract, not prose, game legality, formula correctness, source freshness, Git or model-specific assumptions. Freeze controls with existing resolver; never recalculate formulas here. No model starts during input checking or result viewing.

Frozen contract requires version/recipe/model/configuration/experiment identities, PCG32 generator/hex seed, trial count, optional trace index, positive limits, and 1–64 unique local measure identities with type/unit/boundary definitions. Enum definitions require 1–64 unique string choices. Boolean/enum measures are unitless. Extra experiment-local assumptions and control metadata remain frozen opaque data.

## Run only after reviewed checkpoint and execution approval

[Crate baseline recipe](../examples/crate-puzzle/experiments/route-cost/recipe.md) owns question, clock, assumptions, inputs and model scope. Before execution, inspect code/input/dependencies and Git diff/status, agree checkpoint scope/location, obtain required approvals, and build from exact committed isolated checkout. Tools do not inspect Git or manufacture authorization.

Explicit invocation, after those gates:

```powershell
& "$tools/gd-experiment-run-zig.exe" `
  --program '<reviewed model.exe>' --input '<committed frozen-input.json>' `
  --output '<new approved dated result directory>' `
  --checkpoint '<actual checkpoint commit>' `
  --build '<actual compiler invocation or argument array>' `
  --toolchain '<actual compiler version>'
```

Windows implementation currently requires ASCII arguments/metadata and paths on local fixed/RAM drives. Spaces, quotes and trailing backslashes are quoted for Windows CRT; no shell is used. Frozen JSON content supports UTF-8, including supplementary Unicode. Non-ASCII CLI paths/metadata, network drives, non-Windows execution and large-workload throughput are not established. Existing native matrix/explorer Unicode support is unchanged.

### Ownership and resource behavior

- Create new output directory; existing directory is refused, even if empty. Every evidence file uses exclusive creation. No overwrite, append, silent retry, repair, cleanup deletion or generic design-record writes.
- Read declared disk input, copy original bytes to `input.json`, parse shared contract, and keep recorded copy open against writes/deletion throughout execution/validation. Child receives this copy, not changing original or live design folders. Invalid input never starts model.
- Launch explicit executable suspended. Assign anonymous kill-on-close Windows job before resume. Inherit only NUL stdin and owned stdout/stderr pipe writers through explicit handle list. Job/handle/watchdog setup failure never falls back to unsupervised execution.
- Child working directory is new result directory, recorded in metadata. Models must read only declared frozen input and write stdout/stderr. They must not spawn subprocesses, use network/shell, or modify unrelated files. Owned job still kills descendants after parent exit, including descendants holding inherited pipes.
- Start independent wall watchdog before resume. It can terminate owned job even while capture thread waits on local disk I/O. Capture loop also checks monotonic time. No global PID search or termination of unrelated processes.
- Drain stdout/stderr fairly as raw bytes. Combined captured bytes never exceed frozen `output_bytes`. No text decoding/transcoding, CRLF repair or dropped NUL conversion. On cap, retain bounded prefix and stop job.
- Stop job before result validation; confirm zero active job processes within 1 second. Parent wait has 1-second bound; buffered-tail drain has 1-second bound within same output cap. Execution deadline stops model; bounded cleanup/validation is separate from model wall time. Catastrophic watchdog-join failure exits supervisor with code 2; missing metadata/completion remains incomplete evidence. No hard-real-time or broken-OS/filesystem guarantee.
- Keep partial/error/limit bytes. Unexpected process exit, startup/I/O/metadata error, timeout, cap, missing/truncated completion or malformed records cannot become successful evidence. Startup failure has `launched:false` and null process exit, not a simulated failure outcome.
- Native model already checks its own steps/time/stdout cap. External cap includes both stdout and stderr; external deadline also covers process startup and capture. Frozen limits are sole run limits; changing them requires new reviewed inputs/checkpoint, not an unrecorded CLI override.
- Separate process/job/watchdog is not a security sandbox. Inspected native code still runs with user's permissions. Do not execute untrusted code merely because a recipe names it.

### Evidence and output

```text
input.json         exact supplied frozen bytes, separate from source
stdout.ndjson      raw captured stdout, including malformed/partial bytes
stderr.log         raw captured stderr
supervision.json   actual paths/cwd, checkpoint/build/toolchain, capture status,
                   elapsed time, byte counts, process exit, nested validation
```

Store real results outside design collections and rebuildable native `build/`. Use new dated identity; do not overwrite prior evidence. Retain actual compiled-source checkpoint and external toolchain versions. Opening these files does not execute code. Adoption and authoritative experiment-record updates remain separate public-Attendant operations.

Supervisor stdout is one JSON summary (`schema_version:1`); stderr carries concise human diagnostics. Same summary is exclusively written to `supervision.json` when output directory exists. Metadata write failure cannot return success. `launched` means model resumed under owned job; suspended setup failures are not executions. `windows_error` supplies relevant Win32 failure detail where available, not a design finding.

Exit 0 means complete validated execution, regardless of measurements. Exit 3 means producer/cooperative or external resource limit. Exit 2 means startup/runtime/I/O/protocol/metadata error or other incomplete evidence. No criteria-based failure exit.

## Read captured results without execution

```powershell
& "$tools/gd-experiment-result-zig.exe" `
  --input '<recorded input.json>' --results '<stdout.ndjson>' --exit-code 0 `
  --checkpoint '<recorded checkpoint>' --build '<recorded build>' `
  --toolchain '<recorded toolchain>'
# Add --stopped when supervisor stopped execution, even if parent exit was 0.
```

Use actual process/stop metadata, not guessed values. Read-only validator always emits JSON; it never starts model, loads current controls, computes criteria or repairs output. `--help` prints text; other paths never prompt. JSON summaries are bounded, so field selection/TTY rendering and credentials are not applicable. Machine/agent use is identical to human command use.

`gd_experiment_result.h/.c` is shared graphics-free validation. Contract strings borrow immutable frozen input. Reader owns one event parser plus at most 125,000-byte trial bitmap through explicit allocator; all are released on success/failure. Report strings are static. Allocation refusal retains failure report without executable success.

### Validation rules

- One configuration per stream. Trial identity is configuration plus zero-based index. Future sweeps can validate separate configuration files with repeated indices; multiplexed sweeps are not delivered here.
- Require strict bounded UTF-8 JSON objects, no duplicate decoded keys, BOM, non-finite numbers, underflow-to-zero or missing final newline. Accept LF/CRLF; reject blank/trailing records. No loss-tolerant parsing presented as success.
- Header first, exact protocol/model/helper, checkpoint/build/toolchain and pinned LibYAML/PCG versions. Entire embedded input must match supplied frozen snapshot, including controls, identities, units, assumptions, criteria and limits. Object order/string escape spelling may differ; number lexemes remain exact. No formula evaluator or inferred semantics.
- Trial indices unique and in range; configuration exact. Step records belong only to requested trace trial, precede its trial record, increase strictly, and remain within declared step limit. State must be object. Final trial step count cannot precede retained trace; requested nonempty trace cannot be absent.
- Trial measurements include every declared key. Step measurements may be declared subset. Reject unknown keys and type/enum mismatches; do not coerce booleans/strings/numbers. Integers require integer lexemes within ±(2^53−1).
- Null trial measurement requires matching nonempty undefined reason; defined values cannot have such reason. A fully modeled outcome with undefined goal time can belong to complete execution. Count undefined values; do not drop failed goals or imply criterion satisfaction.
- Require one final completion with recognized complete/limit/error status, exact requested/attempted/completed counts and consistent partial flag. Trial error cannot be downgraded to final limit. Complete execution covers all requested trials; process exit must match final status. External stop always blocks `complete:true`, even if retained producer completion looks valid.
- Invalid input/output or interrupted capture retains parsed counts/error location, but cannot establish successful execution. Producer pre-header error is reported as such, not fabricated header/provenance.

Input limits remain 1 MiB / 4,096 nodes / depth 32. Header adds bounded provenance wrapper: event limit 1 MiB + 32 KiB / 4,224 nodes / depth 33. Maximum trials: 1,000,000; measures: 64; output cap accepted up to 1 GiB by explicit frozen input. Defaults in baseline remain 1,000 trials, 1,000,000 steps/trial, 30 s, 64 MiB. These are implementation bounds, not performance claims.

Validation summary distinguishes `protocol_valid` from `complete`. Producer limit/error may have valid protocol but incomplete execution. Error code/line and retained counts are diagnostics, not repaired output or game findings. No distributions, percentiles, design verdicts or automatic exclusion of undefined outcomes.

## Verification boundary

Checked with installed CJIT 1.2.0 and Zig 0.17.0: 100 input/random + 20 result helper checks per compiler, plus 302 synthetic checks across both compilers. Zig compiles C99 with `-Wall -Wextra -Werror`. Tests cover successful synthetic capture, trace, NUL stdin, input-copy write lock, raw binary preservation, producer runtime/limit/pre-header failures, parent/descendant hangs, stdout/stderr/combined floods, startup failure and existing-evidence refusal. Offline cases cover identities/provenance/input equality/types/undefined values/order/duplicates/completion/trace/exit/stop rules and maximum input byte/node/depth wrappers. Result helper tests cover both reader allocations failing and cleanup counts.

Those engineering checks are not crate model executions or evidence of game outcomes. Subsequent authorized Windows/Zig baseline execution is retained in [owning experiment](../examples/crate-puzzle/game-design/experiments/route-cost.md#results), including exact source checkpoint and raw measurements. Physical disk failure, catastrophic OS/thread failure and non-Windows execution remain untested. New executions follow [recipe](../examples/crate-puzzle/experiments/route-cost/recipe.md) and approval policy. Batch/sweep/replay automation, native aggregate tools and result views remain later work.
