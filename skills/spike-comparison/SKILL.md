---
name: spike-comparison
description: "Use for spike comparisons, comparative technical spikes, bake-offs, or testing competing implementation approaches before adoption. Build a shared test base, implement candidates in parallel Herdr panes and worktrees, then gather results for comparison; also evaluate existing spikes. Deliver a self-contained offline HTML report with tested scenarios, findings, evidence, screenshots, and summary recommendations; skip production implementation, inspection-only technical reviews, and approval of durable design decisions."
---

# Spike Comparison

Compare approaches through bounded experiments. Always deliver self-contained offline HTML report with evidence and summary recommendations, not production adoption.

## Trigger clarification

- Use for new experiments, existing prototypes/results, or a mix. Existing evidence may be enough; do not rebuild working spikes without a reason.
- Use `technical-review` when code and documentation inspection can answer the question without experiments.
- Use `question-research` for public-source research without prototype comparison.
- Use `tradeoff-review` when findings need a durable human design decision. A recommendation is not an approved choice.

## Required read

- Before writing or changing experiment code, load [coding](../coding/SKILL.md). Load domain skills when applicable.
- For every final spike report, load [offline-html-report](../offline-html-report/SKILL.md). Its bundled template, offline resource contract, and validation workflow are required; Markdown notes are supporting artifacts, not final deliverable.
- When using configured Attendant records, load [attendant](../attendant/SKILL.md), run `schema`, and read applicable collection schema and usage before querying or updating. Do not invent a spikes collection or record fields.
- Before parallel execution, load [herdr](../herdr/SKILL.md). Follow its environment gate, CLI discovery, explicit targeting, delivery, bounded waits, focus preservation, and safety rules. Do not guess commands or control another session.
- Spikes are active work: execute directly or through coordinated parallel work. Do not route spike implementation, testing, or review through `delegate-tasks` or fire-and-forget jobs.

## Workflow

1. Frame one decision question.
   - Inspect supplied artifacts, relevant project code, constraints, prior decisions, and existing experiment tools before asking questions.
   - Determine shared testing boundary, target workload, required behavior, hard constraints, and what decision the comparison unblocks. Identify credible candidates through project context and focused source research; retain rejection reasons. Include current or simplest viable approach when relevant.
   - For existing spikes, identify what each actually tested and which claims need verification.
   - Ask one focused question at a time only when missing scope or authority materially changes the comparison. State recommended default.
   - Done when question, alternatives, and boundaries are explicit.

2. Bound investigation and agree on evidence.
   - Use supplied budget. Otherwise propose a bounded first pass, such as one representative scenario per alternative with a fixed number of runs; state limits before execution. Do not invent a user-approved wall-clock deadline.
   - Define common scenarios, inputs, environment, observable checks, and hard pass/fail thresholds. Separate required constraints from preferences.
   - Select only decision-relevant criteria: correctness, performance, resources, integration effort, API ergonomics, operational burden, cost, or failure recovery.
   - Record measurement method, units, repetition count, and aggregation where relevant. Separate setup effort from runtime measurements.
   - Do not invent weights or thresholds that determine the winner. Ask when an unresolved preference materially changes the recommendation.
   - Done when experiment plan and stopping conditions can be checked.

3. Prepare and freeze shared base.
   - Recommended new-comparison flow: validated shared base, candidate shortlist, one candidate per Herdr pane/worktree, central collection and rechecks, then comparison and recommendation. Use direct execution for small or sequential spikes. Existing results need only missing work.
   - Use project convention for spike artifacts; otherwise use `docs/spikes/<comparison-name>/` for report and evidence. Keep candidate code outside production paths, in separately owned directories.
   - Establish shared adapter boundary, fixtures, correctness oracle, runner, build settings, and measurement harness. Keep common glue neutral; allow natural candidate APIs without changing required behavior.
   - Reuse project tools. Save repeatable harnesses with usage, input validation, safe defaults, and no secrets. Validate shared paths before worker launch.
   - Present shared base, shortlist, worker scopes, checks, collection target, and retention plan for human review before parallel launch. Do not launch workers to discover the shared contract.
   - Record exact Git base and shared-file inventory. Commit only with explicit approval. Without commit approval, freeze explicit spike-only overlay with hashes, apply it identically to worktrees from recorded HEAD, and verify before launch. Worktrees do not inherit uncommitted files; never silently stash, reset, or include unrelated changes.
   - For existing results, record provenance, commands, versions, inputs, environment, and measurement method. Mark missing details unknown; do not treat incompatible results as directly comparable.
   - Done when shared base passes checks and reviewed candidate contracts use one reproducible snapshot.

