# Rule declaration reader

Review 01 foundation: inspect approved body format through shared C99. Reader retains declarations and resolves explicit noun/axis applicability, not gameplay meaning. Effect states and quantity targets now bind through explicit actor axes. [State conditions](conditions.md) now compile to target-scoped Boolean programs with unknown truth preserved. Numeric comparisons/context, resulting quantity values, preservation, explicit invocation expansion and bounded consistency comparisons remain pending. See [typed axes and applicability](axes.md). Do not treat successful syntax inspection as completed task or migration.

## Run

From repository root, after an explicit staged headless build:

```powershell
./skills/game-design-workbench/native/build.ps1 -OutputDirectory ./skills/game-design-workbench/native/build/review01-rules
$example = 'skills/game-design-workbench/examples/little-plumber-guy-nes/game-design'
./skills/game-design-workbench/native/build/review01-rules/gd.exe rules `
  --interactions "interactions=$example/interactions" `
  --mechanics "mechanics=$example/mechanics" `
  --nouns "nouns=$example/nouns" --axes "axes=$example/axes" --control-vars "control-vars=$example/control-vars" `
  --json --fields records,cases
```

Require `--interactions`, `--mechanics`, or both. Each source option accepts one ordinary `ALIAS=FOLDER` binding. Optional `--axes`, `--nouns`, `--verbs`, `--control-vars` and repeatable `--related` load metadata/context. Only explicit interaction/mechanic sources contribute rule blocks. No inferred paths, registry, display, Node, agent or disposable projection needed by executable.

JSON on piped/agent stdout; `--format text` overrides. `--fields records,cases,axes,actors,findings` selects data sections, not coverage/errors. Stdout contains result data; stderr contains errors. Exit 0 means declaration/type/name checks completed without errors, including empty/unstructured designs; unresolved open information can still remain. Exit 2 means arguments, loading, parsing, typing, effect binding, bounds, allocation or output failed. No writes or implicit repair.

## Body format

One labelled `gd-rule` code block per record. Multiple named cases, unordered semantically; source order retained for inspection. Existing flat frontmatter keeps identity, status and relationships. Prose stays outside block.

```gd-rule
case powered-side-contact:
    context: Player sideContacts Walker

    Player:
        noun: nouns/hero
        requires: alive AND (tall OR spark) AND NOT protected
        other_axes: ANY
        becomes: small, protected
        set:
            ProtectionTime: {control-vars/safety-window}
        preserve: OTHER

    Walker:
        noun: nouns/walker
        requires: alive
        other_axes: ANY
```

- Domain language, not YAML. Bare `{control-vars/safety-window}` remains an expression, not a flow mapping. No quoting/conversion needed.
- Case header at column 0. Four spaces for case context/participant labels; eight for participant fields; twelve for `set` entries. Blank lines and whole-line `#` comments allowed. Inline `#` remains part of raw value, not a stripped comment.
- Case names and local participant labels: ASCII technical names, at most 128 bytes, matching existing record-name convention. Different local labels can bind same noun type. Duplicate label within a case or duplicate case name within a record is an error. Same case name in different records remains separate.
- Participant fields currently read: `noun`, `requires`, `skip_if`, `other_axes`, `becomes`, `preserve`, `causes`, `set`. Scalar fields take nonempty raw text. Omit field for unspecified information. Empty field is not `ANY`.
- `set:` takes raw `AXIS: EXPRESSION` entries. Preserve every declaration, including repeated axis assignments; never overwrite by file order. Numeric expressions and target-scoped `this` are retained but not yet evaluated here.
- Missing context/conditions/effects stay null. Do not infer unrestricted scope or preservation. Explicit `ANY`, `ALL`, `OTHER` and ordered contact text remain distinguishable from omission, without claiming semantic resolution.
- Empty cases/participants and empty rule blocks can retain incomplete design. Reader does not invent nouns, axes, rules or effects to fill them.
- Unknown fields fail rather than disappear. Explicit multi-participant invocation/binding grammar and semantic checks remain pending; `causes` currently retains only authored reference text.
- Backtick/tilde Markdown fences accepted, with at most three leading spaces. Track other fenced examples so a quoted `gd-rule` block is not mistaken for authority. Multiple authoritative blocks or an unclosed rule fence fail.

## Effect bindings

