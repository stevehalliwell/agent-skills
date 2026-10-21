# Concept and working mode

## Source contract

One live project concept document; default `game-design/concept.md`. An explicitly selected existing document, including README, can remain owner. README links to owner rather than copying intent. Record a non-default owner in project instructions, not a machine manifest. This is not another collection or a full rules database.

Use [concept template](../templates/concept.md) for new material. Keep brief roughly 1–3 pages when useful; no word-count gate. Preserve supplied voice. Player story describes an experience worth recounting, not a feature list or technical input sequence. Detailed rules, configuration, and investigations belong in existing collections. Unknowns are valid; reference documents are not automatically adopted game facts or market evidence.

## Mode header

Keep exactly one ordinary Markdown line before first `##` section, outside frontmatter/code fences:

```text
Working mode: game-design
```

Allowed values: `game-design`, `game-explorer`. Mode is project-wide, not a review/session/investigation override. Missing mode stays unconfigured; malformed/duplicate mode needs correction. Do not infer mode from file presence or invent goals to configure it.

- **Game design:** work toward intended experience for intended player. Compare proposed rules, framework interpretations, and evidence against stated direction. Ask focused questions when necessary intent is unknown; no forced complete interview.
- **Game explorer:** combine possibilities, inspect generated behavior, and pursue promising leads toward potential experiences. Missing destination is not a defect. Candidate directions remain proposed until adopted.

Keep original intent, current goals, working draft, and framework interpretations distinct. Goal revision needs explicit direction. Change mode by a precise header edit, preserving body, constraints, and history. Switching mode does not adopt a goal or abandon constraints.

## Player experience and MDA

For explicit emotional targets or arcs, use [shared emotional-experience guidance](emotional-experience.md): precise meaning, experiencer, target, possible elicitor, and transition when useful. Support simple plain-language briefs and highly specific feelings without requiring a taxonomy, dictionary recognition, or an emotion interview. Audience labels do not establish capability or preference.

Overall intent/player story and individual MDA A aspects are different lenses. An aesthetic record does not replace or translate the entire brief. Compare support, contradiction/tension, missing evidence, discovery outside intent, and possible goal drift. A conflict prompts discussion; it does not authorize revising design or goals. Declared links alone do not prove player experience.

## Git history and comparisons

Commit completed authorized concept changes through ordinary Git. Inspect source/index first; scope commit and preserve unrelated work. Read-only review does not commit existing drafts, initialize Git, fetch missing history, or adopt supplied references.

Use Git status/diff to distinguish committed text from a working draft. Untracked text is not committed intent. Missing Git/history is a stated limitation, not invented provenance. Latest commit affecting concept is not automatically original intent; a blank template is not an adopted goal.

When comparison is requested, select an actual committed intention and record its full commit and path. For renamed source, use former path at that revision:

```sh
git log --follow -- <concept-path>
git show <full-commit>:<path-at-that-commit>
git diff <full-commit> -- <current-concept-path>
```

These are ordinary Git commands, not workbench APIs. Inspect selected historical text and current text before interpreting differences. Report unavailable history rather than silently substituting another revision. Keep comparison selection explicit in findings; do not move it to justify a conclusion. Never run historical code merely to compare documents.
