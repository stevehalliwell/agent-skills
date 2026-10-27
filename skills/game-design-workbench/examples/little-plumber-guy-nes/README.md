# Little plumber guy NES

A larger **authored game-design example**, built to feel familiar: move right, jump, bump blocks, collect rewards, use pipes, manage enemy contacts and reach a finish post. Read [game concept](game-design/concept.md) first.

Not a reconstruction of a published game. No ROM, copied map, screenshots, sprites, source captures or hardware-derived constants. The point is to see a recognizable design expressed through workbench records, then discover where those structures help or fall short. [Crate-puzzle](../crate-puzzle/README.md) remains the smaller starting example.

## What is here

211 records across 15 collections:

- 26 nouns and 12 verbs establish vocabulary and actions. Noun inventories explicitly reference known axes or remain null when unmodeled.
- 9 provisional axes: 8 cover actor life, form, protection/duration and existing pool quantities; Contact supplies only the accepted specimen's ordered context vocabulary. This begins actor/state migration; most interaction/mechanic bodies remain prose, not structured rule coverage.
- 49 conditional interactions retain different outcomes within the same noun–verb cell.
- 24 mechanics and 6 systems own reusable behavior and arrangements.
- 16 skill atoms and 19 challenges describe intended opportunities, demands, alternatives and joins.
- 2 level groups distinguish overworld and optional cellar; challenge links own routes.
- 20 controls and 4 resource extensions separate configuration from runtime pools.
- 8 proposed dynamics, 6 intended aesthetics and 3 proposed investigations distinguish explanations, experience goals and evidence still needed.
- 7 glossary records resolve local vocabulary without copying definitions.

Accepted records define this fictional example's current design, not empirical truth or Nintendo's design. Proposed dynamics/aesthetics remain predictions/intentions. All investigations are unexecuted; there are no participants, generated simulations or measured player outcomes here.

## Read the game, not just the folders

1. Follow the [course](game-design/levels/opening-course.md): safe apron, first walker, supply, canopy, pipes, gap/ceiling route or cellar, shell lane, checkpoint, staircase fork, final gap and finish.
2. Read [first-supply](game-design/challenges/first-supply.md), then its [release rule](game-design/mechanics/supply-release.md). The same block gives small and powered heroes different rewards. [Small-form case](game-design/interactions/bump-growth-supply-small.md) and [powered-form case](game-design/interactions/bump-growth-supply-tall.md) keep those conditions distinct.
3. Read [power ladder](game-design/systems/power-ladder.md). Capability is not a life pool: damage, pickups and shot capacity interact without all becoming one health number.
4. Follow [shell travel](game-design/mechanics/shell-travel.md): a resting object becomes a useful moving hazard, reverses at geometry and can threaten its kicker.
5. Follow [reward detours](game-design/dynamics/reward-detours.md) to [curiosity](game-design/aesthetics/curiosity.md), then [route-choice investigation](game-design/experiments/route-choice.md). This is an intended rules–behavior–feeling account, not evidence that players felt curious.

## Explore through the skill

Load [game-design-workbench](../../SKILL.md) and separately installed Attendant. This example's [registry](.pi/attendant.tables) and [agent instructions](AGENTS.md) bind all roles. Read relevant live schema/usage before record operations. From this example root:

```sh
node <attendant-skill>/scripts/attendant.mjs validate --no-correct
node <attendant-skill>/scripts/attendant.mjs schema
```

Only let schema/query preparation continue when no unrelated corrections are pending. Existing [native tools](../../native/README.md) take explicit folders; they are not installed or built automatically.

Useful requests and what they should expose:

