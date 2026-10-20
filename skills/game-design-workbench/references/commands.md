# Existing tools and executable boundaries

## Record operations

Load separately installed `attendant` skill and follow its matching operation reference. Use its public runner; do not copy its code/guidance, import private modules, or install it automatically. Attendant owns its runtime requirements and command semantics. No workbench-specific CLI, environment setup, manifest, or package installation is needed.

Command placeholders: `<attendant-skill>` is loaded skill directory; `<project>` is target project root. From project or with explicit `--project`:

```sh
node <attendant-skill>/scripts/attendant.mjs validate --no-correct --project <project>
# Only continue against live source when preparation needs no corrections.
node <attendant-skill>/scripts/attendant.mjs schema --project <project>
node <attendant-skill>/scripts/attendant.mjs query --sql '<read-only SQL>' --project <project>
```

Read relevant source when a projection is unnecessary. Schema/query/search preparation can mechanically correct records; apply shared read-only rule before these commands. Correct only authorized in-scope source defects. For writes, follow Attendant's record workflow and inspect exact diff afterward.

## Experiment checkpoints

Native analysis, formula resolution/import/export, views, and simulation tools remain deferred. Do not invent an available command or execute an experiment merely because a document describes one.

When an explicitly authorized experiment has executable implementation:

1. Establish question, assumptions, named inputs, local measures/criteria, generated code, dependencies, command, seed, trials, and limits. Review code before execution. Done: model and recipe are inspectable.
2. Inspect Git status/diff; identify all relevant files, including untracked/ignored inputs. Obtain checkpoint approval under project policy and commit only reviewed paths. Do not force-add secrets or unrelated work. Done: recipe/code/inputs are recoverable from an exact commit.
3. Execute from that committed state, using ordinary checkout/worktree when needed to isolate later edits. Identify external inputs/toolchain versions explicitly. A commit alone does not freeze files read from a dirty working tree. Done: actual execution uses declared inputs.
4. Record commit, command, versions, seed, effective inputs, limits, completion status, and results in owning experiment. Keep rebuildable outputs separate from non-rebuildable evidence. Done: observations and modeled outputs remain distinct from interpretation.

Git and agent workflow provide copying, merge updates, history, and checkpoints. Do not add custom lifecycle programs for these steps. Keep executable code for domain computation, repeatable artifact generation, and tests where it removes actual repetitive work.

## Repository authoring tools

These maintain shipped assets; they are not installed into game projects or invoked during routine review. Node.js `>=24.10.0` for maintainer checks:

```sh
node skills/game-design-workbench/scripts/build-example.mjs --output <empty-directory>
node skills/game-design-workbench/scripts/build-example.mjs --templates --output <empty-directory>
node --test skills/game-design-workbench/test/authoring.test.mjs
```

Run from repository root. Generator reads `model.mjs` for 14 collection contracts, `fixture-data.mjs` for deterministic tutorial records/UUIDs, and authored example concept Markdown. It refuses non-empty output; `--templates` explicitly rewrites shipped generated templates. These are synthetic fixtures, not play evidence. Do not use generator to update a user's live design.

Framework guides and catalog are edited directly; no document generator or JavaScript content mirror. Test integration with external Attendant uses `GD_ATTENDANT_SKILL` only to locate that installed dependency during maintainer tests; tests mark integration skipped when absent. This variable is not part of normal skill use.
