# Setup and collections

## Setup

Bootstrap Attendant source configuration. Markdown stays source of truth; `.attendant/` is generated local state.

Use this workflow for an unconfigured project. If `.pi/attendant.tables` already exists, preserve it and report current setup state. Use the empty-collection workflow below for a new collection, or [Markdown migration](migration.md) for existing records.

1. Inspect project root, `AGENTS.md`, `.pi/attendant.tables`, `.gitignore`, configured collections and their `.schema.md` files, and unconfigured Markdown folders. Ignore generated/vendor folders and ordinary singleton docs; flag folders containing candidate record sets or front matter. State whether Attendant is configured.
   - Done: bootstrap files, current Attendant locations, candidate migration sources, and existing Markdown are known.
2. If candidate record sources exist, explain why migration may preserve more value than a blank setup and ask whether to route to migration. Continue bootstrap only when user chooses fresh setup or candidates are unrelated docs. Create missing `.pi/attendant.tables` as an empty file. Create `.gitignore` if absent, or append `.attendant/` only when no equivalent ignore entry exists. Do not alter existing config lines or ignore rules.
   - Done: user has chosen migration or project has Attendant config and generated state is ignored.
3. Create or update a concise `## Attendant` section in project-root `AGENTS.md`. State that Attendant is in use; identify `.pi/attendant.tables` as the collection configuration; list each configured collection directory and its `.schema.md` path; and state that Markdown is source of truth while `.attendant/` is generated state. If no collection exists yet, say so explicitly. Preserve unrelated instructions and an existing Attendant section's relevant project-specific guidance.
   - Done: `AGENTS.md` tells future agents whether Attendant is in use and where its configuration and schemas are.
4. Normal Attendant actions automatically validate and refresh generated projection; do not invoke manual validation or sync tools as routine setup. Run `node <skill-dir>/scripts/attendant.mjs doctor` only if setup reports an error or health/projection problem needs diagnosis.
   - Done: setup result is understood; any error is reported with path and cause.
5. Explain that setup created no collection, schema, or record. Route new collection work to the workflow below, or existing Markdown work to [migration](migration.md).
   - Done: next workflow is explicit.

Rules:

- Write only missing `.pi/attendant.tables`, the `.attendant/` Git-ignore entry, and the scoped `AGENTS.md` Attendant section.
- Preserve all Markdown, existing config, schemas, records, and unrelated Git-ignore and agent instructions.
- Never infer schema, create a collection, or migrate existing Markdown during setup.

## Add an empty collection

Create one empty Attendant collection. Markdown remains source of truth; `.attendant/` remains generated state.

Use only for a new collection. If `.pi/attendant.tables` is missing, complete setup first. If target folder already contains Markdown records, use [migration](migration.md); do not adopt or move records here.

1. Inspect `AGENTS.md`, `.pi/attendant.tables`, `.gitignore`, configured collection schemas, requested target folder, and nearby unconfigured Markdown folders. Use `node <skill-dir>/scripts/attendant.mjs doctor` when health diagnostics are needed; normal actions automatically validate and refresh generated projection.
   - Done: existing collection names, documented locations, target state, and possible migration sources are known.
2. Ask for collection directory and optional alias. Do not ask for schema fields; collection creation intentionally starts with empty `.schema.md` and `.template.md`.
   - Done: proposed config line and empty source files are concrete.
3. Check target directory and migration fit. Accept only a nonexistent directory or an empty existing directory. Reject duplicate collection alias/name, reserved `__attendant_` name, duplicate config entry, unsafe path, and non-empty directory. If existing Markdown appears intended as collection data, explain that migration preserves and maps it; route there unless user explicitly confirms a separate empty collection.
   - Done: creation cannot overwrite, adopt, or hide source records; migration alternative is considered.
4. Present proposed directory and config line. After user confirms, run:

   ```sh
   node <skill-dir>/scripts/attendant.mjs add-table --directory <directory> [--alias <alias>]
   ```

   Do not write collection files directly, add fields, or create records.
   - Done: runner result reports one declared empty collection and created source paths.
5. Update the project-root `AGENTS.md` `## Attendant` section to list the new collection directory and `.schema.md` path, while preserving existing project-specific Attendant guidance and unrelated instructions. Report collection path/name, empty declared fields, and created source paths. Do not invoke manual validation, sync, or doctor after success; normal actions handle preparation automatically. Use `doctor` only if an error or health/projection problem needs diagnosis.
   - Done: `AGENTS.md` and the operation result identify the new collection and schema; any diagnostic is reported with path and cause.

Rules:

- Preserve existing config lines, schemas, records, `.gitignore`, and unrelated agent instructions.
- Keep schema flat. `.schema.md` defines front-matter fields; `.template.md` defines optional record body copy.
- Stop after empty collection setup; field authoring and record creation are separate workflows.
- Never create nested YAML, database migrations, or an alternate manifest.
- Do not choose a first record or semantic fields for user.
- Use project-relative paths unless user explicitly chooses a configured external directory.
