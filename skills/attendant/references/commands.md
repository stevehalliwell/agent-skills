# Commands and human feedback

`<attendant>` is native CLI resolved by [skill workflow](../SKILL.md#workflow). Quote absolute executable paths. Run from target project or supply `--project` / `-p`. No migration/import command. Missing build: [build reference](build.md).

All commands emit one JSON value on stdout; failures also explain on stderr. Exit 0 means success; 1 means operational/validation failure. Inspect diagnostics even on success: malformed records can be diagnosed/skipped. `@path` reads UTF-8 text input; `-` reads stdin once. Use lossless JSON parsing when integers may exceed JavaScript safe range.

Before create/update or collection-specific lookup, run `schema` and read collection usage. Follow lifecycle and approval rules; examples below assume declared `tasks.status` values `todo` and `done`.

## Create and update

```sh
<attendant> create -c tasks -i @items.json
<attendant> update -c tasks -i @updates.json
```

Both require `--collection`/`-c` and `--items`/`-i`. Input is nonempty array of `{ "name": "safe-slug", "fields": { ... } }`, for one or several records:

```json
[{"name":"write-docs","fields":{"status":"todo","desc":"Write documentation"}}]
```

Only declared fields plus writable `desc` (string) and `tags` (string array) are accepted. Create generates `id`, `name` and `created_at`, applies defaults and body template. Update rejects immutable fields, including `id`, `name` and `created_at`; preserves body and unrelated formatting, but changed values may be reformatted.

Every prospective item/target is validated before source publication. Each file publication is atomic; batch is not cross-file transaction. Completed writes survive later failures, including failed projection refresh. Inspect reported `written`/`failed` and actual source before retry; never assume whole batch rolled back. Collection/feedback bootstrap cleanup differs by platform: [source contract](../native/README.md#source-and-data-contract).

Edit saved Markdown body directly when task requires prose changes; metadata CLI does not replace body. Normal actions reconcile projection afterward.

## Validate and sync

```sh
<attendant> validate
<attendant> validate --no-correct
<attendant> validate --strict
<attendant> sync
```

Default validation applies mechanical metadata corrections. `--no-correct` reports without source changes; `--strict` preserves source and exits 1 for diagnostics or pending corrections. Neither read-only validation mode prepares projection. `sync` prepares/reuses/updates/rebuilds disposable projection without source correction. Do not add routine validation/sync after successful actions.

## Schema, query and search

```sh
<attendant> schema
<attendant> query -s "SELECT 'tasks' AS collection,id,name,source_path FROM tasks WHERE status=:status" -P '{"status":"todo"}' -l 20
<attendant> search -q 'SQLite projection' -c tasks,decisions -l 20
<attendant> doctor
```

`schema` reports collection `name`, absolute `directory`, `schemaPath`, `usagePath`, full Markdown in `usage`, declared `fields` and SQLite/FTS metadata. Missing usage reports `null`; empty file reports `""`. Other usage-read failures fail inspection. Usage is read live from source, not projected as record.

`schema`, `query` and `search` may apply mechanical source corrections before preparing projection. Inspect corrections and diagnostics. For source-preserving diagnosis, use `doctor`; for correction preview, use `validate --no-correct` or `--strict`.

`query` requires `--sql`/`-s`: one authorizer-contained read-only statement. `--params`/`-P` defaults to `{}`; object values must be scalar/null. Projected built-ins are `id`, `name`, `created_at`, `updated_at`, `desc`, `tags`, `hidden`, `source_path` and `source_hash`; collection-specific fields come from schema. Include collection identity as SQL literal; it is not projected column. In Bash, do not backslash-escape JSON already inside single quotes; prefer `-P @params.json` for generated/complex input.

`search` requires `--query`/`-q`; `--collections`/`-c` is comma-separated. Results identify collection and source path. Query/search return `columns`, `rows`, `returned`, `truncated`; empty success has zero rows, not failure. Check `truncated` before claiming complete inventory. `--limit`/`-l` is non-negative integer, defaults to 100 and caps at 10,000; other capacity/output bounds may apply sooner. [Types and limits](../native/README.md#projection-and-limits).

`doctor` diagnoses source/config/projection health read-only; it does not check Git or repair data. Unhealthy doctor exits 1 with diagnostic result. Do not use as routine follow-up.

## Failures

Operational errors emit `{ "error": { "code": "...", "message": "..." } }`, with `written`/`failed` when relevant; exit 1 and explanation on stderr. Strict validation and unhealthy doctor instead return diagnostic results with exit 1. Missing executable is build failure, not empty data; follow build reference rather than switching runtime/storage.

## Human feedback

Ordinary query/search may include `meta_comments.pending` and concise notice. Notice is reminder, not full inbox or permission to edit target. Missing inbox is ordinary empty state; malformed/conflicting feedback remains diagnostic, not automatic repair.

1. Inspect configured `_meta_comments` schema and usage before filtering/updating fields.
2. Query pending comments explicitly; read source Markdown for full human body and rendered context.
3. Check saved target hash and source range. Context-only feedback has zero bounds/empty source text; never guess anchor. Reading does not resolve comment or authorize target changes.
4. Resolve only after requested work/disposition, with explicit update to declared `status: resolved`. Preserve human body; re-query pending count to verify.

```sh
<attendant> query -s "SELECT name,source_path,target_path,target_hash,range_start,range_end,selected_text FROM _meta_comments WHERE status='pending'"
<attendant> update -c _meta_comments -i '[{"name":"comment-name","fields":{"status":"resolved"}}]'
```

GUI source editing and **Save & Send** are separate from CLI metadata writes. [GUI controls](../native/apps/gui/README.md#agent-controls) reveal files and inspect live selections; neither starts viewer or changes human selection on receipt.
