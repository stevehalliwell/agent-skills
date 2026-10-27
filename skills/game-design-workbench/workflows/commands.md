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

Native matrix/skill/MDA analysis/views, challenge-route analysis, and typed control resolution/import planning/export are available; see [native contract](../native/README.md). [Shared experiment helpers and crate baseline](../native/README.md#prepared-experiments) are available; [owning experiment](../examples/crate-puzzle/game-design/experiments/route-cost.md#results) now records one complete baseline run. [Explicit Windows process supervision and read-only result validation](../native/experiments.md) are implemented and checked with synthetic emitters. Batch/sweep/replay automation and result views remain undelivered. No Git/approval/lifecycle automation is added. Agent builds bounded experiments and interprets results using [experiment workflow](experiment.md); use [tuning extension](experiment-tuning.md) only for requested comparisons or search. Do not invent an available runner or execute an experiment merely because a document describes one.

When an explicitly authorized experiment has executable implementation:

1. Establish question, assumptions, named inputs, local measures/criteria, generated code, dependencies, command, seed, trials, and limits. Review code before execution. Done: model and recipe are inspectable.
2. Inspect Git status/diff; identify all relevant files, including untracked/ignored inputs. Obtain checkpoint approval under project policy and commit only reviewed paths. Do not force-add secrets or unrelated work. Done: recipe/code/inputs are recoverable from an exact commit.
3. Execute from that committed state, using ordinary checkout/worktree when needed to isolate later edits. Identify external inputs/toolchain versions explicitly. A commit alone does not freeze files read from a dirty working tree. Done: actual execution uses declared inputs.
4. Record commit, command, versions, seed, effective inputs, limits, completion status, and results in owning experiment. Keep rebuildable outputs separate from non-rebuildable evidence. Done: observations and modeled outputs remain distinct from interpretation.

Git and agent workflow provide copying, merge updates, history, and checkpoints. Do not add custom lifecycle programs for these steps. Keep executable code for domain computation and tests. Copying/editing existing Markdown assets is agent work; do not add code mirrors to regenerate them.

## Review 01 repository migration maintenance

Repository-only `scripts/migrate-workbench-actor-rules.mjs` adopts a temporary reviewed proposal through public Attendant metadata operations and readable fence replacement. It is not an installed authoring command, content generator or evaluator. Original Markdown remains authoritative.

Run `node scripts/migrate-workbench-actor-rules.mjs --help` for required explicit repository/proposal/Attendant/native paths. Default plans only; `--stage NEW_DIR` copies and validates both examples before authorized `--apply`. Exact rule coverage, inside-project mutation collections, strict Attendant validation and native checks are required. Safe rerun skips unchanged fields/fences. Failure reports completed operations and whether partial changes are possible; batches are not transactions. No deletion, automatic repair, frozen-evidence rewrite or experiment execution.

## Repository asset checks

Edit shipped templates, example Markdown, framework guides, and catalog directly. Templates are authoritative starting assets copied into approved destinations; each live project's schema/usage then owns its contract. Example owns its design data. No generator or JavaScript content mirror.

Run from repository root with Node.js `>=24.10.0`:

```sh
node --test skills/game-design-workbench/test/authoring.test.mjs
```

Tests read shipped files and validate ordinary temporary copies through public Attendant; they do not regenerate templates or records. `GD_ATTENDANT_SKILL` locates installed Attendant for maintainer integration tests only; integration is explicitly skipped when absent. This variable is not part of normal skill use.
