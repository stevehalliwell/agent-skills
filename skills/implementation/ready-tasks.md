# Find implementation-ready tasks

Use this workflow when no clear implementation focus is supplied or selected. This is a read-only triage report: do not select, update, create, or implement any task.

1. Inspect `.pi/attendant.tables`. If it configures `tasks`, load and follow `/skill:attendant` and `/skill:task-lifecycle`; read `tasks/.schema.md`; then use Attendant queries/searches to find every task with status `todo`. Treat `todo` as implementation-ready unless its record still has unresolved outcome, acceptance criteria, dependency, or open question that prevents safe implementation. Report such exceptions separately as “Not ready despite status”.
2. If no Attendant `tasks` collection exists, find project task sources (for example TODO files, issue lists, or task Markdown) and identify only items with clear scope and acceptance. State source and readiness criteria used.
3. Read each candidate task record enough to assess affected area, likely work involved, dependencies, risk, and whether it is a low-risk quick win. Do not infer certainty where record lacks evidence.

Return every ready task name exactly, grouped first by **area of impact**. Within each area, group by **involvement**: Quick (small/local), Moderate (multi-file or integration), or Significant (cross-cutting, migration, or uncertain). For every task include:
- exact task name
- short outcome
- likely affected area/files or systems
- involvement and why
- risk: low, medium, or high, with reason
- dependencies/blockers
- quick-win label only when low risk, self-contained, and likely small

End with:
- **Low-risk quick wins:** every qualifying task name, or “None identified”
- **Not ready despite status:** task name plus missing information or blocker
- **How to proceed:** “Reply with exact task name to pursue it.”

Keep report concise. Do not recommend a task unless requested; task names must remain easy to copy verbatim.
