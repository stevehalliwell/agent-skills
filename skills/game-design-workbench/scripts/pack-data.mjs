// Independently authored prompts. Sources inform scope, not copied questionnaires/decks.
export const packs = [
  {
    id: 'mda', title: 'Mechanics, Dynamics, Aesthetics', attribution: 'Robin Hunicke, Marc LeBlanc, Robert Zubek (2004)',
    scope: 'Trace a selected rule or intended experience through authored mechanics, runtime-behavior hypotheses, and aesthetics. Systems group mechanics; they are not dynamics.',
    roles: ['mechanics', 'systems', 'dynamics', 'aesthetics', 'hypotheses', 'experiments', 'observations'], queries: ['mda', 'evidence', 'records'],
    questions: [
      'Which specific rule could produce the named behavior, and under which player choices?',
      'Working backward from the intended feeling, which behavior would make that intention plausible?',
      'Could the same rule produce an opposing experience under another strategy or context?',
      'Which link is a supplied design fact, which is an interpretation, and which has observed support?',
      'What experience has no declared supporting path? Is the path absent from documentation or absent from the design?',
      'What small observation could distinguish the proposed behavior from its strongest competing explanation?'
    ],
    limits: 'A declared mechanic–dynamic–aesthetic chain is not causal proof or emotional measurement. The mda recipe inventories declared links; disconnected-path interpretation remains agent work. Do not demand all eight familiar aesthetic labels.',
    missing: 'Discuss intention without complete links. Mark disconnected records as unknown/unresolved, not failed experiences. Distinguish empty observations from evidence against a claim.',
    fixture: 'Inspect crate-pushing, careful-positioning, and deliberate-tension links. Ask whether the authored challenge intention has support beyond prose; observations are empty.',
    access: 'AAAI abstract inspected; full paper not retrieved. Questions use broad published distinctions, not reproduced passages.',
    sources: [['MDA: A Formal Approach to Game Design and Game Research', 'https://aaai.org/papers/ws04-04-001-mda-a-formal-approach-to-game-design-and-game-research/']]
  },
  {
    id: 'game-grammar', title: 'Game grammar and meaningful play', attribution: 'Related meaningful-play concepts: Katie Salen Tekinbaş and Eric Zimmerman, Rules of Play. Noun–verb case matrix: workbench modeling technique, not an attributed book prescription.',
    scope: 'Inspect a selected noun/action pair, its conditional cases, consequences, feedback, and relevance to later play.',
    roles: ['nouns', 'verbs', 'interactions', 'mechanics', 'challenges'], queries: ['matrix', 'records'],
    questions: [
      'For this action on this target, which conditions choose the outcome? Can more than one case apply?',
      'Can the player perceive what changed, including a defined no-effect outcome?',
      'What later choice or state makes that consequence matter?',
      'Does a contextual noun participate in the condition without being the action target?',
      'When two descriptions disagree, which source owns the rule and which should only link to it?',
      'Is an absent case unknown, deliberately excluded, or genuinely not applicable? What supplied fact establishes that distinction?'
    ],
    limits: 'Matrix rows expand only explicitly declared applicable nouns. Missing Cartesian pairs are not synthesized or condemned. Shared mechanics own common rules; cases own their conditions/consequences/feedback. No automatic reachability or exhaustive coverage proof.',
    missing: 'Keep blank cells unknown. Preserve explicit excluded/not_applicable cases and defined no-effect cases. Do not infer exclusions from genre conventions.',
    fixture: 'Compare three crate/push cases; wall/activate is not applicable; closed exit/walk remains unknown. Blocked pushing has a defined no-effect consequence, not missing design.',
    access: 'MIT Press request returned 403. Search-linked teaching excerpt supports discernible/integrated outcomes; PDF text extraction was incomplete. No full-book review claimed.',
    sources: [['Rules of Play — MIT Press', 'https://mitpress.mit.edu/9780262240451/rules-of-play/'], ['Meaningful Play teaching excerpt', 'https://sites.cc.gatech.edu/classes/AY2006/cs4455_spring/documents/sz-unit11.pdf']]
  },
  {
    id: 'resource-economy', title: 'Resources, flows, and feedback', attribution: 'Ernest Adams and Joris Dormans, Game Mechanics: Advanced Game Design (2012); Machinations approach',
    scope: 'Examine a selected resource and the rules that create, spend, transform, or limit it. Separate resource identity from adjustable quantities.',
    roles: ['nouns', 'resources', 'controls', 'mechanics', 'interactions', 'measures', 'expectations', 'experiments'], queries: ['controls', 'matrix', 'records'],
    questions: [
      'What is the resource, what stores it, and which rule changes it?',
      'Which sources and sinks depend on player choices, elapsed time, or chance?',
      'Does gaining more of this resource make future gains easier, harder, or neither? What intervening rule creates that feedback?',
      'Which capacity or initial-value controls bound the state, and where are conversion assumptions written?',
      'Could a proposed balance change produce different outcomes for different strategies rather than one new average?',
      'What minimal measurement would test the disputed system claim, and which behavior assumptions would that model omit?'
    ],
    limits: 'This release inventories literal controls and formula text; it does not evaluate formulas, resolve units, draw directional flow graphs, or execute simulations. Flow direction stays in rule prose. At most one accepted resource extension per noun is a source invariant to inspect manually here.',
    missing: 'A missing resource extension does not prevent discussing a noun. Missing flow direction is unresolved prose, not an inferred edge. Suggest an experiment without running it.',
    fixture: 'Energy is a noun/resource; initial-energy and push-cost are controls; crate-pushing owns spending behavior. Three required pushes with initial energy 2 raises a conditional feasibility question, not an executed result.',
    access: 'Hogeschool van Amsterdam publication summary inspected; book and Machinations implementation not inspected.',
    sources: [['Game Mechanics: Advanced Game Design — publication record', 'https://research.hva.nl/en/publications/game-mechanics-advanced-game-design/']]
  },
  {
    id: 'gameplay-patterns', title: 'Gameplay design patterns', attribution: 'Staffan Björk, Sus Lundgren, Jussi Holopainen (2003), and later Björk/Holopainen pattern work',
    scope: 'Compare a selected structure with a cited pattern or contrasting configuration. Use patterns as exploratory vocabulary, not required tags.',
    roles: ['mechanics', 'systems', 'dynamics', 'decisions'], queries: ['mda', 'decisions', 'records'],
    questions: [
      'What relationship among rules makes this resemble the proposed pattern beyond sharing a name?',
      'Which context makes that resemblance useful, and which difference makes it misleading?',
      'What other pattern configuration could support the same intended behavior?',
      'Which interaction between two patterns could reverse the expected effect?',
      'Was a similar option previously rejected, and does the old rationale still apply?',
      'What source example or prototype would let the designer compare these alternatives fairly?'
    ],
    limits: 'No exhaustive pattern catalog, automatic recognition, or copied pattern entries. Cite a specific external pattern only after inspecting that source; title similarity is not evidence of implementation.',
    missing: 'Proceed from described rule relationships without pattern labels. If external pattern evidence is unavailable, frame resemblance as an unverified hypothesis.',
    fixture: 'Compare retained pulling rejection with a proposed recovery option; preserve both rationale and new alternative without adopting either automatically.',
    access: 'DiGRA abstract inspected. External catalog is a further-reading link, not a bundled database or a claim of full inspection.',
    sources: [['Game Design Patterns — DiGRA', 'https://dl.digra.org/index.php/dl/article/view/60'], ['Gameplay Design Patterns catalog', 'https://gdp.cse.chalmers.se/index.php?title=Main_Page']]
  },
  {
    id: 'skill-atoms', title: 'Skill atoms and learning chains', attribution: 'Daniel Cook, The Chemistry of Game Design (original article; 2021 repost)',
    scope: 'Inspect a selected learned interaction, its prerequisite chain, authored progression rank, feedback, and challenge opportunities.',
    roles: ['skills', 'challenges', 'levels', 'mechanics'], queries: ['skills', 'routes', 'records'],
    questions: [
      'What choice or action expresses the skill, and what feedback lets the learner update a mental model?',
      'What would distinguish understanding from merely succeeding once?',
      'Which prerequisites are genuinely needed together, and which are only conventional ordering?',
      'Where can the skill be introduced, practised, combined, and transferred under changed conditions?',
      'Does an authored rank disagree with prerequisites? Is that intentional, unresolved, or only a display concern?',
      'Which declared route offers a learning opportunity, and which route bypasses it?'
    ],
    limits: 'Skills use AND prerequisites; absent/null/empty arrays declare no links. Rank is nullable signed conceptual placement, not physical level or graph depth. Recipes expose links/ranks/routes; cycle, rank-conflict, and all-route opportunity checks remain manual until C99 analysis ships. Opportunity does not prove acquired mastery.',
    missing: 'Keep unknown ranks unknown and late roots visible. Optional levels are not required for route discussion. No teaching requirement or learner incapability claim follows from missing practice records.',
    fixture: 'Coordinate is authored rank 2 but depends on route-switch rank 3 and unranked plan-push. A requested edit to coordinate rank 3 changes only that field; descendants are not rewritten.',
    access: 'Lostgarden article inspected, including action/rule/feedback/mental-model loop. Questions independently authored.',
    sources: [['The Chemistry of Game Design', 'https://lostgarden.com/2021/03/13/the-chemistry-of-game-design-2/']]
  },
  {
    id: 'theory-of-fun', title: 'Learning, patterns, and fun', attribution: 'Raph Koster, A Theory of Fun for Game Design; 2003 presentation and book',
    scope: 'Ask what understanding a selected progression invites and whether repetition serves its intended experience.',
    roles: ['skills', 'challenges', 'mechanics', 'hypotheses', 'observations'], queries: ['skills', 'routes', 'evidence'],
    questions: [
      'What new pattern could the player discover here rather than merely execute faster?',
      'Does the next challenge combine familiar ideas in a new way or only change a number?',
      'When repetition is intended, what experience does it support besides novelty?',
      'Which once-interesting decision becomes automatic, and what replaces it?',
      'Could an expert find a new interpretation while a beginner still learns the basic pattern?',
      'What observed choices would distinguish discovery, fluent enjoyment, and disengaged repetition?'
    ],
    limits: 'No fun score. Learning is a perspective, not an exhaustive explanation of enjoyment. Repeated actions can support mastery, comfort, expression, or other intentions; do not label repetition defective without context.',
    missing: 'Use authored skill/challenge descriptions when progression links are incomplete. Lack of observed novelty is not evidence of boredom.',
    fixture: 'Combined-room combines push planning and switch routing; ask what new relationship the combination teaches instead of assuming harder means deeper.',
    access: 'Official book site and author presentation introduction inspected; full book/presentation not extracted.',
    sources: [['A Theory of Fun — official site', 'https://www.theoryoffun.com/'], ['Author introduction to 2003 talk', 'https://www.raphkoster.com/gaming/theoryoffun.shtml']]
  },
  {
    id: 'flow', title: 'Flow: challenge and capability', attribution: 'Mihaly Csikszentmihalyi, Flow: The Psychology of Optimal Experience (1990)',
    scope: 'Explore challenge relative to assumed capability, clear goals, feedback, and conditions for sustained attention in a selected encounter.',
    roles: ['skills', 'challenges', 'mechanics', 'hypotheses', 'observations'], queries: ['skills', 'routes', 'evidence'],
    questions: [
      'What makes the immediate goal understandable before the player commits to an action?',
      'What feedback lets the player judge progress while there is still time to adapt?',
      'Which assumed capability makes this demand manageable, and what supports that assumption?',
      'Where could challenge rise abruptly without a corresponding chance to develop familiarity?',
      'What optional assistance or self-selected challenge could accommodate different current capabilities?',
      'What would a playtest need to observe before calling this absorption rather than confusion or mere persistence?'
    ],
    limits: 'No required cohort/profile dataset, calibrated capability curve, flow score, anxiety score, or boredom score. Keep separate from GameFlow. Authored challenge/rank numbers do not measure a player state.',
    missing: 'State capability assumptions explicitly. Missing observations leave experience unverified; they do not justify predicting anxiety.',
    fixture: 'Examine optional bypass from push-room to combined-room. Ask about feedback and familiarity; do not infer a capability curve from skill ranks.',
    access: 'Publisher requests returned 403. Bibliographic identity and broad conditions checked through source-linked search summaries; full book not read.',
    sources: [['Flow — publisher', 'https://www.harpercollins.com/products/flow-mihaly-csikszentmihalyi'], ['Flow Engine Framework — later research context', 'https://pmc.ncbi.nlm.nih.gov/articles/PMC5973526/']]
  },
  {
    id: 'gameflow', title: 'GameFlow enjoyment conditions', attribution: 'Penelope Sweetser and Peta Wyeth (2005)',
    scope: 'Use applicable enjoyment conditions to question a selected mechanic, encounter, or session: concentration, challenge, skills, control, goals, feedback, immersion, and social interaction.',
    roles: ['mechanics', 'challenges', 'controls', 'hypotheses', 'observations'], queries: ['routes', 'controls', 'evidence', 'records'],
    questions: [
      'What deserves attention now, and what competing presentation could obscure it?',
      'Do controls let the player express the intended decision without unexplained loss of control?',
      'How are immediate goals and longer-term progress made legible?',
      'What feedback distinguishes a misunderstood rule from an unsuccessful strategy?',
      'Which elements support sustained involvement, and which interruptions are intentional?',
      'If social interaction matters here, what activity enables it? If it does not, why is that criterion inapplicable?'
    ],
    limits: 'These are original questions, not a reproduction or scoring implementation of the published evaluation instrument. No aggregate enjoyment score. Flow theory and GameFlow are distinct. Control records are adjustable parameters, not a complete account of player agency or input controls.',
    missing: 'Mark social criteria inapplicable for an intentionally solo experience. Missing goal/feedback prose is a documentation question, not proof players cannot understand the game.',
    fixture: 'Check blocked-push feedback and visible switch state. Social interaction is outside this solo puzzle scope, not a missing mandatory feature.',
    access: 'ACM DOI requests blocked; paper identity/eight-element summary checked through source-linked search. Suggested PDF returned 404. Full evaluation criteria not reviewed or copied.',
    sources: [['GameFlow: A Model for Evaluating Player Enjoyment in Games', 'https://doi.org/10.1145/1077246.1077253']]
  },
  {
    id: 'cognitive-load', title: 'Cognitive load and unfamiliar coordination', attribution: 'John Sweller, Cognitive Load During Problem Solving: Effects on Learning (1988)',
    scope: 'Examine unfamiliar operations, concurrent coordination, problem-solving effort, and information presentation in a selected challenge.',
    roles: ['skills', 'challenges', 'mechanics', 'observations'], queries: ['skills', 'routes', 'records'],
    questions: [
      'What must be coordinated at the same time, rather than merely appearing somewhere in the challenge?',
      'Which parts are assumed familiar, and what source or observation supports that assumption?',
      'Could searching for the next move consume attention needed to understand the underlying rule?',
      'What information is unnecessarily hidden, split across views, or repeatedly reinterpreted?',
      'Would a demonstration, reduced task, or stable cue expose the relationship without removing the intended decision?',
      'What observation would tell us whether assistance supports learning rather than only helping complete this instance?'
    ],
    limits: 'A required-skills list does not establish simultaneous demand. No count-based cognitive-load score or diagnosis. Game-design application is a hypothesis; original laboratory findings are not calibrated mechanic-count limits.',
    missing: 'Ask about concurrent demand and familiarity when absent. Preserve them as assumptions rather than inventing player capability data.',
    fixture: 'Coordinate has two prerequisite chains. Inspect challenge prose before claiming both must be actively maintained at once.',
    access: 'Wiley/ScienceDirect requests blocked. Bibliography and abstract argument about problem solving/schema acquisition checked through source-linked search; full study not read.',
    sources: [['Cognitive Load During Problem Solving: Effects on Learning', 'https://doi.org/10.1207/s15516709cog1202_4']]
  },
  {
    id: 'working-memory', title: 'Working memory and chunking', attribution: 'George A. Miller (1956); Nelson Cowan (2001, later reassessments)',
    scope: 'Explore what must be remembered, what is visible, and how familiarity might combine elements into learned chunks.',
    roles: ['skills', 'challenges', 'mechanics', 'observations'], queries: ['skills', 'routes', 'records'],
    questions: [
      'Which state must the player remember after it disappears from view?',
      'Could a stable cue externalize that memory without solving the intended puzzle?',
      'What appears to an expert as one familiar operation but to a beginner as several independent decisions?',
      'Does a learned combination remain recognizable when presentation or context changes?',
      'Which competing cues need attention together, and which can be processed in sequence?',
      'What would compare a memory-support variant with the original while preserving the intended reasoning?'
    ],
    limits: 'Never use four, seven, or another item count as universal failure/capacity threshold. Chunk size depends on learning and task. No player-capacity dataset or computed memory score.',
    missing: 'Unspecified visibility and familiarity remain unknown. Do not equate number of skills with number of active memory chunks.',
    fixture: 'Ask whether switch state remains visible while planning crate movement. Existing prerequisite links alone cannot answer that presentation question.',
    access: 'Miller full-text mirror inspected for chunking/recoding. Cowan PubMed extraction incomplete; abstract claims checked through source-linked search. No universal threshold inferred.',
    sources: [['The Magical Number Seven, Plus or Minus Two', 'https://psychclassics.yorku.ca/Miller/'], ['The Magical Number 4 in Short-Term Memory', 'https://pubmed.ncbi.nlm.nih.gov/11515286/']]
  },
  {
    id: 'zpd-scaffolding', title: 'Supported challenge and scaffolding', attribution: 'Lev Vygotsky: zone of proximal development; David Wood, Jerome Bruner, Gail Ross: scaffolding/tutoring (1976)',
    scope: 'Explore temporary support, independent performance, demonstrations, practice, and withdrawal of help in a selected learning path.',
    roles: ['skills', 'challenges', 'levels', 'observations'], queries: ['skills', 'routes', 'records'],
    questions: [
      'What could the learner attempt independently, and what currently needs support? Which part is only an assumption?',
      'Could a hint reveal a relevant feature without choosing the whole solution?',
      'Where is safe practice available before a consequence becomes costly?',
      'Does a worked example expose the reasoning or only a sequence to copy?',
      'How might support become less specific as understanding develops?',
      'What later, changed task would let the designer observe independent transfer?'
    ],
    limits: 'ZPD and later scaffolding are related but not identical. No teaching prerequisite, developmental diagnosis, or proof that a player cannot succeed without a declared introduction. Routes are declared structural opportunities, not gameplay reachability.',
    missing: 'Missing support is a question, not automatic failure. Proceed without level records; inspect challenge entry/next links and explicit role lists.',
    fixture: 'Push-room can lead directly to combined-room, bypassing switch-practice. Ask whether support is intentional on that route; do not assert impossible completion.',
    access: 'Oxford reference blocked; ZPD definition checked through source-linked APA/academic search. Tutoring DOI returned 406; PDF extraction incomplete. Full studies not claimed.',
    sources: [['Zone of proximal development — Oxford Reference', 'https://www.oxfordreference.com/display/10.1093/oi/authority.20110803133528287'], ['The Role of Tutoring in Problem Solving', 'https://doi.org/10.1111/j.1469-7610.1976.tb00381.x']]
  },
  {
    id: 'self-determination', title: 'Self-Determination Theory', attribution: 'Edward Deci, Richard Ryan, and collaborators',
    scope: 'Question support for autonomy, competence, and relatedness in a selected activity without inferring an individual psychological state.',
    roles: ['aesthetics', 'mechanics', 'dynamics', 'challenges', 'hypotheses', 'observations'], queries: ['mda', 'routes', 'evidence', 'records'],
    questions: [
      'What choice is meaningful to the intended experience rather than only cosmetically different?',
      'Which constraint helps the player act intentionally, and which might feel controlling?',
      'What feedback lets the player understand growing competence without relying only on external rewards?',
      'Where can a player demonstrate an approach they chose rather than copy a prescribed sequence?',
      'What relationship or sense of connection matters in this scope, if any?',
      'Which observed response could challenge the designer’s motivation hypothesis?'
    ],
    limits: 'Autonomy, competence, and relatedness are common concepts here, not mandatory profile records or independent fun-point scores. Mechanics alone do not measure need satisfaction. Solo play does not automatically imply a relatedness defect.',
    missing: 'Keep motivation hypotheses explicit. No observations means unverified support, not unmet psychological needs.',
    fixture: 'Compare optional route choice with constrained crate movement. Constraints can support deliberate planning; avoid declaring either autonomy support or frustration proven.',
    access: 'Official SDT overview inspected, including three basic needs and support/thwarting distinctions. No questionnaire reproduced.',
    sources: [['Self-Determination Theory — official overview', 'https://selfdeterminationtheory.org/theory/']]
  },
  {
    id: 'quantic-foundry', title: 'Published gamer-motivation vocabulary', attribution: 'Quantic Foundry, Gamer Motivation Model and 2015 profile-v2 explanation',
    scope: 'Compare an intended experience with activities that might support interests described by the published multidimensional motivation vocabulary.',
    roles: ['aesthetics', 'mechanics', 'dynamics', 'challenges', 'hypotheses', 'observations'], queries: ['mda', 'evidence', 'records'],
    vocabulary: 'Action: Destruction/Excitement; Social: Competition/Community; Mastery: Challenge/Strategy; Achievement: Completion/Power; Creativity: Discovery/Design; Immersion: Fantasy/Story. These are attributed model labels, not workbench scores.',
    questions: [
      'Which named interest is actually part of the current design intention?',
      'What repeated player activity supports that interest rather than merely mentioning it in theme or copy?',
      'Could the same mechanic serve different interests through different choices?',
      'What tension appears when supporting two intended interests at once?',
      'Which interests are deliberately outside scope, with no need to add corresponding features?',
      'What play evidence would distinguish the intended appeal from the designer’s preferred interpretation?'
    ],
    limits: 'No questionnaire, preference scoring, cohort collection, audience dataset, personality inference, or single player type. Published empirical model does not validate a workbench-specific support claim.',
    missing: 'Discuss intended interests without surveyed profiles. If no interest is supplied, offer vocabulary as optional hypotheses rather than assigning an audience.',
    fixture: 'Deliberate planning plausibly relates to Challenge/Strategy. That is an interpretation to test, not a measured preference profile for puzzle players.',
    access: 'Model page returned 403. Published labels and paired groups checked against source-linked 2015 explanation search; full questionnaire/scoring not accessed.',
    sources: [['Gamer Motivation Model', 'https://quanticfoundry.com/gamer-motivation-model/'], ['How We Developed the Gamer Motivation Profile v2', 'https://quanticfoundry.com/2015/07/20/how-we-developed-the-gamer-motivation-profile-v2/']]
  },
  {
    id: 'four-keys', title: 'Four Keys to Fun', attribution: 'Nicole Lazzaro / XEODesign; Why We Play Games research (2004)',
    scope: 'Explore possible routes to emotion through challenge/triumph, curiosity, meaningful engagement, and social amusement.',
    roles: ['dynamics', 'aesthetics', 'interactions', 'observations'], queries: ['mda', 'matrix', 'evidence', 'records'],
    vocabulary: 'Hard Fun, Easy Fun, Serious Fun, People Fun are Lazzaro’s labels. They are not synonyms for MDA aesthetics or Koster’s learning account.',
    questions: [
      'What sequence could make overcoming this obstacle feel like earned triumph?',
      'What invites curiosity before the player knows the solution?',
      'What could make the activity personally meaningful or emotionally restorative in this intended context?',
      'If people play together, what shared activity could create amusement rather than merely co-presence?',
      'How could the emotional tone change within a session without losing the main intention?',
      'Which described player reaction would support or contradict the proposed emotional route?'
    ],
    limits: 'No category quota, emotional score, copied questionnaire, or rule that every game needs every key. Links to emotional categories remain interpretations until supported by relevant evidence.',
    missing: 'Keep undeclared social experience inapplicable when appropriate. Do not treat missing observed emotion as evidence of absent emotion.',
    fixture: 'A successful combined-room solution may invite triumph; discovering a switch relation may invite curiosity. Both are hypotheses, not observed reactions.',
    access: 'Author’s Four Keys overview inspected. Full white paper/slides and proprietary methods not reproduced.',
    sources: [['The 4 Keys 2 Fun', 'https://www.nicolelazzaro.com/the4-keys-to-fun/']]
  },
  {
    id: 'bartle', title: 'MUD participation patterns', attribution: 'Richard A. Bartle, Hearts, Clubs, Diamonds, Spades: Players Who Suit MUDs (1996)',
    scope: 'Use historically situated player/world and acting/interacting distinctions to question activities and tensions in multiplayer designs.',
    roles: ['mechanics', 'dynamics', 'challenges', 'decisions', 'observations'], queries: ['mda', 'decisions', 'evidence', 'records'],
    vocabulary: 'Achievers, Explorers, Socialisers, Killers describe approaches discussed in multiplayer MUD research; they are not universal identities.',
    questions: [
      'Which activities concern changing the world, understanding it, interacting with people, or exerting influence over them?',
      'What makes these activities depend on other participants rather than only non-player systems?',
      'Could one encouraged activity prevent another participant from enjoying a different activity?',
      'What consent, recovery, or boundary rule contains harmful conflict where relevant?',
      'Can the same person pursue different approaches at different times?',
      'Is this historical multiplayer frame useful for the selected game, or would applying it distort the design?'
    ],
    limits: 'No universal player typing, percentage allocation, psychological diagnosis, or requirement to serve every category. Do not translate Killers directly into all combat or competitive play without context.',
    missing: 'If multiplayer participation is absent, report limited applicability and stop this lens instead of inventing social features.',
    fixture: 'Solo crate puzzle has limited applicability. Record that boundary; no automatic multiplayer recommendation.',
    access: 'Original article mirror inspected, including MUD scope, four approaches, and movement between them.',
    sources: [['Players Who Suit MUDs — original-text mirror', 'https://www.majormud.com/pwsm.html']]
  },
  {
    id: 'caillois', title: 'Forms and freedoms of play', attribution: 'Roger Caillois, Man, Play and Games (1958; English translation by Meyer Barash)',
    scope: 'Explore competition, chance, make-believe, vertigo, and movement between free improvisation and rule-bound challenge.',
    roles: ['mechanics', 'dynamics', 'challenges', 'aesthetics'], queries: ['mda', 'routes', 'records'],
    vocabulary: 'Agon (competition), alea (chance), mimicry (make-believe), ilinx (vertigo); paidia/ludus describe contrasting tendencies toward improvisation and disciplined rule-bound play.',
    questions: [
      'What is uncertain because of another participant, chance, hidden information, or player interpretation?',
      'What freedom to improvise remains inside the current rules?',
      'Does role-play or make-believe change the available experience, or is it only surface description?',
      'If sensory disorientation matters, what experience and accessibility boundaries are intended?',
      'Which tension between free play and prescribed challenge is purposeful?',
      'Would adding another form of play support the current goal or merely satisfy an unnecessary category checklist?'
    ],
    limits: 'Historical conceptual lens, not a numeric design profile or universal taxonomy test. No requirement to include competition, randomness, role-play, or vertigo.',
    missing: 'Absent forms can be deliberate boundaries. Ask for rules/presentation context rather than inferring chance or sensory experience from theme.',
    fixture: 'Deterministic puzzle rules emphasize prescribed challenge; hypothetical player delay is an experiment assumption, not proof the game itself uses chance.',
    access: 'Google Books publisher description inspected; full translated book not accessed.',
    sources: [['Man, Play, and Games — book record', 'https://books.google.com/books/about/Man_Play_and_Games.html?id=bDjOPsjzfC4C']]
  },
  {
    id: 'schell', title: 'Contextual reflective design questions', attribution: 'Jesse Schell, The Art of Game Design: A Book of Lenses',
    scope: 'Reframe a selected design question through concrete player experience, rules, alternatives, and consequences. This is original reflective guidance, not Schell’s deck.',
    roles: ['mechanics', 'aesthetics', 'challenges', 'decisions', 'hypotheses'], queries: ['decisions', 'mda', 'records'],
    questions: [
      'What does the player need to notice, decide, and feel at this exact moment?',
      'Which rule is essential to that experience, and what would change if it were removed?',
      'What attractive alternative serves the same intention through a different constraint?',
      'What cost appears elsewhere in the design when this local change succeeds?',
      'Which assumption is doing the most work in the current argument?',
      'What small comparison could expose that assumption before a durable decision?'
    ],
    limits: 'No copied lens-card wording, numbering, complete deck, or claim of comprehensive book coverage. Do not use this pack as permission to run other framework reviews.',
    missing: 'Start from supplied facts and scoped alternatives. Unknowns become questions/hypotheses, not invented rules. Preserve rejected alternatives and rationale.',
    fixture: 'Question whether pulling would undermine deliberate placement; retain its prior rejection while exploring a distinct recovery alternative.',
    access: 'Official Art of Game Design page inspected; book/deck content not reproduced or reviewed in full.',
    sources: [['The Art of Game Design — official page', 'https://schellgames.com/art-of-game-design']]
  },
  {
    id: 'playcentric', title: 'Playcentric design and evidence', attribution: 'Tracy Fullerton, Game Design Workshop: A Playcentric Approach to Creating Innovative Games',
    scope: 'Connect intended player experience with a testable question, prototype, observations, and resulting design choices.',
    roles: ['aesthetics', 'hypotheses', 'experiments', 'observations', 'decisions', 'measures', 'expectations'], queries: ['evidence', 'decisions', 'records'],
    questions: [
      'What experience goal is concrete enough to guide a prototype choice?',
      'Which uncertain claim matters most to the next decision?',
      'What smallest prototype could expose that uncertainty without pretending to be the whole game?',
      'What would be observed, measured, or asked, and what result could contradict the claim?',
      'Which evidence comes from people, which from a model, and which remains a designer assumption?',
      'How did the evidence change a decision, and which alternative/rationale should remain available for later reconsideration?'
    ],
    limits: 'A proposed experiment is not execution evidence. Simulated predictions are not observed play. No automatic repairs, playtest execution, or simulation. The evidence query is an inventory; inspect linked record bodies for full method and evidence trail.',
    missing: 'Empty observations honestly mean no observations recorded. Discuss test design without fabricating participants, sample counts, measured outcomes, or completed iterations.',
    fixture: 'Timing experiment is proposed, its 5–12 s range and 1312/5040 deadline probability are analytical references under stated delay assumptions. There are no executed trials or play observations.',
    access: 'Official Game Design Workshop overview inspected: experience goals, prototyping, playtesting, revision. Full book/exercises not reproduced.',
    sources: [['Game Design Workshop — official site', 'https://www.gamedesignworkshop.com/']]
  }
];
