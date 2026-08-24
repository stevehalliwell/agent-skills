---
name: release-readiness
description: "Use when preparing a release, checking README or CHANGELOG drift, or reconciling user-facing and developer-facing docs with changes since last release. Find release baseline, identify changes, update docs, and present edits for user confirmation. Do not publish, tag, version, or commit unless explicitly asked."
---

# Release readiness

Prepare accurate release documentation from repository evidence. Make doc edits; user approves or requests commit.

## Workflow

1. Resolve baseline. Use latest reachable Git tag. If absent, use latest released version heading in CHANGELOG. If absent, find version in package/version files and its introducing Git commit. If no reliable baseline, ask user. State baseline and evidence. Done when comparison range is explicit.
2. Inspect changes. Review commits and diff since baseline, package/version metadata, tests, README, CHANGELOG, and changed public interfaces. Separate confirmed behavior from inference. Check README against GitHub-reader needs: what project is for, what it does, how to use it, then only brief developer setup. Done when user-facing features, developer usage changes, fixes, internal-only changes, and README drift are classified.
3. State proposed release content. Summarize evidence, README drift, CHANGELOG entries, blockers, and unknown release target. Ask user to choose major or minor release when not supplied; do not choose it. Done when intended public claims and release target are explicit.
4. Update docs. Make README concise and user-first: lead with purpose and user-visible behavior; show shortest credible use path; keep developer setup brief. Remove stale status, exhaustive internals, repeated explanation, and developer detail that does not help a GitHub reader; link to focused docs when detail remains useful. Update CHANGELOG for selected release target using existing format. Do not invent dates, release versions, compatibility claims, or undocumented behavior. Done when docs are concise, evidence-backed, and match stated release claims.
5. Check and present. Run smallest relevant docs/resource checks, inspect diff, report changed files and remaining blockers. Wait for user confirmation; commit only after explicit request. Done when release documentation is reviewable.

## Output shape

```text
Release readiness: <baseline> → <target>

Confirmed changes:
- <evidence-backed item>

Documentation updates:
- <README/CHANGELOG change>

Blockers or open decisions:
- <item or None>

Checks:
- <command/result>

Next: review edits, confirm, or ask to commit.
```

## Rules

- Treat Git history and current code/docs as evidence; label inference.
- README order: purpose → user-visible behavior → use → brief configuration/developer setup. Keep each section only when it helps a GitHub reader act.
- Include only user-visible behavior and developer-relevant usage in README/CHANGELOG; omit internal refactors unless externally relevant.
- Keep detailed architecture, resource contracts, exhaustive inventories, and historical status in linked docs, not README.
- Do not publish, deploy, tag, bump versions, alter package metadata, or commit without explicit user request.
- Stop for release scope, compatibility, security, or public-claim ambiguity.
- Preserve established CHANGELOG conventions; do not add a new release section until user selects target.
