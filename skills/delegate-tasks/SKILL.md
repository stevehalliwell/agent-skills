---
name: delegate-tasks
description: "Delegate a fire-and-forget aside, background investigation, independent review, research job, or bounded Codex one-shot while main work continues. Use for self-contained side work whose result can be collected later; use Herdr directly when running inside Herdr, otherwise the bundled job runner. Skip tiny work, dependent steps, tasks needing unstated session context, and coordinated multi-worker implementation or integration; use herdr-parallel-work for that workflow."
---

# Delegate tasks

Hand off self-contained asides without interrupting main work. Use Herdr's lifecycle management inside Herdr; otherwise use the harness-neutral job runner.

## Trigger clarification

Use this skill when each child can receive a complete written task and work independently, usually while the parent continues main work. “Fire-and-forget” means no ongoing orchestration, not permission to ignore failures or trust unchecked results. Use the parent directly when work is small, requires iterative clarification, depends on another child’s intermediate result, or costs more to explain than to perform.

Typical asides: independent review, research, diagnosis, or a bounded artifact. Give any file-writing aside explicit owned paths that do not overlap active parent or sibling edits. Route coordinated implementation needing a reviewed shared base, worker worktrees, or planned integration to `herdr-parallel-work`.

For a single bounded Codex request, use this skill. Background execution is useful only when the parent can continue before results are needed.

## Choose execution route