- `becomes` is a comma-separated list of exact, case-sensitive state names. Bare names resolve when unique among the actor's declared enum axes. `AXIS.STATE` selects an axis by technical name; optional `ALIAS/AXIS.STATE` identifies a loaded definition explicitly, including when other axis references remain unloaded. No inferred applicability, definitions or actor instances.
- Repeated noun-axis references and duplicate value declarations do not create extra matches. Repeated effects/assignments remain separate declarations; no deduplication or last-write-wins.
- `set` target names resolve by axis technical name or canonical reference through that participant's noun inventory. Target must be integer/number; binding does not evaluate expressions or check their RHS units, resulting values or bounds.
- Multiple known matches report `ambiguous`; absent names in the declared inventory/closed vocabulary report `missing`; incompatible types/invalid definitions report their own resolution. These produce sourced errors while retaining raw declarations.
- Unlisted values in open vocabularies, unknown types/applicability and unavailable competing definitions remain `undetermined`, with informational findings. A known bare match cannot exclude an unseen competing axis. Explicit canonical qualification can select a loaded axis despite unrelated unloaded axes.
- `cases[].participants[].state_effects` retains every token's `text`, source `line`, and `target` (`resolution`, canonical `axis`, canonical `state`). Numeric assignments retain raw `axis`/`expression` and add `target`. Omitted `becomes` has no derived declarations, not an inferred unchanged result.
- `coverage.effect_bindings_complete` covers authored targets in retained cases only, not conditions, consistency, required-but-absent effects or complete migration. `state_effect_count` counts declarations, not distinct states or independent evidence. No contradictory-write, preservation or composed-result check yet.
- Limit: 8,192 state declarations and 16,777,216 charged comparison/traversal work units across effect lookups. `symbol_work` and limits remain in coverage. Invalid/overflowing comma lists return no partial list; later lists remain unprocessed after capacity failure. Work exhaustion retains earlier bindings and marks remaining lookups `unavailable`, never false or missing.

## Results and provenance

`data.records` retains every supplied rule source: `record`, `record_id`, `name`, `description`, `status`, authored `coverage`, `source_path`, fence `line`, `case_count`, frontmatter `relationships`, and `rule_block` state:

- `parsed`: at least one case read.
- `empty`: rule fence exists, no cases authored.
- `absent`: no authoritative rule block; prose is not silently converted.
- `invalid`: malformed rule declaration; none of that record's cases returned as valid partial data.

Frontmatter relationships retain declarations, duplicates, source lines and loaded/ambiguous flags from existing loader. Participant noun bindings now resolve against explicit noun roles; actor-axis references resolve against explicit axis roles. Effect bindings and state predicates resolve through those declarations; numeric predicates, ordered context and other body reference text remain unresolved. A structurally parsed block can still have effect-binding errors; raw cases stay visible, not semantically accepted.

`data.cases` retains record identity/status/coverage/path, local case name and line, raw context and line, ordered participants, field lines and all numeric assignments with lines. Source line numbers address original Markdown, including frontmatter and CRLF files. Same-type participants remain distinct local bindings.

`coverage.complete` concerns completed declaration/type/name checks only. Counts expose all source records versus records with cases. Scope text distinguishes predicate logic from full applicability, equivalence, consistency and gameplay claims. `axes`, `actors`, participant binding flags and informational `findings` expose typed declarations and explicit applicability; [axis contract](axes.md) defines readiness limits. Missing blocks are not proof of missing gameplay behavior. Errors retain code, message, path and line; failed records do not discard other successfully parsed records.

Current plumber example: 73 interaction/mechanic records; only accepted authoring specimen has a structured block. Other 72 remain visibly absent. Full migration is still required before task completion.

## Engineering bounds and checks

`gd_rules.c` owns allocator-backed result and copied declaration text; `gd_rule_axes.c` resolves explicit noun/axis declarations; `gd_rule_logic.c` shares bounded state lookup across predicate compilation and effect bindings, without allocation. [Condition contract](conditions.md) defines grammar, derived facts, read-only truth and limits. Effect spans borrow retained declaration text; resolved values borrow immutable model text. Model/provenance borrowed immutably until result destroyed. No recursion, source mutation, formula evaluator duplication, arbitrary code execution or gameplay runtime. Existing file/record/retained-model limits remain in force.

Operation limits: 4,096 cases, 8,192 participants, 8,192 assignments, 4 MiB copied declaration text, 8,192 bytes per rule line. Reject overflow; do not truncate or return a half-parsed record. Parsing scans bounded source text; local identity checks compare within bounded case/participant sets.

Native checks cover approved shape, unknown versus explicit ANY/preservation, same-type local bindings, repeated assignments, source lines, CRLF/fence handling, malformed records, whole-record rollback, capacity limits and allocation failure. `test/native-rules.test.mjs` exercises shipped specimen, metadata/links, field selection, text/JSON, empty/partial/error outputs, explicit scope and source immutability. These checks do not establish visual UX or semantic acceptance.
