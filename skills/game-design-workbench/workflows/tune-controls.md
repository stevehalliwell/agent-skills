# Tune control values

Export values, tweak them in-game, then deterministically update existing literal controls. Native C99 owns selection, types, units, formulas, constraints, dependency checks, and update planning. Public Attendant owns source replacement. No duplicate evaluator, native Markdown writer, old export baseline, or companion snapshot.

## Scope and permission

- Resolve actual control collection alias/directory through public Attendant `schema`; read its fields and usage. Live contract remains authoritative. Native tools never discover project collections.
- Use [native control contract](../native/README.md#control-values-and-exports) for supported grammar, selection, formats, and limits.
- Export/import accepted controls by default. Configure status/name filters for other tuning sets. Repeat the same selection at import. Name exclusions omit output keys, not required dependency reads.
- Import updates `value` only on existing literal controls. Preserve formulas, types, units, bounds, enum choices, statuses, bodies, and identities. Omitted controls retain their definitions; affected formulas recompute from changed inputs.
- A request to import/apply game tweaks authorizes scoped adoption unless project usage requires another gate. A request to preview alone does not. A ready native plan is not itself write permission.

## Export and preview

Example below uses PowerShell 7 and an already configured project. Resolve tool paths from loaded skills; use actual collection alias, not a guessed default.

```powershell
$project = (Resolve-Path '.').Path
$gd = '<workbench>/native/build/gd.exe'
$attendant = '<attendant-skill>/scripts/attendant.mjs'
$alias = 'control-vars' # Example; use actual project alias.
$schema = & node $attendant schema --project $project | ConvertFrom-Json
if ($LASTEXITCODE -ne 0) { throw 'Cannot inspect project schema' }
$collection = $schema.collections | Where-Object name -eq $alias
if (-not $collection) { throw 'Control collection not found' }
$collection.usage # Read usage and fields before operating.
$collection.fields
$mapping = "$alias=$($collection.directory)"
$selection = @('--status', 'accepted') # Or all/status/include/exclude combination.

& $gd controls export --control-vars $mapping @selection --as json
# Save successful raw export as UTF-8; tweak values in-game; save edited file.
$inputFile = (Resolve-Path './game-controls.json').Path
$arguments = @('controls', 'import', '--control-vars', $mapping) + $selection +
    @('--input', $inputFile, '--input-format', 'json', '--json')
$preview = & $gd @arguments | ConvertFrom-Json
if ($LASTEXITCODE -ne 0 -or -not $preview.ok -or -not $preview.data.ready) {
    throw 'Import failed; no source update authorized by this plan'
}
$preview.data.changes
$preview.data.derived
```

CSV/INI use the matching `--as` and `--input-format`. Preserve exported dialect, type/unit columns, and UTF-8. JSON/INI carry typed values but no unit metadata; interpret values in current declared units. CSV type/unit metadata must match current definitions. No implicit scale conversions.

Comparison uses a fresh current export, not historical export state. Unchanged literals are no-ops. Different literals become proposed updates. Stale game values cannot be distinguished from intentional changes. Derived values equal to current output are not direct tweaks; accept changed derived values only when they match the recomputed candidate. Otherwise fail the whole batch. Never guess upstream changes or replace formulas.

## Authorized adoption

Reinspect collection binding/contract and recompute immediately before adoption. Stop if source or planned writes changed after preview; obtain fresh direction when changes affect authorization. `source_digest` is an accidental-change indicator, not a security hash or lock. Public Attendant still validates its own schema and targets.

```powershell
$currentSchema = & node $attendant schema --project $project | ConvertFrom-Json
if ($LASTEXITCODE -ne 0) { throw 'Cannot recheck collection binding' }
$currentCollection = $currentSchema.collections | Where-Object name -eq $alias
$originalFields = ConvertTo-Json -InputObject $collection.fields -Depth 20 -Compress
$currentFields = ConvertTo-Json -InputObject $currentCollection.fields -Depth 20 -Compress
if (-not $currentCollection -or $currentCollection.directory -ne $collection.directory -or
    $currentCollection.usage -ne $collection.usage -or $currentFields -ne $originalFields) {
    throw 'Collection binding or contract changed; inspect again'
}
$fresh = & $gd @arguments | ConvertFrom-Json
if ($LASTEXITCODE -ne 0 -or -not $fresh.ok -or -not $fresh.data.ready) {
    throw 'Fresh import validation failed'
}
$approvedItems = ConvertTo-Json -InputObject @($preview.data.items) -Depth 20 -Compress
$currentItems = ConvertTo-Json -InputObject @($fresh.data.items) -Depth 20 -Compress
if ($fresh.data.source_digest -ne $preview.data.source_digest -or
    $currentItems -ne $approvedItems) { throw 'Source or planned writes changed; review fresh plan' }
if ($fresh.data.items.Count -gt 0) {
    & node $attendant update --project $project --collection $alias --items $currentItems
    if ($LASTEXITCODE -ne 0) { throw 'Attendant update failed; inspect actual source before retry' }
}
& $gd controls --control-vars $mapping @selection --json
```

Pass native `items` unchanged to public Attendant. LLM does not calculate, round, infer, or manually transcribe values. Empty item arrays need no update call. Verify resulting resolved values and untouched definitions; no game or simulation runs implicitly.

Validation failures expose no executable item subset. Candidate validation includes selected controls, required dependencies, and status-admitted declared dependents of changed literals, including dependents omitted from export. This prevents a narrow import silently violating an affected control's bounds or formula.

Attendant preflights the whole batch and atomically replaces each file, but offers no cross-file transaction. Another writer can race the final recheck. On application failure, inspect actual records, report any partial replacement, and stop. Never claim rollback, automatically retry, write Markdown directly, or repair unrelated source.
