# AGENTS.md

Project-specific agent notes only. Global Pi rules already apply.

Adapt to project facts: remove template instructions, irrelevant headings, and empty placeholders; keep material unknowns explicit. Use concise, ordered, structured documentation, with each fact in one place. Pi concatenates context files; refine global rules without contradicting them. State any narrow exception, condition, and reason.

## Read first

- `README.md` — public-facing project purpose, installation, usage, and brief developer setup; do not use it for current work or internal status.
- `.pi/attendant.tables` — configured record collections; load `/skill:attendant` and use its `schema` command to discover every tracked collection, fields, source paths, and usage guidance before planning/querying.
- `.pi/handoff.md` — previous pickup summary, if present.
- Read specific record source paths only after `/skill:attendant` `schema`, `query`, or `search` identifies them.

## Attendant

Attendant tracks tasks and durable decisions through `.pi/attendant.tables`:

- `records/tasks/`: `.schema.md` declares task fields; `.usage.md` defines lifecycle, readiness, approval, and operating rules.
- `records/decisions/`: `.schema.md` declares decision fields; `.usage.md` defines purpose, approval, supersession, and revisit rules.

Read each collection's `.usage.md` before creating, selecting, updating, or resuming its records. Use `schema` to discover current configured paths and full usage text; do not assume these default paths after configuration changes. Schema fields and values are authoritative; report contradictions with usage rather than silently choosing one. Missing usage is not permission to import defaults from another project; resolve material ambiguity with the user.

Markdown is the source of truth. `.attendant/` is generated local state, not a record source. `.template.md` supplies starting record-body copy, not lifecycle rules.

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
