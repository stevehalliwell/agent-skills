# Rule declaration reader

Review 01 foundation: inspect approved body format through shared C99. Reader retains declarations and resolves explicit noun/axis applicability, not gameplay meaning. Condition/context/state-effect resolution, quantity evaluation, explicit invocation expansion and bounded consistency comparisons remain pending. See [typed axes and applicability](axes.md). Do not treat successful syntax inspection as completed task or migration.

## Run

From repository root, after an explicit staged headless build:

```powershell
./skills/game-design-workbench/native/build.ps1 -OutputDirectory ./skills/game-design-workbench/native/build/review01-rules
$example = 'skills/game-design-workbench/examples/little-plumber-guy-nes/game-design'
./skills/game-design-workbench/native/build/review01-rules/gd.exe rules `
  --interactions "interactions=$example/interactions" `
  --mechanics "mechanics=$example/mechanics" `
  --nouns "nouns=$example/nouns" --control-vars "control-vars=$example/control-vars" `
  --json --fields records,cases
```

Require `--interactions`, `--mechanics`, or both. Each source option accepts one ordinary `ALIAS=FOLDER` binding. Optional `--axes`, `--nouns`, `--verbs`, `--control-vars` and repeatable `--related` load metadata/context. Only explicit interaction/mechanic sources contribute rule blocks. No inferred paths, registry, display, Node, agent or disposable projection needed by executable.

JSON on piped/agent stdout; `--format text` overrides. `--fields records,cases,axes,actors,findings` selects data sections, not coverage/errors. Stdout contains result data; stderr contains errors. Exit 0 means syntax scan completed, including empty/unstructured designs. Exit 2 means arguments, loading, parsing, bounds, allocation or output failed. No writes or implicit repair.

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

## Results and provenance

`data.records` retains every supplied rule source: `record`, `record_id`, `name`, `description`, `status`, authored `coverage`, `source_path`, fence `line`, `case_count`, frontmatter `relationships`, and `rule_block` state:

- `parsed`: at least one case read.
- `empty`: rule fence exists, no cases authored.
- `absent`: no authoritative rule block; prose is not silently converted.
- `invalid`: malformed rule declaration; none of that record's cases returned as valid partial data.

Frontmatter relationships retain declarations, duplicates, source lines and loaded/ambiguous flags from existing loader. Participant noun bindings now resolve against explicit noun roles; actor-axis references resolve against explicit axis roles. Other body reference text remains unresolved at this stage.

`data.cases` retains record identity/status/coverage/path, local case name and line, raw context and line, ordered participants, field lines and all numeric assignments with lines. Source line numbers address original Markdown, including frontmatter and CRLF files. Same-type participants remain distinct local bindings.

`coverage.complete` concerns completed declaration/type checks only. Counts expose all source records versus records with cases. Scope text explicitly excludes semantic evaluation, equivalence, consistency and gameplay claims. `axes`, `actors`, participant binding flags and informational `findings` expose typed declarations and explicit applicability; [axis contract](axes.md) defines readiness limits. Missing blocks are not proof of missing gameplay behavior. Errors retain code, message, path and line; failed records do not discard other successfully parsed records.

Current plumber example: 73 interaction/mechanic records; only accepted authoring specimen has a structured block. Other 72 remain visibly absent. Full migration is still required before task completion.

## Engineering bounds and checks

`gd_rules.c` owns allocator-backed result and copied declaration text; `gd_rule_axes.c` resolves explicit noun/axis declarations in that result. Model/provenance borrowed immutably until result destroyed. No recursion, source mutation, formula evaluator duplication or runtime code execution. Existing file/record/retained-model limits remain in force.

Operation limits: 4,096 cases, 8,192 participants, 8,192 assignments, 4 MiB copied declaration text, 8,192 bytes per rule line. Reject overflow; do not truncate or return a half-parsed record. Parsing scans bounded source text; local identity checks compare within bounded case/participant sets.

Native checks cover approved shape, unknown versus explicit ANY/preservation, same-type local bindings, repeated assignments, source lines, CRLF/fence handling, malformed records, whole-record rollback, capacity limits and allocation failure. `test/native-rules.test.mjs` exercises shipped specimen, metadata/links, field selection, text/JSON, empty/partial/error outputs, explicit scope and source immutability. These checks do not establish visual UX or semantic acceptance.
