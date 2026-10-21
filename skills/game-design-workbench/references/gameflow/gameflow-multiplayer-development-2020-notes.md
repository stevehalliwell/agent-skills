# GameFlow in multiplayer development — source notes

Independent research notes. Not source transcription, converted paper, or replacement evaluation instrument. [Actual local source capture](captures/gameflow-multiplayer-2020/gameflow-multiplayer-2020.md) retained separately on 2026-10-21; local-only, not publication-cleared.

## Source and rights

- Original title: *Evaluating GameFlow in a Multiplayer Online Strategy Game Under Development*.
- Authors: Penny Sweetser, Daniel Johnson, Jay Kyburz.
- Date: 2020; ACSW conference, February 04–06, Melbourne, Australia. Paper identifies 11 pages.
- Original DOI: https://doi.org/10.1145/3373017.3373068
- Institutional record: https://openresearch-repository.anu.edu.au/items/dba45f73-579a-437d-8f56-463aaac1682c
- Read PDF: https://openresearch-repository.anu.edu.au/bitstreams/230a1d79-5ffd-4572-aee0-e26e50ed435f/download
- Access date: 2026-10-21.
- Source type: author conference research article; empirical application and critique of GameFlow.
- Reuse basis: PDF states copyright 2020 held by owner/authors, publication rights licensed to ACM. Its permission notice covers qualified personal/classroom copying and abstracting with credit; broader posting/redistribution requires permission. No broad reuse license established. Original notes-only delivery retained no PDF, instrument, score tables, screenshots, or close paraphrase. Actual source Markdown and detected figures are now retained separately for offline reference; no public redistribution approval implied.
- Actual method: web search discovered institutional PDF; `fetch_content` returned only a short title-like extraction. Downloaded PDF with curl to isolated temporary storage. Converted using installed Docling client against shared loopback v1.30.0 service, synchronous Markdown-only extraction, no OCR or image export. Conversion status `success`; read full extracted text, including methods, findings, model critique, and conclusion. No remote OCR or LLM enrichment.
- Coverage limits: full extracted article reviewed. Two-column extraction interleaves some opening metadata; figure content was not interpreted. Notes are selective, not exhaustive. Search initially associated this bitstream with the 2017 genres/platforms paper; actual PDF title and DOI identify this 2020 paper. Delivery uses verified PDF identity, not search-result labeling.

## Research question and method

Earlier GameFlow studies largely concerned completed games and researcher-led expert reviews. This study asks how the model works for an unfinished multiplayer game and whether a wider range of reviewers exposes unclear assumptions. See **1 Introduction** and **3 Method**.

Twelve stakeholders reviewed *Neptune's Pride 2: Triton*: experienced players, researchers, journalists, and the developer. Reviewers had completed at least one game of the beta before reviewing it. A single game can take weeks. Reviewers provided ratings, explanations, and comments on the evaluation criteria themselves. Authors compared the developer's judgments with other reviewers' judgments, prepared feedback for the developer, and interviewed the developer about usefulness. See **3.1–3.2** and **5.1**.

The game is a slow online strategy game in which diplomacy, alliances, information, and other players shape the experience. That setting makes it especially useful for testing assumptions inherited from bounded single-player sessions.

## Findings that matter

### Game boundary changes the answer

The developer sometimes treated a condition as absent when code did not explicitly deliver it. Other reviewers treated the condition as present when it emerged through play. Opponent-generated challenge and player-chosen intermediate goals illustrate this disagreement. Different ratings can therefore reflect different definitions of the evaluated object, not merely different quality judgments. See **4.2**, **4.5**, **5.1**, and **7 Discussion**.

### Low sensory density does not establish low involvement

A sparse interface with little audiovisual stimulation can still support sustained planning, emotional stakes, and attention between short visits. Some conventional immersion descriptions fit this play pattern poorly. Authors question whether the existing immersion category should remain unchanged; they do not establish a universally correct replacement. See **4.1**, **4.7**, and **7**.

### Instrument wording affects evidence

Reviewer comments reveal uncertain meanings, bundled concepts, and assumptions about particular game formats. A statement about what a game ought to offer can also be mistaken for a request to endorse that design principle, rather than assess the actual game. These are measurement problems, not defects that can be solved by averaging more ratings. See **6 Evaluating the GameFlow Model** and **7**.

### Review roles expose different priorities

Developer and player priorities can differ even when they agree on a specific problem. The developer may pursue an intended experience or business/community goal that does not match a player's preferred change. Authors treat those disagreements as meaningful findings. Agreement on the overall mean does not erase element-level disagreements. See **5.1** and **7**.

## Limits and cautions

- One game, one development setting, and 12 selected reviewers do not establish universal reliability or causal effects on enjoyment.
- Experienced players are not interchangeable with first-time players. Prior familiarity shapes what reviewers can notice or take for granted.
- This is an expert-review application with qualitative feedback, not a controlled trial of the resulting recommendations.
- Difficulty judging an internal experience retrospectively remains an evidence limit. A game's structural support for involvement does not prove that each player experienced it.
- The paper reports numerical aggregation. These notes deliberately do not reproduce its instrument or scoring tables and do not propose adding scoring to the workbench.
- Findings concern this multiplayer game. They are not proof about any example game's players or about solo games in general.

## Complementary reading

[2013 author follow-up notes](revisiting-gameflow-detailed-heuristics-2013-notes.md) explain the distinction between enjoyment and flow, and the move from broad elements to genre-specific guidance. This 2020 study adds concrete evidence that context, reviewer interpretation, and evaluation wording remain unresolved concerns.
