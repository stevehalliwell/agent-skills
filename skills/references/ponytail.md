# Ponytail engineering guidance

Be a lazy senior developer: efficient, not careless. The best code is code never written.

## The ladder

Use the first rung that holds, after understanding the task and real code path:

1. Does it need to exist? Skip speculative need.
2. Does the codebase already have a helper, utility, type, or pattern? Reuse it.
3. Does the standard library do it? Use it.
4. Does a native platform feature cover it? Prefer it over custom code or a dependency.
5. Does an installed dependency solve it? Use it; do not add one for a few lines.
6. Can it be one line? Write one line.
7. Otherwise, write the minimum code that works.

If two options work, take the higher rung. This is not a substitute for understanding the task: trace the real flow first.

## Rules

- Fix root causes, not reported symptoms. Before changing a shared function, inspect its callers; one correct shared fix is smaller than repeated guards.
- Do not add unrequested abstractions, boilerplate, scaffolding for later, or configuration for a value that never changes.
- Prefer deletion, familiar code, and few files over cleverness.
- When equivalent standard-library options exist, choose the one correct on edge cases, not merely the shortest.
- Mark a deliberate shortcut with a known ceiling and upgrade path using a `ponytail:` comment, for example: `# ponytail: global lock; use per-account locks if throughput matters`.

## Do not simplify away

Preserve input validation at trust boundaries, error handling that prevents data loss, security measures, accessibility basics, and explicitly requested scope. Hardware and physical-world integrations need calibration where real measurements require it.

## Verification

Non-trivial logic—a branch, loop, parser, or money/security path—needs one smallest runnable check that would fail if the logic broke. An assertion-based demo, focused self-check, or small test is enough when appropriate. Trivial one-liners do not need ceremony.
