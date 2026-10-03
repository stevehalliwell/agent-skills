---
name: style-profile
description: "Create, improve, apply, or verify a Markdown writing-style profile. Use when learning a style from exemplars, drafting or rewriting with a named profile, reviewing style conformance, comparing a corpus, or finding style outliers; select the matching workflow before substantive work."
---

# Style profile

Manage evidence-based writing-style profiles without treating profile metrics as quotas or permission to alter meaning.

## Workflow selection

Identify the requested outcome, then read **only** its workflow:

- **Learn a profile** — derive or improve a reusable profile from exemplars, a corpus, or published posts. Read [workflows/learn.md](workflows/learn.md).
- **Generate with a profile** — create or rewrite Markdown using a named profile while preserving source meaning. Read [workflows/generate.md](workflows/generate.md).
- **Verify a profile match** — review a Markdown document or corpus against a named profile, optionally applying declared deterministic replacements. Read [workflows/verify.md](workflows/verify.md).

If the outcome is ambiguous, ask whether the user wants to learn a profile, generate with one, or verify a match. Complete the selected workflow before changing outcomes.

## Shared format and executable resources

Read [Profile format and measurement rules](references/style-profiles.md) before any selected workflow. Resolve `<skill-dir>` from this loaded skill. Scripts are executable resources, not reading assignments; they use Node.js, winkNLP, and its English model. If missing dependencies block execution, obtain approval for `npm ci --prefix <skill-dir>`; otherwise report the runtime failure directly.

- When learning from local Markdown, execute [Metrics](references/style-profile-metrics.mjs) with `node <skill-dir>/references/style-profile-metrics.mjs [--output FILE] [--paragraph-label-max-words N] <Markdown files...>`.
- When verifying against a saved metrics sidecar, execute [Comparator](references/style-profile-compare.mjs) with `node <skill-dir>/references/style-profile-compare.mjs <profile.metrics.json> <Markdown files...>`. It is report-only; subjective rewrites remain the generate workflow's responsibility.
- After generating with a sidecar, read [Verify](workflows/verify.md) for the report-only comparison and any one-pass redraft recheck. After learning, read it only when the user accepts the offered corpus review.

## Rules
- Keep profile creation, generation, and verification as distinct outcomes; route between workflows only when their stated handoff conditions apply.
- Preserve user meaning, exact technical text, explicit requirements, and accessibility over profile preferences.
- Treat measurements as evidence for review, not quality grades, generation quotas, or automatic rewrite authority.
