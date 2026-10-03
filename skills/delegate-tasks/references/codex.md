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
- `sandbox` defaults to `workspace-write`, which confines reads and writes to the job folder. Every required source file, fixture, and other input must be copied there and named in the task prompt. Use `danger-full-access` only as an explicit task-level exception when the required command cannot run in the workspace sandbox; it permits unrestricted host commands.
- The runner prepends every Codex prompt with the job folder as its only permitted write location, even when `sandbox` is `danger-full-access`; prompts should retain any narrower task-specific boundary.
- Before launching, the adapter queries `codex debug models` and validates the requested model and thinking level against the signed-in account.

## Model and thinking catalogue

`codex debug models` runs before every task. Its signed-in runtime catalogue is authoritative: model access and supported thinking levels vary by Codex CLI version and account. Inspect that same catalogue before setting either field:

```bash
python <skill-dir>/scripts/agent-job.py models codex
```

The command returns a JSON object mapping models exposed by the installed CLI to their supported thinking levels. As of September 23, 2026, OpenAI is rolling out GPT-6 Sol (`gpt-6-sol`) for complex coding and GPT-6 Luna (`gpt-6-luna`) for focused, high-volume tasks. Codex CLI `0.156.1` adds both to its model picker. GPT-6 Astra (`gpt-6-astra`) is intended for harder work and requires CLI `0.153.0` or later. Access still depends on account, rollout, and workspace settings. [OpenAI models](https://developers.openai.com/codex/models) · [Codex changelog](https://developers.openai.com/codex/changelog)

On September 23, 2026, Codex CLI `0.156.1` and Pi's `openai-codex` catalogue both exposed `gpt-6-sol`, `gpt-6-luna`, and `gpt-6-astra`. Their IDs still differ: Pi uses provider-qualified IDs such as `openai-codex/gpt-6-sol`, whereas Codex uses bare slugs. Older CLI versions may omit new models; update the CLI and query again rather than bypassing preflight. OpenAI plans to retire `gpt-5.5` for Codex with ChatGPT sign-in on October 14, 2026; API-key use is not covered by that retirement. [OpenAI models](https://developers.openai.com/codex/models)

Set explicit `model` and `thinking` task fields when task requirements differ from inherited defaults. A task fails preflight if the requested combination is unavailable. Neither the runner nor this snapshot promises a model will remain available.

## Execution contract

The adapter invokes Codex with:

```text
codex --sandbox <workspace-write|danger-full-access> --ask-for-approval never [--search]
  exec [--ephemeral] --skip-git-repo-check -C <job-folder>
  --model <model> -c model_reasoning_effort=<thinking>
  --output-last-message <run>/<task>/result.md --json -
```

The runner writes `execution-prompt.md` beside the result, prepends its write-boundary instruction, and passes that file to Codex on standard input; Codex does not receive Pi’s conversation. The job folder is the writable workspace. The runner never uses sandbox-bypass flags; `danger-full-access` is a consciously recorded task setting, not a bypass. Codex writes its final response to `result.md`; JSON events and stderr are retained beside it. This applies to both blocking and detached launches. Background tasks have independent supervisors which record actual exit codes; a non-empty result is accepted only after successful harness exit, never merely because a file exists.
