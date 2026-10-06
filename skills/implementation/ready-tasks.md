# Find implementation-ready tasks

Use this workflow when no clear implementation focus is supplied or selected. This is a read-only recommendation report: recommend a default task, but do not select, update, create, or implement any task.

1. Inspect `.pi/attendant.tables`. If it configures `tasks`, load and follow `/skill:attendant`; run `schema` and read the configured task schema and usage guidance. Use Attendant queries/searches to find every task in the documented implementation-ready state. Resolve material ambiguity when usage is missing or contradicts the schema; do not infer readiness from a status name alone. Check each candidate for unresolved outcome, acceptance criteria, dependencies, or open questions that prevent safe implementation. Report such exceptions separately as “Not ready despite status”.
2. If no Attendant `tasks` collection exists, find project task sources (for example TODO files, issue lists, or task Markdown) and identify only items with clear scope and acceptance. State source and readiness criteria used.
3. Read each candidate task record enough to assess recorded priority, affected area, likely work involved, dependencies, risk, and whether it is a low-risk quick win. Use the task source's documented priority order; do not assume numeric or label ordering. Do not infer certainty where record lacks evidence.
4. Recommend the highest-priority ready task by default. Quick wins, involvement, risk, and area of impact are secondary factors: use them to break ties within the highest priority, not to promote a lower-priority task. Exclude blocked or unready tasks before ranking. If priorities are absent or incomparable, state that limitation and give a provisional recommendation using the secondary factors without inventing priority.

Lead with **Recommended task:** exact task name, recorded priority, and a short reason. Return every ready task name exactly, ordered by priority before secondary factors. Label involvement as Quick (small/local), Moderate (multi-file or integration), or Significant (cross-cutting, migration, or uncertain). For every task include:
- recorded priority, or “Not recorded”
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
- **How to proceed:** ask the user to choose a task with `ask_user`, marking the recommended task as the default option. If no ready tasks exist, report that instead of asking for a selection.

Keep report concise. Recommend by default when no focus is selected; task names must remain easy to copy verbatim.
