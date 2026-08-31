---
name: attendant
description: "Set up Attendant, create an empty collection/table, create a record, query or search local records, validate records, diagnose projection health, or migrate existing Markdown into Attendant. Use when working with `.pi/attendant.tables`, Attendant collection schemas, or Attendant-managed Markdown; route setup, empty-collection creation, and migration through their guarded workflows. Skip ordinary Markdown editing and non-Attendant data."
---

# Attendant

Operate Attendant through its runner. Markdown is the source of truth; `.attendant/` is disposable generated projection state.

## Workflow

1. Identify the requested operation and read exactly its reference before acting:
   - Setup or an empty collection: [Setup and collection workflows](references/collections.md).
   - Create one record or a validated batch, validate, sync, inspect, query, search, or diagnose: [Record and query commands](references/commands.md).
   - Convert existing Markdown records: [Markdown migration](references/migration.md).
   - Done: the operation's preconditions, confirmation gate, and completion condition are known.
2. Resolve `<skill-dir>` as this `SKILL.md` directory. Run commands from the target project, or add `--project <path>` / `-p <path>`.

   ```sh
   node <skill-dir>/scripts/attendant.mjs <command> [options]
   ```

   Commands emit one JSON result to stdout and diagnostics to stderr. Use the runner rather than writing generated projection files.
   - Done: command location and target project are unambiguous.
3. Follow the selected reference's workflow, including its required inspection and user-confirmation gates. Report the JSON result and any diagnostics with affected paths.
   - Done: source changes, if any, are within the approved workflow and the result is understood.

## Rules

- Require Node.js `>=24.10.0`; migration additionally requires an existing Git commit before apply.
- Use `@path` for UTF-8 option input and `-` for stdin once. JSON options must be objects.
- Use `search` for text lookup. `query` accepts one read-only, authorizer-contained statement.
- Before creating records or selecting, filtering, or ordering by collection-specific fields, read that collection's `.schema.md` or inspect `schema`. Use only declared fields and documented built-ins; never assume fields from another project, template, or skill.
- Use `doctor` only for reported health or projection problems; normal data actions prepare the projection themselves.
- Preserve source Markdown; never edit `.attendant/` directly.
- Keep schemas flat: `.schema.md` declares front-matter fields and `.template.md` supplies optional record-body copy. Record references use `collection/name`.
