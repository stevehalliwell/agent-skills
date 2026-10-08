# Markdown migration

Discover first. Convert only reviewed mappings. Use only for existing Markdown that must become Attendant records; do not use it for a new empty collection. Run migration from target project directory; `--project` / `-p` is not supported.

1. Inspect Git status, existing Markdown folders, front matter, headings, and prose. Complete setup if `.pi/attendant.tables` is missing.
   - Done: source folders, candidate metadata, unsupported nested data, links, and proposed collection boundaries are known.
2. Write `migrations/<slug>.md` with front matter status `draft`. Include `collections` and `files` mapping arrays. Each file needs project-relative `source`, `destination`, SHA-256 `source_hash`, `fields`, and exact body `remove` spans (`start`, `end`, `text`). Explain inferred fields, warnings, and unmapped prose in Markdown body. Apply creates `.template.md` for each new collection: preserve an existing template, otherwise copy body from first mapped record in plan order. Apply also preserves an existing `.usage.md`, or creates an empty one. Do not infer collection purpose or workflow policy from migrated records; author confirmed guidance separately.
   - Done: plan states every candidate source write and body deletion exactly.
3. Re-read plan with user. Identify ambiguous field meaning, invalid flat-schema markers, conflicting destinations, unmapped metadata, stale hashes, unresolved refs, overlapping spans, and record-loss risk. Ask targeted questions; update plan; repeat until no gaps remain. Preserve unmapped prose.
   - Done: user accepts complete mapping; plan status is `ready`.
4. Check the ready plan:

   ```sh
   node <skill-dir>/scripts/attendant.mjs migrate check --plan migrations/<slug>.md
   ```

   Fix reported paths, hashes, collisions, and spans; rerun check after changes. `ok: true` checks mapping mechanics, not schema validity, field values, or references. Inspect those against collection contract separately. Verify each source appears once and each destination belongs to intended collection. Show exact collection/file/body-span actions, template and usage writes, and rollback limits below; ask user for final apply confirmation.
   - Done: check emits `ok: true`; user explicitly confirms apply.
5. Apply the approved plan:

   ```sh
   node <skill-dir>/scripts/attendant.mjs migrate apply --plan migrations/<slug>.md
   ```

   Git must have an existing commit, but plan and unrelated worktree changes may remain uncommitted. Apply creates inferred or empty `.template.md` and missing empty `.usage.md` before moving records. Check/apply results list newly created usage paths under `usages`; plan status becomes `applied`. Apply does not validate records or refresh projection.
   - Done: apply result and actual changed paths are understood; on failure, inspect partial writes before retrying.
6. Run `node <skill-dir>/scripts/attendant.mjs validate --strict`. Resolve only approved in-scope defects and rerun; stop and report paths if correction needs new mapping approval. After validation passes, run `node <skill-dir>/scripts/attendant.mjs sync`. Update project-root `AGENTS.md` Attendant section with new collection, schema, and usage paths, preserving unrelated instructions. Report source changes, Git diff, and remaining diagnostics; use `doctor` only for health/projection failures.
   - Done: source validation and projection refresh pass, and future agents can locate migrated collections.

## Plan shape

```yaml
---
status: draft
collections:
  - directory: records/notes
    alias: notes
    schema: |
      ---
      title: ""
      status: [draft, done]
      ---
files:
  - source: inbox/idea.md
    destination: records/notes/idea.md
    source_hash: <sha256 of source file>
    fields:
      title: Idea
      status: draft
    remove:
      - start: 0
        end: 8
        text: "# Idea\n\n"
---
```

Rules:

- Apply requires existing Git history and matching hashes. Apply is not transactional; errors can leave partial source changes. Existing commit does not preserve uncommitted or untracked source. Before approval, identify how each affected source will be restored; if baseline lacks current content, resolve recovery with user. Never reset unrelated work or assume Git revert recovers untracked files.
- Paths stay inside project root. Never overwrite destination files.
- Do not rewrite links, delete unapproved content, flatten nested data, or create backup copies.
- `.schema.md` defines flat front matter; `.usage.md` defines collection purpose and operating guidance; `.template.md` is body-only record copy.
- Preserve an existing `.template.md`; do not infer templates during normal record creation.
- Apply writes universal Attendant fields plus approved fields, moves files, and removes only exact mapped spans.
