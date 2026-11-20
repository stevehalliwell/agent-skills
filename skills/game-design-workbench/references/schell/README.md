# Schell — full-card reference index

[deck-table.md](deck-table.md) contains **116 English cards and 451 questions** from the user-supplied JSON for Jesse Schell’s *Game Design: A Deck of Lenses*. Five columns: internal index, exact title, functional use note, description, and question list. Existing functional notes remain unchanged; descriptions and questions use supplied wording and order.

[deck-en.json](deck-en.json) preserves the entire supplied JSON block, including UI strings, card metadata, description whitespace, and question arrays. It is reference data, not a runtime dependency or generator. Table cells flatten line breaks and indentation, with literal text inside inline code; raw JSON retains original formatting. No additional web retrieval performed for this conversion.

## Source identity

- Author: [Jesse Schell](https://www.jesseschell.com/). App links to [Schell Games](https://www.schellgames.com/).
- Starting page: [English Card List](https://deck.artofgamedesign.com/#/menu/0/?lang=en).
- Displayed edition: **Tenth Anniversary Edition**. Observed on rendered title screen and in English strings; not inferred from another edition’s numbering.
- Language: English, `lang=en`. Other language data was outside scope.
- Acquisition date: **2026-10-21 UTC**.
- Current content input: raw JSON pasted by the user in this session, saved as `deck-en.json` without reformatting or field changes. All 116 indices and titles match the earlier reference index.
- Original web source: app-loaded [English strings and LensList](https://deck.artofgamedesign.com/assets/strings/en.json). The current file came from the supplied message, not another download.
- Supplied JSON SHA-256: `37beba7665b2ce4c7c2a6865db475c9870ed7ff715dc24a186f26d1f6bf33ddc`. This fingerprints supplied bytes, not the earlier HTTP response below.
- Structure input: app-loaded [application bundle](https://deck.artofgamedesign.com/js/app.95e37080.js). Used to inspect routing, title-menu destinations, sorting, filtering, and Divination’s source list.
- App’s How to Use section identifies *The Art of Game Design: A Book of Lenses* as the deck’s companion source. Retail editions were not treated as evidence for web-card counts.

## Earlier web inspection

The following methods and browser checks describe the earlier inventory acquisition. They were not rerun for the supplied-JSON conversion.

1. Built-in raw fetch returned only the SPA HTML shell at the starting hash route. This was discovery, not card capture. `robots.txt` returned a Netlify Page not found response; no deck robots rules were available there.
2. Inspected the public app bundle and English JSON temporarily. English JSON preserves `67½`, `93½`, `95½`, and `∞`; the bundled fallback data has damaged display symbols. Used live English JSON only after confirming its displayed mapping.
3. Visible `agent-browser` was attempted but executable was unavailable. Used installed Crawl4AI **0.9.2**, Chromium, headless at 1200 × 900, as the bounded rendered fallback. No browser or model installation, OCR, remote LLM enrichment, login, or access-control bypass.
4. Crawl4AI’s HTTP API rejected `js_code` in untrusted config. Used its installed Python API inside isolated Docker containers for trusted one-off inspection. Initial local execution probes needed awaiting and old Vue Router compatibility fixes; only the final run completed enumeration. No broad site crawl occurred.
5. Final browser run opened one seed, read the 116 rendered menu entries, then sequentially visited all 116 explicit card hash routes within that app. About 90 ms between route changes; shared shell/data remained loaded. Also examined title, three instruction routes, and Divination. No recursive link following, random repeated harvesting, asset archive, or external retailer crawl.
6. Compared each active card’s rendered display ID, full card heading, description, and question text against its English data entry. All **116 headings, 116 descriptions, and 451 questions** matched after whitespace/apostrophe normalization. These checks ground the original use notes; that acquisition retained no original card text. The current descriptions and questions instead come from the user's later supplied JSON.
7. Inspected linked author/company provenance and company terms. Once restrictive related-site terms were found, stopped further automated retrieval. See rights below.

Docker image: `unclecode/crawl4ai@sha256:bd36741e7bdd35ddc1a05d9183e1d6d8cefb61dd640d944a25d026b76e917690`. Every run used a unique `crawl4ai-schell-*` container, random API token, loopback-only port, health check, and exit cleanup. Only those containers were removed; shared services were not touched.

## Completeness reconciliation

- English `LensList`: **116 entries**. Rendered unfiltered numerical menu: **116 entries**, same display-ID/title pairs in the same order. Table: **116 substantive rows**.
- Unique display IDs: **116**. Unique internal indices: **116**. Unique titles: **116**. No duplicate rows or missing menu entries.
- Numbered sequence: every integer **1–112**, plus **67½**, **93½**, **95½**, plus **∞**. Arithmetic: `112 + 3 + 1 = 116`. No integer gaps within 1–112.
- Fractional insertions use internal indices `67.5`, `93.5`, `95.5` and route segments `67_5`, `93_5`, `95_5`.
- **∞ — Your Secret Purpose** uses internal index **113**. It is not a printed “113” card, an omitted card, or a duplicate of another lens. No blank-ID card was found.
- Row order follows numerical menu order: fractional insertions follow their preceding integer; infinity follows 112. Alphabetical and random sorts reorder the same list, not supplemental decks.
- Source bundle registers `/card/:index`, but Home’s route handler and actual menu selection use `/menu/:index`. Earlier checked links used the latter, not guessed `/card/` URLs. Per-card links are no longer columns in the table. `/menu/0/` means list/menu state, not card zero.

### Navigation branches

- **Card List:** all 116 menu entries reconciled to table rows and rendered card bodies.
- **Numerical / Alphabetical / Random sort:** source handlers reorder the existing list. No new identities. Only numerical ordering was exhaustively rendered.
- **Random card:** source handler chooses among currently visible list indices. No separate cards.
- **Favorites:** local saved-index filter over the same list, not extra content. No user favorites imported or altered.
- **Suit filters:** source data contains Designer **11**, Player **21**, Experience **34**, Process **7**, Game **52** memberships. Total **125**, not 116, because cards can have multiple suits. Their union covers all 116 rows. Filter handlers hide existing entries; no additional decks. Counts derived from confirmed English data, not separate exhaustive filter-click tests.
- **Slider/card navigation:** uses the same LensList/current index; no additional card collection found in implementation.
- **Divination / New Cards:** rendered Divination route examined; initial four identities were **43, 20, 105, 95**, all already retained. Implementation draws four cards from a copy of the same English list. Repeated deals were unnecessary for coverage.
- **How to Use:** routes `/howto/0/`, `/howto/1/`, `/howto/2/` examined. Three panels: designing a game, using cards, obtaining the book/cards. Desktop DOM contains all three panels; route changes are not three new cards. No supplemental lens list in instructions.
- **Title screen:** examined at `#/?lang=en`; establishes attribution/edition and links to Card List, Divination, instructions, book retailer, physical-deck retailer, author, company, and feedback email. Retail destinations and email are navigation endpoints, not additional in-app cards; not followed/submitted.
- **Author/company pages:** inspected for provenance/rights only. Company Terms of Use examined. No harvesting of their wider content.

## Link and content checks

The earlier web inspection checked all 116 card deep links by rendered router navigation and active-card identity, **not** by counting successful shell HTTP responses. These historical source locators remain here rather than in table columns:

- [1 — Emotion](https://deck.artofgamedesign.com/#/menu/1/?lang=en).
- [67½ — Metaphor](https://deck.artofgamedesign.com/#/menu/67_5/?lang=en).
- [93½ — Presence](https://deck.artofgamedesign.com/#/menu/93_5/?lang=en).
- [95½ — Cheatability](https://deck.artofgamedesign.com/#/menu/95_5/?lang=en).
- [112 — The Raven](https://deck.artofgamedesign.com/#/menu/112/?lang=en).
- [∞ — Your Secret Purpose](https://deck.artofgamedesign.com/#/menu/113/?lang=en).

These are rendered in-session hash-route checks, not 116 independent cold browser loads. Bundle inspection confirms the same route parser handles direct entry. No claim of automated visual-layout or all-device testing.

Source fingerprints, SHA-256 of temporarily inspected response bytes:

- English JSON: `d57b031b54563e1cf48fa7cf91b2a4e4c0d8ed0bf6a2d40dd10b6e7f5bff7af8`.
- App bundle: `444135d99967716b99927709d82c30136c6cd3f3825a9c3834e1eb133cf10a92`.

Current conversion checks: raw file bytes exactly match the supplied JSON code block; JSON parses; 116 unique indices and matching titles; every existing functional note preserved; every description and all 451 ordered questions match supplied values after documented table whitespace normalization; exactly five columns per row. No card HTML or per-card source-link columns remain. Local Markdown links are checked with installed skill-craft `validate-links.mjs`. No fresh browser or remote URL checks were performed.

## Rights and local/source boundary

No explicit permission for wholesale deck republication was found in app navigation, instructions, English content, or bundle. Public readability and public JSON are **not** a republication license.

[Company Terms of Use](https://www.schellgames.com/schell-games-terms-of-use), last revised **Sept. 26, 2024**, grant limited personal, non-commercial copying of a reasonable number of pages, not further reproduction/publication/distribution. They also restrict automated access and non-homepage linking. Their scope covers the company site and related websites, explicitly including JesseSchell.com; the deck domain is not individually named. Applicability to this separate deck host is not resolved here. These terms provide **no suitable full-deck republication permission** and are a material caution for future acquisition or publication. After this discovery, only the same terms page’s raw response was inspected to verify its scope; no further deck retrieval or crawling occurred.

**Local:** raw user-supplied JSON, its descriptions and question lists in the table, bibliographic metadata, source pointers, independent functional use notes, and provenance. This conversion formats supplied text for local reference; it does not establish a redistribution license. Supplied deck content remains **local-only/not-publication-cleared**. User explicitly approved staging and a local Git checkpoint including references after scope review. No push or publication authorized.

**Not retained:** card artwork, app bundle, HTML archive, screenshots, scans, book, or physical deck. JSON retains original artwork identifiers and artist credits, not image files. Earlier temporary web-inspection files were removed; `deck-en.json` is the later user-supplied content, not a restored crawler payload.

No card-coverage gaps remain in the supplied English deck. Rights uncertainty remains a blocker to claiming an openly reusable deck archive or publishing it as a licensed reproduction. Other languages, other editions, retail pages, book contents, and artwork remain excluded. Earlier browser evidence does not establish current remote availability.
