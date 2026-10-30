# SQLite comparison tools

Not application code. Normal builds never compile these programs or apply experimental settings.

- `sqlite-adoption-*`: current CLI/core policy variants. Generator copies current `src/`, `apps/cli/` and vendor layout into owned ignored output, then changes only selected SQLite policy. Current `full` variant rebuild passes after folder cleanup. No timing rerun or new performance claim.
- `sqlite-experiment-*`, `sqlite-tuning-*`: historical eager-FTS baseline under `tests/fixtures/sqlite-experiment-base/`. That frozen context no longer matches current shared YAML/rollback ownership fields; rebuild currently fails. Existing measured artifacts remain evidence. Do not silently substitute current core or treat these as production checks.
- `sqlite-experiment-faults.mjs`: historical profile fault/process-death harness, not power-loss certification.

Run scripts from native project root using `experiments/sqlite/<script>`. All outputs stay under ignored `build/`. No downloads, installed-launcher changes or live-project mutation. Baseline repair/retirement is outside folder cleanup; inspect applications and shared core first.
