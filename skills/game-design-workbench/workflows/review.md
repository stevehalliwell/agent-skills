# Selected review and provenance

## Review

1. Establish requested framework and scope. Read only selected provisional guide; note relevant coverage gaps. Done: questions and evidence limits are known.
2. Read current concept/mode, relevant schemas/usages, records, and retained rationale. In design mode compare with supplied intent; in explorer mode investigate behavior/leads without treating absent destination as a defect. Do not switch mode or adopt goals. Done: interpretations use actual source owners.
3. Inspect Git status and relevant diff. Record source commit and any relevant draft/untracked paths. If original-intent comparison is requested, read explicitly selected commit/path. No automatic source snapshot or hash inventory is required. Do not commit drafts merely to review them. Done: reviewed source state and limitations are stated.
4. Inspect relevant relationships using source and, when useful, Attendant queries. Separate broken data, missing required relationships, unknowns, opportunities, conflicts, unverified hypotheses, and deliberate exclusions. Missing optional data is not invalid. Done: findings cite exact source field/section and distinguish fact from interpretation.
5. Keep computed checks separate from agent judgments. SQL inventories are not framework evaluators. Do not label manual rank/route interpretation as a native check. Before returning, inspect whether relevant source changed during review; reconcile changed portions or state mixed-state limits. Done: reasons, evidence, and coverage are explicit.
6. Return useful findings/questions and limitations. Save dated report only when requested. Done: scope, sources, unsupported checks, and missing inputs remain visible.

## Findings and saved reviews

Chat or ordinary Markdown is sufficient. Optional [structured example](../templates/review-input.json) illustrates fields; no saver, schema engine, or custom report command is required.

Each actionable finding includes:

- Recognizable finding label/code, kind, severity, and origin (`agent` or actually computed check).
- Stable record IDs when available, source paths plus fields/sections, and revision where relevant.
- Trigger/evidence, consequence or design question, and suggested investigation.
- Coverage, assumptions, and limitations. No findings does not establish completeness.

Kinds: invalid/broken, missing required relationship, unknown/unresolved, possible opportunity, conflicting claims, unverified hypothesis, deliberately excluded.

Optional intent relation: supports, contradicts, tension, unknown, outside intent, possible goal drift. Cite concept section and selected historical revision when used. These labels are interpretations, not empirical proof.

If saving is requested, write a new dated Markdown report under `game-design-results/` outside collections. Include framework, scope, date, source commit/draft context, comparison commit/path, performed checks, findings, and limitations. Keep structured data too when useful or requested; do not mandate duplicate formats or generated IDs. Reuse recognizable labels for recurring findings without rewriting old reports.

## Reopening history

Read old reports as historical. Use Git log/diff and cited paths to inspect changes since review; if relevant drafts were uncommitted, exact reconstruction may be unavailable. Say so. Changed inputs, rules, or guidance require fresh inspection before claiming current applicability. Even unchanged source does not make an old interpretation newly run.

Preserve old reports and evidence; no automatic repair, adoption, rerun, or commit follows reopening. Historical hash metadata from retired tooling can remain untouched, but does not require retaining that tooling. Missing revisions/data are explicit reproduction limits.
