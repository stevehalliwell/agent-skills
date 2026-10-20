// Maintainer-only expansions. Independent explanations and applications, not source excerpts.
// Keys match pack-data.mjs. build-packs.mjs validates coverage before generating docs.
export const guidance = {
  mda: {
    concepts: [
      ['Mechanics', 'Rules and constraints that define available actions and state changes. An energy cost belongs to a mechanic; the numerical cost can live in a linked control. Neither establishes how a person will play.'],
      ['Dynamics', 'Behavior that could emerge when rules, choices, and context interact over time. Describe a sequence or recurring relationship, not another inventory of objects. A system is an authored grouping, not automatically a dynamic.'],
      ['Aesthetics', 'Intended qualities of experience, such as deliberate tension. Each is an MDA A aspect, not the overall player experience/player story. Compare both lenses when a brief exists. A declared intention is not an observed feeling, even when a designer considers the supporting rule obvious.'],
      ['Direction of reasoning', 'Design can work backward from an intention toward supporting behavior and rules. Review can work forward from a rule toward several plausible behaviors. Neither direction makes the links causal evidence.']
    ],
    procedure: [
      'Choose one rule or experience intention and a bounded encounter. Read linked bodies, not just names. Record the selected endpoint and relevant context; leave unrelated aesthetics outside this review.',
      'Trace declared mechanic, dynamic, and aesthetic links. For each step, write what the player would actually do or encounter. Mark a missing link as undocumented rather than filling it with a genre assumption.',
      'Label every connection as supplied intent, agent hypothesis, or supported observation. Explain the rule or choice that makes each proposed connection plausible. Keep organizational system membership separate from behavior.',
      'Construct one competing path from the same rule. Change an assumption about knowledge, strategy, or consequence and describe how the experience might differ. Retain both interpretations when neither has evidence.',
      'Propose a discriminating observation only if useful to the decision. State what would weaken the preferred interpretation. Finish with a scoped chain and its uncertainty, not a verdict that the game delivers its intended emotion.'
    ],
    example: [
      ['Authored facts', 'Crate-pushing, careful-positioning, and deliberate-tension are linked design concepts. Energy spending belongs to crate-pushing. Experiment Observations sections contain no play evidence.'],
      ['Interpretation', 'Costly pushes could encourage advance planning, which could support deliberate tension. This assumes players understand the cost, can anticipate consequences, and regard mistakes as meaningful rather than arbitrary.'],
      ['Counterreading', 'The same commitment could instead produce confusion if feedback hides the reason a push failed. This is a competing hypothesis, not an observed problem or a new canonical dynamic.'],
      ['Useful next question', 'What source describes how the cost becomes legible before commitment? If play evidence is later requested, distinguish visible planning and rule explanation from an inferred emotional response.']
    ],
    evidence: [
      'Observe behavior and reported experience separately. A long pause may indicate planning, confusion, interruption, or something else; do not treat elapsed time as a direct tension measure.',
      'Check whether the proposed behavior still occurs when the relevant constraint is absent or understood differently. A comparison can weaken a causal story without establishing a different universal explanation.',
      'Carry forward counterexamples and context. One observed strategy supports a bounded claim, not every player, route, or future tuning value.'
    ],
    traps: [
      'Renaming a mechanic as a dynamic adds no explanation. Replace the label with a concrete account of choices and changing state.',
      'Treating aesthetics as compulsory categories encourages feature filling. Review only the intended experience selected for this question.',
      'A complete link chain can still be wrong. Keep structural completeness separate from evidence that behavior and experience occur.'
    ]
  },
  'game-grammar': {
    concepts: [
      ['Identity and action', 'Nouns identify things; verbs identify actions. Keep target identity separate from contextual objects. A wall can constrain pushing a crate without becoming the target of that push.'],
      ['Conditional case', 'An interaction states when a particular action/target combination has a particular outcome. Conditions, consequences, and feedback belong together so a reader can explain why this case differs from another.'],
      ['Discernible consequence', 'A player needs some way to perceive what an action did or why it did nothing. Feedback can explain a defined no-effect outcome; absence of movement alone does not establish adequate feedback.'],
      ['Integrated consequence', 'An outcome matters through its relationship to later choices and state. A local animation may be perceptible while its strategic consequence remains unclear. These are different review questions.']
    ],
    procedure: [
      'Select one noun/action pair or a small supplied set. Resolve canonical terms and aliases first. Record exactly which target and action are under review; do not expand the full Cartesian product.',
      'Read every declared case for that pair. Extract its conditions, changed state, unchanged state, and feedback. Keep explicit unknown, excluded, not-applicable, and defined-no-effect cases distinct.',
      'Compare conditions for overlap and gaps. Ask what supplied rule chooses between overlapping descriptions. Mark the issue unresolved if the source does not establish precedence; do not invent an execution order.',
      'Trace each consequence into one later decision or challenge. Separate what the rule changes from what presentation communicates. Link shared behavior to its owning mechanic instead of duplicating it in every case.',
      'Return a compact case comparison and source-specific questions. If a new case seems useful, describe it as a proposal. Review does not authorize creating the case, changing feedback, or declaring remaining pairs invalid.'
    ],
    example: [
      ['Authored facts', 'Crate/push has free, wall-blocked, and low-energy cases. Wall/activate is explicitly not applicable. Closed-exit/walk is unknown rather than a declared prohibition.'],
      ['Interpretation', 'Wall-blocked pushing has a defined no-movement outcome and feedback. It is not an empty design cell. Review can still ask whether that feedback explains the relevant condition.'],
      ['Counterreading', 'Two blocked cases could be relevant when both wall and energy conditions hold. Their existence alone does not establish which feedback wins; inspect case prose before claiming ambiguity.'],
      ['Useful next question', 'Does supplied source describe a joint-condition outcome or precedence? If not, retain that uncertainty without converting it into an automatic rule repair.']
    ],
    evidence: [
      'Ask a player to explain an observed outcome only when a playtest is authorized. A correct guess after repeated attempts differs from clear feedback on the first relevant action.',
      'Compare perceived local feedback with understanding of later consequences. Recognizing that a crate moved does not prove understanding of an irreversible placement.',
      'Treat case coverage as authored coverage, not state reachability. A declared combination may never occur in a particular level; proving that requires a different method.'
    ],
    traps: [
      'One matrix cell can contain several meaningful cases. Collapsing them into one sentence hides conditions and may contradict explicit no-effect behavior.',
      'Do not infer missing rules from familiar genre behavior. Unknown closed-exit walking stays unknown until supplied or adopted.',
      'Do not copy shared spending rules into noun identity or verb descriptions. Keep one authoritative owner and link from cases.'
    ]
  },
  'resource-economy': {
    concepts: [
      ['Resource identity and quantity', 'A resource extends a noun identity; its starting amount, capacity, and costs are separate authored controls. Changing a number should not create a different resource or silently alter its meaning.'],
      ['Source, sink, and transformation', 'A source adds quantity, a sink removes it, and a transformation relates quantities or states. Identify the rule and its trigger. Flow direction comes from source prose, not from a generic link between records.'],
      ['Feedback', 'A reinforcing relationship amplifies a change; a balancing relationship counteracts it. Explain the intervening rules and time scale. A rising quantity alone is not proof of either relationship.'],
      ['Strategy dependence', 'The same quantities can produce different outcomes under different decisions, routes, and timing assumptions. An average is not a substitute for inspecting bottlenecks, depleted states, or an alternative strategy.']
    ],
    procedure: [
      'Choose one resource and one disputed claim, such as sufficiency for a declared route. Read noun, resource extension, controls, spending mechanics, and relevant interaction cases. Record the exact claim and scope.',
      'List starting quantity, bounds, changes, and triggers with their source owners. Keep unknown amounts and opaque units explicit. Distinguish authored formula text from evaluated values; this slice does not evaluate formulas.',
      'Trace a concrete action sequence by hand only as a labeled analytical example. State assumptions about successes, failed-action costs, replenishment, and optional routes. Do not call the result an executed simulation.',
      'Examine a contrasting strategy or boundary state. Ask whether costs alter choices, merely delay progress, or permit a dead end under the stated assumptions. Separate quantity feasibility from reachability and desired experience.',
      'Return the flow narrative, disputed assumptions, and one useful measurement proposal. Preserve candidate tuning changes as alternatives. No control override, formula export, balance adoption, or experiment execution follows from this review.'
    ],
    example: [
      ['Authored facts', 'Energy has noun/resource identity; initial-energy and push-cost are controls. Crate-pushing owns spending. The tutorial includes a proposed route requiring three successful pushes.'],
      ['Conditional analysis', 'Starting with energy 2 raises a sufficiency question for three pushes. A conclusion needs the authored cost, replenishment rules, and which attempts spend energy; the resource label alone cannot settle it.'],
      ['Competing options', 'Increasing starting energy, changing cost, adding a recovery source, or choosing another route are distinct proposals with different consequences. None is automatically the right balance fix.'],
      ['Useful next question', 'Is scarcity intended to constrain this route, and can the relevant state actually occur? Keep that design-intent question separate from any hand calculation.']
    ],
    evidence: [
      'A hand-computed route can reveal a conditional inconsistency but cannot establish how players choose routes or whether they perceive scarcity as interesting.',
      'For a later authorized model, specify action policy, failure handling, initial state, and recorded measures. A precise result under hidden behavior assumptions is still misleading.',
      'Inspect extremes as well as typical values. Zero available energy, full capacity, and a costly failed action may expose different rules than a successful route average.'
    ],
    traps: [
      'Do not infer a directed economy graph from an undirected reference list. State missing direction as unresolved.',
      'Do not conflate resource amount with resource identity or mechanic ownership. Tuning data and spending behavior have different homes.',
      'Numerical feasibility is not balance quality. A solvable route can still conflict with intended choices or feedback.'
    ]
  },
  'gameplay-patterns': {
    concepts: [
      ['Pattern as relationship', 'A useful pattern describes a recurring relationship among rules and activities in a context. Sharing a name, theme, or object does not establish that the same relationship exists.'],
      ['Context and consequence', 'Explain when a configuration helps a design intention and what costs it introduces. Patterns are not unconditional best practices; context can reverse an expected benefit.'],
      ['Pattern interaction', 'Two useful structures can reinforce, neutralize, or conflict with each other. Review the combined rules rather than assuming that adding individually attractive patterns improves the whole.'],
      ['Vocabulary and attribution', 'Distinguish an inspected published pattern from an original working description. Use a descriptive local phrase when source evidence is unavailable; do not invent catalog authority for it.']
    ],
    procedure: [
      'Start with the selected rule relationship and intended behavior. Read relevant mechanics, system grouping, dynamic hypotheses, and prior decisions. State what problem the comparison is meant to clarify.',
      'If a specific published pattern is named, inspect that entry before making detailed attribution. Record the source, context, and relevant relationship. Otherwise label the comparison an unverified resemblance or original design description.',
      'Map the relationship onto actual source rules. Identify a meaningful match and a meaningful mismatch. Avoid treating a shared label as a sufficient comparison.',
      'Construct one contrasting configuration serving the same intention. Compare consequences, integration costs, and conflicts with existing rules. Revisit rejected alternatives without silently reviving or adopting them.',
      'Return the narrow comparison, uncertainty, and a fair way to examine the alternatives. Retain each proposal and rationale separately. A pattern label does not authorize implementation or replacement of an existing decision.'
    ],
    example: [
      ['Authored facts', 'Pulling was rejected with rationale; crate-color remains proposed. Deliberate placement is part of the puzzle discussion. A new recovery idea must not erase this memory.'],
      ['Original comparison', 'Describe costly commitment and bounded recovery as working configurations, not verified catalog entries. Ask which rule relationship creates the intended consequence of a placement.'],
      ['Competing option', 'An explicitly proposed recovery action may differ from unrestricted pulling. Compare its conditions and cost against the original rejection rather than assuming every reversible action is equivalent.'],
      ['Useful next question', 'Which aspect of pulling was rejected: reversing commitment, expanding the action set, presentation cost, or something else stated in source? That rationale determines whether the new proposal is genuinely distinct.']
    ],
    evidence: [
      'A published example illustrates a relationship in its own context; it does not prove that transplanting the configuration will create the same behavior.',
      'When comparing prototypes is later authorized, hold the intended problem and observation question stable. A more polished candidate can otherwise appear better for unrelated reasons.',
      'Record cases where the predicted consequence does not occur. A mismatch can reveal an important contextual rule rather than a failed pattern implementation.'
    ],
    traps: [
      'Do not assign mandatory pattern tags to every mechanic. Vocabulary should clarify the selected question, not expand the data model.',
      'A pattern catalog is not a feature shopping list. Adding several configurations can create conflicting incentives and unnecessary rules.',
      'Do not re-propose a rejected option under a new label without confronting its recorded rationale.'
    ]
  },
  'skill-atoms': {
    concepts: [
      ['Learning loop', 'Connect an attempted action, the result and feedback, and a revised understanding. Describe what the player can learn from the difference between intention and outcome, not just what button is pressed.'],
      ['Skill and evidence', 'A skill record describes an intended understanding or expression. Success once may come from chance, copying, or an already known strategy. Repeated explanation and transfer are possible evidence, not automatic requirements or proof.'],
      ['Prerequisite and rank', 'A prerequisite declares an AND dependency in this model. Rank is optional authored conceptual placement. Neither graph depth nor room order determines it, and a null rank must not become zero.'],
      ['Opportunity and mastery', 'A challenge can offer introduction, practice, combination, or transfer. An opportunity on a declared route does not establish that a player used it or acquired understanding.']
    ],
    procedure: [
      'Select one skill or challenge and read its stated understanding, expression, feedback, and evidence sought. Name the learned relationship under review; avoid turning every motor action into a separate mandatory skill.',
      'Inspect immediate prerequisites and relevant declared routes. Keep AND dependencies separate from alternative challenge links. Read descriptions before deciding whether the links represent genuine conceptual needs.',
      'Compare authored ranks without filling nulls or computing replacement values. A prerequisite placed later is a question about intent, not an instruction to reorder the graph.',
      'Trace where the selected skill could be introduced, practised, combined, and used in a changed context. Mark undocumented opportunities and bypasses without declaring those routes impossible.',
      'Return the specific loop, dependency question, and evidence gap. If an edit is explicitly requested, change only the selected authored field. Do not propagate ranks, infer mastered skill atoms, or repair unrelated routes.'
    ],
    example: [
      ['Authored facts', 'Coordinate has rank 2 and AND prerequisites plan-push and route-switch. Route-switch has rank 3; plan-push is unranked. Independent read-switch remains a root at rank 3.'],
      ['Interpretation', 'The coordinate/route-switch ordering invites clarification. It may be intentional conceptual grouping, an unresolved ordering choice, or a mismatch; the declared numbers do not explain the reason.'],
      ['Authorized edit example', 'If the designer requests coordinate rank 3, change that field only. Preserve its identity, body, prerequisites, plan-push null, and both existing switch-chain rank values.'],
      ['Useful next question', 'What should a player understand after combining push planning and switch routing, and what feedback reveals that relationship? That learning question remains even after a rank edit.']
    ],
    evidence: [
      'Distinguish completing one instance, explaining a rule, repeating a choice, and transferring understanding to a changed situation. Each supports a different claim.',
      'Inspect the bypass route as an opportunity question. A player might arrive with prior knowledge; a missing practice record cannot establish incapability.',
      'Native cycle, rank-conflict, and all-route checks are deferred. Manual tracing should state inspected nodes and routes rather than imply exhaustive graph coverage.'
    ],
    traps: [
      'Do not infer rank from prerequisite depth. Late roots and unranked skill atoms are valid authored states.',
      'Do not treat AND prerequisites as interchangeable routes. All declared prerequisite links matter to the conceptual claim.',
      'Fixing one rank does not validate the learning design. Keep structural editing separate from evidence of understanding.'
    ]
  },
  'theory-of-fun': {
    concepts: [
      ['Pattern learning', 'Ask what relationship the player can come to recognize, predict, or use. A new visual arrangement is not automatically a new pattern, and a new number may or may not change the underlying decision.'],
      ['Familiarity and fluency', 'A learned operation can become easier to execute while supporting deeper decisions elsewhere. Fluency is not inherently boredom; repetition may serve comfort, rhythm, expression, or mastery.'],
      ['Combination and transfer', 'Previously learned ideas can form a new relationship or appear under changed constraints. Describe that relationship explicitly rather than treating additional simultaneous mechanics as automatic depth.'],
      ['Perspective, not total explanation', 'This lens emphasizes learning and pattern recognition. It does not reduce every pleasure, emotion, aesthetic intention, or personal preference to a single learning mechanism.']
    ],
    procedure: [
      'Choose one progression segment and its intended experience. Read skill atoms, challenges, mechanics, and any observations. State what learning question this lens should answer.',
      'Describe the decision relationship before and after the segment. Identify what becomes newly understandable, what stays familiar, and what merely changes presentation or quantity.',
      'Examine repetition in context. Ask what repeated action supports and what higher-level choice remains. Do not label a familiar action defective solely because an expert can perform it automatically.',
      'Consider a beginner and an already knowledgeable player as explicit hypothetical perspectives, not measured profiles. Identify where each might still make meaningful discoveries or enjoy fluent action.',
      'Return a bounded learning hypothesis and an alternative explanation for engagement or disengagement. Suggest a useful observation if needed; do not assign a fun score or add novelty without a stated design reason.'
    ],
    example: [
      ['Authored facts', 'Combined-room combines push planning and switch routing. The tutorial supplies prerequisite descriptions but no recorded participant observations.'],
      ['Interpretation', 'The combination could reveal a relationship between irreversible placement and maintaining a switch condition. It could also simply require familiar operations in sequence; challenge prose must distinguish these cases.'],
      ['Competing option', 'A changed arrangement could invite transfer without adding another verb. This remains a proposed comparison, not an adopted challenge or proof that current repetition is weak.'],
      ['Useful next question', 'Can a player explain a newly discovered relationship, or are they applying an already familiar solution? Either outcome needs interpretation relative to the intended experience.']
    ],
    evidence: [
      'Look for changes in prediction, explanation, and strategy, not only faster completion. Speed can reflect motor familiarity, memorization, or a simpler instance.',
      'A statement of enjoyment can support a local experience claim without proving that learning caused it. Preserve competing explanations such as atmosphere, comfort, and expression.',
      'An expert solving immediately does not establish that the challenge teaches nothing. Prior knowledge changes what can be learned from the same encounter.'
    ],
    traps: [
      'More mechanics do not automatically create more learning. Name the relationship that becomes available to understand.',
      'Do not equate all repetition with boredom or all surprise with meaningful novelty.',
      'Do not treat this account as a universal preference model for every player or genre.'
    ]
  },
  flow: {
    concepts: [
      ['Challenge relative to capability', 'Demand matters in relation to what a person can currently do and understand. Authored rank is not a calibrated capability measure, and the same encounter may present different demands to different people.'],
      ['Goals and feedback', 'An actionable immediate goal and interpretable feedback can help a person adjust activity. They need not reveal every future event or solution; uncertainty can be part of an intentional experience.'],
      ['Attention and involvement', 'Sustained attention is a possible experience to investigate. Time spent, silence, repeated attempts, or visible effort alone cannot distinguish absorption from confusion or reluctant persistence.'],
      ['Fit to intention', 'Flow is one perspective on experience, not a mandatory target for every moment. Deliberate interruption, reflection, ambiguity, or discomfort may serve a different supplied intention.']
    ],
    procedure: [
      'Bound the encounter and intended experience. State why sustained involvement is relevant here; do not turn a selected Flow review into a claim that every game should maximize absorption.',
      'Read immediate goals, action feedback, consequences, and declared learning opportunities. Separate what is documented from assumptions about what a player already understands.',
      'Describe one specific demand and its assumed capability. Consider a less familiar and a more familiar perspective as hypotheses. Keep skill ranks out of any invented quantitative capability curve.',
      'Inspect how feedback, optional help, self-selected challenge, or recoverable failure might change that relationship. Treat adjustments as alternatives, not automatic prescriptions to reduce difficulty.',
      'Return the demand/capability assumptions, supporting design facts, and unresolved experience question. If evidence is needed, propose ways to distinguish involvement from confusion without diagnosing anxiety or boredom.'
    ],
    example: [
      ['Authored facts', 'Push-room can lead directly to combined-room or through switch-practice. Source records do not measure player capability or absorption.'],
      ['Interpretation', 'The bypass could increase unfamiliar coordination for someone new to switch routing. It could also suit someone who already understands it. Route structure alone cannot select between those readings.'],
      ['Competing option', 'Optional practice or contextual feedback might support different current capabilities without prescribing one path. This is a design proposal, not evidence that the bypass currently fails.'],
      ['Useful next question', 'Are goals and feedback legible on the bypass route, and what prior understanding is assumed? Answer those questions before attributing a particular emotional state.']
    ],
    evidence: [
      'Combine observations of choices and recovery with voluntary accounts of the experience when testing is authorized. No single visible behavior is a direct flow detector.',
      'Inspect changes across familiarity and context. A challenging first encounter and fluent repeat encounter can support different experience claims without contradicting each other.',
      'Keep persistence, task success, enjoyment, and absorption distinct. A player may succeed without the intended experience or remain involved despite repeated failure.'
    ],
    traps: [
      'Do not plot numeric challenge against skill rank and call the result a measured flow channel.',
      'Do not diagnose anxiety, boredom, or psychological capability from a missing practice link.',
      'Do not substitute GameFlow criteria or scores. Flow theory and the later game-evaluation model are distinct lenses.'
    ]
  },
  gameflow: {
    concepts: [
      ['Evaluation themes', 'GameFlow organizes enjoyment-related questions around eight named elements. Use applicable themes to structure inquiry; this pack supplies original questions, not the published criteria or a scoring reproduction.'],
      ['Control in experience', 'Player control concerns understandable action, agency, and response in the selected activity. Workbench control records are tuning parameters; they do not by themselves describe input mappings or experienced control.'],
      ['Goals and feedback', 'Distinguish knowing what to attempt, seeing the result, and understanding progress. A visible objective can coexist with feedback that leaves an important rule unclear.'],
      ['Applicability', 'Concentration, challenge, skills, control, goals, feedback, immersion, and social interaction are prompts to examine context. Social interaction can be outside scope; omission is not an automatic feature defect.']
    ],
    procedure: [
      'Select an encounter, mechanic, or bounded session and state its intended experience. Identify which GameFlow themes are relevant to that question instead of conducting an unsolicited whole-game checklist.',
      'Read concrete goals, action responses, challenge conditions, and feedback. For each relevant theme, record one source fact or an explicit documentation gap. Avoid judging an interface that source does not describe.',
      'Trace a short attempted action from intention to result. Ask where attention, control, or progress could become unclear. Distinguish an unsuccessful strategy from an unexplained input or rule response.',
      'Mark inapplicable themes and explain the boundary briefly. Consider whether an apparent interruption or constraint serves an intentional experience before recommending removal.',
      'Return a theme-specific question and useful evidence for resolving it. Do not calculate an enjoyment total, reproduce evaluation items, or interpret parameter records as a completed usability review.'
    ],
    example: [
      ['Authored facts', 'The solo puzzle has blocked-push cases and a switch relationship. It does not include recorded play observations or a complete interface specification.'],
      ['Interpretation', 'Blocked-push feedback can be reviewed for whether it explains the relevant condition. Visible switch-state feedback remains a question unless source establishes its presentation.'],
      ['Boundary', 'Social interaction is not a missing requirement in this solo scope. This does not prevent discussing immersion or feedback, and it does not require a multiplayer proposal.'],
      ['Useful next question', 'Can a player distinguish a blocked target from insufficient energy and understand what action remains available? That is a targeted feedback question, not a global enjoyment judgment.']
    ],
    evidence: [
      'Observe attempted actions, perceived responses, and explanations separately. A player recovering after several failures may have learned despite unclear initial feedback.',
      'Compare intended attention with competing presentation only when presentation evidence exists. A record list cannot establish visual clutter, input latency, or immersion.',
      'Retain dissenting observations rather than averaging them into a quality score. They may reveal different assumptions or contexts requiring separate interpretation.'
    ],
    traps: [
      'Do not reproduce the published evaluation instrument under slightly altered labels. Keep these questions independently authored and qualitative.',
      'Do not infer player agency from the existence of adjustable controls such as push-cost.',
      'Do not treat every theme as mandatory or use social inapplicability as a negative score.'
    ]
  },
  'cognitive-load': {
    concepts: [
      ['Coordination demand', 'Ask what information and operations must be handled together at a particular moment. A long prerequisite list may describe learning history rather than simultaneous activity.'],
      ['Familiarity', 'A practiced relationship can be handled differently from several unfamiliar operations. State assumed familiarity and evidence for it instead of assigning a universal cost to each mechanic.'],
      ['Search and learning', 'Effort spent finding a move can differ from effort spent understanding the underlying relationship. A successful solution search does not automatically show what the player learned.'],
      ['Presentation burden', 'Hidden, split, changing, or repeatedly reinterpreted information can add work unrelated to the intended decision. Whether that work is unnecessary depends on the supplied design intention and context.']
    ],
    procedure: [
      'Select a moment or short sequence, not the whole ruleset. Describe the intended reasoning and the operations that must be coordinated at that moment. Mark absent timing or presentation details unknown.',
      'Separate assumed familiar operations from unfamiliar relationships. Use source descriptions and any relevant observations; do not infer familiarity from genre, authored rank, or the presence of a prerequisite record.',
      'Trace information search, rule interpretation, planning, and action. Identify a concrete point where incidental work might compete with understanding the intended relationship.',
      'Propose one targeted support variant, such as stable cues, a reduced example, or a demonstration. State which demand it changes and which meaningful decision it preserves. Keep the variant proposed.',
      'Return the demand hypothesis, alternative explanations, and evidence needed to assess learning. No mechanic-count limit, cognitive-load score, psychological diagnosis, or automatic simplification follows.'
    ],
    example: [
      ['Authored facts', 'Coordinate links push planning and switch routing as prerequisites. Those links do not establish that both must be actively maintained at the same instant.'],
      ['Interpretation', 'If a challenge requires tracking a hidden switch condition while planning an irreversible push, that could create unfamiliar coordination. The conditional claim needs challenge and presentation evidence.'],
      ['Competing option', 'A persistent state cue might remove incidental remembering while preserving placement reasoning. It might instead remove an intended memory demand; intention decides which interpretation matters.'],
      ['Useful next question', 'Where exactly does the player search, remember, or reinterpret information, and what relationship should remain understandable afterward? A prerequisite count cannot answer that.']
    ],
    evidence: [
      'Distinguish immediate completion from later explanation or transfer. Assistance can improve one attempt without demonstrating learning.',
      'Consider alternative causes of failure, including unclear goals, input problems, unfamiliar notation, and missing rule feedback. None is established by a long attempt alone.',
      'If a comparison is later authorized, keep the intended reasoning stable while changing the disputed burden. Do not label the difference a calibrated psychological measurement.'
    ],
    traps: [
      'Two linked skill atoms do not imply two simultaneous mental items. Inspect the actual sequence and context.',
      'Harder is not automatically worse. Remove only work that conflicts with the intended experience or learning question.',
      'Do not present laboratory findings as universal maximum mechanic counts for game encounters.'
    ]
  },
  'working-memory': {
    concepts: [
      ['Active information', 'Identify information needed now that is not readily available in the current presentation. The full number of rules or objects is not the same as the information a player must actively retain.'],
      ['Chunking', 'Familiar elements can form a learned meaningful unit. A designer naming a combination does not make it a chunk for a beginner; recognition depends on learning and context.'],
      ['External support', 'Visible state, stable notation, history, and cues can reduce remembering. They may preserve the intended reasoning or alter it; the design intention determines whether that change is useful.'],
      ['Interference and sequence', 'Information may need attention together, after a delay, or amid competing cues. Describe those conditions instead of treating every visible element as an equally costly memory item.']
    ],
    procedure: [
      'Choose one decision and trace what information is available when it is made. Identify what disappeared, changed, or must be recalled from elsewhere. Mark unspecified presentation unknown.',
      'Separate recognizing visible state from recalling hidden state and reasoning about consequences. Name the actual information involved; do not substitute counts of records, skill atoms, or controls.',
      'Describe how a familiar player might group the information and why a beginner might not. Keep both as assumptions unless relevant observations establish that familiarity.',
      'Consider one stable cue, history aid, or presentation variant. State whether it externalizes information, reveals a solution, or changes the intended memory challenge. Preserve alternatives and trade-offs.',
      'Return the selected memory demand, its context, and a comparison question. Do not apply four, seven, or any other fixed count as a universal failure threshold or capacity diagnosis.'
    ],
    example: [
      ['Authored facts', 'The puzzle links switch routing and crate planning. Source prerequisite links do not specify whether switch state stays visible during a placement decision.'],
      ['Interpretation', 'If switch state disappears while the player plans a push, remembering it could be part of the demand. If it remains visible, a different attention or planning question applies.'],
      ['Competing option', 'A persistent switch cue is a proposed external aid. It may preserve route reasoning while changing remembering, but that effect cannot be assumed without a concrete presentation and intended challenge.'],
      ['Useful next question', 'Which exact state must be retained across which action or delay? Resolve that before drawing conclusions from the number of prerequisite skill atoms.']
    ],
    evidence: [
      'Observe what is forgotten or rechecked, not merely whether an attempt fails. A bad move may reflect rule misunderstanding rather than memory loss.',
      'Compare support variants while preserving the reasoning question. Extra labels can also introduce attention or interpretation costs; more visible information is not automatically better.',
      'Record familiarity and task context in any later observation. A useful chunk for one person or repeated task need not transfer to another arrangement.'
    ],
    traps: [
      'Do not turn historical item-count discussions into universal interface or puzzle limits.',
      'Do not call a multi-step sequence one chunk merely because the designer has given it one name.',
      'Do not assume remembering is unintended friction. Some experiences deliberately involve memory; review fit to the supplied goal.'
    ]
  },
  'zpd-scaffolding': {
    concepts: [
      ['Independent and assisted performance', 'The useful question concerns what someone can currently attempt alone and what assistance changes. A designer assumption about a learner is not an established developmental assessment.'],
      ['Temporary support', 'A cue, demonstration, reduced task, or targeted hint can support a particular difficulty. Explain what the help reveals and what decision remains with the learner.'],
      ['Withdrawal and transfer', 'Support can become less specific or disappear when appropriate. Independent performance in a changed task is a different claim from copying a demonstrated sequence.'],
      ['Related but distinct ideas', 'Vygotsky’s zone of proximal development and the later tutoring/scaffolding work have different origins. This pack uses their relationship as a design perspective, not a single prescribed teaching algorithm.']
    ],
    procedure: [
      'Select one skill and a declared path. Read intended understanding, challenge roles, feedback, and any observations. State what independent and assisted performance would mean in this context.',
      'Identify the specific difficulty the support is meant to address. Keep assumed prior understanding separate from evidence. A missing introduction record is not proof that the player lacks the skill.',
      'Describe one support that leaves a meaningful decision with the player. Compare a cue, explanation, or worked example by what each reveals; avoid treating all hints as equivalent.',
      'Trace where that support could become less specific and where a changed task could expose independent understanding. Respect optional practice and alternate routes instead of imposing a mandatory lesson sequence.',
      'Return the learning/support hypothesis, route boundary, and evidence gap. Propose support changes without creating them. No diagnosis, inferred mastery state, or automatic route repair is permitted.'
    ],
    example: [
      ['Authored facts', 'Push-room offers a direct route to combined-room and a route through switch-practice. Levels are optional; declared challenge links already support a route discussion.'],
      ['Interpretation', 'The direct path bypasses one documented practice opportunity. It does not establish that the player has never learned switch routing or that combined-room is impossible.'],
      ['Competing option', 'A contextual hint in combined-room could support a player who needs it while preserving the bypass. That is a distinct proposal from forcing every player through practice.'],
      ['Useful next question', 'What assistance already exists in source, what would it reveal, and where could a player demonstrate the relationship without it? Keep undocumented answers unresolved.']
    ],
    evidence: [
      'Differentiate completing with a demonstration, applying a partial hint, and solving independently later. These support different claims about the role of assistance.',
      'Inspect whether help reveals a relevant relationship or only an action sequence. A copied solution can conceal unresolved understanding.',
      'Support may help one learner and interrupt another. Record context rather than recommending one compulsory path from route structure alone.'
    ],
    traps: [
      'Do not attribute the later term scaffolding directly to every aspect of Vygotsky’s work.',
      'Do not infer developmental level, inability, or mandatory teaching order from authored graph links.',
      'Do not make support permanently compulsory without considering withdrawal, choice, and the supplied experience goal.'
    ]
  },
  'self-determination': {
    concepts: [
      ['Autonomy', 'Consider whether the player can act with meaningful intention rather than equating autonomy with the sheer number of options. A chosen commitment can coexist with strong rules and constraints.'],
      ['Competence', 'Consider opportunities to understand, act effectively, and recognize growing capability. Easy success, high rewards, and authored skill rank are not direct measures of experienced competence.'],
      ['Relatedness', 'Consider relevant connection and belonging without requiring a multiplayer feature. Whether a particular fictional, social, or shared context supports connection remains a hypothesis about experience.'],
      ['Support and pressure', 'The same rule can be experienced differently depending on explanation, choice, stakes, and context. Mechanics can suggest need-support hypotheses; they do not reveal an individual psychological state.']
    ],
    procedure: [
      'Select one activity and its intended motivation question. Read rules, experience intentions, choices, feedback, and any observations. Do not create mandatory profiles or assign players need scores.',
      'Describe what the player can choose and why that choice might matter. Separate meaningful intention from cosmetic variation or a choice whose consequences source does not explain.',
      'Inspect what communicates understanding and effective action. Ask how constraints and feedback could support competence or create pressure; keep both readings conditional where evidence is absent.',
      'Consider connection only within the selected scope. Mark missing context unknown or irrelevant rather than declaring a solo design psychologically deficient. Do not add social mechanics to satisfy a category.',
      'Return need-support hypotheses, possible tensions, and an observation that could challenge them. Preserve candidate rule changes as proposals; do not infer satisfaction, frustration, or personality from mechanics alone.'
    ],
    example: [
      ['Authored facts', 'Players have a declared route choice, while crate movement is constrained by placement rules. No participant motivation evidence is recorded.'],
      ['Interpretation', 'The route choice could support intentional self-direction; constrained movement could make successful planning meaningful. Neither relationship is established simply by counting choices or restrictions.'],
      ['Counterreading', 'An unexplained consequence could make the same commitment feel arbitrary. That possibility directs a feedback question, not a diagnosis of unmet needs.'],
      ['Useful next question', 'Can the player explain the chosen plan and understand why its outcome followed? If motivation evidence is later requested, keep that evidence distinct from observed task success.']
    ],
    evidence: [
      'Treat player accounts of choice, efficacy, and connection as contextual evidence, not universal truths about the mechanic or a stable psychological profile.',
      'Check competing explanations for continued play, including interest, obligation, rewards, social context, and curiosity. Persistence alone does not establish need support.',
      'Look for tensions as well as support. A proposed challenge increase might change meaningful accomplishment, pressure, and accessibility in different ways.'
    ],
    traps: [
      'More options do not automatically mean more autonomy; easier tasks do not automatically mean more competence.',
      'Solo play does not automatically establish a relatedness deficit or require multiplayer repair.',
      'Do not reproduce questionnaires or convert the three needs into independent fun-point totals.'
    ]
  },
  'quantic-foundry': {
    concepts: [
      ['Motivation vocabulary', 'The published model offers several dimensions of interest rather than one exclusive player identity. Use its attributed labels to clarify intended activities, not to assign an unmeasured preference profile.'],
      ['Activity and appeal', 'An interest-support claim should name what the player repeatedly does. A fantasy theme, completion counter, or strategic label does not by itself establish immersion, achievement appeal, or mastery appeal.'],
      ['Overlap and tension', 'One mechanic can support several interests under different choices. Supporting one interest can also constrain another; identify the actual rule relationship rather than assuming categories are independent feature bins.'],
      ['Version and evidence boundary', 'This pack retains vocabulary from the cited published explanation. It does not claim current questionnaire access, reproduce scoring, or certify that any particular game attracts a measured audience.']
    ],
    procedure: [
      'Start with an interest or experience intention supplied by the designer. If none exists, offer a small optional vocabulary choice rather than classifying the audience automatically.',
      'Select relevant published labels and explain their intended local meaning through activities. Read mechanics, dynamics, challenges, and evidence. Keep the full vocabulary available without reviewing every dimension.',
      'Trace one interest to repeated choices and consequences. Ask what activity, rather than theme or marketing language, supports the connection. Mark the connection an interpretation until evidence supports it.',
      'Examine overlap, conflict, and deliberate exclusions. Compare a plausible alternative appeal of the same activity. Do not add features merely to cover every motivation group.',
      'Return an activity-to-interest hypothesis, its boundary, and a useful disconfirming observation. No survey reconstruction, audience dataset, preference percentage, personality inference, or single player-type assignment follows.'
    ],
    example: [
      ['Authored facts', 'The puzzle emphasizes deliberate planning and constrained crate placement. No surveyed profile or observed motivation is supplied.'],
      ['Interpretation', 'Reasoning about placement can plausibly support Challenge/Strategy interests in the cited vocabulary. This is a statement about intended activity, not a measured description of puzzle players.'],
      ['Counterreading', 'Someone may engage chiefly with completion or curiosity about a solution. The same visible action does not uniquely identify their motivation.'],
      ['Useful next question', 'Which activity is meant to sustain interest after the first solution, and what voluntary account or observed choice could distinguish the intended appeal from a competing explanation?']
    ],
    evidence: [
      'Keep an individual account, a design intention, and a population claim separate. One enthusiastic participant cannot validate an audience segment.',
      'Observed choices may support several interpretations. Repeated retries could reflect challenge interest, completion goals, external pressure, or confusion.',
      'Any later audience research needs its own authorized method and provenance. Published model labels do not supply missing questionnaire results or local validation.'
    ],
    traps: [
      'Do not turn dimensions into mutually exclusive identities or call one person a single motivation type.',
      'Do not infer appeal from theme alone or treat every published label as a required feature.',
      'Do not recreate proprietary questionnaire wording, scoring, or empirical claims from unavailable source material.'
    ]
  },
  'four-keys': {
    concepts: [
      ['Hard Fun', 'Use challenge and earned overcoming as one possible route to emotion. Difficulty alone does not establish triumph; the player’s understanding, investment, and interpretation matter.'],
      ['Easy Fun', 'Use curiosity, exploration, and playful discovery as another route. Easy here should not be read as a universal demand for low challenge or the absence of meaningful decisions.'],
      ['Serious Fun', 'Ask about intended personal meaning or change in experience, such as purposeful engagement or restoration. Keep any claimed emotional effect contextual and unverified without relevant evidence.'],
      ['People Fun', 'Ask what participants do together that might create shared amusement or connection. Mere co-presence is not an established emotional mechanism, and intentionally solo play need not cover this route.']
    ],
    procedure: [
      'Choose an intended emotional moment or short activity. State which key is useful to that question; do not treat all four as compulsory targets for the whole game.',
      'Read the relevant rules, dynamic hypotheses, interactions, and experience intentions. Describe the sequence that could produce the emotion, including anticipation and aftermath where source supports them.',
      'Separate intended feeling from player action and reported reaction. Name assumptions about understanding, stakes, discovery, or social context that connect the sequence to the intended emotion.',
      'Consider one different emotional reading or transition. The same obstacle can invite triumph, relief, annoyance, or disengagement under different conditions; keep unsupported readings hypothetical.',
      'Return the proposed emotional route, supporting facts, and a discriminating question. Do not score emotions, require every key, or adopt presentation or challenge changes during review.'
    ],
    example: [
      ['Authored facts', 'Combined-room brings together push planning and switch routing. The tutorial records no reactions to success or discovery.'],
      ['Interpretation', 'Understanding the switch relationship could invite curiosity and solving the combination could invite triumph. The events and emotional interpretations are distinct claims.'],
      ['Counterreading', 'A solution reached by copying could feel different from a self-derived plan. An unclear failure could change anticipation into confusion; neither response is established by the current source.'],
      ['Useful next question', 'Which moment should change the emotional tone, and what account or behavior would support that interpretation without treating a smile, retry, or completion as a direct emotion measurement?']
    ],
    evidence: [
      'Relate any observed reaction to its timing and context. The reaction after a solution may concern relief, social response, or an unrelated event.',
      'Keep curiosity, triumph, meaning, and shared amusement distinguishable. They can overlap without becoming interchangeable labels for general enjoyment.',
      'A later authorized observation should permit unexpected reactions. Evidence collection that only asks whether the intended emotion occurred can hide competing interpretations.'
    ],
    traps: [
      'Do not translate Hard Fun into maximum difficulty or Easy Fun into trivial mechanics.',
      'Do not equate these keys with MDA aesthetics or Koster’s learning account; their purposes and vocabulary differ.',
      'Do not treat absent social play or unrecorded emotion as a failed category score.'
    ]
  },
  bartle: {
    concepts: [
      ['Historical scope', 'The article concerns multiplayer MUD participation and relationships among activities. Apply that context before borrowing the familiar labels for another game.'],
      ['World and participants', 'Distinguish activities directed toward the game world from those directed toward other people. A non-player character is not automatically equivalent to another participant in this frame.'],
      ['Acting and interacting', 'Examine whether an activity emphasizes exerting effects or engaging with and understanding its target. Use the distinction to discuss concrete activity, not to diagnose a person.'],
      ['Approaches and tensions', 'Achieving, exploring, socialising, and exerting influence over others can coexist or change over time. One encouraged activity can also undermine another participant’s experience; inspect the actual rules and boundaries.']
    ],
    procedure: [
      'Check whether the selected question involves meaningful multiplayer participation. If it does not, report limited applicability and stop this lens. Do not invent social systems to make the frame fit.',
      'For applicable scope, name actual participant activities and their targets. Read rules, dynamic hypotheses, challenge descriptions, and relevant decisions rather than assigning labels from genre.',
      'Use the two distinctions to compare approaches to those activities. Allow the same person to pursue different approaches at different times. Mark uncertain mappings as interpretations.',
      'Inspect interactions between approaches, including consent, disruption, recovery, and shared incentives where relevant. Distinguish intended competitive effects from assumptions about harmful behavior.',
      'Return the activity tension and its design context. No player-type percentages, population balancing prescription, psychological labels, or requirement to support all four approaches is justified.'
    ],
    example: [
      ['Authored facts', 'The crate puzzle is intentionally solo and supplies no multiplayer participation rules. A switch or crate is not another human participant.'],
      ['Applicability result', 'The historical MUD frame has limited use for the selected puzzle. Report that boundary rather than mapping every puzzle activity into a player type.'],
      ['Rejected inference', 'Solving a puzzle does not make its player an Achiever in a validated universal taxonomy. Likewise, moving a crate forcefully does not make the activity equivalent to acting on other players.'],
      ['Useful next question', 'If a later task explicitly introduces multiplayer participation, what participant activities and tensions need inspection? That would be a new supplied scope, not a repair required by this review.']
    ],
    evidence: [
      'For an applicable multiplayer design, observe activities and their effects on other participants. Labels alone do not explain why a conflict or cooperation pattern occurs.',
      'Separate a temporary approach from a stable identity. A person exploring now may compete or socialise later under different conditions.',
      'Historical observations do not establish category percentages or optimal population ratios for a different design. Any such local claim needs separate evidence.'
    ],
    traps: [
      'Do not apply the taxonomy universally after removing its multiplayer context.',
      'Do not equate Killers with all combat, all competition, or a psychological diagnosis.',
      'Stopping an inapplicable lens is a valid outcome; it does not require recommending multiplayer features.'
    ]
  },
  caillois: {
    concepts: [
      ['Agon and alea', 'Competition and chance are different sources of uncertainty in this vocabulary. Not knowing a solution does not establish random rules, and a hypothetical timing distribution does not add chance to the game itself.'],
      ['Mimicry and ilinx', 'Make-believe and vertigo direct attention to different forms of experience. A fictional theme does not fully specify enacted make-believe; described sensory intensity does not establish experienced disorientation.'],
      ['Paidia and ludus', 'Use improvisational freedom and disciplined rule-bound challenge as contrasting tendencies. They are not a maturity ladder, a numeric freedom score, or an exhaustive binary classification of a game.'],
      ['Mixture and intention', 'Several forms can coexist, while others can be deliberately absent. The useful question concerns what a mixture enables or constrains in the selected activity, not how many categories it covers.']
    ],
    procedure: [
      'Choose one activity and read its rules, presentation context, intended experience, and declared challenge structure. Keep thematic description separate from what the player actually does.',
      'Identify which forms, if any, clarify that activity. Explain the concrete basis for each interpretation and mark absent presentation or rule evidence unknown.',
      'Distinguish sources of uncertainty: another participant, random procedure, hidden state, discovery, or interpretation. Do not collapse them into chance because the outcome is initially unknown.',
      'Examine where improvisation remains and where formal constraints define the challenge. Ask whether adding or removing a form would serve the supplied intention rather than satisfy a category quota.',
      'Return the selected conceptual contrast and its boundary. No taxonomy score, universal completeness judgment, or automatic addition of competition, randomness, role-play, or sensory disorientation follows.'
    ],
    example: [
      ['Authored facts', 'The puzzle rules are deterministic. A proposed timing model assumes independent player delays; no timing trials or emotional observations have been executed.'],
      ['Interpretation', 'Prescribed placement challenges make rule-bound play a useful discussion. The amount of improvisational freedom still depends on available approaches and source context.'],
      ['Rejected inference', 'A uniform delay assumption in an experiment does not establish alea in the game rules. Uncertainty about a solution is also not evidence of a random procedure.'],
      ['Useful next question', 'What freedom does the player have within the placement constraints, and is that freedom part of the intended experience? Other forms can remain outside scope.']
    ],
    evidence: [
      'Use rule evidence to establish random procedures or competition; use presentation and activity evidence for make-believe or sensory claims. One source type cannot answer every question.',
      'Historical categories offer vocabulary, not causal proof of enjoyment. Keep interpretation separate from any later player account.',
      'If disorientation is explicitly intended, discuss boundaries and accessibility as design questions without claiming a safety assessment from this conceptual review.'
    ],
    traps: [
      'Do not treat paidia/ludus as a good/bad or primitive/advanced scale.',
      'Do not infer randomness from hidden information, unknown solutions, or modeled human variation.',
      'Do not recommend missing forms merely to make the taxonomy appear complete.'
    ]
  },
  schell: {
    concepts: [
      ['Contextual question', 'A reflective question is useful when it exposes a concrete assumption about an actual player moment. Broad questions that cannot change a decision add little to a bounded review.'],
      ['Perspective and evidence', 'Changing perspective can reveal an alternative interpretation without making it true. Keep the new viewpoint tied to source facts and identify what would support or challenge it.'],
      ['Essential relationship', 'Ask what a rule contributes and what would change without it. A counterfactual can clarify purpose, but it remains a thought experiment until supported by an appropriate comparison.'],
      ['Consequences and memory', 'A local improvement can create costs elsewhere. Retained decisions and rejected alternatives help distinguish a genuinely new option from a renamed version of an old proposal.']
    ],
    procedure: [
      'Select the exact design question and player moment. Read relevant source, intended experience, and prior decisions. State the assumption or choice this reflection should clarify.',
      'Describe what the player needs to notice, decide, and experience in that moment. Separate supplied facts from imagined reactions, and mark missing context explicitly.',
      'Question one important rule through a counterfactual or contrasting proposal. Explain what the alternative preserves, changes, and costs; do not generate unrelated options for breadth alone.',
      'Trace the consequences into adjacent rules, challenges, and retained rationale. Surface the strongest reason against the attractive option rather than treating a fresh perspective as automatic improvement.',
      'Return the narrowed question, distinct alternatives, and the assumption worth examining next. These are original reflective prompts, not numbered Schell lens cards or permission to run other frameworks.'
    ],
    example: [
      ['Authored facts', 'Pulling is rejected with rationale, while deliberate placement remains important to the puzzle discussion. A different recovery proposal has not been adopted automatically.'],
      ['Reflection', 'Ask what commitment contributes to planning and what would remain if a recovery action were possible. The answer should address an actual decision, not merely declare reversibility good or bad.'],
      ['Competing option', 'A bounded recovery action could differ from unrestricted pulling. Compare its rule, cost, and effect against the specific rejection rationale while keeping both alternatives available.'],
      ['Useful next question', 'Which assumption matters most: that commitment creates tension, that recovery reduces frustration, or another supplied intention? Name the comparison needed before choosing.']
    ],
    evidence: [
      'A thought experiment can expose an assumption but cannot establish an experienced outcome. Label its conclusion accordingly.',
      'A fair later comparison should address the disputed rule relationship, not reward whichever candidate has more polish or content.',
      'Retain contrary evidence and the reason an alternative was rejected. Changed context may justify reconsideration without rewriting prior history.'
    ],
    traps: [
      'Do not invent lens-card names, numbers, or attributed wording to give original questions borrowed authority.',
      'Do not turn a reflective review into an unsolicited sweep of every framework.',
      'Do not confuse producing more alternatives with resolving the decision that matters.'
    ]
  },
  playcentric: {
    concepts: [
      ['Experience goal', 'Describe an intended player experience precisely enough to influence a design choice. Keep it separate from a feature list and from a claim that the experience has already occurred.'],
      ['Testable uncertainty', 'Choose a claim whose possible answers could change the next decision. State an outcome that would challenge the preferred interpretation, not only observations that would confirm it.'],
      ['Question-shaped prototype', 'A prototype should expose the uncertainty with an appropriate amount of implementation. A timing model, paper interaction, and playable scene answer different questions and omit different parts of experience.'],
      ['Evidence and iteration', 'Separate assumptions, analytical derivations, executed model output, and observed play. A resulting design choice is another step that needs explicit authority; evidence does not silently adopt an option.']
    ],
    procedure: [
      'Connect one supplied experience goal to a decision that remains open. Read relevant experiments, their hypothesis/expectation/observation sections, measures, and decisions. Incomplete investigation is valid input for discussion.',
      'Write the uncertain claim and a plausible rival explanation. Specify what observation or result could distinguish them. Keep player experience claims separate from numerical model predictions.',
      'Propose the smallest suitable prototype or method. State what it includes, what it omits, what would be recorded, and what assumptions limit the answer. Proposal does not authorize execution or participant recruitment.',
      'Inspect existing evidence provenance before interpreting it. Identify source, method, conditions, and whether the result is analytical, simulated, observed, or merely expected. Empty observations remain empty.',
      'Return the decision-linked question, proposed method, and interpretation boundary. If evidence later exists, explain what it changes and what remains uncertain; retain alternatives and rationale instead of automatically adopting a repair.'
    ],
    example: [
      ['Authored facts', 'The illustrative route has 4 walks and 3 pushes, with base time 5 s. A proposed model assumes independent uniform delays of 0–1 s per action. No trials or participant observations exist.'],
      ['Analytical reference', 'Under those assumptions, total time lies between 5 and 12 s and probability of at most 8 s is 1312/5040, about 0.2603. This is a derivation, not measured success or evidence about real player delay.'],
      ['Interpretation boundary', 'An executed model could later check implementation against the analytical reference. It still would not validate the delay assumptions or show that a deadline creates the intended experience.'],
      ['Useful next question', 'Is the next decision about arithmetic correctness, plausible player timing, or the experience of pressure? Each needs a different method; do not use one precise result to answer all three.']
    ],
    evidence: [
      'Record contradictory outcomes and method limitations, not just a preferred result. An unexpected observation may challenge the model, the hypothesis, or the procedure.',
      'Separate measurement from expectation comparison and design judgment. Meeting a numeric threshold is not proof that the intended experience occurred.',
      'Later simulation execution requires its own explicit one-shot approval and checkpoint protocol. This slice neither runs simulations nor turns an experiment proposal into completed evidence.'
    ],
    traps: [
      'Do not treat a proposed method, analytical probability, or expectation as an executed result.',
      'Do not infer participant counts, motivation, emotion, or capability from a timing distribution.',
      'Do not let evidence storage imply adoption. Preserve the resulting decision, rejected alternatives, and remaining uncertainty separately.'
    ]
  }
};