- When `HERDR_ENV=1`, load `herdr` and use [Herdr asides](#herdr-asides). Let Herdr manage agents and panes; do not scaffold a runner batch, add a detached supervisor, or launch a dedicated orchestrator.
- Otherwise use [Job runner workflow](#job-runner-workflow). Use this route inside Herdr too when explicitly requested or when runner-specific durable manifests or resource isolation are required. Herdr panes do not provide the runner's resource isolation.

## Herdr asides

1. **Define aside.** Write goal, inputs, constraints, modification authority, checks, and deliverable into a complete prompt. Include applicable [result contract](#result-contracts). Do not assume the child knows parent conversation history.
   *Done when child can work independently within explicit scope.*

2. **Launch through Herdr.** Follow `herdr` CLI discovery, layout selection, agent startup, and targeting rules. Substantial assignments, work likely to need user help, and work intended for later user inspection get separate labeled tabs; small transient helpers may use split panes. Preserve user focus. Send prompt without a blocking wait when parent can continue. Record returned pane ID and unique agent name; no separate batch manifest is needed.
   *Done when prompt submission succeeds and child identity is recorded.*

3. **Resume main work; collect later.** Use Herdr's agent state, waits, and reads when result is needed, not a separate polling loop. Inspect blockers or timeouts before intervening; never blindly resend prompt. Follow `herdr` file-output fallback if scrollback cannot recover full result. Inspect actual output and checks before using it; `idle` or `done` alone is not accepted completion.
   *Done when result is inspected, or failure/blocker is reported.*

4. **Finish aside.** Report result and remaining limits. Leave pane available unless closure is authorized; close only recorded workflow-created panes after verifying no essential work remains. Preserve file outputs. For cancellation, use Herdr controls and report unfinished work; never target original or unrelated panes.
   *Done when result and pane disposition are clear.*

## Job runner workflow

1. **Decide whether delegation earns its overhead.** Split only independent, self-contained work. Write all goal, inputs, constraints, deliverable, modification authority, verification, and network requirements into each prompt. Include a task-specific [result contract](#result-contracts); isolated children cannot rely on parent global guidance. For Codex tasks using `workspace-write`, name every required input and ensure it is copied into the job folder; Codex can read and write only that folder.
   *Done when each child can work without Pi conversation history or files outside its available workspace.*

2. **Create the batch folder.** Resolve `<skill-dir>` from this loaded skill. The runner uses Python and the chosen Pi/Codex CLI; report actionable invocation failures instead of adding availability probes. Prefer `.agent-jobs/<name>` in the current project. Read [Batch format](references/batch-format.md) before writing task/default settings. Run:
   ```bash
   python <skill-dir>/scripts/agent-job.py scaffold .agent-jobs/<name>
   ```
   Edit `batch.json` and create one prompt in `tasks/` per task. It is the complete execution contract: task values override `defaults`; do not use command flags for harness, model, thinking, `web_search`, `ephemeral`, `sandbox`, or resource policy.  
   *Done when all task names are unique and prompt paths are non-empty.*

3. **Set harness and resource policy.** Default to `pi`; choose `codex` only when requested or useful for an independent environment. Codex defaults to `sandbox: "workspace-write"`, which can read and write only the job folder; copy required source files and other inputs there before launch. Use `sandbox: "danger-full-access"` only as an explicit per-task exception for commands blocked by that sandbox; the runner prepends the job folder as the task's only allowed write location to every Codex prompt. Before assigning a Pi `model`, inspect its currently available catalogue when needed:
   ```bash
   python <skill-dir>/scripts/agent-job.py models pi
   ```
   Codex uses `web_search` (default false), `ephemeral` (default true), and `sandbox` (default `workspace-write`; `danger-full-access` only per task). Query its signed-in catalogue before assigning a Codex `model` or `thinking` level:
   ```bash
   python <skill-dir>/scripts/agent-job.py models codex
   ```
   Pi uses provider-qualified IDs (for example, `openai-codex/gpt-6-sol`); Codex uses bare slugs (`gpt-6-sol`). Both catalogues currently expose GPT-6, but availability can differ by CLI version and account. Check the selected harness's live catalogue. Pi children start isolated from discovered skills, extensions, context files, templates, and themes; add back only named or explicit allowlisted resources.
   *Done when each child’s available resources are intentional.*

4. **Launch.** Use a blocking invocation only when results are needed now:
   ```bash
   python <skill-dir>/scripts/agent-job.py run .agent-jobs/<name>
   ```
   Otherwise launch asynchronously and continue the parent task:
   ```bash
   python <skill-dir>/scripts/agent-job.py run .agent-jobs/<name> --background
   ```
   This validates the task contract and launches an independent detached supervisor per task, recording launch success or failure in the batch manifest. Supervisors record actual harness exit codes and accept only non-empty results after successful exit; file existence is not completion. Later status checks and joins read durable task state independently of the original launcher, and cancellation targets recorded live process identities even if a manifest is stale.
   *Done when the returned run ID and manifest path are reported.*

5. **Inspect, join, or cancel later.** Do not manually poll while waiting. Use:
   ```bash
   python <skill-dir>/scripts/agent-job.py status .agent-jobs/<name> [--run ID]
   python <skill-dir>/scripts/agent-job.py wait .agent-jobs/<name> [--run ID]
   python <skill-dir>/scripts/agent-job.py result .agent-jobs/<name> [--run ID]
   python <skill-dir>/scripts/agent-job.py cancel .agent-jobs/<name> [--run ID]
   ```
   `wait` blocks until every task reaches a terminal state; a failed task does not return the batch early. Read completed artifacts and report the result path, harness/model where relevant, elapsed time, and any failure, timeout, or missing result before integrating results.  
   *Done when every result used by the parent is traceable to its run artifact.*

## Result contracts

Put the applicable contract directly in each child prompt. Return only relevant sections; no routine tool narration or repeated summary. Compact output does not limit investigation or reasoning. Keep evidence, necessary rationale, safety warnings, and unresolved uncertainty; expand when a short finding would mislead.

- Localization: locations and blockers only. Do not propose fixes unless requested.
- Review: findings with evidence and suggested fixes, plus checks and limits. No findings means no findings within inspected scope, not proof of correctness.
- Implementation: material changes, actual checks, and unresolved risk. Re-reading a file is not sufficient verification of behavior.
- Cite only line ranges actually read; never estimate or invent ranges. If no relevant location exists, say so.

```text
Locations:
- path:line-range — `symbol` — relevance

Findings:
- path:line — problem — evidence — suggested fix

Changes:
- path:line-range — material change

Validation:
- command — pass, fail, unavailable, or blocked — decisive result

Risks:
- unresolved uncertainty or unrun check

Blocked:
- exact missing input or capability
```

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
        ├── task.json           # Resolved background task contract
        ├── supervisor.log      # Background supervisor diagnostics
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

- [Batch format](references/batch-format.md) — read before editing task/default settings.
- [Complete examples](examples/) — read when a runnable configuration or prompt example is needed.
- [Pi harness](references/pi.md) — read for Pi tasks with resource allowlists or isolation requirements.
- [Codex harness](references/codex.md) — read before launching Codex tasks or choosing its sandbox/search policy.
