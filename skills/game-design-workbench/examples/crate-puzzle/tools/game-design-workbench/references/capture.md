# Capture, alternatives, and revision

## Supplied facts and guided input

Read live schema/usage and [model ownership](model.md). Supplied non-tentative facts authorize faithful capture; do not require an interview or reconfirm an explicit choice. Mark consequential unresolved details unknown rather than supplying them. If requested, guide from intended experience to behavior, then rules/actions; this sequence is optional.

Examples:

- `Pushing costs one energy`: accepted fact; control owns amount, mechanic owns spending rule. Link relevant cases/resource rather than copying the cost everywhere.
- `Could crates also pull?`: exploratory proposal, not an accepted verb/rule. Surface prior pulling rationale before creating a genuinely distinct option.
- `Box means the existing crate`: resolve declared glossary mapping; an undeclared synonym needs a material identity decision, not a guessed merge.

No setup is implied by capture. If required storage is absent, use approved existing document or pause for storage/setup choice. Do not silently add collections.

## Retained alternatives

1. Inspect relevant decisions and canonical facts, including inactive options and rationale. Done: new proposals do not silently repeat known rejected ideas.
2. Retain every generated alternative as a separate `decisions` record with `record_kind: proposal`, `status: proposed`, common `alternative_group`, subjects, and useful rationale. Preserve source context; do not invent acceptance checks. Done: alternatives survive beyond chat.
3. On supplied adoption, write adopted facts in their canonical domain records; update decision disposition/rationale and links. Choosing one does not reject siblings. Done: accepted source owns behavior; decision record retains why it was chosen.
4. Rejection/supersession/defunct status retains record and rationale. Do not delete useful history or silently revive inactive definitions. Done: later exploration can find previous reasoning.

## Record writes

Use bundled [Attendant record commands](../vendor/attendant/references/commands.md), not generated SQLite edits. Before a write, run its `validate --no-correct` and inspect pending mechanical corrections. If unrelated source would be corrected by Attendant preparation, stop and get scope for those changes or use a precise source edit instead; do not hide collateral writes.

For a compatible existing collection, create/update one or a reviewed batch through runner. Supply only declared fields and built-ins. Example input file for an explicitly requested rank change:

```json
[{"name":"coordinate","fields":{"rank":3}}]
```

```sh
node <workbench>/vendor/attendant/scripts/attendant.mjs update --collection <skills-alias> --items @rank-update.json --project <project>
node <workbench>/scripts/workbench.mjs validate --project <project> --json
```

Attendant preserves body and immutable identity metadata. It may reserialize frontmatter; inspect semantic diff and any unrelated byte changes. Batch writes are not a cross-file transaction. On failure, inspect actual state before retrying. Never overwrite concurrent source changes using a stale whole-file copy.

## Requested rank edit

- Edit selected authored `rank` only; preserve `id`, `name`, `created_at`, body, unrelated metadata, and all descendants' authored ranks.
- Rank accepts null or JSON-safe signed integer in intended model contract. Zero and negative ranks are valid. No automatic prerequisite-depth conversion.
- Explain remaining conflicts rather than repairing them. Derived display movement is not source mutation; native display/resolver is deferred.
- For tutorial coordinate 2 → 3, use exact update above. Compare selected record and every other source file before/after; query skills again. Do not claim prerequisite conflict detection is automated in this release.

## Definition changes

Glossary owns canonical spelling/aliases and chooses inline meaning **or** a canonical documentation target such as `nouns/crate`. Edit that one owner. Domain references stay `alias/name`; aliases do not change technical identity. Explicit renaming/moving between collections requires reviewed reference migration, not string replacement across guessed synonyms.

Run declared relationship queries and inspect exact source references to assess a change. Report unknown prose dependencies and unsupported transitive/game-state impact analysis. Validate changed source; do not commit unless separately authorized.
