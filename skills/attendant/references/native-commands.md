# Native commands and feedback

Read for explicit native evaluation. Setup/missing build: [build reference](build.md). Existing JS workflow stays default until rollout approval. Native has no migration/import command.

`<native>` means `<skill-dir>/native/build/attendant-zig.exe` or CJIT counterpart. Run from target project, or supply `-p <project>`. Preserve existing collection setup/record approval gates and inspect schema/usage before managed data changes.

## Commands

```sh
<native> --help
<native> doctor -p /path/to/project
<native> validate --strict -p /path/to/project
<native> validate --no-correct -p /path/to/project
<native> sync -p /path/to/project
<native> schema -p /path/to/project
<native> query -p /path/to/project -s 'SELECT name,status FROM tasks WHERE status=:status' -P '{"status":"todo"}'
<native> search -p /path/to/project -q 'phrase' -c tasks -l 20
<native> add-table -p /path/to/project -d records/tasks -a tasks
<native> create -p /path/to/project -c tasks -i '[{"name":"example","fields":{"status":"todo"}}]'
<native> update -p /path/to/project -c tasks -i '[{"name":"example","fields":{"status":"done"}}]'
```

Example `tasks.status` assumes schema declares those values; never invent fields from examples. Create/update use nonempty arrays of `{name, fields}`. Text options accept `@path` (UTF-8) or `-` (stdin once). Long option names are listed in `--help`.

- Markdown is authoritative; never edit projection directly. `doctor` is read-only; default `validate` applies mechanical metadata corrections. `validate --strict` and `--no-correct` do not write. `schema`/`query`/`search` may correct source before preparing cache. Inspect reported corrections/diagnostics.
- Query accepts one authorizer-contained read-only SQL statement. Query/search return `columns`, `rows`, `returned`, `truncated`; empty successful result has zero rows, not failure.
- Errors emit `{ "error": { "code": "...", "message": "..." } }`, with `failed`/`written` when relevant. Exit 1; explanation also on stderr. Strict validation/unhealthy doctor return diagnostic result with exit 1. Check source state before retrying mutation; completed writes may remain.
- Integer JSON tokens may exceed JavaScript safe range; use lossless parsing when precision matters. Data types and limits: [native contract](../native/README.md#source-and-data-contract).

## Human feedback

Ordinary `query`/`search` results may contain `meta_comments.pending` and concise notice. Notice is reminder, not full inbox and not authority to edit target. Missing inbox is ordinary empty state; malformed/conflicting feedback remains diagnostic, not automatic repair.

1. Inspect configured `_meta_comments` schema and usage before filtering/updating fields.
2. Query pending comments explicitly; inspect returned source Markdown for full human body and rendered context.
3. Check saved target hash and any supplied source range. Context-only feedback has zero bounds/empty source text; never guess anchor. Reading does not resolve comment or authorize target changes.
4. Resolve only after requested work/disposition, with explicit `update` setting declared `status` to `resolved`. Preserve human comment body. Re-query pending count to verify.

```sh
<native> query -s "SELECT name,source_path,target_path,target_hash,range_start,range_end,selected_text FROM _meta_comments WHERE status='pending'"
<native> update -c _meta_comments -i '[{"name":"comment-name","fields":{"status":"resolved"}}]'
```

GUI source editing and **Save & Send** are separate from metadata CLI writes. Agent reveal/inspection: [GUI contract](../native/apps/gui/README.md#agent-controls). Neither control command starts GUI or changes human selection on receipt.
