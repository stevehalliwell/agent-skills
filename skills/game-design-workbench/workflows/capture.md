# Capture, alternatives, and revision

## Supplied facts and guided input

Capture supplied non-tentative facts faithfully; no interview or reconfirmation. For overall experience/player story, update bound concept owner in supplied voice and commit completed authorized change through concept workflow. For one MDA aesthetic, use aesthetics record. For investigations, use experiment stage and body headings; do not create separate hypothesis, expectation, or observation collections.

Examples:

- `Pushing costs one energy`: accepted fact; control variable owns amount, mechanic owns spending rule. Link relevant cases/resource rather than copying the cost everywhere.
- `Could crates also pull?`: exploratory proposal, not an accepted verb/rule. Surface prior pulling rationale before creating a genuinely distinct option.
- `Box means the existing crate`: resolve declared glossary mapping; an undeclared synonym needs a material identity decision, not a guessed merge.

No setup is implied by capture. If required storage is absent, use approved existing document or pause for storage/setup choice. Do not silently add collections.

## Alternatives and user-owned retention

1. Inspect relevant canonical facts and any retained rationale. Consult existing project-level decisions only when useful, following that collection's actual schema/usage. Done: available context is known; no second decision archive is created.
2. Generated options remain proposals, not accepted rules. Save them only when wanted, in relevant domain records or chosen existing project storage. Done: no compulsory record for every conversational alternative.
3. On supplied adoption, update canonical domain definitions. Selecting one option does not reject siblings unless user says so. Done: accepted source owns behavior.
4. Users may retain ignored options with statuses such as rejected, defunct, inactive, or disabled, or request deletion. Follow live schema; distribution status is free text, default proposed. Inspect affected references before requested deletion; do not silently remove extra records or reactivate ignored ones. Done: user controls retention.

## Record writes

Use loaded Attendant skill's record-command workflow. Before a write, run its `validate --no-correct` and inspect pending mechanical corrections. If unrelated source would be corrected by Attendant preparation, stop and get scope for those changes or use a precise source edit instead; do not hide collateral writes.

For a compatible existing collection, create/update one or a reviewed batch through runner. Supply only declared fields and built-ins. Example input file for an explicitly requested rank change:

```json
[{"name":"coordinate","fields":{"rank":3}}]
```

```sh
node <attendant-skill>/scripts/attendant.mjs update --collection <skill-atoms-alias> --items @rank-update.json --project <project>
node <attendant-skill>/scripts/attendant.mjs validate --no-correct --project <project>
```

Attendant preserves body and immutable identity metadata. It may reserialize frontmatter; inspect semantic diff and any unrelated byte changes. Batch writes are not a cross-file transaction. On failure, inspect actual state before retrying. Never overwrite concurrent source changes using a stale whole-file copy.

## Requested rank edit

- Edit selected authored `rank` only; preserve `id`, `name`, `created_at`, body, unrelated metadata, and all descendants' authored ranks.
- Explain remaining conflicts rather than repairing them. Derived display movement is not source mutation; native skill CLI/view computes declared prerequisite/rank findings.
- For tutorial coordinate 2 → 3, use exact update above. Compare selected record and every other source file before/after; recompute with native skill command or reopen clean graph. Findings concern declared structure, not actual learning.
- Native viewer can edit rank/prerequisite drafts and return a JSON proposal, not apply it. Read [draft handoff](../native/README.md#draft-editing-and-json-handoff). Check current record identity and values against report before authorized update; resolve stale differences explicitly, never replay a stale whole record. Supply only authorized edited fields. Agent chooses Attendant or precise source edit under guards above; native viewer has no authoritative writer.

## Definition changes

Glossary owns canonical spelling/aliases and chooses inline meaning **or** a canonical documentation target such as `nouns/crate`. Edit that one owner. Domain references stay `alias/name`; aliases do not change technical identity. Explicit renaming/moving between collections requires reviewed reference migration, not string replacement across guessed synonyms.

Run declared relationship queries and inspect exact source references to assess a change. Report unknown prose dependencies and unsupported transitive/game-state impact analysis. Validate changed source.
