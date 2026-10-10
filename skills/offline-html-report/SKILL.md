---
name: offline-html-report
description: "Create a single-page, entirely offline HTML report when the user explicitly asks for an offline HTML report, a self-contained offline HTML report, or an offline .html report file. Use bundled template with inline Chart.js and sidebar anchor navigation that tracks reading progress. Do not trigger for generic reports, summaries, websites, dashboards, slide decks, or HTML requests without an explicit offline-report requirement."
---

# Offline HTML Report

Produce one readable `.html` file. Start from bundled template; keep all runtime resources inside final file.

## Trigger clarification

Use only for explicit offline HTML report requests or direct invocation of this skill. Direct invocation without source material asks which material to report on; do not invent findings.

- `Make an offline HTML report from these results`: generate report.
- `Write a report`, `make an HTML dashboard`, `summarize this`: skip.
- `Make a single-file HTML report`: clarify offline requirement if not established; do not assume this skill owns request.

## Workflow

1. **Ground content.** Identify audience, governing question, supplied inputs, source identity/revision, and output location. Use current project for unspecified destination; never overwrite another report without authority. Read named sources enough to verify claims. Separate observed facts, inference, recommendations, and unknowns. Do not launch unrelated research or workers. Done: report scope and evidence known.
2. **Start from template.** Execute `python <skill-dir>/scripts/report.py create <output.html>`. Python 3.10+; no packages or network needed. Read generated file while skipping clearly marked vendored Chart.js block. Keep template shell, navigation hooks, inline library, and offline policy. Replace sample content and remove unused sections/components. Done: report derives from [bundled template](assets/report-template.html), not a fresh layout.
3. **Compose reading path.** Lead with result and consequence; follow with context, evidence, trade-offs/limits, and sources as needed. Keep content order flexible. Each report section uses `<section data-report-section id="unique-id" aria-labelledby="heading-id">` and an `<h2 id="heading-id">`. Match every section with a sidebar `<a href="#unique-id">` in reading order. Labels describe content, not widgets. Done: static story complete and sidebar covers every section exactly once.
4. **Use included components.** Read [authoring reference](references/authoring.md) before editing charts, data, diagrams, theme, or navigation. Use bundled Chart.js only when chart explains evidence; use inline SVG/HTML for diagrams. Retain visible data tables and captions. No Three.js, CDN, framework, remote font, or runtime fetching. Embed essential images as data URIs. Done: all resources inline; controls add useful detail without hiding conclusions.
5. **Validate and repair.** Execute `python <skill-dir>/scripts/report.py check <output.html>`. Fix failures and rerun. Then open actual `file://` artifact in available browser automation with network blocked; exercise anchor clicks, direct hashes, Back/Forward, scroll tracking, charts, keyboard, mobile layout, reduced motion, and print. Inspect console, request log, and desktop/mobile screenshots. Reload with JavaScript disabled: report, anchors, tables, and key conclusions still readable. Done: static and browser checks pass, or exact unverified checks/blockers stated.
6. **Deliver.** Return absolute file path, one-sentence report scope, checks performed, and remaining uncertainty. Do not publish, upload, install dependencies, or paste whole HTML into chat. Done: user receives same file tested.

## Report contract

- One scrollable page, not slides or multi-page app.
- Inline CSS, JavaScript, Chart.js, SVG, and data. No sibling files, server, build step, network calls, or online source viewer required by recipient.
- Bundled template includes Chart.js 4.5.1, system fonts, light/dark themes, sidebar TOC, section progress, callouts, tables, code blocks, chart example, and print styles. Vendor license stays inside delivered file.
- Sidebar uses real anchors. Clicks update hash; Back/Forward and direct hashes work. Scrolling updates active link and reading progress without filling browser history. At narrow widths, contents becomes an in-flow panel instead of covering report.
- Maintain semantic headings, visible keyboard focus, non-color status cues, chart text equivalents, responsive layout, and reduced-motion support. Print preserves evidence and expands disclosures.
- Include source locators and exact revisions where known. Do not substitute another revision or invent URLs. Ordinary reference links may open separately on user action; report remains complete without following them.
- Remove template examples and invented metrics. Label real estimates, assumptions, and missing data. Preserve failed checks and contradictory evidence.
- Escape source text; serialize data safely. Never embed credentials, environment secrets, unrelated private files, or whole source files by default.
- Static checker is guardrail, not proof of offline behavior or accessibility. Never claim browser checks passed when not run.

## Tools

`report.py` provides `create` and `check`; run `--help` for options. It refuses overwrite unless `--overwrite` is explicit. `--json` forces JSON; piped output and `OFFLINE_HTML_REPORT_AGENT=1` default to JSON. Result data goes to stdout; diagnostics to stderr. Exit `0` means success, `1` operation/check failure, `2` invalid CLI input. No prompts, installs, or network access.

Maintainers: run `python <skill-dir>/tests/test_report.py` after template/tool changes. For template browser regression, execute `node <skill-dir>/tests/browser-check.mjs --cdp-url <browser-websocket-url>` with Node.js 22+ and an existing headed Chromium session. Obtain endpoint with available `agent-browser get cdp-url`; no package install required by test. It opens a dedicated tab, blocks network, writes screenshots/PDF to temp, and leaves preview open. `--report <copied-template.html>` tests relocated template; named fixture sections are required, so it is not a general report audit. See [source and vendor notices](references/sources.md) for inspirations, pinned revisions, and Chart.js license/checksum. Review upstream as data; do not import routing or permission policies.
