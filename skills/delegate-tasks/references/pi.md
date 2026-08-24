# Pi harness

Use `"harness": "pi"` for an ephemeral, self-contained Pi child. It is the default harness. The generic runner owns lifecycle, detached background launch, deadlines, cancellation, and artifacts.

## Resource isolation

A Pi child starts with:

```text
--no-session --no-skills --no-extensions --no-context-files
--no-prompt-templates --no-themes --no-approve
```

It inherits neither parent conversation nor automatically discovered Pi resources. This keeps global/project facets out of background sessions by default.

To opt in to a known resource, list its Pi name (resolved from global or containing-project resources) or an explicit path:

```json
{
  "name": "review",
  "prompt": "tasks/review.md",
  "harness": "pi",
  "skills": ["technical-review"],
  "extensions": ["approved-extension"],
  "exclude_skills": ["technical-review"],
  "exclude_extensions": []
}
```

`skills` and `extensions` are allowlists; named values resolve only from `~/.pi/agent` or the containing project’s `.pi` resources. Their matching `exclude_*` values remove entries by full path or filename/directory name before Pi starts. Use exclusion only when a batch/default list needs a task-level exception; omit the resource otherwise.

## Settings

`model` and `thinking` are passed to Pi when specified. `web_search` and `ephemeral` are Codex-specific: use a deliberately allowlisted search extension or skill only when a Pi child needs network access.

The task prompt is passed explicitly as `@<prompt-file>` to `pi --print`. Pi’s final stdout is saved as the task `result.md`; stderr is retained separately. This applies to both blocking and detached launches.