- **“Show what happens when I bump each kind of block in different forms.”** Matrix cases distinguish small brick bump, powered break, conditional supply release and spent no-payout behavior. Do not collapse them into one generic rule.
- **“What changes when the player takes the cellar?”** Follow `pipe-steps → bonus-entry → coin-cellar → bonus-return → shell-lane`. This skips first-gap, low-ceiling and flower-shelf. Preserve carried form/time and one-way return; do not invent route probabilities.
- **“Is final-gap supported by earlier opportunities?”** Use Skills/Challenges and `scaffolding`. Current data gives 18 demanded-skill checks: 17 have prior opportunity on all declared routes; finish-approach has none before its first test. This is a discussion prompt, not proof of difficulty or acquired skill. Skipped optional encounters do not automatically imply missing competence.
- **“Trace rules that might support manageable tension.”** MDA connects contact risk, power preservation and clock trade-offs to an intended experience. Explain alternatives and missing play evidence, not a causal verdict.
- **“Try walk-speed 3 without adopting it.”** Resolve a temporary control override. Shared formulas update walk-tile-time; current baseline resolves walk/run tile times to 0.5/0.25 s and stable coin-bank capacity to 49 coins. Do not overwrite derived values or canonical controls.
- **“What remains undecided?”** Surface explicit unknown cases and [model limits](model-limits.md), not a silent repair. Read [proposed experiments](game-design/experiments/) as questions, not completed tests.

Headless examples, from this directory after an explicit native build:

```powershell
$gd = '../../native/build/gd.exe'
& $gd matrix --nouns nouns=game-design/nouns --verbs verbs=game-design/verbs `
  --interactions interactions=game-design/interactions --related mechanics=game-design/mechanics --json
& $gd scaffolding --skill-atoms skill-atoms=game-design/skill-atoms `
  --challenges challenges=game-design/challenges --related mechanics=game-design/mechanics --json
& $gd controls --control-vars control-vars=game-design/control-vars --json
& $gd rules --interactions interactions=game-design/interactions --mechanics mechanics=game-design/mechanics `
  --axes axes=game-design/axes --nouns nouns=game-design/nouns --control-vars control-vars=game-design/control-vars `
  --json --fields axes,actors,cases,findings
```

Existing native view can open the same folders in Skills, Challenges and Relationships modes. Use [documented view invocation](../../native/README.md#skill-prerequisites-and-ranks), replacing crate-puzzle folders with this example; no separate example viewer or copied runtime.

## How this example was authored

**Approximate instruction to the LLM**, consolidated from the request and clarification, not a verbatim prompt:

> Use game-design-workbench to build a much larger example called “Little plumber guy NES.” Author a familiar opening side-scrolling platform course: ground and gaps, enemies, blocks and pickups, power changes, an optional pipe room, rewards, retries and a finish. Lay out a coherent game design using existing records rather than reverse-engineering a published game. Make relationships and conditional outcomes concrete. Preserve uncertainty and distinguish intended experience from observed evidence. Keep references and actual game screenshots out of the repo. Use the example to expose limits in current structures and tools; do not build more infrastructure.

Workbench parts used:

1. **Concept/model/setup:** established an authored `game-design` concept, separate example root and the 15 shipped contracts. Attendant created collections and record identities; no new schema system.
2. **Capture:** wrote rules, cases, configuration, resources and glossary. Source Markdown remains authoritative. No generated content mirror is required to maintain the example.
3. **Skills/challenges:** authored AND prerequisites, ranks, introduction/practice/demand roles and branching course routes.
4. **MDA:** separated rules, possible behavior and intended feelings, linking proposed investigations instead of inventing evidence.
5. **Existing deterministic tools:** checked controls, matrix, prerequisites, routes and MDA over this same data. Tool results helped identify the final composite-skill support gap without automatically changing the design.

No reference lookup was needed to author this fictional design. No external reference material is retained. No experiment execution, control adoption, UI automation, commit or publication is implied by opening this example.

## Checks and remaining limits

Current headless checks read 26 nouns / 12 verbs / 49 interaction cases in 26 occupied cells; 16 skills with 19 prerequisite links and no cycle findings; 19 challenges; 20 resolved controls; 44 MDA nodes and 74 core links. Counts describe authored data, not exhaustive gameplay coverage. Maintainer tests validate a temporary copy through public Attendant and the existing native consumers.

[Model limits](model-limits.md) records state predicates, conditional route support, reset behavior, physical geometry and cross-control constraints that are currently prose rather than executable contracts. No rendered-performance or manual-UX acceptance claim. Data is useful before it is a complete game implementation.
