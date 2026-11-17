# Forge chain

Small fictional [game-explorer model](game-design/concept.md) for multi-step resource conversion. Two recipes share explicit intermediate and fuel pools. Not a playable game, inventory planner or balance recommendation.

```text
Smelt: ready furnace; spend 2 ore and 1 fuel; credit 1 ingot.
Forge: ready forge; spend 1 ingot and 1 fuel; credit 1 key.
```

## Source owners

24 records across six collections:

- Six [nouns](game-design/nouns/): four materials and two stations. Actor classifications remain unspecified.
- Five [axes](game-design/axes/): four nonnegative integer quantities with distinct units; one closed readiness vocabulary. No initial values.
- Four [resources](game-design/resources/): explicit ore, fuel, ingot and key pools. No initial/capacity controls or inferred ownership.
- Six [control variables](game-design/control-vars/): recipe costs/yields, separate from runtime stocks.
- Two [mechanics](game-design/mechanics/): [smelt](game-design/mechanics/smelt.md) and [forge](game-design/mechanics/forge.md), each with one authoritative `gd-rule` case.
- One [system](game-design/systems/smithing.md): declared composition, not execution order.

[Registry](.pi/attendant.tables) and [agent instructions](AGENTS.md) bind these folders. Contracts come from shipped templates; mechanics omits unused optional `verbs` reference because this partial example has no verb collection. No new field types or native semantics.

## Why the chain connects

Both recipe participants explicitly declare:

```text
pools:
    IngotAmount: resources/ingot-stock
```

Pool reference plus axis establishes intermediate identity. Same noun, participant label or generic metadata reference would not suffice. Both recipes also explicitly bind `resources/fuel-stock`, preserving two separate fuel debits. Furnace/forge readiness enables recipes without consumption; each case explicitly preserves its station's Readiness.

## Inspect through existing CLI

From this example directory, after building current native CLI:

```powershell
$gd = '../../native/build/gd.exe'
$bindings = @(
  '--mechanics', 'mechanics=game-design/mechanics',
  '--nouns', 'nouns=game-design/nouns',
  '--axes', 'axes=game-design/axes',
  '--resources', 'resources=game-design/resources',
  '--control-vars', 'control-vars=game-design/control-vars',
  '--related', 'systems=game-design/systems'
)
& $gd controls --control-vars control-vars=game-design/control-vars --json
& $gd rules @bindings --json
& $gd resources @bindings --json
```

Copy opaque endpoint IDs from Resource overview. Current explicitly bound facets:

```powershell
$from = 'facet:pool:resources%2Fore-stock:axes%2FOreAmount'
$to = 'facet:pool:resources%2Fkey-stock:axes%2FKeyAmount'
& $gd resources @bindings --from $from --to $to --accepted-only --fields routes --json
```

Expected answer: one conditional witness through smelt, shared ingot facet and forge. Four route steps retain checked debit/credit directions. Activation bundles retain both fuel costs, quantity guards and both station-readiness requirements. Readiness is not a debit or produced material.

**Do not read that as a recipe execution plan.** Output retains `conditional: true`, `feasibility: not_checked` and overall incomplete coverage. Current inventory, starting readiness, capacity, geometry, scheduling and final outcomes are not supplied or proven. Even known signed deltas do not establish a feasible sequence or total inventory budget.

## Resource uses and pivotal connections

[Q10 walkthrough](resource-questions.md) distinguishes declared uses from graph degree, two input witnesses from alternative recipes, and undirected incidence bypasses from resource conversion. Shared fuel connects recipes structurally even when a temporary ingot-pool split removes the directed conversion witness. No quality score or condition-aware pivotality claim.

## Identity controls

Focused tests use temporary copies only:

1. Rebind Forge's Ingot participant to a separate proposed reserve pool, still referencing the same noun and axis. Native report retains two resolved pool facets. Ore-to-key query then returns no known witness; it must not merge pools by noun, local label or generic metadata links. Reserve is proposed so the fixture does not introduce a second accepted resource extension for the same noun.
2. Remove explicit ingot binding from both recipes. Identically named Ingot participants then retain distinct case-local facets, not an inferred shared pool; no known ore-to-key witness.

No known witness is not an impossibility proof. Neither control rewrites checked-in rules or adds active alternative recipes.

## Repeat checks

From repository root, with explicit existing executable paths:

```powershell
$env:GD_NATIVE_EXE = '<current native gd executable>'
$env:GD_ATTENDANT_EXE = '<installed native Attendant executable>'
node --test skills/game-design-workbench/test/forge-chain.test.mjs
```

Without executable variables, source checks still run; optional native/Attendant checks report skipped, not passed. Tests cover source contracts, resolved recipe controls, typed rule bindings, conditional witness/side guards, distinct-pool and unbound controls, declared uses, incidence-versus-conversion boundaries, source-byte preservation and strict validation of a temporary managed copy.

Current direct inspection: strict Attendant validation24 records, no diagnostics/corrections; native declarations12 nodes/18 relations, six known quantity deltas, no findings. Ore-to-key retains one two-activation conditional witness, no search limit. No new engine feature, viewer launch or experiment execution required.
