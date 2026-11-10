# Setup and collections

Use native CLI resolved as `<attendant>` by [skill workflow](../SKILL.md#workflow). Run from target project or supply `-p <project>`. Missing build: [build reference](build.md). Markdown stays source of truth; `.attendant/` is disposable generated state.

## Setup

Bootstrap source configuration for unconfigured project. If `.pi/attendant.tables` exists, preserve it and report current setup state; use empty-collection workflow below for a new collection.

1. Inspect root, `AGENTS.md`, `.pi/attendant.tables`, `.gitignore`, configured collection schemas and unconfigured Markdown folders. Ignore generated/vendor folders and ordinary singleton docs. Flag existing record sets or front matter; native migration/import is unavailable.
   - Done: bootstrap state, configured collections and existing record sources are known.
2. Preserve existing Markdown. If existing record sets appear intended for conversion, report unsupported migration and stop conversion; do not register/adopt folders or replace them with blank collections. Continue fresh setup only when explicitly requested or candidates are unrelated docs. Create missing `.pi/attendant.tables` empty. Create `.gitignore` if absent, or append `.attendant/` only if no equivalent entry exists. Preserve existing config lines and ignore rules.
   - Done: fresh setup does not replace/adopt existing records; config exists and generated state is ignored.
3. Create/update concise `## Attendant` section in root `AGENTS.md`: identify native Attendant skill, `.pi/attendant.tables`, configured directories and `.schema.md`/`.usage.md` paths. Require reading schema/usage before operating on records. State Markdown is source of truth and `.attendant/` generated. Say explicitly if no collections exist. Preserve unrelated instructions and relevant project-specific guidance.
   - Done: future agents can locate configuration, schemas and usage.
4. Setup creates no collection, schema or record. Report paths and route requested empty collection to workflow below. Normal CLI actions prepare projection; do not invoke validation/sync as routine setup. Use `<attendant> doctor` only for health/projection problems.
   - Done: setup result and any blocker are understood.

Rules:

- Write only missing config, generated-state ignore entry and scoped `AGENTS.md` section.
- Preserve Markdown, schemas, records and unrelated instructions.
- Never infer schema, create collection or convert records during setup.

## Add an empty collection

Use only for new empty collection. Complete setup first if config is missing. Non-empty folders cannot be adopted here; preserve them and report unsupported conversion.

1. Inspect `AGENTS.md`, config, ignore rules, configured schemas, requested target and nearby record folders. Use `doctor` only when health diagnostics are needed.
   - Done: names, source paths and target state are known.
2. Use supplied directory/alias; ask directory only if missing. Alias defaults to directory basename. Do not ask schema fields: creation intentionally starts with empty `.schema.md`, `.usage.md` and `.template.md`. Purpose/usage authoring is separate; do not invent policy.
   - Done: proposed config line and empty files are concrete.
3. Accept nonexistent or empty directory only. Reject duplicate alias/name/config entry, reserved `__attendant_` name, unsafe path and non-empty directory. Preserve possible existing record sources; require explicit request before creating separate empty collection alongside them.
   - Done: creation cannot overwrite, adopt or hide records.
4. Present directory and config line; require explicit confirmation before creation, reusing approval of same proposal. Run:

   ```sh
   <attendant> add-table --directory <directory> [--alias <alias>]
   ```

   Do not write collection files directly, add fields, author usage or create records. Inspect output and any reported partial writes before retry; platform-specific publication cleanup is documented in [source contract](../native/README.md#source-and-data-contract).
   - Done: CLI reports declared empty collection and created source paths, or precise partial result/blocker.
5. Update root `AGENTS.md` Attendant section with new directory/schema/usage paths and read-usage rule. Preserve existing guidance. Report name/path, empty declared fields and created source paths. Do not run manual validation/sync/doctor after success; normal actions prepare projection.
   - Done: guidance matches created collection and diagnostics are reported.

Rules:

- Preserve existing config, schemas, records, ignore rules and unrelated instructions.
- Keep schema flat. Schema defines fields, usage defines purpose/rules, template supplies optional body copy.
- Preserve collections missing usage; do not retrofit policy.
- Stop after empty setup. Field/usage authoring and record creation are separate steps.
- Do not choose first record or semantic fields for user.
- Use project-relative paths unless user explicitly chooses configured external directory.
- No nested YAML, database migration or alternate manifest.
