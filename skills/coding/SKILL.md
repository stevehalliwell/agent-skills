---
name: coding
description: "Use before any code change: writing, modifying, generating, patching, or deleting code. Load as soon as code changes are needed, including when implementation, web-implementation, or another delivery skill applies. Mandatory engineering layer for small, traced, proportionately verified changes. Skip recommendation-only work and documentation-only edits with no code change."
---

# Coding

Make the smallest correct code change after understanding the real path.

This skill is the mandatory engineering layer for every code change. Other delivery skills may add workflow requirements; they do not replace this one.

## Required read

Before changing code, read [Ponytail engineering guidance](references/ponytail.md) and [Simple versus easy](references/simple-vs-easy.md).

- When performance, dispatch, concurrency, or test strategy affects the change, also read [Casey Muratori: simple, good code](references/casey-muratori-good-code.md).
- When introducing or changing a module boundary or caller-facing interface, also read [John Ousterhout: software design](references/ousterhout-software-design.md).

Done when applicable constraints are known and every new abstraction removes current complexity or repetition.

## Workflow

1. Understand the change. Trace the affected path, callers, existing patterns, and relevant edge cases before proposing or editing code. Done when the real change point is known.
2. Choose the smallest solution. Apply the Ponytail ladder and reuse existing code, platform features, and installed dependencies before adding code or abstractions. Prefer the change that leaves difficult future work understandable, not the one that is merely quickest or most familiar to write. Done when the chosen approach is the simplest correct option.
3. Change and clean up. Implement the requested scope, remove obsolete local references created by replacement, and mark deliberate constrained simplifications with a `ponytail:` comment. Done when the requested behavior is complete without speculative scaffolding.
4. Verify proportionately. Run the smallest credible check for non-trivial logic; for bug fixes, prefer a reproduction that fails before the fix and passes after it. Correct in-scope failures and rerun affected checks; if correction is blocked or requires broader scope, report the blocker. State any unrun validation and remaining risk without claiming a pass. Done when verification matches the change's risk and unresolved failures are explicit.

## Code shape

- Extract repeated values, values with domain meaning, and values defined by a specification into descriptive constants or enums. Use the named protocol/specification constant when one exists.
- Keep a self-explanatory one-off literal inline when a named constant would add indirection without meaning. A constant is not configuration: do not create mutable configuration for a value that never varies.
- Use guard clauses, early returns, and `continue` to keep the main path shallow. Do not force early exits where they obscure resource cleanup, transaction boundaries, or the normal flow.
- Prefer an enum or equivalent named mode when a parameter selects behavior. Avoid boolean parameters whose `true`/`false` meaning is unclear at the call site; retain a boolean for a clear predicate or binary state.
- Separate logical code blocks with a blank line: setup, validation, main work, and result/cleanup. Do not add whitespace mechanically between every statement.

## Rules

- Preserve requested scope; do not simplify away validation at trust boundaries, data-loss protection, security, accessibility basics, or explicitly requested behavior.
- Apply a more specific workflow skill alongside this layer when its trigger fits.
