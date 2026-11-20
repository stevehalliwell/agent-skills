# Resource uses, alternatives and pivotal connections

Q10 walkthrough over unchanged [Forge chain recipes](README.md). Existing native reports support bounded agent answers. No new metric, recipe or engine feature needed for these checks.

## Which resources have few roles?

Read typed quantity declarations and their owning cases, not raw graph degree:

- **Ore:** one spending recipe, smelt; no authored credit/replenishment rule.
- **Fuel:** two spending recipes, smelt and forge; no authored credit. Shared stock creates potential competing uses, but actual contention depends on inventory and scheduling, neither supplied.
- **Ingot:** smelt credits and forge spends the same explicit pool. Intermediate producer/consumer role differs from fuel despite both connecting to two recipes.
- **Key:** forge credits; no authored spending/use rule. That is an explicit boundary of this demonstration, not an underuse defect or quality judgment.

Requirements and spending remain separate declarations. Readiness enables and is explicitly preserved; it is not consumed. Material quantities, pool bindings and system membership remain distinct concepts.

Native `direct-connections` reports all four resource-role metadata nodes with one distinct partner each: their pool facet. Those equal counts do not mean equal gameplay roles. Pool-facet incident counts also include guards and metadata bindings, not just uses. No role-diversity score or cross-resource quality ranking inferred.

## What alternatives exist?

- Ore-to-key returns one conditional witness using smelt, shared ingot pool and forge.
- Fuel-to-key returns two witnesses: direct fuel participation in forge, or fuel participation in smelt followed through ingots to forge.
- Both fuel witnesses still use the **same forge recipe**. Direct witness retains its ingot requirement; it is not a fuel-only key recipe. Longer witness retains ore, both fuel debits and station guards.
- Only forge authors key production. Two witnesses from a selected input are not two alternative key-producing recipes, independent sufficient causes or interchangeable inventory plans.

All witnesses remain conditional with `feasibility: not_checked`. No initial stocks, actual sequence, aggregate fuel budget or final result proven. Counts apply to this small retained source scope, not every possible design.

## Which connections are pivotal?

Two native views answer different questions:

1. **Resource witnesses** follow checked debit/credit direction with whole guard/side bundles. Sole returned ore-to-key witness includes shared ingot pool.
2. **Generic structural queries on Resources** traverse **undirected incidence**, including side inputs and metadata. Shared fuel connects smelt and forge even though neither produces fuel. In this graph, smelt and forge dominate the key target from the ore start; neither ingot nor fuel facet does, because each supplies an incidence connection between the recipes.

Current native selected support path passes through fuel. Asking for a structural bypass avoiding fuel finds an incidence path through ingot. This does **not** show crafting without fuel: both recipes still debit fuel in their complete source declarations. Asking to avoid ingot returns unavailable because it is not on that selected support path; unavailable is not proof that no alternate path exists.

Stronger identity control: temporary variant gives Forge a distinct ingot reserve pool. Directed ore-to-key Resource witness disappears, yet incidence reachability survives through shared fuel. Structural connectivity is therefore not evidence of resource conversion, compatible inventory or execution.

These are configured-start, retained-graph observations. Dominators are not universal gameplay requirements, player importance or condition-aware flow pivotality. A starting key stock is unspecified, not assumed zero.

## Replay

Use `$gd`, `$bindings`, `$from` (ore) and `$to` (key) from [README](README.md#inspect-through-existing-cli). Current native IDs below are explicit pool facets, not metadata role nodes:

```powershell
$fuel = 'facet:pool:resources%2Ffuel-stock:axes%2FFuelAmount'
$ingot = 'facet:pool:resources%2Fingot-stock:axes%2FIngotAmount'
& $gd resources @bindings --from $fuel --to $to --accepted-only --fields routes --json
& $gd query --question direct-connections --projection resources @bindings --json
& $gd query --question structural-dominators --projection resources @bindings `
  --from $from --to $to --json
& $gd query --question support-bypass --projection resources @bindings `
  --from $from --to $to --avoid $fuel --json
& $gd query --question support-bypass --projection resources @bindings `
  --from $from --to $to --avoid $ingot --json
```

Inspect `scope.incidence`, `walk_direction`, query completion, native Resource coverage and bypass availability before interpreting output. Finished generic incidence analysis can report complete while Resource report remains incomplete for applicability/composed outcomes. Those flags describe different scopes, not contradictory proofs.

Focused checks in `test/forge-chain.test.mjs` cover typed uses, two fuel witnesses sharing one key-producing recipe, incidence counts/dominators/bypass boundaries and incidence connectivity surviving the disconnected-pool control. Full focused run:12/12 tests, including8 example checks and4 existing Resource regressions; no skips with both native executables supplied. Authored `game-design/` bytes unchanged by checks.

Direct query artifacts: `../../native/build/forge-chain-q10-1794864839184/`. No player evidence, graphical acceptance, source-rule changes or new report/API implied. Dedicated condition-aware pivotality or role-diversity report remains unimplemented; this walkthrough does not silently add either.
