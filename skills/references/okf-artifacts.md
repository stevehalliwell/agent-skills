# OKF research artifacts

Use for standalone research or incidental standards verification unless user explicitly asks to discuss only.

## Root and bundle

Ask for OKF bundle root when unknown. Retain supplied path in current agent session context; do not create project config or runtime state. After user supplies root, create missing:

- `index.md` with `okf_version: "0.2"` frontmatter;
- `references/`;
- `research/`.

## Reference

Create one Markdown note per source under `references/`, with non-empty frontmatter `type: Reference`, title, description, tags, and generated metadata. Include source URL, summary, evidence/excerpts, relevance, confidence, and limits.

## Research synthesis

Create one Markdown note per question under `research/`, with non-empty frontmatter `type: Research Synthesis`, title, description, tags, generated metadata, and linked `sources`. Include question, findings with Markdown footnotes, recommendation, uncertainty, and limits. Use standard Markdown links and footnotes for provenance.

Do not write artifacts for discussion-only work. Keep evidence distinct from inference and recommendations.
