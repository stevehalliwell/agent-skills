---
name: attendant
description: "Set up Attendant, create an empty collection/table, create or update records, query or search local records, validate records, diagnose projection health, read human feedback, or reveal/inspect a native Markdown workspace. Use for `.pi/attendant.tables`, collection schemas/usage, or Attendant-managed Markdown. Route setup and empty collections through guarded workflows. Skip ordinary Markdown editing and non-Attendant data; migration/import is not supported."
---

# Attendant

Use native CLI for metadata and queries; native GUI for human Markdown editing and feedback. Markdown is source of truth; `.attendant/` is disposable SQLite projection state. No Node.js runtime required.

## Trigger clarification

Use for Attendant operations or managed records. Skip ordinary Markdown and non-Attendant data. On explicit invocation, inspect target project's `.pi/attendant.tables` and requested operation; ask one focused question only if operation remains unknown.

- `Find open tasks` with configured Attendant: inspect schema/usage, then query declared fields.
- `Read human comments`: inspect feedback schema/usage, then query pending comments.
- `Show this record in Attendant`: use GUI reveal; human chooses **Go**.
- `Edit README`: skip Attendant unless native workspace interaction was requested.
- `Migrate existing Markdown`: report unsupported operation; preserve files and config. Do not resurrect retired JS or invent a migration command.

## Workflow

1. Read reference for requested operation:
   - Setup or empty collection: [Setup and collection workflows](references/collections.md).
   - Create/update, validate/sync, schema/query/search, diagnosis or human feedback: [Commands and feedback](references/commands.md).
   - Start GUI, reveal file or inspect live selection: [GUI workflow and agent controls](native/apps/gui/README.md).
   - Missing build, explicit build or GUI dependency setup: [Build reference](references/build.md).
   - Done: preconditions, approval gates and operation's completion condition are known.
2. Resolve `<skill-dir>` as directory containing this file. Resolve `<attendant>` to quoted absolute path of `native/build/attendant-zig.exe` on Windows or `native/build/attendant-zig` on macOS. Use built CJIT counterpart when Zig output is absent. Build one installed compiler's target if neither exists, following build reference; return to original target project afterward. Do not install tools, acquire GUI dependencies implicitly, fall back to JS or reuse spike binaries.

   ```sh
   # Run from target project; -p also accepts explicit project root.
   <attendant> schema -p /path/to/project
   ```

   CLI supports Windows x64 and macOS arm64. GUI is separate and not required for CLI work. Node.js is used only by development/build tools, not record operations.
   - Done: executable and target project are unambiguous; missing build is resolved or exact blocker reported.
3. Follow selected workflow, including inspection and user-confirmation gates. Commands emit one JSON value on stdout; failures also explain on stderr. Check exit status and diagnostics, not stdout presence alone. Before retrying mutation, inspect actual source and reported `written`/`failed` paths; completed writes may remain. Correct only supported in-scope defects and rerun failed action. Report unresolved blockers, source paths and remaining diagnostics; include raw JSON only when requested.
   - Done: source changes are within approved scope and result is understood.

## Collection contract

Each collection keeps three separate source files:

- `.schema.md`: flat YAML front matter declaring fields, types, defaults and allowed values.
- `.usage.md`: purpose, field meanings, lifecycle, operating rules and approval requirements. Not record-body copy or additional schema.
- `.template.md`: optional body copy for new records.

Run `schema` to discover source paths, declared fields and full `.usage.md` text. Read usage before creating, updating, selecting or resuming records. Empty file supplies no guidance; missing file reports `usage: null`. Do not infer rules from collection name or another project's defaults. Resolve material ambiguity with user. Schema declarations remain authoritative; report conflicting guidance rather than choosing silently.

Author usage from confirmed purpose and rules. Do not invent policy or copy external skill instructions without request. Usage is not projected as record or enforced by schema validation.

## Rules

- `@path` reads UTF-8 option input; `-` reads stdin once. Create/update items are arrays of `{name, fields}`; fields and query params are objects.
- Before collection-specific lookup or writes, read schema/usage. Use declared fields and documented built-ins only.
- `schema`, `query` and `search` may apply reported mechanical metadata corrections before preparing projection. `doctor`, `validate --no-correct` and `validate --strict` preserve source; strict validation fails for diagnostics or pending corrections.
- Use `search` for text lookup. `query` accepts one read-only, authorizer-contained SQL statement. Normal actions prepare projection; do not add routine validation/sync churn. Use `doctor` for health/projection problems.
- Preserve Markdown; never edit `.attendant/` directly. Edit source bodies as Markdown; metadata writes use CLI.
- Reading feedback or inspecting selection grants no editing authority and never resolves feedback. Resolve only by explicit status update after requested work/disposition.
- Keep schemas flat. Record references use `collection/name`. Migration/import is unavailable.
