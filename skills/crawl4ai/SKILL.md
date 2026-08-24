---
name: crawl4ai
description: "Crawl or scrape a website, fetch webpage content, extract an article from a URL, save a webpage as Markdown, render JavaScript pages, batch-fetch URLs, capture screenshots/MHTML, or retain raw HTML locally with Crawl4AI. Use when built-in web fetch fails or lacks usable public-page content and browser rendering could recover it, or when structured multi-page crawling is needed. Skip normal web research, authenticated sites, CAPTCHAs, paywalls, or sites user lacks permission to crawl."
compatibility: "Docker running locally; Crawl4AI image unclecode/crawl4ai@sha256:bd36741e7bdd35ddc1a05d9183e1d6d8cefb61dd640d944a25d026b76e917690 available or pullable."
---

# Crawl4AI

Crawl rendered public pages → retained project-local artifacts.

## Trigger clarification

Use for public-page retrieval where browser rendering, repeated crawl calls, screenshots/MHTML, raw response retention, or project-local Markdown output matter. For one static article, prefer built-in fetch while it returns usable content. For broad web research, use web search.

After built-in web fetch fails or returns incomplete content, consider one bounded Crawl4AI fetch when its browser-rendered DOM could plausibly recover requested public content. Do not retry with Crawl4AI for authentication, paywall, CAPTCHA, permission, robots, or unavailable-source failures; report those blockers.

Completion: Crawl4AI is needed for requested source and output.

## Required read

Read [`../docker-local/SKILL.md`](../docker-local/SKILL.md) before starting the isolated service.

## Workflow

1. Discover page shape when needed.
   - When pagination URLs, item links, selectors, or rendered content shape are unknown, inspect base URL before designing crawl. Do this automatically when user supplied target and intended data are clear; do not ask permission for reconnaissance.
   - Use built-in fetch for static HTML/link discovery when it returns usable content. Use one bounded Crawl4AI fetch when rendered DOM, browser state, or visual layout determines discovery.
   - Identify pagination rule, item-detail URL rule, extraction signals, duplicates, and user-requested bounds. Keep reconnaissance at base page plus minimum linked sample needed; do not expand full crawl yet.
   - Record discovered rule in run metadata or project crawl code. Done when crawl can follow explicit bounded URLs/selectors.

2. Bound crawl.
   - Derive seed URL(s), output directory, requested artifacts, page/item limits, and discovery rule from request. Ask only if needed to prevent unintended scope. Default: one URL, `tmp/crawl4ai.<timestamp>/`, HTML + Markdown + JSON; no link following.
   - Choose capture depth independently for listing/index pages and opened item/detail pages:
     - **Text** — Markdown + response JSON. Use for content extraction only.
     - **Source** — Text + raw HTML + `view.html`. Default; use for selectors, structured data, and replayable page inspection.
     - **Visual** — Source + screenshot. Use for layout, rendered state, or visual evidence.
     - **Archive** — Visual + MHTML. Use for offline browser evidence; storage-heavy.
     - **Complete** — Archive + explicitly downloaded requested files/media. Embedded media URLs in HTML/MHTML are not local copies; download only named assets or an explicitly bounded media set.
   - Treat “screenshots/full HTML” as applying only to page types user names. If scope is unclear, default listing pages to Source and item pages to Text; state split in report.
   - Honor site terms, rate limits, and user-provided scope. Crawl4AI enforces `robots.txt` in request config. Never log in, evade access controls, solve CAPTCHAs, or crawl private/paywalled content.
   - Done when scope cannot expand unintentionally.

