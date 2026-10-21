# Gameplay design patterns

Pack: `gameplay-patterns` · version 1
Attribution: Staffan Björk, Sus Lundgren, Jussi Holopainen (2003), and later Björk/Holopainen pattern work

Provisional application guide; pending human review. Source model and workbench procedure are distinguished below. Coverage is bounded to one paper, one methods page, and one inspected catalog entry, not the full catalog.

## Contents

- [Purpose and locus](#purpose-and-locus)
- [Structural vocabulary](#structural-vocabulary)
- [Pattern anatomy and relations](#pattern-anatomy-and-relations)
- [Two inspected pattern examples](#two-inspected-pattern-examples)
- [Inputs and interpretation](#inputs-and-interpretation)
- [Apply to a selected design slice](#apply-to-a-selected-design-slice)
- [Worked comparison and output](#worked-comparison-and-output)
- [Practical questions and counterchecks](#practical-questions-and-counterchecks)
- [Common mistakes and transfer limits](#common-mistakes-and-transfer-limits)
- [References](#references)

## Purpose and locus

Gameplay patterns make recurring interaction structures discussable across games and disciplines. Their purpose is not to prescribe a correct game. They connect a recognizable configuration with ways to realize it, likely consequences, contextual choices, and other patterns. A shared name helps recall an already-understood relationship; the name alone is not its meaning.

The 2003 paper deliberately departs from a pure problem–solution template. Game design includes creating possibilities, not only fixing defects. A pattern can describe an existing game, inspire an unfamiliar implementation, or expose a trade-off without identifying anything as broken. Patterns offer a vocabulary between an entire genre and one game's exact rules.

**Locus:** interaction among players, and between players and game components, at a stated scale. Unit of analysis is a recurring configuration of those interactions. It may span a single choice, a repeated encounter, or several play sessions. An object, theme, implementation function, or isolated rule is not automatically a pattern. Ask what participants can do, know, and affect through the arrangement.

Use this lens to:

- Explain how a selected rule arrangement resembles or differs from an inspected pattern.
- Compare alternative realizations of the same intended relationship.
- Trace how one configuration enables, changes, or conflicts with another.
- Make tacit design knowledge explicit without requiring established catalog terminology.

Do not use it to certify fun, infer player mastery or feelings, prescribe features, or score completeness. Narrative and presentation can matter to an interaction, but this interaction-centered model is not a complete account of either. Concept/player experience remains broader than individual MDA aesthetics; a pattern called Tension does not automatically establish an MDA aesthetic intention or an experienced emotion.

The paper proposes uses from ideation and concept refinement through analysis, comparison, and exploration of new media. It also acknowledges limited validation of these methods. This workbench uses a bounded comparison rather than the paper's suggested whole-collection scan or random-pattern ideation. No automatic framework sweep follows selection of this guide.

## Structural vocabulary

The paper presents two connected but separable tools: a structural framework for describing game components, and patterns for describing interaction involving those components. Patterns can be used with another structural framework. Workbench records therefore need not reproduce the paper's ontology.

At highest abstraction, **game instance**, **game session**, and **play session** delimit playing activity logically and temporally. The paper's short account does not fully define their boundaries; its companion structural-framework paper is not captured here. State the actual boundary used in a comparison instead of treating these terms as interchangeable or inventing a formal session hierarchy. For recovery analysis, distinguish continuing after an interruption from replacing current state with an earlier state.

The 2003 account groups components into three categories:

- **Bounding:** goals, rules, and game modes describe allowed and disallowed activities. Use this category to ask what constrains a configuration and under which mode it applies. It is not a category for room geometry alone.
- **Temporal:** actions, events, end conditions, evaluation functions, and closures describe execution over time. Trace a relevant sequence: what can be done, what happens, and how an outcome is evaluated or brought to a close. The paper lists these terms but does not supply full definitions of each here.
- **Objective:** players, interfaces, and physical or virtual game elements. “Objective” here does not mean goals or unbiased evidence. Tokens, avatars, cards, and movable objects are examples. Game elements have control/action structures governing how players affect state, and information structures governing what players know, including attributes and scores.

These categories provide different views of the same interaction, not a compulsory three-part form. For example, a hypothetical recovery rule has permission conditions, a restore sequence, and an interface acting on stored state. Omitting the information view can hide that the player remembers an outcome even after the game state is restored.

**Patterns versus concrete rules:** a pattern describes a reusable relationship; a concrete rule determines how one game realizes it. “Restore this saved state” still leaves open when saving is allowed, which state survives restoration, whether other players must agree, and what the player remembers. Pattern identification does not fill those rule gaps. Workbench `mechanics` remain owners of shared rules; pattern vocabulary does not redefine that role or claim the paper's critique of loosely used “mechanics” invalidates the local contract.

## Pattern anatomy and relations

The 2003 template supports reading and comparison, not mandatory record fields:

- **Name:** short mnemonic for a described concept. Recognizing a title is not enough to apply it.
- **Description and examples:** identify the interaction, its possible scales, and instances used to explain it. Examples anchor meaning; they do not prove every game with a similar feature behaves alike.
- **Consequences:** possible or likely effects, benefits, and costs. Read these as conditional claims to examine in context, not guaranteed outcomes.
- **Using the Pattern:** choices needed to realize a general arrangement in a particular game. Several implementations can share the same relationship.
- **Relations:** connect the pattern to other descriptions, implementation possibilities, and conflicts.

In the paper, a **superior pattern** is more abstract and may be realized through the given pattern; a **subpattern** can help realize the given pattern. “Superior” means neither better design nor higher priority. **Conflicting patterns** are difficult to combine, not necessarily logically impossible. The collection is a network, not a strict tree: one pattern can serve several more abstract patterns. The authors acknowledge that these relation types do not adequately express every case, including groups that help instantiate one another.

The later inspected **Save-Load Cycles** revision uses a richer relation vocabulary:

- **Can Instantiate / Can Be Instantiated By:** possible resulting characteristics / possible means of realizing this pattern. For example, the entry lists Extra Chances as a possible result and Save Files as a possible enabling structure.
- **Can Modulate / Can Be Modulated By:** changes to another pattern's operation / contextual structures that change this one's operation. The entry lists Real-Time Games in the first direction and Save Points in the second. Restricting save locations changes access to recovery without necessarily removing it.
- **Potentially Conflicting With:** arrangements whose intended effects may be weakened or complicated by this pattern, such as Irreversible Events or Multiplayer Games.
- **Possible Closure Effects:** heading is present but empty in this entry. Do not infer its formal meaning or invent content from that absence.

These meanings are supported by the inspected entry's prose and examples; no catalog-wide relation specification was inspected. Preserve its “can” qualifiers and direction. Do not silently equate this later vocabulary with every 2003 superior/subpattern relation. A listed relation is not a measured causal link, transitive proof, or authorization to import all connected patterns. Linked entries were not captured or reviewed.

## Two inspected pattern examples

### Paper Rock Scissors in the 2003 paper

The worked entry uses a cyclic, non-transitive advantage relationship: rock defeats scissors, scissors defeats paper, paper defeats rock. Its point is not having exactly three themed objects. Choices depend on what an opponent chooses; the paper discusses guessing, misleading opponents, and gathering information about their behavior.

The entry contrasts immediate resolution with long-term investments in different abilities. It connects repeated short games with opportunities to learn opponents' habits, and longer commitments with information-gathering arrangements. This shows why **structure plus information plus time scale** matters. A hypothetical game with three options but one universally advantageous option would not match that central cyclic relationship. A visible fixed opponent sequence also changes the inference problem, even if the advantage cycle remains.

The paper names Tension, Randomness, and Game Mastery among consequences or relations. Attribute these as the authors' account, not observed player states. Its claim that a player has no winning strategy should not become a theorem about every implementation: probabilities, payoffs, information, and opponent behavior can differ. Here, preserve the narrower comparison: no option defeats every other option in the stated cycle. Do not infer that learning an opponent's habits proves mastery or guarantees success.

### Save-Load Cycles, inspected revision 26347

This entry describes using saving and loading to obtain a desired game state, including repeated returns after an unwanted result. Merely preserving progress between visits is not evidence that a player engages in repeated outcome-seeking cycles.

Its implementation discussion connects state storage/restoration with Save Files or facilitators, save-location restrictions, interface access, and multiplayer agreement. Its consequences include interruptibility and planning between sessions, but also downtime and a break between the sequence the player experienced and the game's restored history. Restoring state need not erase knowledge gained on the abandoned attempt. The entry describes this as information carried between sessions and discusses retrying local segments and avoiding penalties.

Use these distinctions to compare **what is restored**, **what is retained**, and **when restoration is available**. Do not treat the entry's broad statements about removing tension or enabling smooth learning as established effects for every player. Limited checkpoints, retained costs, slow restoration, or unpredictable outcomes could change the claimed consequence. Those are hypothetical contextual counterchecks, not additional inspected catalog claims.

The entry identifies itself as an update to a pattern in *Patterns in Game Design* (2004). That book was not acquired; no book-to-wiki comparison is possible here. The methods wiki page also remains incomplete: Ideation, Conceptualization, and Prototyping say “To be written.” It does support distinguishing the pattern approach from this particular collection and permits a useful subset or locally developed vocabulary. The procedure below is a workbench adaptation, not a recovered missing method.

## Inputs and interpretation

Roles: `mechanics`, `systems`, `dynamics`.

Inspection topics: MDA relationships and relevant source records.

Read only inputs needed for the selected question:

- Concept/mode and intended experience, or an explicitly open exploration question.
- Actual rules and system arrangement; relevant interactions, information/feedback, resources, and named `control-vars` where they affect the relationship.
- Lifecycle and retained rationale, especially rejected alternatives and proposed additions.
- Dynamic claims and any experiment evidence, keeping intended, predicted, and observed behavior distinct.
- The actual selected pattern entry and its version. If unavailable, use a descriptive working phrase or mark resemblance unverified; do not reconstruct authority from a title or source card.

Missing context limits the comparison; it does not justify inventing rules. A missing declared relationship is not proof that an action is unavailable. In game-explorer mode, an absent destination is not a defect; compare possible leads without adopting one.

Keep the existing 14 roles. No required pattern tags, new schemas, or separate measures, decisions, or skills collection. Existing project decisions remain user-owned. Experiments own hypotheses, local measures, methods, observations, and interpretation. Record reading and guide application do not authorize record edits, experiments, implementation, or adoption.

## Apply to a selected design slice

1. **Frame the comparison.** Name one interaction, its scale, and the question patterns should clarify. State concept direction or exploration context. Done: scope excludes an unrequested game-wide inventory.
2. **Describe the arrangement without labels.** Trace permission, state transition, information available, and relevant repetition/recovery boundaries. Cite actual rule owners and lifecycle. Done: another reader can understand the configuration without knowing pattern names.
3. **Read and bound the source analogy.** Identify entry/revision, defining relationship, implementation choices, and relevant consequence claims. Done: at least one substantive match and mismatch are explicit; unavailable details remain unknown.
4. **Trace selected relations.** Explain which concrete rule would enable, modify, or conflict with the chosen relationship. Use only relevant connections; do not fill every relation heading. Done: each claimed connection has a direction, condition, and source or hypothetical label.
5. **Construct a fair contrast.** Change one meaningful arrangement while preserving the comparison's intention where possible. Explain costs and possible failure cases. Check prior rejection rationale before presenting a renamed alternative. Done: current design and proposed contrast remain separate; neither is adopted.
6. **Return a bounded artifact.** Supply source facts, analogy, contrast, conditional consequence, countercheck, and evidence gap. If a saved review is requested, follow the review contract; otherwise chat is sufficient. Done: reader can challenge the interpretation without confusing it with a game fact or executed test.

Stop when the selected question is explained or missing inputs block a defensible comparison. Do not continue into catalog harvesting. If later investigation is authorized, define its question and local evidence in an experiment; this application itself neither designs a compulsory trial nor executes one.

## Worked comparison and output

**Selected slice:** recovery after a committed crate push. Compare current restart with a hypothetical local save/restore option, using Save-Load Cycles as a source analogy.

**Authored tutorial facts, not play observations:**

- [Concept](../examples/crate-puzzle/game-design/concept.md), “Razor and purpose” and “Intended experience”: `game-explorer`; candidate direction is thoughtful commitment and replanning. Player story is explicitly hypothetical, not a room specification or participant quote.
- [Crate pushing](../examples/crate-puzzle/game-design/mechanics/crate-pushing.md): successful push moves one cell and spends `push-cost`; blocked or insufficient-energy pushes move nothing and cost nothing. Pulling is unavailable.
- [Pulling proposal](../examples/crate-puzzle/game-design/verbs/pull.md), “Disposition and rationale”: rejected to retain irreversible-push planning. This is an actual stated reason, not an open list of guessed reasons.
- [Restart](../examples/crate-puzzle/game-design/mechanics/challenge-restart.md): accepted rule reconstructs starting arrangement. Missing challenge/dynamic links are a documentation question, not evidence restart is unavailable.
- [Careful positioning](../examples/crate-puzzle/game-design/dynamics/careful-positioning.md): prediction, not observation. [Crate color](../examples/crate-puzzle/game-design/mechanics/crate-color.md) remains proposed, not adopted recovery feedback.

**Example review artifact — agent interpretation; no source edits:**

> **RECOVERY-SCOPE — possible opportunity / unverified hypothesis.** Scope: one push and the subsequent recovery choice. Source state: tutorial Markdown as inspected; no playable layout or player observations.
>
> **Match:** current restart and the inspected Save-Load Cycles entry both concern replacing current state to permit another attempt. Both can leave the player knowing something learned on the abandoned attempt.
>
> **Mismatch:** the authored restart reconstructs the challenge's start. No arbitrary save point, quick-load interface, or repeated saving to secure a favorable intermediate outcome is specified. Therefore do not tag current design as a verified implementation of the full catalog entry.
>
> **Hypothetical contrast:** allow a player-selected snapshot before a push and restore that snapshot's arrangement and energy afterward. This is a new illustration, not an existing rule or adopted proposal. Interface, access restrictions, and other restored state would still need definition.
>
> **Conditional consequence:** local restoration could reduce replay of already-understood setup and permit low-cost attempts. It could also weaken commitment within an attempt, directly engaging the rationale for rejecting pulling. Snapshot restoration is structurally different from pulling, but that difference alone does not answer the shared commitment concern.
>
> **Countercheck:** full restart already provides another attempt. If the meaningful difference is only repeated setup effort, compare recovery granularity rather than “recovery versus no recovery.” Retained knowledge might support replanning under either arrangement; repeated attempts alone would not show understanding or satisfaction.
>
> **Evidence needed:** actual room arrangement, defined restore scope, and evidence about how players choose and explain attempts. The pattern suggests questions, not a verdict. No severity of design defect established; no adoption recommended by this comparison alone.

[Energy/planning investigation](../examples/crate-puzzle/game-design/experiments/energy-encourages-planning.md) has no defined measures or observations and is not executed. [Route-cost investigation](../examples/crate-puzzle/game-design/experiments/route-cost.md) is also unexecuted; its analytical timing reference assumes a prescribed legal route and correct decisions. Neither validates this recovery comparison or demonstrates player learning. Any later investigation belongs in an authorized experiment, not a new measures collection or this guide.

## Practical questions and counterchecks

Use questions that discriminate between arrangements rather than reward familiar names:

- If the pattern name disappeared, could the rule trace still explain the match? If not, replace naming with description.
- Which exact permission, information boundary, or repetition interval makes the relationship possible? What changes if it is removed?
- Does the consequence require players to use an available option repeatedly, or does availability itself change choices? Do not conflate these claims.
- Which state resets, which cost survives, and which knowledge cannot be reset? Does “irreversible” mean within one attempt or across all attempts?
- Is a listed conflict about feasibility, coordination, or loss of an intended effect? A multiplayer agreement problem differs from a single-player penalty becoming avoidable.
- Does the contrast preserve the same goal and context, or quietly compare different experiences? Keep broader concept intent separate from an MDA aesthetic label.
- What finding would weaken the interpretation? Look for non-use, unexpected costs, an alternative explanation, or a contextual mismatch, not only confirming examples.

For later authorized comparisons, keep task and relevant presentation comparable. Faster recovery could come from interface polish rather than a different rule relationship. An observed retry count alone cannot establish motivation, capability, mastery, or emotion. Such interpretations require their own evidence and limitations.

## Common mistakes and transfer limits

- **Catalog as shopping list:** adding individually attractive patterns can introduce conflicting incentives or unnecessary rules. Compare one relevant relationship, not coverage totals.
- **Relations as deterministic chain:** “can instantiate” does not mean “always causes,” and a linked name does not import a full verified entry. Explain the local mechanism and uncertainty.
- **Abstraction as priority:** superior/subpattern labels describe realization relationships, not which design should win.
- **Renamed rejected option:** confront retained rationale. Local restoration and pulling differ in operation but may both affect commitment.
- **Source template as data model:** structural categories and entry sections organize thought. They are not required schemas, computed recognizers, or compulsory category filling.
- **Availability as behavior:** a save function does not prove repeated save/load behavior; an accepted prediction does not become an observation.

The paper reports pattern harvesting, interviews, prototype/workshop experience, and early validation claims, while explicitly calling the work initial and further validation necessary. It also identifies uncertain pattern granularity, difficult navigation, and insufficient relation types. Treat those as methodological limits, not defects solved by giving every rule a tag. The methods page's defense of non-orthodox subsets supports local descriptions; it does not establish that every newly coined term is a useful published pattern.

Transfer requires checking scale, information, state persistence, medium, and social context. Named examples in the paper and wiki are source illustrations, not games independently inspected for this guide. Catalog references to patents are historical leads, not legal advice or clearance.

No full-catalog coverage or readiness claim. Excluded sources include the structural-framework companion paper, the 2004 book, and linked pattern/game pages. In particular, missing/redlink destinations such as Reversibility were not verified as independent entries. Their names may be discussed only as relations reported by Save-Load Cycles.

Figure/conversion limit: the paper capture exported 0 images; capture index reports no substantive figure identified, but no page screenshots or facsimile inspection occurred. No diagram-derived categories or relationships are asserted here. Wiki captures contain no substantive article images. Paper line breaks, page furniture, and wiki raw reference markup remain conversion artifacts. Do not fill genuine source gaps from memory.

## References

Actual local captures reviewed; descriptions above are independently authored, not reproduced entries. Captures are private offline reference material, not publication-cleared resources.

- Staffan Björk, Sus Lundgren, Jussi Holopainen (2003), [*Game Design Patterns* — full local paper](../references/gameplay-patterns/captures/game-design-patterns-2003.docling/game-design-patterns-2003.md). Locators: “The Need for a Common Language for Games”; “Development of Our Model”; “An Interaction-Centric Model for Games” / “Structural Framework” and “Game Design Patterns”; “Paper Rock Scissors”; “Applying Game Design Patterns”; “Discussion,” especially validation, collection granularity, sub/superior relations, navigation, and stereotyping. Copyright notice: all rights reserved.
- Gameplay Design Patterns wiki, maintained by Staffan Björk and Jussi Holopainen; individual page authorship not established (undated, revision 91), [*Pattern-Based Methods* — full local page](../references/gameplay-patterns/captures/pattern-based-methods.md). Locators: “Using Patterns for Analysis”; “Using Patterns for Designing” including unfinished subsections; “Using Patterns for Becoming Aware of Ones own Knowledge”; “The Question of Orthodoxy.” No content license established.
- Gameplay Design Patterns wiki, same maintainer attribution and unresolved page authorship (undated, revision 26347), [*Save-Load Cycles* — full local entry](../references/gameplay-patterns/captures/save-load-cycles-revision-26347.md). Locators: opening definition and examples; “Using the pattern” including diegetic/interface/narration aspects; “Consequences”; every “Relations” subsection; “History” and book reference. No content license established; linked entries and cited book not reviewed.
- [Capture index, provenance, exclusions, conversion and rights context](../references/gameplay-patterns/README.md). Three bounded full-text captures dated 2026-10-21; independent notes/cards are separate from source text. Local retention does not authorize redistribution.
