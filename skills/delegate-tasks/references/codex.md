# Codex harness

Use `"harness": "codex"` in a task for a bounded, non-interactive Codex CLI invocation. The generic runner owns task lifecycle, detached background launch, deadlines, cancellation, and result artifacts; this harness only translates one task into `codex exec`. Do not use it for a continuing Codex conversation or work the prompt cannot fully specify.

## Task settings

```json
{
  "name": "implementation-review",
  "prompt": "tasks/implementation-review.md",
  "harness": "codex",
  "model": "gpt-5.6-sol",
  "thinking": "high",
  "web_search": false,
  "ephemeral": true,
  "sandbox": "workspace-write"
}
```

- `model` defaults to `gpt-5.6-terra`.
- `thinking` defaults to `medium`.
- `web_search` defaults to `false`; enable it only when the task needs current web information.
- `ephemeral` defaults to `true`. Set it to `false` only when a task has a demonstrated need for a non-ephemeral Codex invocation.
- `sandbox` defaults to `workspace-write`, which confines reads and writes to the job folder. Every required source file, fixture, and other input must be copied there and named in the task prompt. Use `danger-full-access` only as an explicit task-level exception when the required command cannot run in the workspace sandbox; it permits unrestricted host commands.
- The runner prepends every Codex prompt with the job folder as its only permitted write location, even when `sandbox` is `danger-full-access`; prompts should retain any narrower task-specific boundary.
- Before launching, the adapter queries `codex debug models` and validates the requested model and thinking level against the signed-in account.

## Model and thinking catalogue

`codex debug models` runs before every task. Its signed-in runtime catalogue is authoritative: model access and supported thinking levels vary by Codex CLI version and account. Inspect that same catalogue before setting either field:

```bash
python ~/.pi/agent/skills/delegate-tasks/scripts/agent-job.py models codex
```

The command returns a JSON object mapping every currently available model to its supported thinking levels. On Codex CLI `0.145.0`, the observed catalogue included `gpt-5.6-sol`, `gpt-5.6-terra`, `gpt-5.6-luna`, `gpt-5.5`, `gpt-reserve`, and `codex-auto-review`; do not use this snapshot as an availability guarantee.

Set explicit `model` and `thinking` task fields when task requirements differ from inherited defaults. A task fails preflight if the requested combination is unavailable.

## Execution contract

The adapter invokes Codex with:

```text
codex --sandbox <workspace-write|danger-full-access> --ask-for-approval never [--search]
  exec [--ephemeral] --skip-git-repo-check -C <job-folder>
  --model <model> -c model_reasoning_effort=<thinking>
  --output-last-message <run>/<task>/result.md --json -
```

The runner writes `execution-prompt.md` beside the result, prepends its write-boundary instruction, and passes that file to Codex on standard input; Codex does not receive Pi’s conversation. The job folder is the writable workspace. The runner never uses sandbox-bypass flags; `danger-full-access` is a consciously recorded task setting, not a bypass. Codex writes its final response to `result.md`; JSON events and stderr are retained beside it. This applies to both blocking and detached launches.
