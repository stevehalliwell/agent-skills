# Codex harness

Use `"harness": "codex"` in a task for a bounded, non-interactive Codex CLI invocation. The generic runner owns task lifecycle, detached background launch, deadlines, cancellation, and result artifacts; this harness only translates one task into `codex exec`. Do not use it for a continuing Codex conversation or work the prompt cannot fully specify.

## Task settings

```json
{
  "name": "implementation-review",
  "prompt": "tasks/implementation-review.md",
  "harness": "codex",
  "model": "gpt-6-sol",
  "thinking": "high",
  "web_search": false,
  "ephemeral": true,
  "sandbox": "workspace-write"
}
```

- `model` defaults to `gpt-5.6-terra` in this runner (not Codex CLI's own recommended default). Set an explicit model for new work after checking the signed-in catalogue; this fallback is retained for older CLI installs.
- `thinking` defaults to `medium`.
- `web_search` defaults to `false`; enable it only when the task needs current web information.
- `ephemeral` defaults to `true`. Set it to `false` only when a task has a demonstrated need for a non-ephemeral Codex invocation.
- `sandbox` defaults to `workspace-write`. It restricts writes to allowed workspace/temporary roots; it does not confine all reads to the job folder. See [Agent approvals & security](https://developers.openai.com/codex/agent-approvals-security).
- The runner instructs every Codex child not to read or write outside the job folder, even with `danger-full-access`. This prompt boundary is not host-level enforcement. Copy every required source file, fixture, and input there and name it in the prompt; retain narrower task-specific boundaries.
- Use `danger-full-access` only as an explicit task-level exception with user approval when a required command cannot run in the workspace sandbox; it permits unrestricted host commands.
- Before launching, the adapter queries `codex debug models` and validates the requested model and thinking level against the signed-in account.

## Model and thinking catalogue

`codex debug models` runs before every task. Its signed-in runtime catalogue is authoritative: model access and supported thinking levels vary by Codex CLI version and account. Inspect that same catalogue before setting either field:

```bash
python <skill-dir>/scripts/agent-job.py models codex
```

The command returns a JSON object mapping installed CLI model slugs to supported thinking levels. Pi uses provider-qualified IDs; Codex uses bare slugs. Examples are illustrative, not availability promises. If a required model is missing, check account access and CLI version, then query again; do not bypass preflight.

Set explicit `model` and `thinking` task fields when task requirements differ from inherited defaults. A task fails preflight if the requested combination is unavailable. Neither the runner nor this snapshot promises a model will remain available.

## Execution contract

The adapter invokes Codex with:

```text
codex --sandbox <workspace-write|danger-full-access> --ask-for-approval never [--search]
  exec [--ephemeral] --skip-git-repo-check -C <job-folder>
  --model <model> -c model_reasoning_effort=<thinking>
  --output-last-message <run>/<task>/result.md --json -
```

The runner writes `execution-prompt.md` beside the result, prepends its write-boundary instruction, and passes that file to Codex on standard input; Codex does not receive Pi’s conversation. The job folder is the working directory and prompt-defined read/write boundary. The runner never uses sandbox-bypass flags, but `danger-full-access` still disables sandbox restrictions; recording it does not make it safe. Codex writes its final response to `result.md`; JSON events and stderr are retained beside it. This applies to both blocking and detached launches. Background tasks have independent supervisors which record actual exit codes; a non-empty result is accepted only after successful harness exit, never merely because a file exists.
