# Crate-and-switch example

63 records across 14 collections. Includes six skill atoms, three crate/push cases, one executed timing investigation, one unexecuted planning investigation, and a working concept. No project-local workbench runtime or nested copy of tutorial data is required.

## Game concept

[Concept and current working mode](game-design/concept.md). Separate [MDA aesthetic: deliberate tension](game-design/aesthetics/deliberate-tension.md).

## Start

Load [workbench skill](../../SKILL.md) and separately installed Attendant. Follow its record/query workflow. Run from this example root, replacing `<attendant-skill>` with loaded skill directory:

```sh
node <attendant-skill>/scripts/attendant.mjs validate --no-correct
# Only continue against live source when preparation needs no corrections.
node <attendant-skill>/scripts/attendant.mjs schema
node <attendant-skill>/scripts/attendant.mjs query --sql 'SELECT name, rank, prerequisites FROM "skill-atoms" ORDER BY name'
```

For a filtered query without shell-quoting ambiguity, put SQL below in a UTF-8 file and use Attendant's `--sql @<file>` input:

```sql
SELECT name, rank, prerequisites FROM "skill-atoms"
WHERE status = 'accepted' ORDER BY name;
```

Read [inspection topics](../../workflows/inspect.md) for cases, routes, MDA links, configuration, glossary, and experiments. Inspect non-correcting validation before allowing query preparation to touch source. Use source directly when projection would cause unrelated changes.

Expected: coordinate rank 2; plan-push null; switch-chain ranks 3. Three distinct crate/push cases. Closed-exit walking unknown; wall activation not applicable. Route-cost has completed stage with retained simulation evidence; energy/planning remains proposed. Declared glossary maps box to crate and shove to push; `shoves` is not a declared alias.

## Try bounded requests

1. `Inspect crate/push cases`: compare conditions, consequences, feedback, and known/no-effect/unknown coverage.
2. `Review coordinate using skill-atoms`: inspect both prerequisite chains and authored rank, without claiming native rank analysis.
3. `Change coordinate rank to 3; preserve every other authored field/rank`: inspect full diff; only selected rank should change semantically.
4. `What would removing pushing affect?`: trace declared dependencies; retain uncertain consequences.
5. `Inspect route-cost experiment`: start at [worked experiment record](game-design/experiments/route-cost.md). It links generated C99 test, frozen inputs and reproduction recipe; defines seven actions, 1,000 trials, clock boundary and 8 s / 90% criterion. One baseline simulation has run; no playtest has run. [Experiment workflow](../../workflows/experiment.md) describes how an agent generates this artifact.
6. `Prepare a timing investigation and temporary speed candidates`: follow [agent-led tuning preparation](experiment-tuning.md). Common resolver supplies baseline/candidates; agent owns experiment and interpretation. Preparation is not execution or baseline adoption.

## Investigations

`experiments/energy-encourages-planning` retains original hypothesis UUID and claim as a proposed playtest investigation. Method remains undesigned. It is separate from timing model: human planning and assumed route delays ask different questions.

`experiments/route-cost` owns executed timing test and former route-time criterion, including original criterion UUID and authored null thresholds. Measures heading defines outputs; Results links actual trial output, summary, validation and provenance. Pulling remains a rejected verb proposal; crate-color remains a proposed mechanic. Their identities/rationale survive without a decisions collection. Named configuration lives in `game-design/control-vars/`.

## How the timing example was created

Approximate consolidated instruction to the LLM, reflecting choices clarified during the work; not a verbatim original prompt:

> Use game-design-workbench to turn the existing route-cost investigation into one small timing experiment. Use named controls for four walks followed by three successful pushes. Assume an independent uniform 0–1 s delay before each action; time from before first delay through final push completion. Generate a small C99 test and freeze its inputs. After checkpoint/run approval, run 1,000 seeded trials, compare measured completion times with the existing 8 s / 90% criterion, and record results and limitations. Keep code, inputs and raw evidence with this example. Do not change game controls or build a general experiment framework.

Workbench parts used:

- **Design records:** [route-cost](game-design/experiments/route-cost.md), linked mechanics and [named controls](game-design/control-vars/) supplied question, rules, measures and criterion.
- **Control resolution:** native `controls --json` supplied effective typed values, formulas and dependencies. Generated model consumed frozen values rather than evaluating formulas itself.
- **Agent-led experiment:** LLM generated [test code and frozen inputs](experiments/route-cost/), reused shared C99 input/random/measurement helpers and existing bounded execution/result checks, then interpreted actual output. Ordinary Git supplied source checkpoint; Attendant updated record fields, with Markdown body edits for method and findings.
- **Reusable instructions:** [experiment workflow](../../workflows/experiment.md) was distilled from this worked example afterward. It captures the question → small test → authorized run → recorded findings loop; it was not an already-complete workflow followed verbatim during creation.

All experiment-specific material lives here: owning record under `game-design/experiments/`, code/input/recipe under `experiments/route-cost/`, and retained run evidence under [game-design-results/route-cost/](game-design-results/route-cost/). Shared workbench helpers remain outside the example; they are not a second copy of its data.

## Saved history

Earlier baselines and dated reviews remain unchanged/historical. Their hashes and CLI-era metadata do not require retaining old tooling. For current review, follow [review workflow](../../workflows/review.md); record commit plus relevant draft paths, and explicitly select an actual committed intention only when comparing intent. Do not relabel old evidence as freshly run.