4. Implement candidates directly or in coordinated parallel work.
   - Recommend Herdr for independent candidate implementations. If unavailable, report parallel blocker and offer direct execution; never substitute background delegation.
   - Assign one candidate to each named worker in its own fresh worktree and unfocused Herdr pane. Launch independent workers before waiting. Record owner/coordinator identity, pane IDs, worktrees, branches, base inventory, and status in project-local run record.
   - Give each worker decision question, candidate pin, frozen base, owned paths, exclusions, common checks, budget, reporting route, commit authority, and required report evidence: commands, logs, measurements, screenshots where applicable, captions, and limitations. Require `coding`, applicable domain skills, and `herdr`. No further agents without authority.
   - Workers implement only assigned candidate and isolated evidence. Mark shortcuts, mocks, omitted production concerns, source changes, licenses, and provenance. Never replace a failed candidate with a fake implementation or change common fixtures to make it pass.
   - Shared defects go to coordinator. Revise shared base centrally, redistribute identical snapshot to affected workers, and rerun affected checks. Workers do not patch frozen common paths independently.
   - Use targeted messages and bounded waits. Inspect actual artifacts, diffs, checks, and blockers; idle status is not completion. Workers report durable result paths, not only scrollback.
   - Preserve focus. No OS-level keyboard/mouse control or exclusive desktop input. Browser work uses `agent-browser` with separate named sessions; request human help for required desktop actions.
   - Done when each candidate has implementation, isolated checks, retained evidence, and report, or explicit failure/blocker.

5. Gather, recheck, and measure centrally.
   - Bring candidate implementations and raw evidence into reviewed collection target. Inspect owned-directory diffs; integrate only scoped changes using approved commits or reviewed patches. Preserve original evidence before closing panes.
   - Keep candidates separately selectable through common harness; collection does not mean combining incompatible implementations into one production path.
   - Verify collected sources match worker snapshots. Rebuild and rerun common correctness checks centrally; retain integration failures and fixes.
   - Run final comparative measurements only after workers stop competing for shared resources. Serialize benchmarks on shared hardware; parallel implementation is not parallel measurement.
   - Coordinator owns consolidated comparison. Workers provide candidate findings, not final ranking.
   - Done when collected candidates pass central checks or remain explicit failures, and final comparable evidence is retained.

6. Retain and assess execution evidence.
   - Run common scenarios under comparable conditions. Record exact commands, dependency versions, environment, inputs, outputs, failures, and evidence paths.
   - Apply consistent warm-up, caching, repetition, and concurrency policy where those affect results. Avoid competing benchmark runs on shared resources.
   - Check correctness before interpreting speed or resource results. Retain failed setups and failed checks; distinguish tooling failure from an approach failing a requirement.
   - Label each finding measured, source-backed, estimated, or unknown. Link external claims to source and access date.
   - Capture screenshots of tested visual behavior, candidate outputs, and failures where useful. Record candidate, scenario, run identity, and what each image demonstrates. For nonvisual tests, use logs, data, or code excerpts instead of decorative screenshots. State unavailable captures explicitly; never fabricate evidence.
   - Existing evidence may satisfy a criterion without rerunning it when provenance and conditions fit. Explain mismatches and rerun only the smallest useful check within budget.
   - Stop when decision evidence is sufficient or budget is exhausted. Report inconclusive findings and smallest missing experiment instead of silently extending scope.
   - Ask before actions involving paid services, sensitive data, destructive changes, publishing, or other authority not supplied by the user.
   - Done when results are traceable and unrun checks remain explicit.

