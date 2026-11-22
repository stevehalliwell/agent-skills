# Control values and exports

Shared C99 resolver, export set selection, and read-only import planning are implemented. [Tuning workflow](../workflows/tune-controls.md) applies native-validated values through public Attendant; no new source writer, lifecycle wrapper, or JavaScript evaluator. Inspection defaults to all statuses; exports default to accepted controls. Explicit filters can select drafts without changing their status. Export does not itself adopt design decisions.

```powershell
$controls = @('--control-vars', 'control-vars=./examples/crate-puzzle/game-design/control-vars')
./native/build/gd.exe controls @controls --json
./native/build/gd.exe controls @controls --from control-vars/push-duration --set control-vars/walk-speed=4 --json
./native/build/gd.exe controls export @controls --as json
./native/build/gd.exe controls export @controls --as csv
./native/build/gd.exe controls export @controls --as ini
./native/build/gd.exe controls export @controls --status all --exclude push-cost --as json
./native/build/gd.exe controls export @controls --include walk-duration --include push-duration --as json
./native/build/gd.exe controls export @controls --status accepted --status proposed --exclude push-cost --as csv
./native/build/gd.exe controls import @controls --input ./game-tweaks.json --json
./native/build/gd.exe controls import @controls --input ./game-tweaks.csv --input-format csv --fields items
```

Run from skill folder. No collection defaults. Optional `--related` loads context, not additional control definitions. `--from` selects one control and required dependencies. Unrelated unspecified controls do not block that focused resolution. `--fields controls` keeps inspection envelope/errors. Explicit `--format text` selects readable inspection.

- Selection: repeat `--status STATUS`, `--include NAME`, and `--exclude NAME`; `--status all` must stand alone among status filters. Names may be bare stable keys or qualified references. Exclusions win. Explicit includes or `--from` bypass the implicit accepted-only export default unless an explicit status filter is supplied; explicit status filters intersect included names. Unknown/ambiguous names fail. At most 64 entries per filter kind, subject to CLI argument bound.
- Name filters select exported keys, not dependency availability. A named derived export may read upstream controls without exporting them. In status-filtered/accepted-default mode, dependencies must also satisfy status policy; report an excluded-status dependency rather than silently promoting it. Inspection retains dependency records and marks output roots with `requested`.
- Types: `integer`, `number` (binary64), `boolean`, `enum`. Preserve `0`, `false`, empty allowed enum strings, and exact allowed-value spelling. No string/numeric/boolean coercion.
- Exactly one value authority: typed `value`, or nonempty `expression` with `value: null`. Unknown/unspecified values cannot produce resolved exports.
- Numeric controls require explicit unit text; quoted `"1"` means dimensionless. Literal values/overrides carry declared units. Formula constants are dimensionless unless annotated, e.g. `0.2[s]`.
- Formula grammar: decimal/scientific constants, `{ALIAS/NAME}`, unary `+ -`, binary `+ - * /`, parentheses; normal precedence. Functions: `min(a,b)`, `max(a,b)`, `clamp(value,lower,upper)`, `abs`, `floor`, `ceil`, `round`. Round ties away from zero. Dollar-prefixed references are rejected with `controls.reference_syntax`; use braces without `$`. No assignments, comparisons, arbitrary calls, or code evaluation.
- Unit grammar: case-sensitive ASCII symbols, `*`, `/`, signed integer powers. `m/s` equals `m*s^-1`; `ms` is not `s`. No implicit scale registry/conversions. Explicit conversion example: `{config/duration-ms} * 0.001[s/ms]`. Unit parentheses are not supported.
- Addition/subtraction/min/max/clamp require equal canonical units; multiplication/division combine units. Rounding/absolute value preserve units. Bounds apply in declared units. Invalid bounds, zero division, nonfinite results, and underflow to zero fail. Finite nonzero subnormal values are supported.
- Integers stay within `[-9007199254740991,9007199254740991]`. Literal values and final integer-consumer results must be integral; integral decimal `1.0` is valid. Genuine fractional formula operands/intermediates are supported, but decimal text that rounds to an integer during parsing is rejected. No implicit truncation. Explicit rounding functions may produce integers. Integer formulas also reject intermediate arithmetic outside safe range. Arithmetic uses binary64, not arbitrary-precision rational math.
- `--set ALIAS/NAME=VALUE` is temporary. Numeric text and lowercase booleans are parsed strictly; enum values are raw argument text, not JSON-quoted strings. Override uses existing type/unit/constraints; never modifies baseline or formula. Overriding a formula temporarily replaces its computation, so unused original dependencies are not required.
- Missing/ambiguous/non-control references, dependency cycles, invalid operands, and limit/allocation failures have structured source-backed errors. Cycle diagnostics distinguish unresolved dependency regions without claiming every blocked node belongs to the cycle itself.

Exports use the same result, sorted by stable control name. JSON is an object with typed scalar values. CSV has `key,type,value,unit` columns with conventional quoting; formula-like enum text stays unchanged and warns on stderr. Never auto-open CSV. INI dialect has one `[controls]` section, numeric/boolean literals, and JSON-quoted/escaped enum strings. These are resolved artifacts, not formula definitions. Export stdout is raw selected format; `--json`, `--format`, and `--fields` do not apply to exports. Failed resolution returns exit 2 and JSON error envelope, never partial CSV/INI/game JSON.

`gd_controls.c` owns allocator-backed results borrowing immutable model metadata and override enum strings. Bounded shunting-yard compilation and iterative dependency resolution avoid recursion/code execution. Limits: 4,096 bytes and 256 bytecode tokens per formula, 128 parser/evaluation stack entries, 32,768 total tokens; 256-byte units, 16 canonical terms, 32-byte symbols, exponent magnitude 32; 64 enum choices per control. CLI accepts at most 64 overrides, also subject to existing 128-argument Windows bound. Errors never silently truncate. Usual record/file/text limits still apply.

`controls import` reads an explicit UTF-8 file (at most 1 MiB); `--input-format json|csv|ini` defaults JSON. Same status/name filters choose admitted keys. `--fields changes,derived,items` selects large sections while preserving ready/applied flags, digest, coverage, and errors. No `--set` during import. Return a read-only update plan, never source edits. All invalid batches have `ready:false` and empty `items`; successful no-op batches have an empty item array and need no Attendant update.

JSON supports a flat object of typed scalar values, full JSON escapes/surrogate pairs, and no duplicate keys, nulls, arrays, or nested objects. CSV requires the exact four-column export header; quoted multiline enum text is supported. INI accepts one `[controls]` section, blank lines, whole-line `;`/`#` comments, and the documented literal/quoted-string dialect, not arbitrary INI. Unknown/out-of-scope keys, duplicate keys, changed CSV types/units, invalid enum choices, unsafe integers, and constraint failures block the batch.

Compare edited values with a fresh current export, without old baseline snapshots. Update literals only. Preserve omitted controls and all definition metadata. Unchanged derived inputs are not direct tweaks; a changed derived input must match recomputed candidate output or the whole batch fails. Never replace a formula or infer upstream changes. Validate status-admitted declared dependents of changed literals even when excluded from export, so narrow imports cannot silently break affected formulas/bounds. Reverse dependency scanning is bounded at 32,768 known references.

`gd_control_import.c` owns bounded flat-format parsing, typed merge planning, and affected-scope validation; it borrows immutable model metadata and owns current/candidate results plus decoded input text. It never adds another authority. Recheck source and planned values before authorized public-Attendant application. Public Attendant replacements are individually atomic, not a cross-file transaction; failures can leave partial replacement and must not trigger silent retry/rollback.
