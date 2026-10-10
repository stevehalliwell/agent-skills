# Sources and Vendor Notices

## Design sources

Downloaded four source skills to temporary storage for review; none installed. New instructions, template, and runtime written for this skill. Borrowed workflow ideas, not upstream code or fixed visual templates.

- [Joshua Thomas: HTML Artifacts](https://github.com/joshuadavidthomas/agent-skills/tree/516dee7a422b90937b2958d11c03694154ab9c09/html-artifacts). Evidence ledger, static reading path, strict offline checks, accessibility, safe embedded data. MIT; copyright 2025 Josh Thomas.
- [Plannotator: effective-html](https://github.com/plannotator/effective-html/tree/2ac1dfecb0f2474e75260cb6d3c9b9d6d9b5062e/skills/html). Subject-led composition, honest charts, restrained operational reports. MIT; copyright 2026 plannotator.
- [wjhuang88: onepage](https://github.com/wjhuang88/onepage-skill/tree/71d5e2af03719bedb285c37143434a53ee604488/skills/onepage). Chapter planning and summary-to-detail structure. MIT; copyright 2025 wjhuang88.
- [Warp: readout](https://github.com/warpdotdev/common-skills/tree/7c4eb852de8d7c3e68c81802e1c59db9a611f914/.agents/skills/readout). Sidebar navigation, source provenance, complete static document. MIT; copyright 2026 Denver Technologies, Inc.

Rejected: broad automatic HTML routing, runtime GitHub fetching, token entry, CDN highlighting, mandatory child agents, global report folders, rigid metric-card heroes, and Unix-only delivery commands.

## Chart.js

Vendored Chart.js 4.5.1 from [published npm archive](https://registry.npmjs.org/chart.js/-/chart.js-4.5.1.tgz). Source JavaScript lives inline in `assets/report-template.html`, under `script#vendor-chartjs`; no sibling runtime asset needed.

- [License](../assets/chartjs-LICENSE.txt): MIT, copyright 2014–2024 Chart.js Contributors.
- [Vendor manifest](../assets/vendor.json): version, source, archive checksum, original and embedded JavaScript SHA-256.
- Removed source-map URL because external map is not bundled. Library header retained. Full license embedded in template and every copied report.
- No Three.js; user selected Chart.js-only library set.

Update only as explicit maintenance work. Fetch pinned package, verify source/version/license, replace inline vendor block and license together, update manifest checksums, then rerun static tests and network-blocked browser checks. Never replace with CDN tag or load latest version at report generation time.
