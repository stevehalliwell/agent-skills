# Environmental resource: single-use supply opportunity

Existing nouns, systems and rules describe this opportunity without a new entity, numerical pool or resource schema. This walkthrough answers one Q8 example; it does not classify every useful world object as a resource.

## Source-backed answer

- **Opportunity:** [supply-block](game-design/nouns/supply-block.md) offers one authored reward. [First supply](game-design/challenges/first-supply.md) places a conspicuous growth supply above safe floor; taking it is optional. This is an opportunity at a world object, not a quantity carried by Hero.
- **System:** [reward economy](game-design/systems/reward-economy.md) explicitly contains [supply release](game-design/mechanics/supply-release.md), alongside coin collection, score awards and earned lives. System membership comes from that declared link, not from graph proximity.
- **Ownership and identity:** no declaration assigns ownership of Block to Hero. Block remains `nouns/supply-block` after spending; `nouns/spent-block` is a descriptor, not another physical instance. `is_actor` remains unclassified. Rule-local Block bindings do not identify the same runtime block across cases.
- **Awareness:** first-supply is authored as conspicuous. Other supplies can be concealed; a valid hit reveals them. Release, spent-state and form-change feedback are intended cues, not observed player awareness. No initial visibility is inferred for every block.
- **Use:** `mechanics/supply-release#growth-small` requires rising, small Hero; unspent Block with growth content; and ordered underside contact. It conditionally proposes Block becoming spent. Releasing a moving Grow-cap remains an explicit unknown effect: spawn geometry/motion are unconfigured. Release is not collection or immediate growth.
- **Exhaustion:** [SupplyState](game-design/axes/SupplyState.md) distinguishes unspent/spent. `spent-no-payout` explicitly preserves SupplyState; prose says later hits dispense nothing while Block remains solid. Preservation is authored here, not inferred from a missing transition.
- **Renewal and lifetime:** [death and retry](game-design/mechanics/death-and-retry.md) says later retry restores course rewards while retaining score and coin bank. [Reward economy](game-design/systems/reward-economy.md) names repeated collection and leaves farming balance open. Exact reset granularity, instance correspondence and staging remain unknown. No typed Block reset to unspent is retained by this chart; prose renewal is not an executable reset edge.

## Agent workflow

Read linked sources for arrangement, awareness and reset prose. Use native queries for typed state proposals and exact local bindings. Do not turn this qualitative environmental opportunity into a shared inventory pool.

From this example directory, after building current native CLI:

```powershell
$gd = '../../native/build/gd.exe'
$bindings = @(
  '--nouns', 'nouns=game-design/nouns',
  '--verbs', 'verbs=game-design/verbs',
  '--axes', 'axes=game-design/axes',
  '--interactions', 'interactions=game-design/interactions',
  '--mechanics', 'mechanics=game-design/mechanics',
  '--resources', 'resources=game-design/resources',
  '--control-vars', 'control-vars=game-design/control-vars',
  '--related', 'systems=game-design/systems'
)
& $gd query --question state-chart --projection actors @bindings `
  --object nouns/supply-block --axis axes/SupplyState --json
& $gd query --question state-chart --projection actors @bindings `
  --object nouns/supply-block --axis axes/SupplyState `
  --case 'mechanics/supply-release#spent-no-payout' --participant Block --json
& $gd rules @bindings --case 'mechanics/supply-release#growth-small' `
  --assume 'Hero:axes/Motion=rising' --assume 'Hero:axes/Form=small' `
  --assume 'Block:axes/SupplyState=unspent' --assume 'Block:axes/SupplyContent=growth' `
  --context 'Hero axes/Contact.undersideContacts Block' --fields analysis --json
```

Expected interpretation:

1. Whole-noun chart retains source cases, other participants, full guards and unknown effects. Binding hypotheses are not runtime instances or reachable scenes.
2. In spent-no-payout, unspent is excluded with no retained transition. Spent retains conditional explicit preservation. This does not make omitted behavior a self-loop.
3. Growth-small has supplied contact truth, but applicability remains unknown because valid single-use hit is not fully encoded. Retained spent write stays conditional despite supplied enum facts. No spawn or collection is invented.

`resources` also retains SupplyState facets and state-write declarations. Their identity is `case_participant_axis`, with `pool: null`; that does not connect all Block bindings into one inventory. Resource-extension initial/capacity metadata is unnecessary for this example's enum-state opportunity. Explicit-pool multi-step conversion remains a separate Q9 exercise.

## Result and limits

Current representation is sufficient for this source-backed agent walkthrough. No new fields, resource record, pool binding or gameplay rule needed. Awareness, ownership and renewal still require source reading; no finished structured query returns all three. A complete executable reward-reset model and observed discoverability would require additional authored detail or play evidence, not a guessed edge or classifier.

No claim of physical reachability, actual payout, successful collection, final outcome, player understanding or rendered/pointer acceptance.

## Maintainer checks

Read-only replay: `scripts/audit-workbench-design-questions.mjs` from repository root. Supply a current executable and a new output directory. Three probes cover this example; eight assertions check conditional spending, retained spawn unknowns, excluded unspent hypothesis, explicit spent preservation and remaining applicability unknown.

Latest artifacts: `../../native/build/environmental-resource-audit-1794861986/`. Full audit:32 successful probes; all authored `game-design/` file hashes unchanged. Environmental chart:14 local bindings,28 hypotheses,24 proposals,28 activations; finite query complete, source incomplete, no retention omissions. Axis vocabulary remains proposed/open, not exhaustive. Focused spent probe retains two hypotheses. Attendant validation:225 records, no diagnostics or pending corrections. No experiment, source-rule edit or viewer launch.