3. Start isolated local API.
   ```bash
   image='unclecode/crawl4ai@sha256:bd36741e7bdd35ddc1a05d9183e1d6d8cefb61dd640d944a25d026b76e917690'
   token="$(openssl rand -hex 24)"
   port="$((12000 + ($$ % 30000)))"
   container="crawl4ai-$PPID-$$"
   docker run -d --rm --name "$container" \
     -e "CRAWL4AI_API_TOKEN=$token" \
     -p "127.0.0.1:${port}:11235" --shm-size=1g "$image"
   ```
   - Use unique container name, loopback-only port, random token. Poll `GET /health` with `Authorization: Bearer $token` for up to 30 seconds before crawl.
   - For user-requested OpenAI LLM summaries/extraction only, first test without printing value: `[ -n "${OPENAI_API_KEY:-}" ]`. If present, add `-e OPENAI_API_KEY` and `-e LLM_PROVIDER=openai/<user-selected-model>` to `docker run`; pass no LLM token in request JSON. If user says smallest/cheapest without naming model, default to `openai/gpt-4.1-nano` and report choice. If absent, state `LLM unavailable: OPENAI_API_KEY is not set; continuing Crawl4AI without LLM enrichment.` then complete ordinary crawl. Never read or copy Pi `auth.json`/OAuth credentials to supply this variable.
   - Ensure cleanup with `docker rm -f "$container"` on success, error, or interruption.
   - Done when health endpoint returns success or failure states exact blocker.

4. Crawl through API.
   ```bash
   curl --fail-with-body -sS "http://127.0.0.1:${port}/crawl" \
     -H "Authorization: Bearer $token" -H 'Content-Type: application/json' \
     --data @request.json > response.json
   ```
   - Request body shape:
   ```json
   {
     "urls": ["https://example.com/"],
     "crawler_config": {
       "cache_mode": "BYPASS",
       "check_robots_txt": true,
       "wait_for_images": true
     }
   }
   ```
   - This is Text/Source config. Add `"screenshot": true` for Visual; add both `"screenshot": true` and `"capture_mhtml": true` for Archive/Complete. Batch known URLs in bounded groups; correlate each result by canonical returned `page.url`, never batch-array position. Use explicit page/item caps and visited URL set for discovery crawls. Treat individual `success: false` responses as failures even if HTTP request succeeds.
   - For LLM enrichment, use Crawl4AI server `/llm/<encoded-url>?q=<bounded instruction>&provider=openai/<user-selected-model>` after ordinary crawl, or its background-job equivalent for many URLs. Keep instruction narrow, model/budget explicit, and retain raw crawl artifacts alongside generated answer. Do not add `LLMExtractionStrategy` to untrusted `/crawl` JSON; this server rejects it.
   - Done when every requested URL has explicit result or recorded error.

5. Retain usable output.
   - Save original `response.json` per page. Also save `page.html` from `html`, `page.md` from `markdown.raw_markdown`, `page.mhtml` from `mhtml`, and decode `screenshot` base64 to `screenshot.png` when requested/present.
   - For viewable rendered HTML, add `<base href="<page.url>">` inside `<head>` to separate `view.html`; retain raw `page.html` unchanged.
   - Keep output project-local. Never overwrite existing crawl output without user approval. Write run metadata: image digest, seed URLs, bounds, config, timestamps, and result counts.
   - Done when artifacts can be inspected without re-crawling.

6. Report.
   - State URL count, success/failure counts, output path, retained formats, and any scope/bounds reached. Include failed URL/error summaries, not full page bodies.
   - Done when user can inspect results or act on blocker.

## Rules

- Pin image by digest. Record resolved repo digest in run metadata.
- Do not expose API beyond `127.0.0.1` or reuse user containers/ports.
- Use `cache_mode: "BYPASS"` unless user explicitly requests Crawl4AI caching. Use `check_robots_txt: true` so Crawl4AI enforces robots rules; report skipped URLs from its results.
- Default to least capture: screenshots and MHTML only when needed; preserve them when visual/browser evidence is requested.
- LLM enrichment is opt-in only. Use ambient `OPENAI_API_KEY` if available; Pi provider/OAuth credentials are not transferable. Missing key is a reported degradation, not crawl failure.
- Keep extraction/discovery selectors site-specific in project code, not this global skill.
