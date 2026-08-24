# Upgrade an OKF v0.1 bundle to v0.2

Use only with explicit user approval. Read specification §13 and every section governing a field being migrated.

1. Establish the source version and create a reviewable migration plan: affected concept files, root index, legacy timestamps, citation sections, and any local tooling. Do not change files yet. Done when the user can review the migration scope.
2. After approval, migrate only transformations supported by the v0.2 specification and existing source evidence. Convert legacy timestamps to `generated` only when the actor is known or supplied; do not invent it. Preserve unknown fields and original claims. Done when every conversion has an evidence trail.
3. Add or update the root `index.md` version declaration to `okf_version: "0.2"` when the bundle targets v0.2. Keep index structure compliant. Done when the declared target version is accurate.
4. Validate the full bundle with the validation workflow, then report converted files, any skipped ambiguous data, and remaining warnings. Done when the post-upgrade conformance result is known.
