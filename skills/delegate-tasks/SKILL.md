---
name: delegate-tasks
description: "Delegate tasks, launch parallel subagents, run a Codex one-shot, kick Codex off in the background, start a background Pi agent, check delegated-job status, wait for delegated tasks, cancel jobs, or get independent reviews concurrently. Use for bounded self-contained investigations, second opinions, or independent implementation/research work; skip tiny work, dependent steps, interactive Codex conversations, and tasks needing unstated session context."
---

# Delegate tasks

Launch durable delegated jobs through the harness-neutral runner.

## Trigger clarification

Use this skill only when each child can receive a complete written task and work independently. Use the parent directly when the work is small, requires iterative clarification, depends on another child’s intermediate result, or would cost more to explain than to perform.

For a single bounded Codex request, use this skill. Choose background mode only when the parent can genuinely continue before the results are needed.

## Workflow

1. **Decide whether delegation earns its overhead.** Split only independent, self-contained work. Write all goal, inputs, constraints, deliverable, modification authority, verification, and network requirements into each prompt. For Codex tasks using `workspace-write`, name every required input and ensure it is copied into the job folder; Codex can read and write only that folder.
   *Done when each child can work without Pi conversation history or files outside its available workspace.*

2. **Create the batch folder.** Prefer `.agent-jobs/<name>` in the current project. Run:
   ```bash
   python ~/.pi/agent/skills/delegate-tasks/scripts/agent-job.py scaffold .agent-jobs/<name>
   ```
   Edit `batch.json` and create one prompt in `tasks/` per task. It is the complete execution contract: task values override `defaults`; do not use command flags for harness, model, thinking, `web_search`, `ephemeral`, `sandbox`, or resource policy.  
   *Done when all task names are unique and prompt paths are non-empty.*

3. **Set harness and resource policy.** Default to `pi`; choose `codex` only when requested or useful for an independent environment. Codex defaults to `sandbox: "workspace-write"`, which can read and write only the job folder; copy required source files and other inputs there before launch. Use `sandbox: "danger-full-access"` only as an explicit per-task exception for commands blocked by that sandbox; the runner prepends the job folder as the task's only allowed write location to every Codex prompt. Before assigning a Pi `model`, inspect its currently available catalogue when needed:
   ```bash
   python ~/.pi/agent/skills/delegate-tasks/scripts/agent-job.py models pi
   ```
   Codex uses `web_search` (default false), `ephemeral` (default true), and `sandbox` (default `workspace-write`; `danger-full-access` only per task). Pi children start isolated from discovered skills, extensions, context files, templates, and themes; add back only named or explicit allowlisted resources.  
   *Done when each child’s available resources are intentional.*

4. **Launch.** Use a blocking invocation only when results are needed now:
   ```bash
   python ~/.pi/agent/skills/delegate-tasks/scripts/agent-job.py run .agent-jobs/<name>
   ```
   Otherwise launch asynchronously and continue the parent task:
   ```bash
   python ~/.pi/agent/skills/delegate-tasks/scripts/agent-job.py run .agent-jobs/<name> --background
   ```
   This preflights and launches every task as a detached process, then records launch success or failure in the batch manifest. Later status checks and joins reconcile task PIDs and artifacts independently of the launching process; batches survive parent exit.  
   *Done when the returned run ID and manifest path are reported.*

5. **Inspect, join, or cancel later.** Do not manually poll while waiting. Use:
   ```bash
   python ~/.pi/agent/skills/delegate-tasks/scripts/agent-job.py status .agent-jobs/<name> [--run ID]
   python ~/.pi/agent/skills/delegate-tasks/scripts/agent-job.py wait .agent-jobs/<name> [--run ID]
   python ~/.pi/agent/skills/delegate-tasks/scripts/agent-job.py result .agent-jobs/<name> [--run ID]
   python ~/.pi/agent/skills/delegate-tasks/scripts/agent-job.py cancel .agent-jobs/<name> [--run ID]
   ```
   `wait` blocks until every task reaches a terminal state; a failed task does not return the batch early. Read completed artifacts and report the result path, harness/model where relevant, elapsed time, and any failure, timeout, or missing result before integrating results.  
   *Done when every result used by the parent is traceable to its run artifact.*

## Artifacts

```text
.agent-jobs/<name>/
├── batch.json
├── tasks/
│   └── <task>.md
├── .agent-jobs/current.json
└── runs/<run-id>/
    ├── manifest.json
    └── <task>/
        ├── manifest.json
        ├── events.log
        ├── stderr.log
        ├── result.md
        └── execution-prompt.md  # Codex only
```

Every launch receives a unique run ID. The runner retains all task prompts and run artifacts; cleanup is a deliberate user/project action, never runner behavior.

## Batch example

```json
{
  "deadline": 1200,
  "defaults": { "harness": "pi", "thinking": "high" },
  "tasks": [
    { "name": "api-review", "prompt": "tasks/api-review.md" },
    { "name": "tests-review", "prompt": "tasks/tests-review.md" },
    { "name": "second-opinion", "prompt": "tasks/second-opinion.md", "harness": "codex" }
  ]
}
```

## Rules

- Do not include credentials, full conversation transcripts, or unnecessary parent context in prompts or manifests.
- Keep background jobs only for work whose result is not needed before the parent can proceed; cancel them explicitly by run ID.
- Use Codex only for bounded, non-interactive work that its task prompt fully specifies.

## Harness references

- [Batch format](references/batch-format.md) — canonical task/default settings and resolution.
- [Complete examples](examples/) — runnable batch configuration and self-contained Pi/Codex prompts.
- [Pi harness](references/pi.md) — default isolation and explicit skill/extension allowlists.
- [Codex harness](references/codex.md) — Codex model, thinking, search, sandbox, and artifact behavior.
