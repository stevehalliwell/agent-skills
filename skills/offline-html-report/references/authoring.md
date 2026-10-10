# Report Authoring

## Template boundaries

Run `scripts/report.py create` first. Template already contains Chart.js; copying file requires no bundler, package install, or network.

Edit title, description, language, metadata, sections, sources, chart data, and `style#report-specific`. Preserve:

- Offline Content-Security-Policy.
- `script#vendor-chartjs` and embedded license. Do not read minified vendor code into model context or modify it.
- Shell hooks: `report`, `report-toc`, `reading-progress`, `reading-status`, `theme-toggle`, `print-report`, `report-runtime`.
- Section attributes and matching TOC anchors.

Read content before vendor marker, then runtime after end marker. Use targeted edits; never regenerate inline library from memory.

## Sections and anchors

```html
<li><a href="#findings">Findings</a></li>

<section data-report-section id="findings" aria-labelledby="findings-heading">
  <h2 id="findings-heading">Findings</h2>
  <p>Verified result and consequence.</p>
</section>
```

Use stable, unique, URL-safe IDs. Sidebar order matches section order. Use `<h3>` for detail; keep top-level report sections in main reading flow. Native anchors handle clicks, initial hashes, keyboard, and browser history. Runtime only updates current location and progress; it does not rewrite hash during scrolling.

Desktop sidebar stays visible and scrolls independently if long. Mobile contents appears before report; do not overlay content. Progress reflects scroll distance through report, not task completion or proof that reader read each section.

## Chart.js

Included version: 4.5.1, UMD build. No time/date adapter bundled; use category/numeric axes or preformatted date labels. Do not add CDN adapters.

Template initializes each `canvas[data-chart]` from embedded JSON script named by attribute:

```html
<figure>
  <div class="chart-frame only-js">
    <canvas id="duration-chart" data-chart="duration-data" role="img"
      aria-label="Measured duration by run; exact values in following table."></canvas>
  </div>
  <figcaption>Measured duration in seconds. Source: supplied run log.</figcaption>
</figure>
<script type="application/json" id="duration-data">
{"type":"bar","data":{"labels":["Run A","Run B"],"datasets":[{"label":"Duration (seconds)","data":[3,5],"backgroundColor":"#477bc4"}]},"options":{"scales":{"y":{"beginAtZero":true,"title":{"display":true,"text":"Seconds"}}}}}
</script>
```

Example values demonstrate syntax, not report facts. Provide corresponding visible semantic table. Keep canvas label, table, units, and JSON consistent. Runtime disables animation, sizes chart to container, and follows theme changes. Tables carry evidence without JavaScript and in print.

Choose line for time trends, ordered bars for magnitude, scatter for relationships, and tables for exact values. Label scales, denominators, estimates, and missing values. Never use chart to conceal small sample or uncertainty. Add filtering only when requested or clearly useful; show active state and provide reset.

## Diagrams and evidence

Use inline SVG/HTML, not another library. Give diagram title, scope, named elements, labelled relationships, and text equivalent. Prefer separate overview/detail to unreadable giant graph.

Tables use captions and scoped headers inside `.table-wrap`. Code uses escaped `<pre><code>`. `.callout` supports findings; `.callout.warning` supports uncertainty. `<details>` holds optional depth; summary and conclusion remain outside it. Print opens disclosures and uses chart tables.

Source references use path/line, exact revision, input name, dataset field, or public URL when known. Keep links secondary; include cited findings/excerpts needed to understand report offline. Do not import readout's online code viewer, token form, or fallback-to-HEAD behavior.

## Safe data and resources

HTML-escape text with standard serializer. For JSON embedded in script elements, serialize then replace `<` with `\u003c`; parse `textContent`. Render untrusted strings with `textContent`, not `innerHTML`. Never interpolate arbitrary source text into executable JavaScript.

Embed images/fonts only when essential and licensed. Prefer system fonts and SVG. No remote scripts/styles/images, relative resource paths, CSS imports, `fetch`, WebSockets, service workers, analytics, or online source loading. Recipient needs only browser and delivered HTML file.

## Verification

Static check detects common packaging defects, vendor drift, invalid JSON, duplicate IDs, broken anchors, and sidebar/section mismatch. It cannot prove runtime safety or semantic correctness.

Browser-check actual final file with network blocked:

- No page errors, failed resource loads, or attempted HTTP(S) resource requests.
- Click each sidebar anchor; verify hash and selected location. Test direct hash, Back, Forward, fast scroll, top, and bottom.
- Progress starts at 0 and reaches 100 at end of scrollable document; scrolling does not add history entries.
- Chart loads from embedded library; visible values match table/source.
- Theme and keyboard controls work. Reduced motion disables smooth scrolling.
- Desktop sidebar remains visible; mobile widths 320–390 CSS px have no page overflow.
- JavaScript-disabled reload preserves content, anchors, tables, and sources; hides inactive controls.
- Print keeps findings, tables, code, sources, and disclosures readable.

Fix and recheck. If browser unavailable, deliver file only with explicit browser-validation gap; do not describe report as verified.
