# Style document generator

Create or rewrite Markdown with selected profile.

## Required read

Read `../references/style-profiles.md` before work. Done when profile resolution and format rules are known.

## Workflow

1. Gather target, audience, one-sentence thesis, profile name, source facts, constraints, requested length/structure, and input form: notes, outline, rough prose, or finished prose. Separate facts, judgements, uncertainty, future intent, required wording, and gaps in the source. For new files, target is a Markdown path; existing files may be supplied path or chat text. Done when target, source form, and meaning boundary are known.
2. Resolve profile project-first, then global. If missing, report both checked paths; direct user to the learn workflow or another name. Read its generation guidance, variation/modes, representative excerpts, optional `Verification calibration`, and same-named `.metrics.json` sidecar when present; select the closest target mode and 1–2 calibration excerpts before drafting. Verification calibration guides later review only; it is not a drafting quota. Do not average incompatible corpus modes or imitate an excerpt’s subject matter. When the sidecar exists, plan a report-only verification pass after drafting; for chat output, use a temporary Markdown file and remove it after verification. Done when profile, mode, drafting anchors, and verification path are resolved or a blocker is stated.
3. Make a source map before writing: retain the source point order unless a clearer order is evident, identify the reasoning or consequence behind each point, and mark any connective material that would be newly inferred. For note conversion, expand only from supplied facts and directly supported implications; keep qualifications, deliberate repetitions, open questions, and blunt reactions when they carry the author’s judgement. Ask before resolving an ambiguous point or adding a missing rationale. Done when every output paragraph has a source basis.
4. For file output, confirm target path before writing. Chat output needs no path. For existing files, show or summarise proposed changes before overwrite unless user explicitly requests direct apply. For rewrites over 200 lines, identify source meaning and profile gaps section by section; wait for approval before deleting sections, restructuring flow, or changing technical content. Done when write authority and structural-change boundary are known.
5. Draft or rewrite using the selected mode and calibration excerpts. Apply the profile’s composition moves as well as voice, vocabulary, sentences, structure, punctuation, and anti-pattern rules. Match the source’s natural density: do not convert a working update into a tidy essay, add generic transitions or conclusions, or manufacture a more certain, balanced, or marketable position. Preserve stated meaning, confirmed facts, links, code blocks, commands, and required headings. Done when output is source-led, mode-matched, and free of unsupported claims.
6. Verify and make one targeted redraft when needed. First, check the source map: every fact, qualification, technical detail, and link survives, while no inference became a fact. Then compare rhythm, paragraph movement, pivots, labels, and endings with the selected calibration excerpts. When the sidecar exists, run the verify workflow in single-review, report-only mode after the draft is written. Capture its complete `style-profile-compare` result and report every metric comparison, weighted group score, and tracked standalone-label measurement. Use the profile's calibration to identify review candidates, but do not redraft because an individual metric—or a tracked standalone-label rate—crosses a numeric threshold. Make one targeted redraft only where a material qualitative profile mismatch is also supported by the weighted comparison, preserving source-map material and higher-priority clarity/accessibility requirements; rerun the source check and verification pass, then report both passes. Do not make further automatic redrafts: identify remaining differences and their trade-offs. Without a sidecar, report verification unavailable and complete the qualitative comparison. Metrics guide targeted revision; they are not quotas or permission to alter meaning. Done when content fidelity, complete metric evidence or its limitation, and any required one-pass redraft are explicit.
7. Return Markdown with Style application trace. Done when profile, selected mode/anchors, applied rules, preserved material, metric comparisons, any redraft, assumptions/questions are explicit.

## Rules

- New docs lead with thesis unless user specifies another structure.
- Profile never overrides accessibility, clarity, exact technical text, or explicit user requirements.
- Start with content or action; skip generic welcome or industry preamble.
- Use concrete examples for claims; state meaningful trade-offs, limits, uncertainty.
- End with last useful information. No summary padding or generic closing.

## Output shape

```markdown
## Style application
- Profile: <resolved path>
- Mode and anchors: <selected profile mode and excerpts>
- Applied: <key composition and language rules>
- Preserved: <facts, uncertainty, structure, technical text>
- Metric comparison: <all initial and, when redrafted, final comparison values; remaining material differences>
- Assumptions or open questions: <item>
```

## Boundaries

- Markdown only.
- No profile registry, automatic selection, or non-Markdown generation.
- Use the verify workflow for report-only conformance review.
