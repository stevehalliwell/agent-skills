# Complete batch example

Copy this directory into a project job folder, then adapt task names, prompts, and the model/resource settings before launch.

```text
mkdir -p .agent-jobs
cp -R ~/.pi/agent/skills/delegate-tasks/examples .agent-jobs/review
python ~/.pi/agent/skills/delegate-tasks/scripts/agent-job.py run .agent-jobs/review
```

`batch.json` demonstrates every current task setting:

- shared: `deadline`, `harness`, `model`, `thinking`;
- Pi: `skills`, `extensions`, `exclude_skills`, `exclude_extensions`;
- Codex: `web_search`, `ephemeral`, `sandbox` (`workspace-write` by default; `danger-full-access` only for an explicit task-level exception).

The runner prepends the job folder as Codex's only permitted write location, including when its sandbox is `danger-full-access`.

The `technical-review` skill exists in this global installation. The empty `extensions` list is intentional; add only an installed extension name or path that the child needs. The prompts demonstrate the self-contained context contract; replace their generic “named local implementation” inputs with exact paths and decisions for real work.
