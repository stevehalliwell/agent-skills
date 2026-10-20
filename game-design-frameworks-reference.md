# Game Design Workbench — Frameworks & Research References

**Purpose:** Reference list for the agent developing the local-first game-design skill, C99 tools, analysis CLIs, simulation experiments, and visualisers.

**Design principle:** One set of human-editable Markdown/frontmatter records, viewed through many interchangeable design frameworks. Each framework is a way to **query, interpret, visualise, or question** the same game; it is **not** a separate database or a required set of fields on every record.

This document collects the frameworks and adjacent methodologies discussed so far. Links preferentially point to the original paper, author's own explanation, academic record, or publisher. They are for further reading, not permission to reproduce books, lens cards, proprietary questionnaires, or diagrams wholesale.

## At a glance

| Framework or reference | Primary perspective | Best fit in workbench |
|---|---|---|
| MDA — Mechanics, Dynamics, Aesthetics | How rules produce behaviour and experiences | Relationship model, trace/coverage view |
| Noun–verb game grammar / meaningful play | Possible actions, targets, consequences, feedback | Interaction matrix, gap finder |
| Game Mechanics / Machinations approach | Feedback loops, resources, internal economies | Small C99 simulations and plots |
| Gameplay design patterns | Recurring mechanic/interaction configurations | Pattern reference and exploratory lens |
| Daniel Cook's skill atoms | Learning loops and prerequisite chains | Skill graph, progression diagnostics |
| Raph Koster's Theory of Fun | Pattern discovery, learning, mastery, repetition | Learning-depth questions |
| Flow theory | Challenge relative to capability | Challenge-vs-skill timeline |
| GameFlow | Enjoyment conditions, including goals and feedback | Evaluative checklist/lens |
| Cognitive Load Theory | Demands on limited mental processing | Simultaneous novelty/dependency view |
| Miller's 7 ± 2; Cowan's 3–5 chunks | Capacity and chunking | **Cautionary** working-memory lens |
| Zone of Proximal Development / scaffolding | Challenge just beyond independent ability, with support | Learning-gap and support analysis |
| Self-Determination Theory | Autonomy, competence, relatedness | Motivation/experience support matrix |
| Quantic Foundry motivation model | Multi-factor player preferences | Optional audience-motivation lens |
| Nicole Lazzaro's Four Keys to Fun | Different routes to positive game emotion | Experience/feeling lens |
| Bartle player types | Patterns of behaviour in multiplayer MUDs | Historical/genre-specific question pack |
| Caillois's forms of play | Competition, chance, role-play, vertigo | Forms-of-play lens |
| Jesse Schell's Book of Lenses | Reflective questions from different perspectives | Contextual question packs |
| Tracy Fullerton's Game Design Workshop | Player experience goals, prototypes, playtesting | Intentions, experiments, evidence |

---

## A. Game structure, interactions, and emergence

### A1. MDA — Mechanics, Dynamics, Aesthetics

**Authors:** Robin Hunicke, Marc LeBlanc, Robert Zubek (2004).