7. Compare and verify conclusions.
   - Compare each alternative against the same criteria. Reject hard-constraint failures before weighing preferences.
   - Explain unequal test coverage, environment differences, prototype maturity, and uncertainty. Do not rank incomparable measurements as if conditions matched.
   - Recommend an alternative, conditional choice, or inconclusive outcome. Explain confidence through evidence quality and coverage, not fabricated precision.
   - State what would change the recommendation. Include adoption work and risks not resolved by prototype success.
   - Check every decisive claim against retained evidence. Correct unsupported conclusions and recheck affected comparisons. If evidence cannot support a conclusion within budget, downgrade it or report the blocker.
   - Done when recommendation follows evidence and limitations are visible.

8. Build offline report and close comparison.
   - Always generate one self-contained offline `.html` report through `offline-html-report`, including incomplete, failed, and inconclusive spikes. Use project report convention; otherwise save `docs/spikes/<comparison-name>/report.html`.
   - Include what was tested, how it was tested, findings, supporting evidence, captioned screenshots where applicable, comparison, limitations, and summary recommendations. Embed decisive evidence, images, and chart data so report remains useful without network, worktrees, or sibling files. Include exact source/run locators for raw retained evidence; external links supplement, not replace, report content.
   - Run bundled static checker and actual offline browser checks required by report skill. Inspect report screenshots; fix and recheck failures. If blocked, deliver report with exact unverified checks and blockers rather than claiming completion.
   - Update an existing tracked task only through its configured lifecycle; spike completion does not automatically make production work ready.
   - Keep reusable evidence and run inventory. Owner closes only workflow-created panes after verifying results are retained and workers are no longer writing or awaiting essential help. Never close original or unrelated panes.
   - Remove worktrees only with cleanup authority, verified source/evidence retention, and clean status including untracked files; inspect ignored files for needed data. Preserve dirty or uncollected worktrees. Never force removal or delete user-owned work; report cleanup blockers.
   - Report result path, recommendation, confidence, checks run, checks not run, and decision or follow-up needed.
   - If comparison interrupted other work, resume only independent work or work already authorized by the result. Load [tradeoff-review](../tradeoff-review/SKILL.md) when a durable design choice needs approval; do not adopt the winner automatically.
   - Done when offline HTML report is delivered with validation status, retained evidence, and summary recommendations; production adoption remains separate.

## Output shape

Final artifact is offline HTML, not Markdown or chat-only summary. Following outline defines report content, not file format. Include criterion-by-criterion comparison as bullets or a compact table when useful.

```text
# Spike comparison: <decision question>

## Result
Recommendation: <alternative / conditional / inconclusive>
Confidence: <high / medium / low, with evidence-based reason>

## Scope and plan
Candidates, shared-base identity, constraints, budget, stopping conditions, common scenarios, worker scopes, collection target.

## What was tested
Scenarios, inputs, versions, environment, commands, methods, checks run and not run.

## Findings and comparison
For each criterion: result for each candidate, supporting evidence, limitations.
Distinguish measured findings, source-backed claims, estimates, and unknowns.

## Evidence and screenshots
Embedded decisive logs, data, excerpts, charts, and captioned screenshots where applicable.
Raw evidence locators and provenance; unavailable captures stated explicitly.

## Risks and gaps
Unrun checks, incompatible evidence, prototype shortcuts, adoption work.

## Summary recommendations
Recommended direction, rationale, confidence, decision needed, and smallest follow-up.
No automatic production adoption.

## Report validation
Static and offline browser checks performed; failures and unverified checks.

## Comments
Incidental notes or errata, only when present.
```

## Invocation examples

- “Spike SQLite vs PostgreSQL for this workload.” Establish shared harness and candidate contracts; recommend reviewed base followed by one candidate per Herdr pane/worktree, central rechecks, and sequential comparative measurements.
- “Compare these two proof-of-concept branches.” Inspect existing evidence first; rerun only checks needed for a fair comparison. Preserve current working tree.
- “Which library looks easier to maintain?” Use `technical-review` unless answering requires experimental evidence.
- “Ship the winner.” Treat as separate implementation request; resolve any remaining material design gate before adoption.
