# Typed axes and explicit applicability

Review 01 foundation. `gd rules --axes ALIAS=FOLDER --nouns ALIAS=FOLDER` reads typed definitions and resolves explicit participant-to-noun and noun-to-axis bindings. Supply interaction/mechanic source bindings too; see [rule reader](rules.md). No inferred applicability, state execution or runtime mutation.

User approved and Attendant registered `game-design/axes as axes` in both examples. Shared templates/live contracts now include typed definitions and optional noun applicability. Crate has 3 axis records; plumber has 8. Existing noun IDs/statuses/bodies retained; all 32 noun records now have a structured inventory or explicit null. This is an initial typed subset, not full rule/state migration.

Sections: [Source contract](#source-contract) · [Actor applicability](#actor-applicability) · [Headless facts](#headless-facts) · [Engineering and checks](#engineering-and-checks).

## Source contract

Axis is one Markdown record. Flat frontmatter lists known states or numeric constraints; body explains meaning and unknowns. No per-state files or serialized expression trees.

Named-state example, with ordinary generated identity metadata omitted here:

```markdown
---
name: Life
value_type: enum
allowed_values:
  - alive
  - dead
closed: false
---
Life status. Known states, not a claim about reachable combinations.
```

Integer quantity example:

```markdown
---
name: Amount
value_type: integer
unit: life
minimum: null
maximum: null
---
Count for an actor that explicitly declares this axis. No starting quantity inferred.
```

Numeric quantity example:

```markdown
---
name: ProtectionTime
value_type: number
unit: s
minimum: null
maximum: null
---
Protection duration. No clamp, decay rule or initial duration implied.
```

- `value_type`: `unspecified`, `enum`, `integer`, `number`. Omitted/null/empty type remains unspecified. Unsupported type is a sourced error, not coercion.
- `allowed_values`: up to 64 nonempty strings for enum axes. Keep duplicates and report informational finding; duplicate declaration is not another independent state. Empty/open vocabularies support incomplete designs. Explicit closed empty vocabulary remains an empty authored domain, not a parser error or reachability claim.
- `closed`: boolean for named-state vocabulary; omitted defaults open. Does not constrain combinations across independent axes. Numeric axes do not take `closed: true` or named values. Explicit null/quoted boolean is not silently converted.
- `unit`: shared numeric unit grammar from controls. Symbols remain opaque; no automatic conversion. Missing/empty unit remains unknown, not dimensionless. Explicit `1` means dimensionless.
- `minimum`, `maximum`: optional numeric literals in declared unit, inclusive constraints. Reuse shared finite-number and exact safe-integer checks. Fractional/unsafe integer bounds, malformed units and reversed bounds are sourced errors. No implicit correction, clamp or runtime rejection behavior. Omitted bounds do not establish a complete numeric domain.
- Enum axes do not take numeric units/bounds. Definitions do not own actor instances' initial/current values and are not global mutable controls.
- Same definition format may describe interaction-scoped context vocabulary. Loading such a definition does not add it to every actor; ordered context parsing remains pending.

## Actor applicability

Canonical actor remains a noun. Explicit `axes` references select applicable definitions:

```markdown
---
name: hero
axes:
  - axes/Life
  - axes/ProtectionTime
---
Actor intent and remaining unknowns.
```

- Omitted/null `axes`: unspecified applicability. Do not infer it from conditions, effects or prose.
- Explicit `axes: []`: empty currently declared inventory, distinguishable from omission/null. Not proof of complete design or future absence of other axes.
- Preserve reference order and duplicates. Repeating a reference does not create a second independent axis.
- References need explicit `--axes` and `--nouns` roles to resolve. Aliases need not match role names. Loading nouns through `--related` preserves text, not noun-role authority.
- Rule participant labels remain case-local bindings. Two labels can independently bind the same noun type. No binding guessed from spelling, type similarity or implicit inheritance.

Effect state names and numeric assignment targets now resolve through explicit applicability; see [effect bindings](rules.md#effect-bindings). `requires`, `skip_if`, numeric expressions, ordered context, preservation and rule invocation remain pending. State/quantity mentions never synthesize missing definitions.

## Headless facts

`--fields axes,actors,cases,findings` selects new facts while retaining coverage/errors.

- `axes`: identity/status/source metadata; raw declared type/unit/values; closed/default distinction and source lines; typed numeric bounds; `definition_valid`, `type_known`, `unit_known`.
- `actors`: identity/status/source metadata, `axes_declared`, `applicable_axes_complete`, and every axis reference with source line/resolution.
- `cases[].participants`: raw noun reference plus `noun_resolution` and `applicable_axes_complete`; distinct local identities remain intact.
- Reference resolution: `resolved`, `unloaded`, `missing`, `wrong_role`, `ambiguous`, `unavailable`, `unspecified`. Effect symbols additionally report `undetermined`, `wrong_type`, `invalid_definition`. A failed load makes negative loaded-alias claims unavailable rather than proving absence.
- `findings`: bounded informational gaps/repeated declarations, with source/path/line/reference. Missing information does not become false or a quality defect. Invalid typed metadata is instead a normal sourced operation error.

`coverage.complete` still concerns successful declaration reading/typed metadata checks, not gameplay semantics. Additional flags:

- `axis_definitions_complete`: axis collection supplied; each loaded axis has supported type and numeric units where needed; no load/typed-definition errors. Does not close open vocabularies or prove state-space completeness.
- `bindings_complete`: all supplied noun axis inventories explicitly declared; all authored noun/axis references resolve to expected roles; no load errors. Does not prove conditions/effects valid, reachable combinations or behavior consistency.
- `findings_omitted`: always retained, including narrow data selection.

Headless text reports typed-axis/actor/readiness counts and definitions. JSON retains structured detail. A completed scan with incomplete declarations can exit 0 with readiness flags false; malformed typed metadata exits 2. Neither result executes gameplay or authorizes repair.

## Engineering and checks

`gd_rule_axes.c` validates typed metadata using shared number/unit helpers and resolves immutable model references into the allocator-owned rule result. No separate formula evaluator, schema engine, source writer or allocation inside resolution. Bucket actor-axis edges once; retain authored order/duplicates. Work bounded by existing records/edges/participants and 64 values per axis. Findings capped at 128 with explicit omitted count.

Checks cover open/closed/empty/incomplete vocabularies, numeric types/units/bounds, exact negative zero, malformed metadata and boolean field duplication, same-type participants, omitted/null/empty applicability, all reference-resolution states, duplicate retention and bounded findings/value limits. CLI fixtures verify JSON/text, field selection, source lines and source immutability. No visual or gameplay acceptance implied.
