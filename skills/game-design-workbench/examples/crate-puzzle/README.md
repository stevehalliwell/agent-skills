# Crate-and-switch example

63 records across 14 collections. Includes six skill atoms, three crate/push cases, two unexecuted investigations, and a working concept. No project-local workbench runtime or nested copy of tutorial data is required.

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

Expected: coordinate rank 2; plan-push null; switch-chain ranks 3. Three distinct crate/push cases. Closed-exit walking unknown; wall activation not applicable. Experiments have proposed stage, not completed tests. Declared glossary maps box to crate and shove to push; `shoves` is not a declared alias.

## Try bounded requests

1. `Inspect crate/push cases`: compare conditions, consequences, feedback, and known/no-effect/unknown coverage.
2. `Review coordinate using skill-atoms`: inspect both prerequisite chains and authored rank, without claiming native rank analysis.
3. `Change coordinate rank to 3; preserve every other authored field/rank`: inspect full diff; only selected rank should change semantically.
4. `What would removing pushing affect?`: trace declared dependencies; retain uncertain consequences.
5. `Inspect route-cost experiment`: read local delay assumptions, embedded 8 s / 90% criterion, and analytical reference. No simulation or playtest has run.

## Investigations

`experiments/energy-encourages-planning` retains original hypothesis UUID and claim as a proposed playtest investigation. Method remains undesigned. It is separate from timing model: human planning and assumed route delays ask different questions.

`experiments/route-cost` owns timing method and former route-time criterion, including original criterion UUID and authored null thresholds. Measured outputs live under its Measures heading. Pulling remains a rejected verb proposal; crate-color remains a proposed mechanic. Their identities/rationale survive without a decisions collection. Named configuration lives in `game-design/control-vars/`.

## Saved history

Earlier baselines and dated reviews remain unchanged/historical. Their hashes and CLI-era metadata do not require retaining old tooling. For current review, follow [review workflow](../../workflows/review.md); record commit plus relevant draft paths, and explicitly select an actual committed intention only when comparing intent. Do not relabel old evidence as freshly run.
