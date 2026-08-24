---
name: okf
description: "Create, update, validate, repair, migrate, or keep an Open Knowledge Format (OKF) bundle aligned with its codebase and the OKF v0.2 specification. Use for OKF bundles, knowledge bundles, concept Markdown files, stale or drifted docs, index.md/log.md upkeep, OKF conformance, and v0.1-to-v0.2 upgrades; skip ordinary Markdown documentation that is not an OKF bundle."
---

# OKF bundle alignment

Maintain one source-aware OKF workflow without requiring a collection of companion skills.

## Trigger clarification

Confirm the target is an OKF knowledge bundle: a directory tree of Markdown concept documents with YAML frontmatter, normally named `okf/`. For an ambiguous documentation directory, inspect it before treating it as OKF. Stop if it is not an OKF bundle.

## Required reference

Use [OKF v0.2 specification](ref/OKF-SPEC-v0.2.md) as the authority. Before acting, read the sections relevant to the selected mode; always consult §3, §4, and §11. Do not substitute local convention or this skill's summaries for the specification.

## Workflow

1. Locate the bundle root. Prefer the nearest parent `okf/` for the files being changed; otherwise identify the configured bundle root. Inventory Markdown files, reserved `index.md`/`log.md`, and any declared `okf_version`. Done when the root and current version/state are known.
2. Select one mode and read its workflow:
   - **Create or update concepts, including code-alignment upkeep:** [maintain](workflows/maintain.md)
   - **Audit or repair conformance and documentation drift:** [validate](workflows/validate.md)
   - **Migrate a v0.1 bundle to v0.2:** [upgrade](workflows/upgrade.md)
   If more than one mode is needed, complete upgrade before maintain or validate. Done when the sequence is explicit.
3. Apply the selected workflow. Preserve existing producer-defined frontmatter keys and document meaning; do not fabricate sources, verification events, actors, timestamps, or implementation facts. Done when all selected workflow completion criteria hold.
4. Report the bundle root, mode, files changed, validation result, and remaining warnings or unknowns. Done when the user can distinguish completed alignment from deferred work.

## Deterministic validation

Run `python scripts/validate_okf.py <bundle-root>` from this skill directory for structural conformance. Install its dependency once with `python -m pip install -r scripts/requirements.txt`. Add `--drift` for Git-based resource freshness warnings, and use `--strict-links` or `--strict-drift` only when the repository adopts those stricter policies.

## Rules

- `type` is the only universally required concept-frontmatter field; optional fields must not be treated as required.
- Reserved filenames are reserved at every directory level. `index.md` and `log.md` are not concepts.
- Broken concept links are allowed by the specification. Report them as navigation warnings only; do not fail conformance or silently delete the links.
- Use `generated.at` only for a meaningful content change and set it from the current UTC time. Preserve `generated.by` when known; otherwise ask rather than inventing an actor.
- Treat `verified` as evidence, not an editing stamp: changing content does not create verification. Preserve it unless the user gives an evidence-based reason to update it.
- Do not perform a v0.1-to-v0.2 migration without an explicit request or approval.

## Reference maintenance

The checked-in specification snapshot is `ref/OKF-SPEC-v0.2.md`, sourced from `https://github.com/GoogleCloudPlatform/knowledge-catalog/blob/main/okf/SPEC.md`. When the upstream specification is refreshed, replace the snapshot, retain its versioned filename or add a new versioned snapshot, and update this section if the default authority changes.
