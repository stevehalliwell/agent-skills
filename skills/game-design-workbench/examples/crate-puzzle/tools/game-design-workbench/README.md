# Game Design Workbench

Slice 1: a local-first agent skill and direct CLI for one Markdown design model. Includes 19 Attendant collection contracts, 18 attributed original question packs, guarded setup/update, declared-data queries, term resolution, and dated review storage. No npm install or service. Requires Node.js `>=24.10.0`.

## Try included example

Open `examples/crate-puzzle/` as a project and follow [walkthrough](examples/crate-puzzle/README.md). Delivered example includes its own tool snapshot; no global skill is required. The copy of tutorial source inside installed tools is documentation, not configured user-design facts.

From canonical repository root:

```sh
node skills/game-design-workbench/scripts/workbench.mjs query skills --project skills/game-design-workbench/examples/crate-puzzle --fields name,rank --json
node skills/game-design-workbench/scripts/workbench.mjs query matrix --project skills/game-design-workbench/examples/crate-puzzle --json
node skills/game-design-workbench/scripts/workbench.mjs lenses --json
```

Expected: six skills, coordinate rank 2, plan-push null, three distinct crate/push cases. Closed-exit walking is unknown; wall activation is explicitly not applicable. These are source facts, not engine execution or design verdicts.

For an agent, load [skill instructions](SKILL.md). Narrow requests work best: `Inspect declared crate/push cases`, `Review combined-room with skill-atoms; do not change source`, or `Capture pushing costs one energy`.

## New project

Run [setup proposal](references/setup.md), review combined changes, then approve exact plan before apply. Empty canonical collections stay empty; installer never invents game facts. Existing source/config and local tool edits are preserved or block update. All helpers run from installed `tools/game-design-workbench/`; Attendant runner/YAML/license notices are bundled.

## Available boundaries

- Query recipes show declared data and source paths, not computed game semantics.
- Alias lookup reports unknown/ambiguous/inactive identities; it does not guess synonyms.
- Framework review is agent interpretation through one selected pack, with citations/limits.
- Saved review input retains declared evidence origin. Structure/hash validation does not certify its truth.
- Generic read-only validation covers Attendant schema/reference checks; null schema fields do not enforce workbench scalar subtypes.

Native C99 loader/checks, cycle/rank/all-route analysis, formula evaluation/unit resolution/control exports, raylib views, and simulation/sweep/replay/expectation tools are deferred. Unsupported commands fail explicitly. No aggregate quality, fun, or psychological score.

## Direct use and maintenance

Read [CLI contract](references/cli.md) for flags, envelopes, limits, errors, and maintainer commands. Read [model ownership](references/model.md) before changing definitions. Review does not authorize repair, setup, experiment execution, or commits.

Canonical source contains `test/workbench.test.mjs` and pack/fixture generators. Project snapshots exclude maintainer scripts/tests, preserve vendored bytes, and pin owned text to LF so recorded tool hashes survive ordinary Git checkout. Windows with Node v24.16.0 verified; other platforms and native runtimes are not claimed tested.
