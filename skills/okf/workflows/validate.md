# Validate or repair an OKF bundle

Use for conformance checks, stale-document audits, post-edit alignment, or repair. Read specification §§3, 4, 6, 8, 9, and 11; read §5 or §10 when those frontmatter families appear.

1. Run `python scripts/validate_okf.py <bundle-root>` from the skill directory. Add `--drift` to compare locally referenced resources with Git history. Done when deterministic conformance errors and non-blocking warnings are listed with paths.
2. Check soft alignment separately: compare concepts to their declared sources, inspect `stale_after`, and map known recent code changes to concepts. Flag missing coverage and stale claims as warnings unless a stricter local policy exists. Done when warnings are distinguished from specification failures.
3. Repair only defects supported by the bundle or source evidence. Ask before choosing a concept type, actor, provenance, verification event, lifecycle status, or facts that cannot be established. Done when repairs are evidence-based.
4. Re-run the validator and report: conformant/non-conformant, repaired paths, warnings, and unresolved questions. Done when the final status is reproducible.
