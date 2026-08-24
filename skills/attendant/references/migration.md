# Markdown migration

Discover first. Convert only reviewed mappings. Use only for existing Markdown that must become Attendant records; do not use it for a new empty collection.

1. Inspect Git status, existing Markdown folders, front matter, headings, and prose. Complete setup if `.pi/attendant.tables` is missing.
   - Done: source folders, candidate metadata, unsupported nested data, links, and proposed collection boundaries are known.
2. Write `migrations/<slug>.md` with front matter status `draft`. Include `collections` and `files` mapping arrays. Each file needs project-relative `source`, `destination`, SHA-256 `source_hash`, `fields`, and exact body `remove` spans (`start`, `end`, `text`). Explain inferred fields, warnings, and unmapped prose in Markdown body. Apply creates `.template.md` for each new collection: preserve an existing template, otherwise copy body from first mapped record in plan order.
   - Done: plan states every candidate source write and body deletion exactly.
3. Re-read plan with user. Identify ambiguous field meaning, invalid flat-schema markers, conflicting destinations, unmapped metadata, stale hashes, unresolved refs, overlapping spans, and record-loss risk. Ask targeted questions; update plan; repeat until no gaps remain. Preserve unmapped prose.
   - Done: user accepts complete mapping; plan status is `ready`.
4. Check the ready plan:

   ```sh
   node <skill-dir>/scripts/attendant.mjs migrate check --plan migrations/<slug>.md
   ```

   Fix all reported paths, hashes, collisions, and spans. Show exact collection/file/body-span action, then ask user for final apply confirmation.
   - Done: check emits `ok: true`; user explicitly confirms apply.
5. Apply the approved plan:

   ```sh
   node <skill-dir>/scripts/attendant.mjs migrate apply --plan migrations/<slug>.md
   ```

   Git must have an existing commit, but plan and unrelated worktree changes may remain uncommitted. Apply creates inferred or empty `.template.md` before moving records. Normal Attendant actions automatically validate and refresh generated projection; do not run doctor after successful migration. Use `node <skill-dir>/scripts/attendant.mjs doctor` only if an error or health/projection problem needs diagnosis. Report changed paths and Git diff; plan status becomes `applied`.
   - Done: apply result and any diagnostics are understood.

## Plan shape

```yaml
---
status: draft
collections:
  - directory: records/notes
    alias: notes
    schema: |
      ---
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
        end: 14
        text: "# Idea\n\n"
---
```

Rules:

- Apply requires existing Git history and matching hashes; Git diff/revert is rollback.
- Paths stay inside project root. Never overwrite destination files.
- Do not rewrite links, delete unapproved content, flatten nested data, or create backup copies.
- `.schema.md` defines flat front matter; `.template.md` is body-only record copy.
- Preserve an existing `.template.md`; do not infer templates during normal record creation.
- Apply writes universal Attendant fields plus approved fields, moves files, and removes only exact mapped spans.
