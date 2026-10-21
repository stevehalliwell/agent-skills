# The Chemistry of Game Design — source notes

**Independently authored research notes. Not source conversion or transcription.**

Actual source now retained separately: [complete article Markdown](captures/chemistry-of-game-design/article.md). See [capture inventory](README.md) for method, figure blocker, and rights.

## Source and acquisition

- Original title: *The Chemistry of Game Design*.
- Author: Daniel Cook.
- Date: originally published in 2007 on Gamasutra, according to author's introduction; author repost dated 2021-03-13. Page metadata reports modification on 2024-03-28. Notes describe retrieved repost, not a verified copy of 2007 edition.
- Original publication link supplied by author: https://www.gamasutra.com/view/feature/129948/the_chemistry_of_game_design.php
- Inspected author-hosted URL: https://lostgarden.com/2021/03/13/the-chemistry-of-game-design-2/
- Access date: 2026-10-21.
- Source type: primary framework essay by framework author; not an experimental validation paper.
- Method: web search discovery; `fetch_content` readable HTML extraction; complete 35,493-character extracted article read through `get_search_content`. Raw HTML fetched separately to check author/date metadata and reuse notices.
- Reuse basis: no suitable republication license established. Searches of retrieved raw HTML for `creativecommons`, `license`, and `copyright` found no matches. Absence of notice is not permission. Original notes-only pass retained no article or diagrams. Subsequent local-only capture retains actual article and available diagrams; republication permission remains unestablished. No workshop slides or PDF acquired.
- Coverage limits: prose reviewed, including references and MDA comparison. Embedded diagrams were not independently visually inspected. No diagram reconstruction, study replication, linked neuroscience review, or workshop conversion performed.

## Framework account

All numbered notes below summarize Cook's proposed model, not established universal facts about players. Section names provide locations in linked source.

1. **Describe learning, not just system components.** Cook argues that a list of verbs, tokens, or rules misses interaction with a changing human learner. His proposed unit connects what a player attempts with what they come to understand. A skill can concern physical execution or conceptual navigation. Location: **2. The Foundations Of A Model Of Game Design**, **Player Model**, **Skill**.

2. **The repost explicitly includes decision before action.** Its interaction loop has five named parts: Decision, Action, Simulation, Feedback, Modeling. These distinguish noticing available possibilities, intervening, changing game state, perceiving that change, and revising expectations. An action can be a button press or a larger sequence once lower-level behavior is available. Location: **3: Interaction Loops**. Short source anchor: “updates their mental models”.

3. **Feedback must make a relationship learnable.** Cook's Mario example links a control input to an observed jump. Repetition can let the player test a possible explanation; one surprising event need not communicate the relationship. Subsequent experimentation can reveal another use for the learned behavior. Location: **3: Interaction Loops**, **This Thing We Call Play**.

4. **Chains model enabling relationships.** Cook links the skill acquired through one loop to actions needed by another. A chain is therefore a directed account of how understanding may develop, rather than only a timeline of rooms or inputs. His Tetris example illustrates how a mechanically small game can contain many learnable relationships. Location: **Chaining Of Game Mechanics**. The diagram's detailed edges were not transcribed.

5. **Progress state and continued use differ.** Cook distinguishes skills not attempted, partially learned, mastered, actively reused, and abandoned through lost interest. Reusing a known action may support learning something more complex; repetition is not necessarily acquisition of another basic skill. Location: **4. Status Of Atoms In The Skill Chain**, **Active Skills**.

6. **Breakdown can occur between connected skills.** A player may learn an action without discovering a useful next application. Conversely, loss of interest in a later application can remove the reason to keep using earlier actions. Cook proposes tracking these breakdowns during playtesting. Location: **Burnout**, **Early Stage Burnout**, **Later Stage Burnout**, **Burnout Is Our Gateway To Testability**.

7. **Starting knowledge changes the chain.** Assumed controller fluency can block newcomers before intended learning begins. Already-known content can bore experienced players. Cook therefore treats prior skills as a design input rather than assuming an identical starting state. Location: **5. Advanced Elements Of A Skill Chain**, **Pre-existing Skills**, **Lack Of The Correct Initial Skills**, **Pre-mastery Of Skills Taught In The Game**.

8. **Evocation and functional learning are not identical.** Cook discusses arcs as cues that activate existing associations without necessarily producing a useful new in-game capability. Familiar imagery can nevertheless help players form causal expectations. Location: **Evocative Stimuli using Arcs**. See also [later clarification](loops-and-arcs-source-notes.md).

## Interpretation and limits

- Article supplies a useful author-origin account of interaction loops, chains, prior knowledge, and breakdowns. It does not supply a controlled evaluation of this project's game or a validated mastery classifier.
- Cook's claims about pleasure, neurochemistry, evolutionary incentives, limited foresight, and mastery occurring once are theoretical claims in this essay. They were not independently verified here. Do not promote them into settled empirical laws.
- Source's suggested instrumentation motivates observation; a logged successful action alone does not identify a player's mental model.
- No source passage reviewed establishes this project's nullable rank, AND-prerequisite storage rules, or route-analysis conventions. Those are local modeling choices, not imported source requirements.
- No source proves that any example game's intended chain matches actual player experience.

## Follow-up leads, not acquired evidence

- Article links an author workshop on interaction loops and chains: https://drive.google.com/file/d/1MscwnQIrsz2EWnPU4dtk_Oj16IegIaeJ/view?usp=sharing . Slides were not downloaded, reviewed, or licensed here.
- Original 2007 publication would permit edition comparison. This pass relies on author's explicit repost statement, not proof that every sentence or diagram appeared unchanged in 2007.
