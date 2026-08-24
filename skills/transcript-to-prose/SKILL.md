---
name: transcript-to-prose
description: "Turn an SRT, subtitle file, YouTube transcript, caption stream, or timestamped spoken text into readable paragraphs while keeping the original words and order nearly unchanged. Use for cleaning captions into prose, removing subtitle overlap or obvious sequential stutters, and making a transcript easier to read; do not use for summaries, rewrites, articles, or essays that change the content."
---

# Transcript to Prose

Restructure a transcript for reading without substantively rewriting it.

## Workflow

1. Read the supplied transcript and identify its format: SRT/VTT blocks, timestamped caption lines, or plain transcript text. Retain the original as source material, then do the prose conversion directly in this agent.
   Done when the input order, wording, and thought transitions are understood.

2. Remove sequence-only subtitle noise: timestamps, cue numbers, `>>`-style cue markers, non-speech caption markers, caption overlap, and immediately repeated words or phrases that are clear stutters or duplicate caption carry-over. Keep repetitions that add emphasis, meaning, or are not clearly accidental.
   Done when only unambiguous caption artefacts or sequential duplication are removed.

3. Join caption fragments into complete sentences and group each continuous thought into coherent paragraphs. Place paragraph boundaries at substantive transitions, speaker changes, or natural pauses—not at fixed time or word-count intervals. Add minimal capitalization and punctuation where needed for legibility; preserve the speaker's wording, order, tone, uncertainty, and claims.
   Done when no paragraph splits a sentence or evident continuous thought.

4. Return clean paragraphs as Markdown. Omit timestamps by default; retain or add sparse timestamp anchors only when the user requests traceability.
   Done when the result is easy to read and remains source-faithful.

## Rules

- Treat this as a structure change, not a content change.
- Do not summarize, explain, fact-check, correct terminology, fill gaps, combine distinct ideas, or add headings unless requested.
- Do not remove hedging, false starts, side remarks, or non-sequential repetition unless they are unmistakable caption artefacts or stutters.
- Do not use a fixed-gap, fixed-length, or other mechanical paragraphing pass as the conversion; it cannot reliably preserve thoughts across caption boundaries.
- Preserve speaker labels when present. Ask before inventing labels or separating unlabeled speakers.
- Flag unintelligible fragments or uncertain duplicate removal rather than guessing.
