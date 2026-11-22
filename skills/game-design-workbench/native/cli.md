# Native CLI quickstart

[Build](build.md) first. Commands below use `gd` for the built executable; substitute its path if it is not on `PATH`. Use the CLI on its own or with [Explorer](explorer.md).

## Supply source folders

```sh
gd --help
gd query --list --json
gd matrix --nouns 'nouns=game-design/nouns' --verbs 'verbs=game-design/verbs' \
  --interactions 'interactions=game-design/interactions' --json
```

Each binding is `--ROLE ALIAS=FOLDER`. Use the project's actual role paths and aliases so `ALIAS/NAME` references resolve. Native tools do not look up `.pi/attendant.tables`, assume defaults, or interpret schemas. Supply repeatable `--related ALIAS=FOLDER` for relevant linked records. Only immediate, non-hidden `.md` files are read. Record names, not UUIDs, resolve references; queries may also expose opaque typed IDs for derived nodes. Use exact selectors from the same loaded scope.

## Read results

- `--json` or `--format json|text` selects format. Pipes, `GD_AGENT=1`, and nonempty `CI` default to JSON. `--fields` selects supported data sections, not coverage and errors. For named questions, check the catalogue for supported projections, bindings, options, and fields.
- Exit `0` means the operation completed, including empty results or design findings. Exit `2` means argument, load, runtime, or output failure. When available, failure JSON carries `ok:false`, errors, and partial data. Diagnostics go to stderr.
- Check source completeness, query/work limits, and evidence omissions separately. `coverage.complete` does not certify schema validity or full knowledge of the game. Unloaded aliases are not confirmed missing records. No native result authorizes a repair or source change.

## Interaction counts

`matrix` groups cases by noun and verb, including different dispositions and multiple cases per pair. `connectivity` counts distinct active partners, not cases. Active means accepted and `coverage: defined`; inactive pairs do not merge active components. Context nouns are not additional verb targets. The Noun interactions viewer instead counts distinct cases with explicit co-participants, including a same-noun pair only for two distinct bindings. Neither measure is frequency, directed flow, or a count of feasible interactions.

## Structural questions

```sh
gd query --question reachability --projection skills \
  --skill-atoms 'skill-atoms=game-design/skill-atoms' \
  --from 'skill-atoms/move' --direction forward --json
```

Replace the root with an actual reference. Use `gd query --list --json` for supported query/projection combinations and selectors.

- **Counts and comparison:** `direct-connections` distinguishes partner counts from link rows. `related-structure` compares shared and unique neighbors; `--lens same-role|changed-role` concerns incident relation kind/direction, not gameplay roles. `role-shapes` groups structural signatures, not semantic equivalents. `adjacency` summarizes retained projected links.
- **Paths:** `reachability` and `related-paths` trace selected relation kinds. Path queries use `--from` or repeated `--start`; check `--to` requirements per question. Structural `forward` follows projected links; `backward` reverses traversal, not the stored links. Skills points prerequisite to dependent. Relationships follows authored endpoints and excludes `next` from default path scope; include it explicitly when tracing later challenges. Matrix and Resources use undirected incidence, never flow.
- **Necessity:** `structural-dominators` identifies mandatory nodes for a target within the selected graph and start scope. Inspect `dominates_target`, not highlight membership. `support-bypass` tests alternatives for eligible support-path candidates; unavailable evidence is not proof that no avoidance path exists. Check witness truncation before citing a full path.
- **Gaps:** `declaration-gaps` inventories consumed references across the loaded reader context. Check `declaration_gap_inventory` under `findings` for unresolved identities, fields, and source lines. Resolved identity can still have a wrong-role or type finding. The report does not inventory absent optional fields or prove the design complete.
- **Similarity:** discover feature IDs with `similarity-features`, then supply explicit `--feature ID[=WEIGHT]` options to `similarity-map`. See [similarity](similarity.md) for scope, component evidence, and limits.

## MDA traces

```sh
gd mda --mechanics 'mechanics=game-design/mechanics' \
  --dynamics 'dynamics=game-design/dynamics' --aesthetics 'aesthetics=game-design/aesthetics' \
  --from 'mechanics/example' --direction forward --json
```

Replace the root; omit it for the unrooted overview. Optional `--systems` adds systems without requiring them as bridges. `forward` in `mda` traces dependents toward aesthetics; `backward` follows authored basis; `both` follows connected associations. On those same authored MDA links, structural `reachability --direction backward` follows toward dependents. Check command and start scope before comparing directions.

Inspect `nodes`, `links`, `paths`, `gaps`, and `findings`. A shortest retained witness is one association chain, not the whole explanation. Keep source status, raw links, unresolved boundaries, and any truncated path prefix visible. Distinguish design intent from observed dynamics and experience. See [MDA framework](../frameworks/mda.md).

## Other commands

[Actors, effects, and states](actor-effects.md) · [Rule declarations](rules.md) · [Named rule and resource delegates](named-queries.md) · [Skills and challenge routes](skill-routes.md) · [Resources](resources.md) · [Control values](control-values.md) · [Experiment supervision](experiments.md)
