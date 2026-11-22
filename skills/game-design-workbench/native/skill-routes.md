# Skills, prerequisites, and challenge routes

## Skill prerequisites and ranks

```sh
gd skill-atoms --skill-atoms 'skill-atoms=game-design/skill-atoms' --json
gd-view view explorer --mode skills --skill-atoms 'skill-atoms=game-design/skill-atoms'
```

Add `--challenges ALIAS=FOLDER` for route inspection and repeatable `--related ALIAS=FOLDER` for linked context. **Skills** draws prerequisite → dependent links. Ordered layouts show prerequisite depth or rank bands; Free treats placement as presentation. Rank bands compress gaps. All authored ranks (including null, zero, and negative), statuses, isolates, and raw unresolved links remain inspectable.

- `authored_rank` is written in the record. `prerequisite_depth` is the longest complete prerequisite path. `minimum_rank` comes from finite prerequisite ranks; `display_rank` takes the higher of authored rank and minimum when present. Same-rank prerequisites are valid. Unranked is not zero.
- Cycles retain edges as conflicts. Unresolved/cyclic ancestry prevents complete placement; do not treat missing references as roots or label every affected descendant cyclic. Read findings and placement completeness separately from source-load coverage.
- The viewer can [draft changes](explorer.md#draft-editing-and-json-handoff) and recompute related route checks without changing disk. **Relationships** shows already-consumed raw dependencies and context, not additional relationships inferred from prose.

## Skill understanding, combinations and unlocks

Use `skill-atoms --from ALIAS/NAME` for prerequisite/unlock chains and shared-link comparison candidates. Provide repeated `--known ALIAS/NAME` for hypothetical knowledge and inspect `exploration`. Explorer offers equivalent knowledge and comparison controls in **Skills**. The named `skill-unlocks` query is also available.

All direct prerequisites are AND requirements. Supplying a known skill does not add its ancestors; eligible dependents do not become known automatically. `no_declared_prerequisites`, `needs_more`, `all_declared_prerequisites_supplied`, and `unresolved` describe declared eligibility, not learning. Comparison candidates share exact prerequisite or mechanic references; read the source before calling encounters equivalent, contrasting, or transferable.

## Challenge route opportunities

```sh
gd scaffolding --skill-atoms 'skill-atoms=game-design/skill-atoms' \
  --challenges 'challenges=game-design/challenges' --fields checks,witnesses,findings --json
```

**Challenges** shows the same opportunity analysis, entry points, `next` links, roles, and source evidence. Use it to compare where players encounter a demand and what earlier visits might prepare them. The analysis checks declared entry-to-visit routes, including revisits, and expands demands through transitive prerequisites.

- `requires` and `tests_mastery` are demands. Only earlier `introduces` or `practises` count as prior opportunities. Introducing and demanding a skill at the same challenge raises a review question, not an automatic flaw.
- `all_routes`, `some_routes`, `absent`, and `unknown` refer to the named skill under the loaded route model. Missing entries, broken links, and unreachable demands can make coverage unknown. Check `routes_complete` and `requirements_complete` separately. A retained witness may omit its prefix; do not present the suffix as a full route.
- A declared route is not physical or game-state reachability. Prior practice is not observed understanding, mastery, readiness, or quality. Compare with design intent and player evidence before adopting a revision.
