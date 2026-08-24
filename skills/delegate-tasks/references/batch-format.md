# Batch format

`batch.json` is the complete execution contract. The runner has no invocation-level model, reasoning, network, or harness overrides.

```json
{
  "deadline": 1200,
  "defaults": {
    "harness": "pi",
    "model": "gpt-5.6-sol",
    "thinking": "high",
    "skills": [],
    "extensions": []
  },
  "tasks": [
    {
      "name": "api-review",
      "prompt": "tasks/api-review.md"
    },
    {
      "name": "independent-opinion",
      "prompt": "tasks/opinion.md",
      "harness": "codex",
      "model": "gpt-5.6-terra",
      "thinking": "low",
      "web_search": true,
      "ephemeral": true,
      "sandbox": "workspace-write"
    }
  ]
}
```

## Resolution

1. The task inherits every setting from `defaults`.
2. Its own fields override inherited values.
3. Task `deadline` overrides `defaults.deadline`, which overrides the batch-level `deadline` (default: 1200 seconds).
4. The selected harness translates the resulting effective task settings. Settings unsupported by a harness remain explicit in the manifest but do not silently enable a capability.

## Fields

| Field | Scope | Meaning |
| --- | --- | --- |
| `name` | task | Unique safe artifact-directory name. |
| `prompt` | task | Non-empty prompt path inside the job folder. |
| `harness` | default/task | `pi` (default) or `codex`. |
| `model` | default/task | Harness-specific model selection. |
| `thinking` | default/task | Harness-specific reasoning level. |
| `web_search` | Codex default/task | Enable Codex web search; default false. |
| `ephemeral` | Codex default/task | Request Codex ephemeral mode; default true. |
| `sandbox` | Codex default/task | `workspace-write` (default) confines reads and writes to the job folder; copy every required input there and name it in the task prompt. `danger-full-access` permits unrestricted host commands. |
| `deadline` | batch/default/task | Wall-clock seconds for one task. |
| `skills`, `extensions` | Pi default/task | Explicit resource allowlists. |
| `exclude_skills`, `exclude_extensions` | Pi default/task | Remove named allowlist entries for one task. |

`web_search`, `ephemeral`, and `sandbox` are translated by Codex only. Use task settings, not command flags, to keep a durable, reviewable record of every delegated invocation. `danger-full-access` is an explicit per-task exception, never a batch default.