**Primary source:** [MDA: A Formal Approach to Game Design and Game Research — AAAI](https://aaai.org/papers/ws04-04-001-mda-a-formal-approach-to-game-design-and-game-research/)

**Idea:** Mechanics are the components/rules; dynamics are the runtime behaviours those mechanics produce in combination; aesthetics are the intended experiential responses. The familiar aesthetic vocabulary includes sensation, fantasy, narrative, challenge, fellowship, discovery, expression, and submission.

**Workbench use:** Show linked chains such as `mechanic → dynamic → aesthetic`; find unconnected intended experiences, dynamics with no identified supporting mechanics, and mechanics without a stated experiential purpose. Display both forward and reverse traces.

**Inputs:** Mechanics, rules, dynamics, intended aesthetics, hypotheses, observation/evidence links.

**Caution:** A declared link to an aesthetic is a **designer hypothesis**, not proof of what players feel. “Systems” and “dynamics” should not be treated as synonyms: systems organise rules; dynamics emerge when those rules operate.

### A2. Noun–verb game grammar and meaningful play

**Related source:** [Rules of Play: Game Design Fundamentals — Katie Salen Tekinbaş & Eric Zimmerman, MIT Press](https://mitpress.mit.edu/9780262240451/rules-of-play/)

**Idea:** Represent the game in terms of objects/nouns, actions/verbs, interaction conditions, outcomes, and feedback. Salen and Zimmerman provide related concepts of meaningful play, rules, systems, and discernible/integrated action outcomes. The proposed **noun–verb interaction matrix is a workbench modelling technique**, not a claim that the book prescribes this specific matrix.

**Workbench use:** Noun–verb tables, interaction graph, implied-but-undefined combinations, unused interactions, inconsistent consequences for similar actions, inaccessible feedback, and interaction coverage by level/challenge.

**Inputs:** Nouns, verbs, interaction records, preconditions, state changes, feedback, challenge usage.

**Caution:** A blank cell can mean unknown, deliberately excluded, impossible, or not yet considered. Do not treat every empty interaction as a design defect.

### A3. Game Mechanics: Advanced Game Design / Machinations approach

**Authors:** Ernest Adams and Joris Dormans (2012).

**Sources:** [Game Mechanics: Advanced Game Design — academic publication record](https://research.hva.nl/en/publications/game-mechanics-advanced-game-design/) · [Book listing — Google Books](https://books.google.com/books/about/Game_Mechanics.html?id=_Azio0txIdAC)

**Idea:** Resource flows, sources/sinks, converters, feedback loops, balancing, and simple executable models can expose surprising dynamics before the full game exists.

**Workbench use:** Connect a named dynamic or expectation to a tiny C99 experiment. Run deterministic steps, seeded Monte Carlo trials, parameter sweeps, and plots. Trace the relevant controls back to mechanics/rewards/spawn rates.

**Inputs:** State, rules, controls, random distributions, seeded experiments, probes/measures, expectations.

**Example:** “Players typically reach rank 3 in 20–30 minutes” depends on XP thresholds, spawn rate, XP sources, combat pace, and reward randomness. Run a distribution of time-to-rank-3 outcomes, not just one average.

**Caution:** This is **experimenting on selected systems**, not recreating a full game engine. Simulated results depend heavily on assumptions about player behaviour.

### A4. Gameplay design patterns

**Authors:** Staffan Björk, Jussi Holopainen and collaborators.

**Sources:** [Game Design Patterns — DiGRA paper](https://dl.digra.org/index.php/dl/article/view/60) · [Gameplay Design Patterns collection](https://gdp.cse.chalmers.se/index.php?title=Main_Page) · [Patterns in Game Design — book overview](https://books.google.com/books/about/Patterns_in_Game_Design.html?id=IFQfyODK4wAC)

**Idea:** Recurring game structures and interaction patterns form a reusable vocabulary, including relationships between patterns.

**Workbench use:** Link mechanics/dynamics to known patterns; search for related or contrasting patterns; ask what combinations are unexplored. Helpful as a **reference library or exploratory lens**, not a required ontology.

**Inputs:** Mechanic relationships, patterns, examples, optional designer annotations.

**Caution:** Pattern matching is interpretive. Do not automatically imply a game contains a pattern merely because its terminology overlaps.

---

## B. Learning, challenge, and player capability

### B1. Skill atoms / skill chains — Daniel Cook

**Primary source:** [The Chemistry of Game Design — Lostgarden](https://lostgarden.com/2021/03/13/the-chemistry-of-game-design-2/)

**Idea:** Games expose players to interaction/feedback loops that they learn and combine. Skills can be not-yet-exercised, developing, mastered, actively practised, or no longer interesting to exercise. More complex skills depend on earlier learning.

**Workbench use:** Skill prerequisite graph; where skills are introduced, exercised, combined, tested and revisited; orphan skills; missing prerequisites; abrupt combinations; potential intermediate challenges.

**Inputs:** Skill atom, prerequisites, mechanic/feedback loop, evidence of understanding, related challenges/levels, assumed mastery.

**Caution:** A level that contains an action does not prove the player learned it; exposure, understanding, practice, mastery, and transfer are different concepts.

### B2. A Theory of Fun for Game Design — Raph Koster

**Sources:** [A Theory of Fun — official site](https://www.theoryoffun.com/) · [Raph Koster's book and earlier presentation](https://www.raphkoster.com/gaming/theoryoffun.shtml)

**Idea:** Pattern recognition, learning, discovery, and the acquisition of mastery are important to many kinds of game enjoyment. Repetition can continue to require effort without introducing much new understanding.

**Workbench use:** Ask whether later challenges teach new patterns, combine learned capabilities, demand new strategies, or merely increase numeric difficulty. Identify long progressions with no new connections between skill atoms.

**Inputs:** Skills, combinations, challenge progression, novelty, intended learning, player evidence.

**Caution:** Not an algorithm that can compute “fun”; games also support other experiences besides mastery and learning.

### B3. Flow theory — Mihaly Csikszentmihalyi

**Primary source:** [Flow: The Psychology of Optimal Experience — publisher](https://www.harpercollins.com/products/flow-mihaly-csikszentmihalyi)

**Idea:** Deep engagement is more likely when perceived challenges and skills are well matched, with clear goals and understandable feedback, among other conditions.

**Workbench use:** Plot an **estimated** challenge-demand curve against the **assumed** player-capability curve across levels/encounters. Highlight possible spikes, long plateaus, and moments for recovery.

**Inputs:** Encounter demands, skill prerequisites, assumed mastery, challenge estimates, goals/feedback.

**Caution:** A designer-authored challenge number cannot prove flow or predict a particular player's anxiety/boredom. Treat the visualisation as an assumption explorer and compare with playtests.

### B4. GameFlow — Sweetser and Wyeth

**Primary source:** [GameFlow: A Model for Evaluating Player Enjoyment in Games — ACM DOI](https://doi.org/10.1145/1077246.1077253)

**Idea:** An evaluative model for player enjoyment, drawing on conditions including concentration, challenge, skills, control, clear goals, feedback, immersion, and social interaction.

**Workbench use:** A checklist/question pack applied to a level, mechanic or playtest session; flag missing goals, unclear feedback or excessive simultaneous demands even when the difficulty progression looks reasonable.

**Inputs:** Goals, controls, feedback, challenge, skill expectations, player observations, social features where relevant.

**Caution:** This is a **different framework from Flow theory**; avoid collapsing its more specific evaluation criteria into a single “flow score.”

### B5. Cognitive Load Theory — John Sweller

**Primary source:** [Cognitive Load During Problem Solving: Effects on Learning — Cognitive Science, 1988](https://onlinelibrary.wiley.com/doi/10.1207/s15516709cog1202_4)

**Idea:** A learner's limited working capacity interacts with prior knowledge and schemas. A task may be possible but difficult to learn from when too many unfamiliar operations must be coordinated.

**Workbench use:** Highlight novel skills required simultaneously; new combinations of individually mastered skills; switching between mental tasks; and cases where necessary information must be memorised instead of remaining visible.

**Inputs:** Skill dependencies, familiarity assumptions, concurrent demands, information/feedback availability, external aids.

**Caution:** Do not pretend that a count of mechanics is a scientifically calibrated cognitive-load measurement. Use “possible overload / inspect” rather than a hard failure.

### B6. Miller's “7 ± 2,” Cowan's working-memory research, and chunking

**Primary sources:** [George A. Miller, The Magical Number Seven, Plus or Minus Two (1956), full text](https://psychclassics.yorku.ca/Miller/) · [Nelson Cowan, The Magical Number 4 in Short-Term Memory (2001)](https://pubmed.ncbi.nlm.nih.gov/11515286/) · [Cowan's retrospective on Miller (2015)](https://pmc.ncbi.nlm.nih.gov/articles/PMC4486516/)

**Idea:** Working-memory limits are real, but **7 ± 2 is not a universal capacity threshold for game mechanics**. Later work highlights smaller estimates (often around 3–5 unfamiliar chunks under particular experimental conditions). Prior learning allows multiple elements to be represented as one familiar chunk.

**Workbench use:** Contrast beginners' independent demands against expert “chunked” capabilities. Examine how practice and skill combinations change the effective amount a challenge asks the player to actively manage.

**Inputs:** Active concepts, novelty, prior mastery, learned composites, visible cues/external memory.

**Caution:** Never implement “more than seven things = bad design” or “more than four things = impossible.” The number depends on task, knowledge, attention, and measurement method.

### B7. Zone of Proximal Development (ZPD) and scaffolding

**Sources:** [Zone of Proximal Development — Springer encyclopedia](https://link.springer.com/rwe/10.1007/978-3-319-98390-5_60-1) · [Vygotsky ZPD — Oxford Reference](https://www.oxfordreference.com/display/10.1093/oi/authority.20110803133528287) · [Wood, Bruner & Ross, The Role of Tutoring in Problem Solving (1976)](https://doi.org/10.1111/j.1469-7610.1976.tb00381.x)

**Idea:** Distinguish what a learner can do independently from what becomes possible with suitable support. Scaffolding introduces temporary assistance that can later be removed. ZPD and the later scaffolding literature are related, not interchangeable terms.

**Workbench use:** Find challenges that might be achievable with hints, demonstrations, safe practice, reduced constraints or intermediate lessons; highlight skill leaps lacking any offered support.

**Inputs:** Prerequisites, challenge demands, assistive feedback/hints, demonstration, progression order, assumed player capability.

**Caution:** This originated in learning and developmental theory; game-design use is an analogy/hypothesis, not a diagnosis of the player.

---

## C. Motivation, emotion, and forms of play

### C1. Self-Determination Theory (SDT)

**Authors:** Edward Deci, Richard Ryan and collaborators.

**Sources:** [Self-Determination Theory — official theory overview](https://selfdeterminationtheory.org/theory/) · [Ryan & Deci, Self-Determination Theory and the Facilitation of Intrinsic Motivation (PubMed)](https://pubmed.ncbi.nlm.nih.gov/11392867/)

**Idea:** Autonomy, competence and relatedness are central psychological needs. Their support or frustration can shape motivation.

**Workbench use:** Associate design intentions and observed play with meaningful choice, opportunities to demonstrate competence, and relationships or social connection. Compare intended motivations with actual mechanic/dynamic support.

**Inputs:** Decisions, constraints, feedback, mastery opportunities, social interactions, audience intentions, playtest evidence.

**Caution:** These are not independent “fun point” scores. Player needs and emotional outcomes are not directly inferred from a mechanic's existence.

### C2. Quantic Foundry Gamer Motivation Model

**Source:** [Gamer Motivation Model — Quantic Foundry](https://quanticfoundry.com/gamer-motivation-model/) · [How We Developed the Gamer Motivation Profile v2](https://quanticfoundry.com/2015/07/20/how-we-developed-the-gamer-motivation-profile-v2/)

**Idea:** Twelve measured motivations in six paired groups: **Action** (Destruction, Excitement), **Social** (Competition, Community), **Mastery** (Challenge, Strategy), **Achievement** (Completion, Power), **Creativity** (Discovery, Design), **Immersion** (Fantasy, Story). These provide a multidimensional profile, not one player “type.”

**Workbench use:** Optional motivation-support matrix crossing intended player interests with mechanics/dynamics/challenges. Ask whether the game's main promises are reflected in the available player activities.

**Inputs:** Target audience hypotheses, intended motivations, mechanics/dynamics, rewards, observed player responses.

**Caution:** The published model is empirical but its questionnaire and derived scoring system are not ours to reproduce without permission. Use the published vocabulary with attribution and an independently authored question pack.

### C3. Nicole Lazzaro's Four Keys to Fun

**Primary source:** [The 4 Keys 2 Fun — Nicole Lazzaro / XEODesign](https://www.nicolelazzaro.com/the4-keys-to-fun/)

**Idea:** Four ways games may evoke positive emotion: **Hard Fun**, **Easy Fun**, **Serious Fun**, and **People Fun**.

**Workbench use:** Qualitative lenses asking where challenge and triumph, exploration and curiosity, personally meaningful experiences, and social amusement occur. Link examples to dynamics and observed player reactions.

**Inputs:** Dynamics, aesthetics, player interactions, discovery moments, playtest observations.

**Caution:** Do not conflate these categories with MDA's eight aesthetics or Koster's learning-centred account; they offer different perspectives. Author original questions rather than copying proprietary materials.

### C4. Bartle's player types

**Author:** Richard Bartle (1996).

**Source:** [Hearts, Clubs, Diamonds, Spades: Players Who Suit MUDs — original text mirror](https://www.majormud.com/pwsm.html)

**Idea:** Achievers, Explorers, Socializers and Killers emerged as a model of participation in **multiplayer MUDs**, emphasising player/world orientation and acting/interacting.

**Workbench use:** Optional historical or multiplayer-specific question pack about activities the game encourages and tensions between different kinds of participation.

**Inputs:** Player activities, social mechanics, exploration, competition, rewards.

**Caution:** Not a universal personality classification or a requirement to cater to all four types. Especially weak as a default lens for solo puzzle games.

### C5. Roger Caillois's forms of play

**Source:** [Man, Play and Games — publisher/book listing](https://books.google.com/books/about/Man_Play_and_Games.html?id=bDjOPsjzfC4C)

**Idea:** Four broad forms of play: **agôn** (competition), **alea** (chance), **mimicry** (simulation/role-play), **ilinx** (vertigo/disorientation); also the contrast between open-ended **paidia** and structured **ludus**.

**Workbench use:** Ask which forms dominate a game or moment, whether random elements support or undermine mastery, and whether the player has opportunities for freeform play versus rule-bound challenge.

**Inputs:** Random mechanics, competitive systems, roles, movement/sensory effects, player freedoms, constraints.

**Caution:** A vocabulary for interpreting play, not a complete player motivation profile or testable numerical score.

---

## D. Design process, critique, and evidence

### D1. The Art of Game Design: A Book of Lenses — Jesse Schell

**Primary source:** [The Art of Game Design / Book of Lenses — Schell Games](https://schellgames.com/art-of-game-design/)

**Idea:** Examine one design from many perspectives by asking focused questions about experience, challenge, interaction, balance, community, economy, and more.

**Workbench use:** Contextual **question packs**. Select a mechanic, skill, level, dynamic or aesthetic and receive relevant questions. Some can invoke deterministic queries; others necessarily ask the designer to reason or gather player evidence.

**Inputs:** Whatever design records are in scope, plus unanswered questions/design decisions.

**Caution:** A lens is generally not a schema field or a numerical validator. For a free/open skill, **write original questions inspired by the methodology** and link to the source; don't reproduce the complete copyrighted deck/book.

### D2. Game Design Workshop / playcentric design — Tracy Fullerton

**Sources:** [Game Design Workshop — official book site](https://www.gamedesignworkshop.com/) · [Tracy Fullerton's publications](https://www.tracyfullerton.com/writing) · [Publisher page](https://www.routledge.com/Game-Design-Workshop-A-Playcentric-Approach-to-Creating-Innovative-Games/Fullerton/p/book/9781032607009)

**Idea:** Begin with intended player experience and design goals, prototype, observe play, test assumptions, and iterate. Structured playtesting is central to design rather than something reserved for late production.

**Workbench use:** Represent an intention or hypothesis, link the mechanics/dynamics intended to support it, connect tests/observations/results, and track resulting changes to the design.

**Inputs:** Design pillars and hypotheses, prototype identifiers, test plans, observations, evidence, design decisions, revisions.

**Caution:** The workbench should distinguish **designer intent**, **modelled/simulated result**, and **observed player behaviour**. None substitutes for the others.

---

## E. Additional analysis perspectives discussed (not separate theories to implement wholesale)

These are recurring perspectives we discussed alongside the named frameworks. They can be implemented with the same shared data and a small number of generic views.

| Perspective | Example questions / views | Framework connections |
|---|---|---|
| **Affordances and feedback** | Can the player tell what is possible? Can they observe what happened and why? | Meaningful play, MDA, GameFlow, skill atoms |
| **Agency and meaningful choice** | Are there viable alternatives, different consequences, dominant strategies? | SDT autonomy, meaningful play, Schell |
| **Failure and recovery** | What is lost on failure? Can the player experiment safely? Is retry friction obstructing learning? | Skill atoms, flow, cognitive load |
| **Level/challenge progression** | Introduced vs required skills; pace of combinations; opportunity for mastery or transfer | Skill atoms, ZPD, flow, Koster |
| **Game feel and responsiveness** | Do input and feedback support the desired interpretation of each action? | MDA, GameFlow, Schell |
| **Design pillars / intentions / hypotheses** | Why does the mechanic exist? Which dynamic and player feeling does it support? | MDA, Fullerton |
| **Playtest observation and evidence** | What did players do, understand, misunderstand, or report? | Fullerton, GameFlow, motivation lenses |
| **Economy, progression and randomness** | Do specified rules meet expected outcomes across seeds and behaviours? | Adams/Dormans, MDA |
| **Cross-framework tension** | One lens suggests simplification while another suggests more discovery. What's the trade-off? | Any pair or group of lenses |

---

## F. Implementation guidance for the agent

### F1. Shared dataset; independent lenses

Maintain human-editable Markdown/frontmatter source records. Use Attendant's collection/reference/query capabilities where helpful, without making the visualisers dependent on a particular task manager, ORM, database or long-running service.

Possible base record categories: `nouns`, `verbs`, `interactions`, `mechanics`, `systems`, `dynamics`, `aesthetics`, `skills`, `challenges`, `levels`, `controls`, `measures`, `expectations`, `experiments`, `hypotheses`, `observations`, and `decisions`. These are starting vocabulary, **not mandatory types for every game**.

Framework-specific classifications should generally be derived, attached as optional annotations, or requested when needed—not mandated on every file.

### F2. Every analysis needs a machine-readable counterpart

Any finding visible in a matrix, dependency graph, timeline, coverage plot, or simulation chart should also be available to the agent through a CLI or structured export. The visualiser and CLI should use the same C99 analysis logic where practical.

Examples of **illustrative intentions, not fixed command syntax**:

```sh
design validate
design inspect level-05 --lens skill-atoms
design inspect level-05 --lens cognitive-load
design inspect level-05 --all
design gaps --format json
design interactions --matrix --format csv
design simulate progression --seeds 10000 --format csv
```

### F3. Pack interface: small and optional

A framework pack might supply:

1. **Sources and explanatory notes** — what the theory actually claims and limitations.
2. **Relevant data requirements** — types/relationships it can examine and inputs it may ask for.
3. **Queries** — deterministic graph/index/coverage operations.
4. **Diagnostics** — missing links, cycles, prerequisite gaps, unexplored combinations.
5. **Visualisations** — matrix, graph, progression timeline, run distribution, sensitivity plot.
6. **Questions** — human/agent interpretation where deterministic findings are insufficient.

A pack may contain only questions or references; it does not have to implement every item.

### F4. Report epistemic status explicitly

Prefer findings that distinguish:

- **Missing** — a required relation or input is absent under a declared rule.
- **Unknown** — the author has not supplied enough information.
- **Explicitly excluded** — intentionally unsupported or irrelevant.
- **Possible issue** — heuristic/design question, not a proven defect.
- **Simulated** — supported by a particular model, settings and seeds.
- **Observed** — backed by playtest/telemetry evidence.

Avoid a universal “game design score” and avoid turning theory into spurious precise metrics.

### F5. Strong cross-lens example

A level requires four individually taught skills in an unfamiliar combination:

- **Skill-atom analysis:** each prerequisite exists; no individual missing-skill error.
- **Flow analysis:** demand appears broadly aligned with assumed capability.
- **Cognitive-load analysis:** the new four-way combination may demand high simultaneous attention.
- **ZPD/scaffolding analysis:** no preceding smaller combination or temporary assistance exists.
- **Fullerton/playtest analysis:** ask whether actual players hesitate, fail or discover the intended coordination.

That combined observation is more useful than any single theory score: **the player may understand each skill individually without being ready to coordinate all four**.

### F6. Practical sequencing

1. Make the shared Markdown model and generic links searchable.
2. Implement noun–verb interaction coverage and skill prerequisite/progression checks.
3. Add mechanics → dynamics → aesthetics/intention trace views.
4. Add design questions (Schell-style original prompts and Fullerton-style hypothesis/evidence records).
5. Add challenge/flow/cognitive-load perspectives as *heuristic* views.
6. Add lightweight C99 seeded simulations of selected economic/progression dynamics.
7. Add optional motivation and play-type lenses once the core views are proving useful.

**Success criterion:** Adding one fact to the source data makes several different views and/or analyses more useful, and the agent can inspect the same relationships that a human sees.
