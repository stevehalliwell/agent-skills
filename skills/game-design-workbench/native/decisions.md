# Contextual decision assessments — CLI contract

Read-only authored assessment query. No automatic classification, rule evaluation, player model, viewer control or quality score. Use [shared vocabulary and intent checks](../references/decision-structure.md) to discuss whether a demand serves intended experience.

- [Source](#source)
- [Query](#query)
- [Results and matching](#results-and-matching)
- [Failure and bounds](#failure-and-bounds)
- [Verification](#verification)

## Source

Challenge owns task/context and repeatable fenced YAML `decision-assessment` blocks. Keep explanatory prose alongside blocks. Frontmatter remains flat block YAML; no new collection or nested frontmatter.

One block describes one situation. `id` is a technical name local to exact Challenge identity; it is not a global runtime identity. Example from first Walker assessment:

````markdown
```decision-assessment
id: walker-survival-small
task: Get past Walker alive.
context: Small, unprotected Hero; harmful side contact is lethal, while bypass and stomp are alternatives.
evidence_status: authored-assessment
rule_cases:
  - "interactions/touch-walker-small#lethal-side"
demands:
  - kind: knowledge
    intent: undecided
  - kind: execution
    intent: undecided
```
````

Fields:
- Required `id`, `task`, `demands`. Empty demands sequence means no demand entries classified, not no gameplay demand.
- `id`/`kind` technical grammar: `[A-Za-z0-9][A-Za-z0-9_-]{0,127}`. Each demand requires `kind`, matched exactly. Labels are open authored vocabulary, not exclusive or automatically inferred. Prefer canonical terms; do not merge synonyms silently.
- Optional demand `intent`: wanted, unwanted or undecided. Omission means undecided. Explicit null or other values fail. This records designer intent, not whether player enjoys or succeeds at demand.
- Optional `context` and `evidence_status`: nonempty text or null. Omission/null stays unknown. Evidence status is a declared claim; query does not verify observations or causality.
- Optional `rule_cases`: sequence of authored reference strings. Omission/empty remains no declared references. Query returns strings without proving resolution, invocation or applicability.
- Unknown/duplicate fields, duplicate kinds within one assessment and duplicate assessment IDs within one Challenge fail. Same local ID in different Challenges remains distinct.

Recognize backtick/tilde fences with at most three leading spaces and matching closing marker of at least opening width. Track other code fences too, so quoted examples do not become assessments. Close every assessment fence. Parse one YAML document without aliases, anchors or explicit tags. Text fields do not coerce numbers/booleans; quote text that could be read as another scalar type.

Assessment labels can describe current/proposed mechanical demands without a playtest. Whether players find them difficult, enjoyable or meaningful remains a separate evidence question. Game judgment, intended-but-not-present demands and numeric timing filters are not structured by this first query.

## Query

```sh
gd query --list --json
gd query --question decision-assessments --projection challenges \
  --challenges challenges=PATH/TO/challenges \
  --demand dexterity --intent undecided --json
```

Require exactly one explicit Challenge collection binding. Dedicated `gd_load_challenges` shares source scan/identity/remap code; legacy `gd_load`, graph and learning-query role requirements stay unchanged. Optional `--demand` and `--intent` each accept one value; repeats fail. Do not accept unrelated rule, graph, scenario, control or display parameters. Other query families reject these selectors. All supplied Challenge statuses remain included and labelled; no hidden accepted-only filter.

JSON/text selection follows normal CLI conventions: noninteractive/GD_AGENT/CI defaults JSON; `--format` overrides. stdout is result data; stderr is diagnostics. `--fields summary,assessments` selects data members only and requires JSON; parameters, coverage and errors remain.

## Results and matching

Filter demand and intent on same entry in same assessment. Never combine labels across entries or situations.

- No selectors: return every authored assessment, including empty demand sequences.
- Demand only: return assessments containing that exact kind.
- Intent only: return assessments with any entry declaring/defaulting to that intent.
- Both: same entry must match both.

Return each assessment once. Row retains canonical record reference/ID/status, local assessment ID, task, context, declared evidence status, rule references, all demand/intent entries, per-entry `matched`, source path and lines. Mark which entries matched rather than discarding surrounding context.

`data.summary` counts source records, retained assessments/demand entries and matched assessments/demand entries. These are not interaction executions, gameplay frequency, difficulty or quality. `data.assessments` contains matching rows. Envelope retains question/projection, requested parameters, read-only coverage, fixed limits, errors and breadcrumbs.

Empty successful result means no retained explicit assessment matches; it does not establish absence of a demand in gameplay. Prose-only Challenges have no typed rows. Current pilot declares no reaction/dexterity entries, so those filters legitimately return empty results; do not manufacture example classifications to populate query.

## Failure and bounds

Exit 0: successful query, including empty results. Exit 2: arguments, allocation, loading, malformed data or exhausted bounds. Failures set `ok:false`, `coverage.complete:false`; emit no assessment rows or success-shaped counts. Summary is null when selected. No partial inventory masquerades as complete. Sources never change.

Fixed operation bounds:
- 4,096 assessments total; 16 per Challenge.
- 16 demands and 16 rule references per assessment; 32,768 aggregate entries for each.
- 64 KiB per YAML block; 2,048 YAML events; nesting depth 8.
- 4 MiB retained assessment text, plus existing source-loader limits.
- Technical names at most 128 bytes; other scalar text at most 4,095 bytes.

Result/text storage uses caller allocator on heap and borrows immutable model identities until destruction. Vendored libyaml parses events; bounded iterative construction uses its existing document primitives. No recursion, new dependency, unlimited document loader or source reread during assessment parsing. Libyaml's own bounded-input allocations remain outside caller allocator, as with existing metadata parsing.

Viewer/IPC integration is unavailable and advertised as such. No viewer launch, navigation, selection, analysis Run, draft or camera change follows a CLI query.

## Verification

```sh
GD_NATIVE_EXE=PATH/TO/FRESH/gd.exe node --test \
  skills/game-design-workbench/test/native-decisions.test.mjs \
  skills/game-design-workbench/test/native-queries.test.mjs
```

Default checks cover contextual binding, unknown intent, evidence/reference retention, empty results, JSON/text/field output, nested/CRLF fences, malformed/duplicate/alias/tag data, local limits and source-byte invariance. Add explicit `GD_DEEP_CHECKS=1` for aggregate-capacity and 4 MiB text-exhaustion probes. Synthetic dexterity/intent fixtures stay separate from authored examples. Existing source consumers and native core need regression checks; this is not GUI, restart or platform acceptance.
