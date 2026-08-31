---
name: release-readiness
description: "Assess whether a project is ready for a public release, prepare release readiness, check release blockers, or review README/CHANGELOG drift. Use when asked 'is this ready to release?', before publishing, or when a release-readiness check is triggered. Inspect repository evidence, identify blockers and documentation gaps, make only evidence-backed doc edits, and report a release verdict. Do not publish, tag, version, bump package metadata, or commit unless explicitly asked."
---

# Release readiness

Assess public-release readiness from repository evidence. On load, begin this workflow immediately; do not respond only that the workflow was loaded. Make only evidence-backed documentation edits; user approves or requests a commit.

## Workflow

1. Establish repository state and baseline. Inspect Git status, latest reachable Git tag, CHANGELOG, package/version metadata, and project release instructions. Use latest tag as baseline; otherwise use latest released CHANGELOG version; otherwise find the commit that introduced the current version. If no reliable baseline exists, record it as a release blocker rather than stopping before inspection. Done when comparison range and pre-existing working-tree changes are explicit.
2. Inspect release evidence. Review commits and diff since baseline; current package/version metadata; tests and CI/release configuration; README; CHANGELOG; and changed public interfaces. Run the project’s smallest relevant test, lint, build, or documentation checks when discoverable. Separate confirmed behavior from inference. Check README against GitHub-reader needs: what project is for, what it does, how to use it, then only brief developer setup. Done when user-facing features, developer usage changes, fixes, internal-only changes, documentation drift, check results, and release risks are classified.
3. Give a verdict. State **Ready**, **Ready with decisions**, or **Not ready**, with every blocker and decision tied to repository evidence. Summarize proposed public release content and identify an unknown release target as a decision, not a reason to defer the assessment. Done when user can decide whether public release may proceed.
4. Update docs when safe. Correct README drift immediately when evidence supports the change. Update CHANGELOG only when user has supplied or selects a release target, preserving its existing format. Do not invent dates, release versions, compatibility claims, or undocumented behavior. Done when safe documentation edits are concise, evidence-backed, and match stated release claims.
5. Check and present. Inspect the final diff and report changed files, remaining blockers, decisions, and checks. Wait for user confirmation before any commit. Done when release assessment and any documentation edits are reviewable.

## Output shape

```text
Release readiness: <Ready | Ready with decisions | Not ready>

Baseline and scope:
- <baseline evidence and comparison range>

Confirmed release content:
- <evidence-backed item>

Blockers:
- <blocker or None>

Decisions needed:
- <decision or None>

Documentation updates:
- <README/CHANGELOG change or None>

Checks:
- <command/result>

Next: <specific release-readiness action>
```

## Rules

- Treat Git history and current code/docs as evidence; label inference.
- README order: purpose → user-visible behavior → use → brief configuration/developer setup. Keep each section only when it helps a GitHub reader act.
- Include only user-visible behavior and developer-relevant usage in README/CHANGELOG; omit internal refactors unless externally relevant.
- Keep detailed architecture, resource contracts, exhaustive inventories, and historical status in linked docs, not README.
- Do not publish, deploy, tag, bump versions, alter package metadata, or commit without explicit user request.
- Stop for release scope, compatibility, security, or public-claim ambiguity.
- Preserve established CHANGELOG conventions; do not add a new release section until user selects target.
