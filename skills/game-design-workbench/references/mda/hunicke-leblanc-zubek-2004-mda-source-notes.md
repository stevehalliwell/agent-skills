# MDA paper — independently authored source notes

## Source and rights

- Original title: *MDA: A Formal Approach to Game Design and Game Research*.
- Authors: Robin Hunicke, Marc LeBlanc, Robert Zubek.
- Date: 2004. Developed through GDC Game Design and Tuning Workshops, 2001–2004.
- Source type: AAAI workshop research paper.
- Original publication record: https://aaai.org/papers/ws04-04-001-mda-a-formal-approach-to-game-design-and-game-research/
- Author-hosted full paper inspected: https://www.cs.northwestern.edu/~hunicke/MDA.pdf
- Access date: 2026-10-21.
- Method: `web_search` discovery; `fetch_content` retrieved AAAI abstract but only a title-sized PDF result. Downloaded author-hosted PDF with curl into isolated temporary scratch. Converted locally with Docling Serve v1.30.0 at loopback, using installed skill client, asynchronous request, no OCR, no images. Read successful text conversion, then wrote these selective notes independently.
- Reuse basis: no explicit redistribution or adaptation license established from inspected record, extracted paper text, or author site. Public availability does not establish permission. These are source notes, **not a full-source Markdown conversion**. Later local-retention pass added [actual full paper Markdown and figures](captures/hunicke-leblanc-zubek-2004/MDA.md), captured 2026-10-21; local-only, not publication-cleared. Original PDF remains online, not retained locally.
- Fidelity limits: covers paper's substantive sections in selective summary. Figure geometry, exact page layout, and graphical labels were not checked; conversion omitted images. Section names below locate claims without relying on uncertain PDF page mapping. Paper presents a design method and illustrative analyses, not a controlled validation of emotional effects.

## Core contribution

MDA supplies three connected views for reasoning about games. Its goal is shared analysis across design, engineering, criticism, and research, not a recipe that guarantees enjoyment. Interactions among implementation choices can change play in ways designers did not foresee. Moving between concrete implementation and desired experience helps expose those dependencies. [Paper: “Introduction,” “Towards a Comprehensive Framework.”]

### Distinguish authored structure, behavior, and experience

- **Mechanics:** implementation-level components and procedures; later discussion also includes available player actions and controls, together with game content. The paper's examples therefore use this term more broadly than numerical tuning alone.
- **Dynamics:** behavior during play as mechanics interact with player input and with changing game state over time. Listing weapons or rules does not yet explain a dynamic; describe how their use unfolds.
- **Aesthetics:** desired emotional qualities of interacting with the game. This is not a synonym for graphics or asset style.

These views connect, but do not coincide. Authoring an action does not fully specify its use or how a particular player will experience it. [Paper: “MDA,” “Mechanics.”]

### Designer and player perspectives differ

Designers can reason from implementation through resulting play toward experience. Players encounter experience and behavior before understanding underlying mechanisms. The paper recommends considering both perspectives to avoid feature-first reasoning and to notice effects that cross abstraction levels. It explicitly treats the views as causally connected; that conceptual claim is not evidence that a particular proposed chain has occurred in an untested game. [Paper: “MDA as Lens.” Final sentence here distinguishes methodological framing from empirical support.]

## Aesthetics as a working vocabulary

The paper introduces eight familiar labels: Sensation, Fantasy, Narrative, Challenge, Fellowship, Discovery, Expression, Submission. Its purpose is more precise conversation than a single judgement of “fun.” The list is explicitly open-ended, not exhaustive. Games can pursue several qualities with different emphasis; players and occasions can differ. [Paper: “Aesthetics.”]

Two details matter when citing this taxonomy:

1. The paper's own Quake example includes Competition, beyond the eight listed labels. Treating the list as a closed classification would misrepresent its use.
2. Aesthetic labels state a design concern; they do not provide a validated questionnaire, measurement scale, or universal formula for appeal.

The aesthetic-model discussion then asks what conditions make a selected experience plausible. For competition, merely placing opponents in a game is insufficient: participants need investment in winning and usable information about relative success. [Paper: “Aesthetic Models.”]

## Dynamics need explanatory models

The paper moves from desired experience to models of interaction rather than stopping at genre labels. Its examples include cooperation supported by divided information or goals hard to accomplish alone, and expressive play supported by opportunities to personalize or alter game content. These are proposed supporting relationships, not guarantees for every player. [Paper: “Dynamic Models.”]

Two contrasting model types show what analysis can do:

- **Probability:** a two-dice movement model can inform estimates of progress around a board. This is an account of state transitions, not direct emotional measurement.
- **Feedback:** Monopoly illustrates how accumulated advantage can make further advantage easier to obtain. Authors connect widening disparity to reduced investment among trailing players. That interpretation motivates changes worth trying; the paper does not report a controlled study of those players.

Keep the distinction between an explanatory model and a list of components. A feedback account describes what changes, what that change enables, and how the result feeds back into later play. [Paper: “Dynamic Models.”]

## Mechanics and tuning form an iterative loop

The Monopoly discussion proposes several possible interventions: aid trailing players, burden leaders, or accelerate the game. These approaches need not have the same experiential or thematic consequences. Even a promising balancing rule can introduce costs elsewhere; more calculation can make competitive standing harder to understand. [Paper: “Mechanics,” “Tuning.”]

The useful methodological sequence is:

1. Identify desired experience and suspected behavior.
2. Build a model that makes a concrete relationship inspectable.
3. Propose a rule or parameter change.
4. Playtest and adjust rather than treating the model as final confirmation.

This sequence summarizes the paper's analysis, not an experimental protocol supplied by the authors. Paper does not specify sample sizes, emotional instruments, or statistical acceptance thresholds.

## AI example: evaluate components in context

The extended example changes the goals and audience of a tag-like interaction across three hypothetical designs. Changes in intended experience alter required agent behavior, navigation, sensing, coordination, and state representation. A simple demonstration and a stealth-oriented game cannot be assessed against one context-free notion of good AI. Technical sophistication alone does not establish that a component serves the intended play. [Paper: “MDA at Work,” “First Pass,” “Second Pass,” “Third Pass,” “Wrapping Up.”]

The audience assumptions in these examples are historical design assumptions, not demographic findings. Do not reuse them as evidence about age or gender preferences.

## Research-use boundaries

- Use this paper to ground meanings, relationships, and iterative design reasoning.
- Cite hypothetical examples as illustrations, not measured outcomes.
- Distinguish desired aesthetic, predicted behavior, and observed experience when using these notes in later research.
- The paper contains no observations of this repository's example game. It cannot establish that any mechanic here delivers tension, planning, or another player response.

## Comments

Companion [author-site notes](leblanc-eight-kinds-of-fun-author-site-notes.md) document provenance and small vocabulary differences. Those differences should not be silently normalized into a single verbatim source.
