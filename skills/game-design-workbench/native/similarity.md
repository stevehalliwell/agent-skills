# Typed similarity and MDS kernel

`gd_similarity.{h,c}` implements Review 11's numerical kernel. `gd_similarity_projection.{h,c}` owns native feature catalogue/extraction; `gd_similarity_output.{h,c}` owns typed output and reuses existing source serializers. Named `similarity-features` / `similarity-map` queries share canonical structural preparation; no new graph evaluator. Viewer controls/separate canvas implemented and CJIT compile-checked; lifecycle review active, full build/window/manual checks deferred.

## Inputs and ownership

- Pass explicit allocator and immutable `GD_SimilarityInput`.
- Borrow IDs, feature descriptors, row-major cells, named strings and set members until destruction.
- Require unique nonempty canonical entry IDs. IDs, feature names, units and named values use existing 8,192-byte native text bound.
- Accept 0–4,096 entries and 1–256 selected features. Each set has at most 32,768 sorted unique nonnegative snapshot identities. Empty set is known; absent cell is missing.
- Caller supplies explicit finite nonnegative weights, normally 1. Require at least one positive weight. No rarity/prevalence weighting. Zero-weight selected features still require known values.
- Return NULL for invalid input/allocation failure. Work exhaustion returns unavailable limited result, not guessed coordinates. `input_complete` must hold before treating inventory counts/ranges as final.

## Distances

Only complete rows participate. Missing any selected feature excludes row from numeric ranges, distances, landmarks, fitting and distortion. Retain missing cells externally for source-backed reasons; do not impute values or compare shared known dimensions.

- Numeric: absolute difference divided by observed complete-row range. Constant range contributes 0. Preserve close differences at large offsets; halve endpoints only if direct range subtraction overflows.
- Named: exact equality contributes 0; difference contributes 1. No ordinal/semantic interpretation.
- Set: Jaccard distance, `1 - intersection / union`. Both empty contributes 0. Members belong to loaded context, not selected-entry offsets.
- Combine through weighted mean. Normalize weights by largest weight to avoid sum overflow without changing intended ratios.
- `gd_similarity_distance` returns optional unweighted components and weighted mean under independent charged budget. It does not mutate construction work. A failed call publishes no valid distance; ignore any partially filled component buffer.

## Fit and disclosure

- Sort canonical IDs before selecting/fitting; input row reorder does not change identity-to-coordinate mapping.
- Up to 32 complete rows: classical MDS over all complete rows.
- Larger inputs: 32 deterministic farthest-first landmarks, starting at smallest canonical ID; ties use canonical order. Cache landmark distances and fit every complete row, not a sampled entry subset.
- Normalize landmark distances by reported `distance_scale` before squaring, then restore coordinate scale. This preserves tiny but representable distances that direct squaring would erase. A fit with positive scale but wholly unrepresentable coordinates fails explicitly.
- Double-center normalized squared distances. Cyclic Jacobi eigensolver uses fixed pair order and at most 64 sweeps. Use largest two positive eigenvalues, with canonical landmark-order sign convention. Do not manufacture separation for coincident/constant entries.
- Retain significant negative eigenvalue count/mass for non-Euclidean input. Mass uses normalized squared-distance scale. Two-dimensional projection can distort otherwise valid distances.
- Distortion checks all pairs up to 8,192 pairs. Larger inputs use 8,192 evenly spaced ordinals in canonical-ID pair order, including first/last pair. `distortion_complete` distinguishes exhaustive check from sample. Report pair count, `distance_scale`, squared error and squared source distance. Both sums use distances divided by `distance_scale` to avoid squared-distance underflow; normalized stress is `sqrt(squared_error / squared_distance)` when denominator is positive.
- `failure` distinguishes work limit, fit-sweep limit and numerical failure. No coordinates/placed flags survive failed fit or distortion completion. Successful fit of zero complete rows is an empty checked result, not proof that missing rows are similar.

## Native projection features

- Borrow immutable `GD_StructureResult` and project until projection destruction. Require canonical forward context and ordinary partner lens; feature directions describe projection endpoints, not inferred execution.
- Discover finite feature IDs through `gd_similarity_feature_catalogue`. Select exact advertised IDs; no query grammar. Reject unknown/duplicate features and invalid/all-zero weights.
- Support retained partner/row/kind counts; incoming/outgoing row counts only for directed projections; authored rank where native reader supports it; source status and loaded role; finite Actor query's selected-variant value; typed direct/reached target sets.
- `selected-variant-value` describes selected local binding's hypothesis context, not an inferred state of each partner. Preserve native axis type/unit and input spelling; numeric values reuse native parser, including integral decimals.
- Relationship feature IDs disclose direct/reached plus outgoing/incoming/adjacent and exact native kind. `all-selected-kinds` uses applied context's relation filters. It is a union, not an ordered-path grammar. Caller must choose/disclose continuation boundaries explicitly.
- Entry mask only chooses comparison rows. Alternatively supply up to 64 exact native IDs; repeated IDs form a set, input order does not change placement. Reject mask plus ID list, missing/ambiguous/hidden IDs, oversized IDs and count overflow. Matching charges bounded native-ID work. Other shown context nodes remain set members. Set identities use canonical snapshot nodes, never selected-entry offsets. Source/partial coverage remains available through borrowed context; retained counts/sets describe supplied projection, not unknown external declarations.
- Native closure computes reached sets in separate owned scratch. It does not mutate source analysis. Keep per-closure 16,777,216 search ceiling and global feature/fit work separate. Unsupported projection features are not advertised; incidence projections have adjacent sets, never fabricated flow direction.
- Absent authored scalar fields and nodes without variant context retain distinct missing reasons. Empty retained sets are known. Any missing selected feature excludes row through numerical kernel.
- Native graph/query construction truncation prevents fitting; do not silently fit a dropped-entry prefix. Work, storage and source-construction limits publish no coordinates. `table_complete` must hold before inspecting feature cells as a complete table.
- Feature projection owns at most 67,108,864 bytes, including numerical result/temporary fit peak; borrowed structural/project context excluded. Report current/peak owned bytes separately. Native opaque identity helpers retain their fixed native construction bounds; this layer additionally charges emitted identity bytes. No universal application-memory claim.

