# Explain an MDA trace without inventing player evidence

Worked Q7/Q11 answer over existing fictional course. No design-rule changes or framework-wide review.

## Source-backed answer

Enemy-contact rules belong to power-ladder. Proposed power-protection predicts changes in how players approach risk after gaining capability. Manageable-tension names an intended experience associated with that prediction. Links establish this authored account, not causation or observed player response.

1. **Accepted mechanic — [enemy contact](game-design/mechanics/enemy-contact.md).** Harmful side contact treats small and powered forms differently. `walker-powered` declares small/protected results and preservation of local Lives Amount under its whole guard; protected enemy contact differs from gap/clock hazards. Contact geometry and unmodeled conditions still matter. These are rule declarations, not a report that a particular collision occurred.
2. **Accepted system — [power ladder](game-design/systems/power-ladder.md).** Explicitly lists enemy-contact alongside growth, spark upgrade and spark launch. Arrangement connects capability acquisition/use with damage recovery. It is not a numeric health bar or proof of increasing player competence.
3. **Proposed dynamic — [power protection](game-design/dynamics/power-protection.md).** Explicitly references power-ladder and enemy-contact. Predicts that players change approach to preserve gained capability. Source also names the opposite possibility: extra survivability could encourage aggressive experimentation. Neither behavior is observed.
4. **Proposed aesthetic — [manageable tension](game-design/aesthetics/manageable-tension.md).** Explicitly references power-protection. Intends stakes around gaps, shells or time while player retains a sense that considered action can help. Unclear shell state or tight timing might instead create helplessness. This is one intended experience aspect, not the entire [player story](game-design/concept.md), a tension score or evidence that play feels this way.

[First-course comprehension](game-design/experiments/first-course-comprehension.md) remains a proposed playtest. No playable build, participants or observations recorded. It does not validate caution, aggression, comprehension or emotional response.

## Why the two forward directions differ

Source stores basis references on the dependent record: power-ladder lists enemy-contact; power-protection lists power-ladder; manageable-tension lists power-protection.

- **Domain `mda --direction forward`:** follows incoming basis references toward dependent systems, dynamics and aesthetics.
- **Generic structural `query --projection mda --direction forward`:** follows projected link endpoints from referencing record to its declared basis.
- **Structural backward:** reverses traversal over those endpoints, matching domain forward when using the same MDA links and scope.
- Neither command rewrites source/stored links or establishes causal direction. Matrix/Resource generic structural graphs instead use undirected incidence, not production flow.

With the four explicit MDA collections and no extra filters, enemy-contact gives9 selected/reachable nodes for domain forward and structural backward. Domain backward and structural forward retain only the root in this MDA projection. Root is included in these counts; this does not say mechanic has no other references. Domain both selects44 nodes. Core graph has44 nodes/74 links; other noun/control/experiment references remain outside these four bindings.

## Native witness versus explanation

Native domain query retains one shortest parent witness per selected node plus selected/boundary links. Manageable-tension's shortest witness has three records: enemy-contact, power-protection and manageable-tension. Power-protection directly lists the mechanic, so no system step is required in that shortest witness.

Four-record explanation above is supported by additional raw declarations through power-ladder. Do not invent a system node in the native shortest path or treat dependent links as independent corroborating evidence. All selected/boundary links remain available for source inspection; shortest witness is not exhaustive path enumeration.

## Replay

From this example directory, after building current native CLI:

```powershell
$gd = '../../native/build/gd.exe'
$bindings = @(
  '--mechanics', 'mechanics=game-design/mechanics',
  '--systems', 'systems=game-design/systems',
  '--dynamics', 'dynamics=game-design/dynamics',
  '--aesthetics', 'aesthetics=game-design/aesthetics'
)
& $gd mda @bindings --from mechanics/enemy-contact --direction forward --json
& $gd query --question structural-dominators --projection mda @bindings `
  --from mechanics/enemy-contact --direction backward --json
& $gd query --question structural-dominators --projection mda @bindings `
  --from mechanics/enemy-contact --direction forward --json
& $gd query --list --json --fields questions
```

Domain output preserves record statuses, original source/target references, fields, source paths/lines and reversed path-step flags. Structural output preserves original endpoints and labels traversal separately. Discovery's `direction_semantics` field explains direction for supported structural path questions; help includes paired MDA examples.

## Check and interpret the traces

From repository root, set `GD_NATIVE_EXE` to the built CLI and run `node --test skills/game-design-workbench/test/native-mda-direction.test.mjs skills/game-design-workbench/test/native-structure-queries.test.mjs` to check direction, provenance, and output. Unset executable skips native checks.

Core reference completeness is not whole-game completeness, applicability, causation, observed dynamics, or guaranteed experience.
