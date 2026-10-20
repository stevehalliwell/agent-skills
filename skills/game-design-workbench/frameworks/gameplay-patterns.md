# Gameplay design patterns

Pack: `gameplay-patterns` · version 1
Attribution: Staffan Björk, Sus Lundgren, Jussi Holopainen (2003), and later Björk/Holopainen pattern work

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

Compare a selected structure with a cited pattern or contrasting configuration. Use patterns as exploratory vocabulary, not required tags.

## Inputs and query support

Roles: `mechanics`, `systems`, `dynamics`.

Inspection topics: MDA relationships and relevant source records.

## Working concepts

- **Pattern as relationship:** A useful pattern describes a recurring relationship among rules and activities in a context. Sharing a name, theme, or object does not establish that the same relationship exists.
- **Context and consequence:** Explain when a configuration helps a design intention and what costs it introduces. Patterns are not unconditional best practices; context can reverse an expected benefit.
- **Pattern interaction:** Two useful structures can reinforce, neutralize, or conflict with each other. Review the combined rules rather than assuming that adding individually attractive patterns improves the whole.
- **Vocabulary and attribution:** Distinguish an inspected published pattern from an original working description. Use a descriptive local phrase when source evidence is unavailable; do not invent catalog authority for it.

## Apply to selected slice

1. Start with the selected rule relationship and intended behavior. Read relevant mechanics, system grouping, dynamic hypotheses, and any existing project decisions. State what problem the comparison is meant to clarify.
2. If a specific published pattern is named, inspect that entry before making detailed attribution. Record the source, context, and relevant relationship. Otherwise label the comparison an unverified resemblance or original design description.
3. Map the relationship onto actual source rules. Identify a meaningful match and a meaningful mismatch. Avoid treating a shared label as a sufficient comparison.
4. Construct one contrasting configuration serving the same intention. Compare consequences, integration costs, and conflicts with existing rules. Revisit rejected alternatives without silently reviving or adopting them.
5. Return the narrow comparison, uncertainty, and a fair way to examine the alternatives. Retain each proposal and rationale separately. A pattern label does not authorize implementation or replacement of an existing decision.

## Original prompts

1. What relationship among rules makes this resemble the proposed pattern beyond sharing a name?
2. Which context makes that resemblance useful, and which difference makes it misleading?
3. What other pattern configuration could support the same intended behavior?
4. Which interaction between two patterns could reverse the expected effect?
5. Was a similar option previously rejected, and does the old rationale still apply?
6. What source example or prototype would let the designer compare these alternatives fairly?

## Worked example

- **Authored facts:** Pulling was rejected with rationale; crate-color remains proposed. Deliberate placement is part of the puzzle discussion. A new recovery idea must not erase this memory.
- **Original comparison:** Describe costly commitment and bounded recovery as working configurations, not verified catalog entries. Ask which rule relationship creates the intended consequence of a placement.
- **Competing option:** An explicitly proposed recovery action may differ from unrestricted pulling. Compare its conditions and cost against the original rejection rather than assuming every reversible action is equivalent.
- **Useful next question:** Which aspect of pulling was rejected: reversing commitment, expanding the action set, presentation cost, or something else stated in source? That rationale determines whether the new proposal is genuinely distinct.

## Evidence and counterchecks

- A published example illustrates a relationship in its own context; it does not prove that transplanting the configuration will create the same behavior.
- When comparing prototypes is later authorized, hold the intended problem and observation question stable. A more polished candidate can otherwise appear better for unrelated reasons.
- Record cases where the predicted consequence does not occur. A mismatch can reveal an important contextual rule rather than a failed pattern implementation.

## Common traps

- Do not assign mandatory pattern tags to every mechanic. Vocabulary should clarify the selected question, not expand the data model.
- A pattern catalog is not a feature shopping list. Adding several configurations can create conflicting incentives and unnecessary rules.
- Do not re-propose a rejected option under a new label without confronting its recorded rationale.

## Missing data and evidence limits

Proceed from described rule relationships without pattern labels. If external pattern evidence is unavailable, frame resemblance as an unverified hypothesis.

No exhaustive pattern catalog, automatic recognition, or copied pattern entries. Cite a specific external pattern only after inspecting that source; title similarity is not evidence of implementation.

## References

- [Game Design Patterns — DiGRA](https://dl.digra.org/index.php/dl/article/view/60)
- [Gameplay Design Patterns catalog](https://gdp.cse.chalmers.se/index.php?title=Main_Page)

Access checked 2026-10-19: DiGRA abstract inspected. External catalog is a further-reading link, not a bundled database or a claim of full inspection.
