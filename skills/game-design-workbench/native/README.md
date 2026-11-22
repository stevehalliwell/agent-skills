# Choose an analysis or Explorer view

Use the CLI for repeatable results, explicit inputs, and JSON evidence. Use **Explorer** to browse relationships, compare cases, and inspect their source side by side. Both read supplied Markdown collections and share native analyses where offered. Neither runs the game or replaces Attendant validation.

- **Start:** [Build](build.md) · [CLI quickstart](cli.md) · [Explorer usage](explorer.md)
- **Work together:** [shared controls](controls.md) · [local viewer IPC](ipc.md) · [draft handoff](explorer.md#draft-editing-and-json-handoff)

## Start with a design question

| Question | Use | Read the result as |
| --- | --- | --- |
| What happens when two things interact? | **Noun interactions** to find cases; **Selection & evidence** or `rules --case` to inspect one. [Rules and outcomes](outcomes.md) | Conditional proposals under supplied facts, not an automatic scene outcome. |
| Which different declarations lead to the same outcome? | **Effects** or `effect-contributors`. [Actors, effects, and states](actor-effects.md) | Source definitions and mapped placements, not independent sufficient causes or event frequency. |
| How does the response change with state? | **Actors** or `actor-states` to compare finite variants; `state-coverage` for an inventory-wide check. [Actors, effects, and states](actor-effects.md) | Differences under explicit assumptions, not severity or measured difficulty. |
| Can an object change state and still affect others? | **State charts** (Statechart or Grid) or `state-chart`. [State charts](actor-effects.md#state-charts) | Conditional transitions and side effects, not reachable combinations or inferred gameplay roles. |
| Where are interactions sparse or varied? | **Verb usage**, **Noun interactions**, `matrix`, `connectivity`. [Interaction counts](cli.md#interaction-counts) | Different counts for different questions. Inspect cases before calling a pair uniform, absent, or defective. |
| Which skills combine? Which encounters change a familiar demand? | **Skills**, **Challenges**, `skill-atoms --from`, and authored `decision-assessments`. [Skills and routes](skill-routes.md) · [Decision assessments](decisions.md) | Prerequisite sets and authored contrasts, not measured transfer or a controlled comparison. |
| Which connections are shared, mandatory, or bypassable? | **Relationships**, graph layers, or `reachability`, `structural-dominators`, `support-bypass`. [Structural questions](cli.md#structural-questions) | Paths and necessity in the chosen graph, not gameplay traffic or feasibility. |
| Where does a resource appear, including in the environment? | **Resources**, `resource-declarations`, and linked noun, system, and rule records. [Resource declarations](resources.md) | Authored roles and bindings. Ownership, awareness, and reset behavior need source evidence. |
| How might one resource become another? | **Resources**, `resources --from … --to …`, or `resource-routes`. [Resource witnesses](resources.md#conditional-witnesses) | A conditional chain with side inputs and guards, not a feasible plan or guaranteed outcome. |
| Which resources have few uses or depend on one connection? | Compare **Resources** declarations and directed witnesses with `direct-connections` or structural queries. [Resources](resources.md) | Incidence and witnessed conversions answer different questions. A structural bypass need not support conversion. |
| How do rules connect to intended experience? | **MDA**, `mda --from`, or the [MDA framework](../frameworks/mda.md). [MDA traces](cli.md#mda-traces) | Authored associations from mechanics, through dynamics and optional systems, toward aesthetics; not observed emotions or causality. |
| What references are unresolved in a partial design? | **Diagnostics**, source inspector, `declaration-gaps`. [Structural questions](cli.md#structural-questions) | Consumed references and scope gaps. Unloaded is not missing; not every gap has a canvas node. |
| Which objects look similar along selected dimensions? | **Built-in structural / similarity questions**, `similarity-features`, `similarity-map`. [Similarity](similarity.md) | An approximate 2D view of chosen measures. Check exact component distances and unplaced entries. |
| What would a proposed encounter change about a level sequence? | **Challenges**, `scaffolding`, then [review](../workflows/review.md) or [experiment](../workflows/experiment.md). [Route opportunities](skill-routes.md#challenge-route-opportunities) | Changes to declared prior practice on routes, not evidence of level quality, traversal, or learning. |

**Related tools:** [Resolve and export control values](control-values.md) without changing formulas or source. [Prepare and supervise an authorized experiment](experiments.md) when a question needs measurement instead of source inspection.
