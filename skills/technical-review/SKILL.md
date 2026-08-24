---
name: technical-review
description: "Use when reviewing implementation feasibility, architecture, maintainability, code or data flows, dependencies, integrations, delivery risk, or technical trade-offs before coding. Use for a technical review, design critique, feasibility check, or recommendation between implementation options. Review and recommend; do not edit code or silently make architectural decisions."
---

# Technical review

Turn a technical question into an evidence-based recommendation with explicit trade-offs and checks.

## Workflow

1. Frame the decision. State desired outcome, constraints, compatibility requirements, non-goals, and decision deadline. Done when the review question is narrow enough to answer.
2. Trace the system. Inspect relevant callers, data flows, interfaces, configuration, dependencies, tests, and operational paths. Record file paths or source evidence. Done when the affected path and preserved behavior are known.
3. Identify options. Include the existing or simplest viable path, then alternatives only when they solve a real constraint. Compare correctness, complexity, maintainability, performance, security, cost, and reversibility. Done when material trade-offs are explicit.
4. Check failure modes. Cover invalid input, boundaries, compatibility, migration or rollout, observability, recovery, and test gaps relevant to the decision. Done when risks have owners or mitigations.
5. Recommend. Choose an option only when evidence and stated authority permit it; otherwise present a decision with a preferred option and rationale. State assumptions and triggers that would change the recommendation. Done when a developer can act without guessing the basis.
6. Capture uncertain standards research. Unless user explicitly requests discussion only, follow [OKF research artifacts](../references/okf-artifacts.md) when authoritative research informs the review. Done when provenance is saved or opt-out is explicit.
7. Define validation. Specify focused tests, checks, manual verification, and review points. Done when the recommendation has observable acceptance checks.

## Output shape

```text
## Decision and constraints
## Findings
## Options and trade-offs
## Risks and mitigations
## Recommendation
## Validation plan
## Open decisions
```

## Rules

- Inspect real code paths before judging feasibility.
- Distinguish observed behavior, assumptions, and recommendations.
- Prefer the smallest existing mechanism that satisfies the requirement.
- Preserve compatibility and call out behavior changes.
- Treat security, data loss, migration, and rollback risks explicitly.
- Do not implement, refactor unrelated code, or claim validation that was not run.
- Escalate cross-feature architecture, public API, migration, or cost decisions to a broader trade-off review when needed.
