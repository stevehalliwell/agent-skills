# Concept and working mode

## Source contract

One live project concept document; default `game-design/concept.md`. An explicitly selected existing document, including README, can remain owner. Installed manifest stores `concept_path`; README links to owner rather than copying intent. This is not another collection or a full rules database.

Use [concept template](../templates/concept.md) for new material. Keep brief roughly 1–3 pages when useful; no word-count gate. Preserve supplied voice. Player story describes an experience worth recounting, not a feature list or technical input sequence. Detailed rules, tuning, and investigations belong in existing collections.

Template covers purpose/razor, intended player, intended experience, player story, play over time, features/hook, platform/context, constraints/risks, open questions, and optional related products/commercial assumptions. Unknowns are valid. Reference documents are not automatically adopted game facts or current market evidence.

## Mode header

Keep exactly one ordinary Markdown line before first `##` section, outside frontmatter/code fences:

```text
Working mode: game-design
```

Allowed values: `game-design`, `game-explorer`. Mode is project-wide, not a review/session/investigation override. Missing mode stays unconfigured; malformed/duplicate mode is invalid. Do not infer mode from presence of a concept file or invent goals to configure it.

- **Game design:** work toward intended experience for intended player. Compare proposed rules, framework interpretations, and evidence against stated direction. Ask focused questions when necessary intent is unknown; no forced complete interview.
- **Game explorer:** combine possibilities, inspect generated behavior, and pursue promising leads toward potential experiences. Missing destination is not a defect. Candidate directions remain proposed until adopted.

Keep original intent, current goals, working draft, and framework-derived interpretations distinct. Do not silently move comparison target to justify findings. Goal revision needs an explicit decision; prior intent remains recoverable through its exact commit.

Switch explicitly, preserving body, constraints, original intent, and prior findings. Switching mode does not adopt a goal or abandon existing constraints. Setup plan/apply with `--mode` offers guarded header-only switching; precise source edit is also valid. Read current concept/mode before interpretation.

## Player experience and MDA

Overall intent/player story and individual MDA A aspects are different lenses. An aesthetic record does not replace or translate the entire brief. Compare support, contradiction/tension, missing evidence, discovery outside intent, and possible goal drift. A conflict can prompt revising design or goals; it does not authorize either. Declared links and framework findings alone do not prove player experience.

## Git history and comparisons

Commit completed authorized concept changes. Use ordinary Git history, not frozen baseline copies. Inspect source/index first; scope commit to intended concept changes and preserve unrelated work. Mixed unrelated edits require resolution rather than blind staging. No helper commits, initializes Git, fetches history, or adopts supplied references automatically. Read-only review does not commit existing drafts.

Unsupported/missing Git degrades to unavailable metadata, not invented history. Git reads prohibit lazy fetch/replacement objects/prompts, with 5 s and 2 MiB output bounds per call.

Current context reports latest commit affecting concept, content hashes, and `committed`, `working_draft`, `untracked`, `missing`, or `git_unavailable` state. Untracked text is also a working draft; unavailable history leaves draft/commit state unverified, not clean. CRLF checkout normalization alone is not a content draft; byte hashes remain recorded. Latest concept commit is **not automatically original intent**. A blank template is not an adopted goal.

Select an actual committed intent revision when comparing with original direction. Full commit ID, not moving HEAD/branch. For renamed source, supply former project-relative path:

```sh
node <workbench>/scripts/workbench.mjs context --project <project> --intent-ref <full-commit> --intent-path <former-path.md> --json
```

Command returns current text/mode and selected historical text without writing files. Without selection, comparison remains absent. If history is unavailable, report limitation; do not fabricate or silently substitute current intent. Historical review remains readable even when its comparison commit is no longer available.

Snapshots capture current concept automatically, mode, Git/draft state, and selected comparison path/commit/hash—not a copy of historical text. Source, mode, binding, selected comparison, and concept revision changes affect freshness. Unrelated commits do not by themselves change concept revision.
