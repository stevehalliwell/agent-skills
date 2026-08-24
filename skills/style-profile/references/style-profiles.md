# Style profiles

The style-profile skill resolves profile `<name>` in this order:

1. `.pi/style-profiles/<name>.md` from current project, with optional `.pi/style-profiles/<name>.metrics.json` sidecar.
2. `~/.pi/style-profiles/<name>.md`, with optional `~/.pi/style-profiles/<name>.metrics.json` sidecar.

If neither Markdown profile exists, report both checked paths and ask user to use the learn workflow to create a profile or supply another name. Do not silently select another profile. A profile created from local Markdown exemplars writes its metrics sidecar alongside the Markdown profile; a missing sidecar makes quantitative comparison unavailable, not inferred.

## Profile format

```markdown
---
name: <lowercase-hyphen-name>
description: <whose style and intended use>
---

# Style profile: <name>

## Sources
- <source path or URL> — <word count, genre, origin/permission note, and claim boundary>

## Observed

### Voice
- Tone: <...>
- Perspective: <...>
- Formality: <...>

### Vocabulary
- Preferred terms: <...>
- Avoided terms: <...>
- Jargon and contractions: <...>

### Sentences
- Length and variation: <...>
- Fragments and questions: <...>

### Composition
- Opening, point development, transitions, qualifications, standalone labels, and ending moves: <...>

### Structure
- Paragraphs, lists, headers, and code-block use: <...>

### Variation and modes
- <which genres, periods, or source formats differ; which mode fits each intended target>

### Punctuation
- Em dashes, semicolons, Oxford comma, other preferences: <...>

### Anti-patterns
- <deterministic pattern to avoid>

### Verification calibration
- Cadence and structure weight: 45%
- Voice and punctuation weight: 35%
- Lexicon weight: 10%
- POS composition weight: 10%
- Standalone-label maximum words: 7

Weights are profile-specific comparison priorities. They are normalised among available groups; they are not drafting quotas. A standalone label is a single-sentence prose block at or below the configured word limit. It is excluded from paragraph cadence and progression, but its rate remains tracked. Markdown `#` headings are excluded before prose analysis. After changing the word limit, regenerate the metrics sidecar with `--paragraph-label-max-words <number>`.

### Metrics
- Metrics data: `<name>.metrics.json` — schema version and one corpus-level `metrics` summary containing statistical summaries, per-document comparison distributions, paragraph progression summaries, POS word-type percentages, vocabulary counts, and bounded common/rare word lists; it excludes source text, paths, raw arrays, per-document results, and complete word-frequency tables.
- <human-readable interpretation of a relevant distribution or descriptive statistic, with corpus basis>

## Interpretation
- <working judgement supported by observations>

## Generation guidance
- Target mode: <when to use it and closest corpus sources>
- Calibration excerpts: <1–2 representative-excerpt labels to compare while drafting>
- Compose: <evidence-backed opening, development, pivot, and ending moves>
- Preserve when expanding notes: <source-led uncertainty, repetitions, point order, judgements, or other constraints>
- Avoid: <specific genericising or mode-mismatch failure patterns>

## Open questions
- <evidence gap or rule needing user confirmation>

## Representative excerpts
### <label>
- Source: <path or public URL>
- Permission/origin: <note>
- Demonstrates: <style feature>

> <50–150-word excerpt>
```

## Measurement

The metric scripts use winkNLP and its English model. Install their locked dependencies after obtaining these skills with `npm ci --prefix <path-to-style-profile-skill>` (use the equivalent path on Windows).

Use the bundled metric script for repeatable local Markdown evidence:

- `references/style-profile-metrics.mjs [--output <metrics.json>] [--paragraph-label-max-words <number>] <Markdown files...>` — corpus words; filtered vocabulary counts and common/rare words; Universal POS word-type counts and percentages across articles, sentences, and paragraphs; sentence-length distribution; paragraph word and sentence lengths; corpus paragraph progression; list ratio; header depth; code-block frequency; punctuation counts per 1,000 words.
- `references/style-profile-compare.mjs <profile.metrics.json> <Markdown files...>` — calculate each target’s metrics, compare them with robust per-document corpus distributions, and return grouped, weighted heuristic statistical-deviation results. It never modifies source files.

`references/style-profile-metrics.mjs` emits machine-readable JSON plus a concise human summary. Its JSON retains one corpus-level `metrics` summary with statistical summaries, per-document comparison distributions, paragraph progression summaries, Universal POS word-type percentages, vocabulary counts, and bounded common/rare word lists, so a generated target can be compared meaningfully. Common/rare vocabulary excludes conjunctions, pronouns, determiners, prepositions, particles, numeric/mixed tokens, and one-character tokens. It excludes source text, paths, raw arrays, per-document results, and complete word-frequency tables. Sentence rhythm is not one number: report sentence-length distribution, variation, short-sentence rate, paragraph cadence as measurable proxies; retain qualitative observations in profile. Markdown-aware parsing excludes frontmatter, code blocks, URLs, quoted excerpts from prose metrics where appropriate.

Metrics support comparison and heuristic verifier findings. The comparator reads the profile's `Verification calibration` group weights, tracks standalone-label use separately, and reports robust distance from corpus centres. They do not replace representative excerpts, become gates, or authorize automatic rewrites. In corpus review, use scores to find candidates, then assess qualitative evidence before recommending exclusion or a separate mode. Without a repeatable measurement source, report metrics unavailable; never estimate.

## Rules

- Profiles describe observed preferences; do not claim to reproduce a person exactly.
- Treat corpus variation as evidence, not noise: direct generation to the closest documented mode rather than an imagined corpus average.
- Generation guidance translates observations into bounded drafting decisions; it does not authorize invented facts, reasons, or certainty.
- Preserve provenance and permission notes for excerpts. Do not copy third-party text without reuse permission.
- Include at least three representative excerpts when profile comes from exemplars and permission permits reproduction; otherwise state limitation.
- Keep anti-patterns deterministic when possible so verifier apply mode can change only exact matches.
- Record metrics as evidence, not gates. Qualitative rules remain valid when metric is unavailable.
