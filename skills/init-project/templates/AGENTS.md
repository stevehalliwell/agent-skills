# AGENTS.md

Project-specific agent notes only. Global Pi rules already apply.

Before committing: replace or delete every `[TBD]` item. Keep only verified, durable facts. Pi concatenates context files; refine global rules without contradicting them. State any narrow exception, condition, and reason.

## Read first

- `README.md` — public-facing project purpose, installation, usage, and brief developer setup; do not use it for current work or internal status.
- `.pi/attendant.tables` — configured record collections; load `/skill:attendant` and use its `schema` command to discover every tracked table and fields before planning/querying.
- `.pi/handoff.md` — previous pickup summary, if present.
- Read specific record source paths only after `/skill:attendant` `schema`, `query`, or `search` identifies them.

## Verified commands

Record commands actually run in this repo. Keep agent-only flags, order, prerequisites, expected duration, and known failures here; link `README.md` for human explanation.

- Setup/install: [TBD]
- Test: [TBD]
- Focused test: [TBD]
- Lint/typecheck/build: [TBD]
- Required order, prerequisites, expensive checks, bench/profile policy: [TBD]
- Local wrappers/skills: [TBD]

## Project map and coding rules

- Key source/test/config/generated paths: [TBD]
- Naming: [TBD]
- Formatting: [TBD]
- Error handling: [TBD]
- Ownership/lifetime/resources: [TBD]
- Public API compatibility: [TBD]
- Perf-sensitive areas: [TBD]
- Security/data constraints: [TBD]

## Protected paths

Do not edit unless task explicitly targets them or rule below says otherwise:

- Generated/build/cache: [TBD]
- Vendored/deps: [TBD]
- CI/release config: [TBD]
- Binary/media/serialized assets: [TBD]
- Lockfiles policy: [TBD]
- Backups/archives: [TBD]

## Project-specific doc policy

- `README.md`: public, human-first purpose, installation, usage, and brief developer setup; current state belongs in Attendant records or `.pi/handoff.md`, and detailed contributor instructions belong in `CONTRIBUTING.md` or another focused document.
- `CHANGELOG.md`: [TBD]
- Attendant collections: canonical tracked state; discover configured collections with `/skill:attendant` `schema`, query with its `query`/`search` commands, edit Markdown records as source.
- `.pi/handoff.md`: agent-only resume pointer; update via `/skill:wrap-up`; link records or include exact Attendant query.

## Done means here

- Project-specific acceptance: [TBD]
- Required validation commands and expected result: [TBD]
- Test/doc/update requirements for changed behavior: [TBD]
- If a required check cannot run, record blocker and remaining risk.
