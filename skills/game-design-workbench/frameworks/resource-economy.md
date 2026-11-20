# Resources, flows, and feedback

Pack: `resource-economy` · version 1
Attribution: Ernest Adams and Joris Dormans, Game Mechanics: Advanced Game Design (2012); Machinations approach

## Contents

- [Scope](#scope)
- [Inputs and query support](#inputs-and-query-support)
- [Working concepts](#working-concepts)
- [Apply to selected slice](#apply-to-selected-slice)
- [Original prompts](#original-prompts)
- [Worked example](#worked-example)
- [Evidence and counterchecks](#evidence-and-counterchecks)
- [Common traps](#common-traps)
- [Missing data and evidence limits](#missing-data-and-evidence-limits)
- [References](#references)

## Scope

Examine a selected resource and the rules that create, spend, transform, or limit it. Separate resource identity from adjustable quantities.

## Inputs and query support

Roles: `nouns`, `resources`, `control-vars`, `mechanics`, `interactions`, `experiments`.

Inspection topics: control variables, typed axes, interaction/mechanic cases, explicit pool bindings and source records. Native controls resolve checked formulas/units. Resource inspection retains declarations, unknowns and conditional structural witnesses.

## Working concepts

- **Resource identity and quantity:** A resource extends a noun identity; its starting amount, capacity, and costs are separate authored control variables. Changing a number should not create a different resource or silently alter its meaning.
- **Source, sink, and transformation:** A source adds quantity, a sink removes it, and a transformation relates quantities or states. Identify the rule and its trigger. Requirements enable without consumption; checked negative/positive deltas supply debit/credit direction. Join cases only through explicit pool+axis identity, never a generic reference link or shared noun name.
- **Feedback:** A reinforcing relationship amplifies a change; a balancing relationship counteracts it. Explain the intervening rules and time scale. A rising quantity alone is not proof of either relationship.
- **Strategy dependence:** The same quantities can produce different outcomes under different decisions, routes, and timing assumptions. An average is not a substitute for inspecting bottlenecks, depleted states, or an alternative strategy.

## Apply to selected slice

1. Choose one resource and one disputed claim, such as sufficiency for a declared route. Read noun, resource extension, control variables, spending mechanics, and relevant interaction cases. Record the exact claim and scope.
2. List starting quantity, bounds, changes, and triggers with their source owners. Keep unknown amounts and units explicit. Distinguish authored formula text from values checked by shared native resolver. Preserve quantity expressions, target bindings and full-case guards; do not duplicate evaluation.
3. Inspect native conditional Resource witnesses where explicit pool+axis bindings support joins. State assumptions about successes, failed-action costs, replenishment and optional routes. Label any hand-traced sequence separately. Neither witness nor hand calculation proves feasibility or executed simulation.
4. Examine a contrasting strategy or boundary state. Ask whether costs alter choices, merely delay progress, or permit a dead end under the stated assumptions. Separate quantity feasibility from reachability and desired experience.
5. Return the flow narrative, disputed assumptions, and one useful measurement proposal. Preserve candidate tuning changes as alternatives. No control override, formula export, balance adoption, or experiment execution follows from this review.

## Original prompts

1. What is the resource, what stores it, and which rule changes it?
2. Which sources and sinks depend on player choices, elapsed time, or chance?
3. Does gaining more of this resource make future gains easier, harder, or neither? What intervening rule creates that feedback?
4. Which capacity or initial-value control variables bound the state, and where are conversion assumptions written?
5. Could a proposed balance change produce different outcomes for different strategies rather than one new average?
6. What minimal measurement would test the disputed system claim, and which behavior assumptions would that model omit?

## Worked example

- **Authored facts:** Energy has noun/resource identity; initial-energy and push-cost are named configuration values. Crate-pushing owns spending. The tutorial includes a proposed route requiring three successful pushes.
- **Conditional analysis:** Starting with energy 2 raises a sufficiency question for three pushes. A conclusion needs the authored cost, replenishment rules, and which attempts spend energy; the resource label alone cannot settle it.
- **Competing options:** Increasing starting energy, changing cost, adding a recovery source, or choosing another route are distinct proposals with different consequences. None is automatically the right balance fix.
- **Useful next question:** Is scarcity intended to constrain this route, and can the relevant state actually occur? Keep that design-intent question separate from any hand calculation.

## Evidence and counterchecks

- A hand-computed route can reveal a conditional inconsistency but cannot establish how players choose routes or whether they perceive scarcity as interesting.
- For a later authorized model, specify action policy, failure handling, initial state, and recorded measures. A precise result under hidden behavior assumptions is still misleading.
- Inspect extremes as well as typical values. Zero available energy, full capacity, and a costly failed action may expose different rules than a successful route average.

## Common traps

- Do not infer a directed economy graph from an undirected reference list. State missing direction as unresolved.
- Do not conflate resource amount with resource identity or mechanic ownership. Tuning data and spending behavior have different homes.
- Numerical feasibility is not balance quality. A solvable route can still conflict with intended choices or feedback.

## Missing data and evidence limits

A missing resource extension does not prevent discussing a noun. Unauthored pool bindings, unknown quantities and unresolved direction remain explicit gaps; prose alone does not create native flow edges. Suggest an experiment without running it.

Native controls resolve supported formulas/units. Resource inspection retains enabling/debit/credit declarations, explicit identities and conditional structural witnesses, with source/limit coverage. Generic incidence links are not flow; witnesses do not establish feasibility, execution, gameplay frequency or final outcomes. Inspection does not authorize tuning or experiment execution. At most one accepted resource extension per noun remains a source invariant; generic schema validation and native semantic checks have distinct coverage.

## References

- [Game Mechanics: Advanced Game Design — publication record](https://research.hva.nl/en/publications/game-mechanics-advanced-game-design/)

Access checked 2026-10-19: Hogeschool van Amsterdam publication summary inspected; book and Machinations implementation not inspected.
