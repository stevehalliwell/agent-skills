---
name: coding
description: "Write, change, fix, or review ordinary code when a more specific workflow does not apply. Use Ponytail engineering judgment to understand the real path, prefer the smallest correct solution, avoid speculative abstraction, and leave proportionate runnable verification. Use implementation for agreed tracked work, web-implementation for web delivery, and technical-review for recommendation-only review."
---

# Coding

Make the smallest correct code change after understanding the real path.

## Required read

Read [Ponytail engineering guidance](../references/ponytail.md) before changing code. Done when the applicable constraints are known.

## Workflow

1. Understand the change. Trace the affected path, callers, existing patterns, and relevant edge cases before proposing or editing code. Done when the real change point is known.
2. Choose the smallest solution. Apply the Ponytail ladder and reuse existing code, platform features, and installed dependencies before adding code or abstractions. Done when the chosen approach is the simplest correct option.
3. Change and clean up. Implement the requested scope, remove obsolete local references created by replacement, and mark deliberate constrained simplifications with a `ponytail:` comment. Done when the requested behavior is complete without speculative scaffolding.
4. Verify proportionately. Leave or run one smallest credible check for non-trivial logic; state any unrun validation and remaining risk. Done when verification matches the change's risk.

## Code shape

- Extract repeated values, values with domain meaning, and values defined by a specification into descriptive constants or enums. Use the named protocol/specification constant when one exists.
- Keep a self-explanatory one-off literal inline when a named constant would add indirection without meaning. A constant is not configuration: do not create mutable configuration for a value that never varies.
- Use guard clauses, early returns, and `continue` to keep the main path shallow. Do not force early exits where they obscure resource cleanup, transaction boundaries, or the normal flow.
- Prefer an enum or equivalent named mode when a parameter selects behavior. Avoid boolean parameters whose `true`/`false` meaning is unclear at the call site; retain a boolean for a clear predicate or binary state.
- Separate logical code blocks with a blank line: setup, validation, main work, and result/cleanup. Do not add whitespace mechanically between every statement.

## Rules

- Preserve requested scope; do not simplify away validation at trust boundaries, data-loss protection, security, accessibility basics, or explicitly requested behavior.
- Prefer a more specific workflow skill when its trigger fits.
- Do not add frameworks, fixtures, or broad test suites unless the task needs them or the user requests them.