## Shared CLI / viewer frontend

- `query --question similarity-features --projection NAME`: advertise native IDs/types/units for supplied scope. No values/fit claimed.
- `query --question similarity-map --projection NAME --feature ID[=WEIGHT] ...`: select finite advertised features, default weight 1. Shared qualifier parser uses native literal resolver; no expressions, inference or query grammar.
- Optional repeated `--entry ID`, default all shown. Optional inspected pair requires both `--from` and `--to` within selected rows. Existing typed selectors resolve pair; exact native IDs select entries.
- Retain status/relation/Relationships one-hop scope filters; no starts/avoid/lens/direction. Feature IDs carry direct/reached native endpoint direction. Default Relationships scope excludes `next`; `--relation all` opts in. Scalar/incidence scope never implies execution.
- Default JSON data fields `features,points,evidence`. Explicit field selection replaces defaults. `inputs` and `source` are opt-in large sections: typed cells/context node indices, exact native embedded source result. Source formatter is not duplicated. JSON/text/stdout/stderr/exit status follow existing query family conventions.
- Zero-work output uses null unvalidated feature descriptors/points, not fabricated defaults. Unknown/incomplete ranges stay null. Pair distances use separately bounded native call and remain available when validated inputs exist but MDS fit is unavailable.
- Shared result owns similarity child and destroys it before borrowed structural/query/project context. Candidate failure retains old result. Source construction truncation includes upstream Resource graph even when derived Effect graph finishes its retained prefix.
- Viewer adds native feature/entry list fields, exact input/missing inspection, paginated exhaustive set members and unplaced rows, point/pair selection, original coordinate/typed component display and explicit source navigation.
- Similarity canvas owns separate camera. Uniform display scaling only; never force layout, per-point displacement, focus offsets or synthetic missing hub. Ordinary pose/settling/camera/Find data stay separate. Active-map point source is linked native definition; no array-key reinterpretation.
- Source replacement rebuilds owned candidate, guards explicit entries and feature type/unit, and rejects loss of previously available fit. Pending fields block reload. Ad-hoc point inspection resets; explicit pair re-resolves from identities. Camera retained. Full event-loop/window/visual usefulness checks remain joint gate.

## Bounds and checks

Construction, text comparisons, cell validation, distance evaluation and fit use at most 67,108,864 charged work units. Fixed result/scratch arrays are bounded separately; no source snapshot/cell-storage ownership is hidden in kernel allocation. Two checked allocations: 90,384-byte result and 1,114,496-byte temporary fit scratch, 1,204,880-byte combined peak on current Windows/CJIT build. `gd_similarity_peak_owned_bytes` exposes compiler-specific combined size. Scratch released before return.

CJIT headless tests cover typed components/weights, missing-row exclusion/ranges, canonical reorder, narrow large-offset and opposite-sign extreme numeric ranges, tiny representable weighted distances, aborted distortion coverage, constant/coincident/empty/single-row cases, non-Euclidean diagnostics, exact/one-short/zero work, invalid identity/weights/capacities and all 4,096 entries through landmark fitting. Native feature fixture verifies mixed-kind Jaccard distance 0.5 with two selected challenges retaining shared ancestry/mechanic context, exact direct-kind direction, known empty sets, missing scalars, unknown-feature rejection, zero work and eight allocation-failure stages with zero tracked leaks. Independent review checked 424 direct/reached cells across all nine projections, typed numeric/integer/enum values and units, missing/hidden/empty selections, work/storage caps and all eight allocation failures. Found/fixed upstream Resource truncation leak for Effects; native capacity regression and independent recheck withhold map. Numerical precision/underflow/distortion fixes rechecked. Sixteen native output groups pass strict finite JSON, exact embedded source/cells/components/limits and immutable ownership. Parent CJIT native 123/123 plus CLI checks pass, including named discovery/map, weighted/context-preserving entry selection, continuation 0.5 versus 2/3, zero work, zero-weight missing exclusion and structured errors. Node/other compiler/window checks not run; CLI/viewer integration independent review active. Full compiler/Node/viewer checks remain deferred to joint user gate.
