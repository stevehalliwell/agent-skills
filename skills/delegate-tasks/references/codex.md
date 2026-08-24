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

`codex debug models` runs before every task. Its signed-in runtime catalogue is authoritative.

Current expected catalogue:

| Model | Thinking levels |
| --- | --- |
| `gpt-5.6-sol` | `low`, `medium`, `high`, `xhigh`, `max`, `ultra` |
| `gpt-5.6-terra` | `low`, `medium`, `high`, `xhigh`, `max`, `ultra` |
| `gpt-5.6-luna` | `low`, `medium`, `high`, `xhigh`, `max` |
| `gpt-5.5`, `gpt-5.4`, `gpt-5.4-mini`, `gpt-5.3-codex-spark` | `low`, `medium`, `high`, `xhigh` |

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
