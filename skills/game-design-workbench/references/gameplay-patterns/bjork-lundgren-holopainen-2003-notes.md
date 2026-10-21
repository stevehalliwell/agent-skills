# Game Design Patterns — source notes

Independent research notes, not source conversion or a pattern-catalog reproduction.

Actual source text: [complete local Docling capture](captures/game-design-patterns-2003.docling/game-design-patterns-2003.md). Capture method, checks, and local-only rights limits: [README](README.md).

## Source record

- Original title: *Game Design Patterns*.
- Authors: Staffan Björk, Sus Lundgren, Jussi Holopainen.
- Date: 2003; DiGRA record lists 2003-01-01. Conference held 4–6 November 2003.
- Type: conference research paper, *Level Up*, printed pp. 180–193.
- Original URL: https://dl.digra.org/index.php/dl/article/view/60
- DOI: https://doi.org/10.26503/dl.v2003i1.60
- Inspected PDF: https://dl.digra.org/index.php/dl/article/download/60/60
- Accessed: 2026-10-21.
- Initial notes method: web search discovery; `fetch_content` readable and raw HTML for publication metadata and actual PDF URL; `fetch_content` built-in PDF text extraction; read entire 15-page temporary Markdown extraction. Notes authored separately from extracted text. That notes pass did not use Docling; separate retained capture linked above now does.
- Reuse basis: restricted. PDF opening copyright notice reserves rights to authors, Utrecht University, and DiGRA; permits short quotations for criticism/review, not general reproduction. Full extracted source text now retained separately for private offline reference; no publication clearance claimed. Original PDF remains online, not retained locally.
- Coverage limits: extraction inspected text across entire paper, including references. Two-column layout became long paragraphs; no figure/layout fidelity claimed. These notes select arguments relevant to gameplay-pattern vocabulary and its limits, not every historical comparison or reference.

## Main contribution

Paper proposes an interaction-centered language usable across game design and analysis. Genre labels group games too coarsely and inconsistently. Names for mechanics are useful starting points, but often omit how a rule operates in context or relates to other rules. Patterns add that relational knowledge. [Paper, pp. 181–185]

This is not simply Alexander's problem-and-solution approach transplanted into games. Authors want patterns to support invention and description as well as repair. Defining every pattern as a solution risks treating creative choices only as ways to remove defects. Some broad effects can also be produced by more specific patterns, so a single fixed problem/solution pairing is insufficient. [pp. 183, 185]

Structural framework and pattern collection developed together but need not be adopted together. Paper explicitly allows patterns to be used with another structural framework. Its framework distinguishes boundaries of play, allowed activity, temporal execution, and physical/virtual participants and objects; patterns describe interaction involving these components. [pp. 184–185]

## What a pattern description contributes

Pattern format separates several kinds of knowledge. [p. 185]

- Name serves recall after reading; intuitive interpretation of a name is not enough.
- Description identifies recurring interaction, examples, and scale of occurrence.
- Consequences discuss possible effects and trade-offs, not an unconditional endorsement.
- Use discussion identifies context-specific choices needed to realize the interaction.
- Relations connect broad characteristics, more specific realizations, and configurations difficult to combine.

Later discussion describes collection as a network rather than a strict tree. Authors acknowledge limits of the initial relation types, including cases where patterns jointly realize one another. A list of independent tags would lose this part of the model. [p. 189]

## How authors developed candidates

Initial work combined transformations of existing mechanics, analysis of games and concepts from other fields, and interviews with 9 professional game designers. Mechanics were not copied one-for-one: candidates could merge, split across abstraction levels, or be discarded. [pp. 183–184]

Candidate development involved identifying interaction, explaining examples, considering what removing it would change, writing a description, trying a simple prototype, and evaluating usefulness and descriptive sufficiency. These activities became recursive rather than a fixed sequence. More than 200 candidates existed at publication; this count describes work in progress, not a fully validated catalog. [pp. 183–184]

Removal is a useful analytic countercheck: if deleting the alleged relationship changes nothing relevant, shared terminology alone provides weak evidence for a match. This is a reading of the authors' analysis method, not an additional published pattern.

## Inspected example: Paper Rock Scissors

Paper's worked entry concerns non-transitive advantages and trying to anticipate another participant's choice. Its important relationship is cyclic advantage, not merely three objects with different names. [pp. 186–187]

Entry distinguishes immediate confrontation from longer-term strategic investment. Access to opponent information and repeated interaction matters: these can give anticipation or learned opponent tendencies a role rather than leaving choices effectively uninformed. Related information, investment, and ability configurations change how the same broad relationship works. [p. 186]

These are authors' design claims and examples, not a controlled measurement of tension or mastery. A local design with three abilities does not thereby establish this pattern, balanced outcomes, or the predicted player response.

## Uses and evidence limits

Authors discuss inspiration, concept elaboration, communicating design rationale, solving gameplay issues, analysis, comparison, and exploring new media. Describing alternatives by replacing one relationship with another can help explain design decisions. These are proposed uses, not one mandatory pattern workflow. [pp. 187–188]

Paper is explicit that substantial feasibility evidence remained to be collected. Workshop analysis and experimental prototypes provide early experience, not proof of effectiveness across projects. Discussion also identifies effort of building a collection, uncertainty over pattern granularity, and difficulty locating useful entries inside a large interconnected set. [pp. 188–190]

Authors reject the idea that patterns require repetitive games: knowledge of recurring elements can support novel combinations. That argument does not establish that any particular combination will work. Published examples support comparison; observations of a selected game's players still require separate evidence. [p. 190]
