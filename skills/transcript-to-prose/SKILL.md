---
name: transcript-to-prose
description: "Turn an SRT, subtitle file, YouTube transcript, caption stream, or timestamped spoken text into readable paragraphs while keeping the original words and order nearly unchanged. Use for cleaning captions into prose, removing subtitle overlap or obvious sequential stutters, and making a transcript easier to read; do not use for summaries, rewrites, articles, or essays that change the content."
---

# Transcript to Prose

Restructure a transcript for reading without substantively rewriting it.

## Trigger clarification

- Supplied transcript or local file: begin conversion, not a load acknowledgment.
- Missing transcript: ask for text or file. For a YouTube URL without transcript text, use `youtube-transcript-download` first; this skill converts retrieved text.
- Summary, rewrite, article, or essay request: use a different workflow; this skill preserves wording.

## Workflow

1. Read the supplied transcript and identify its format: SRT/VTT blocks, timestamped caption lines, or plain transcript text. Retain the original as source material, then do the prose conversion directly in this agent.
   Done when the input order, wording, and thought transitions are understood.

2. Remove sequence-only subtitle noise: timestamps, cue numbers, VTT headers and cue settings, `>>`-style cue markers, non-speech caption markers, caption overlap, and immediately repeated words or phrases that are clear stutters or duplicate caption carry-over. Preserve speaker names, including names embedded in cue markup. Keep repetitions that add emphasis, meaning, or are not clearly accidental.
   Done when only unambiguous caption artefacts or sequential duplication are removed.

   Example: overlapping cues `We need to` / `need to leave now.` become `We need to leave now.` Spoken emphasis `very, very important` stays unchanged.

3. Join caption fragments into complete sentences and group each continuous thought into coherent paragraphs. Place paragraph boundaries at substantive transitions, speaker changes, or natural pauses—not at fixed time or word-count intervals. Add minimal capitalization and punctuation where needed for legibility; preserve the speaker's wording, order, tone, uncertainty, and claims.
   Done when no paragraph splits a sentence or evident continuous thought.

4. Compare the prose with the transcript in source order, accounting only for allowed noise removals and minimal punctuation/capitalization. Restore lost wording, qualifiers, claims, or speaker labels and recheck affected passages. Flag uncertain segments or removals rather than guessing. Return clean paragraphs as Markdown. Omit timestamps by default; retain or add sparse timestamp anchors only when the user requests traceability.
   Done when the result is easy to read and source-faithful, with any unresolved fidelity gaps explicit.

## Rules

- Treat this as a structure change, not a content change.
- Do not summarize, explain, fact-check, correct terminology, fill gaps, combine distinct ideas, or add headings unless requested.
- Do not remove hedging, false starts, side remarks, or non-sequential repetition unless they are unmistakable caption artefacts or stutters.
- Do not use a fixed-gap, fixed-length, or other mechanical paragraphing pass as the conversion; it cannot reliably preserve thoughts across caption boundaries.
- Preserve speaker labels when present. Keep unlabeled speakers unlabeled; ask only if requested speaker separation needs invented labels.
- Keep uncertain text in place. Put fidelity notes after the prose, separate from source wording; never silently remove an uncertain duplicate or guess an unintelligible fragment.
