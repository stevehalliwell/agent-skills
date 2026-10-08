---
name: cli-design
description: "Design, build, review, or improve a command-line interface (CLI), especially commands meant for humans, shell scripts, CI, or AI agents. Use before implementing CLI commands, output formats, automation, authentication, errors, or agent workflows. Produce a predictable dual human-and-machine interface; skip ordinary library APIs and graphical interfaces."
---

# CLI Design

Build CLIs that remain usable at a terminal and reliable under automation.

## Trigger clarification

Start by inspecting relevant commands, help, tests, and caller usage. For review requests, report findings without changing code; for implementation requests, apply this workflow to the agreed change. If no target command or behavior is supplied or discoverable, ask one focused question. Skip ordinary library APIs and graphical interfaces.

## Workflow

1. Define command intent, required inputs, side effects, and success result before changing code. Inspect existing flags, output formats, exit codes, and script callers; preserve established contracts unless a breaking change is agreed. Keep one command responsible for one clear operation. Done when command behavior, compatibility constraints, and boundaries are stated.

2. Establish the output contract.
   - Reserve stdout for command data.
   - Send human-oriented tables, progress, prompts, warnings, and diagnostics to stderr.
   - For new interfaces, use stdout TTY detection to choose readable result data at a terminal and JSON when piped. Explicit output flags take precedence; agent mode defaults to JSON. Preserve existing defaults for established interfaces.
   - Keep human display tables on stderr; do not duplicate required result data there. Document which stdout formats callers can expect.
   - Return valid structured output that scripts and agents can consume without scraping prose.
   Done when callers can pipe or parse stdout without handling display text.

3. Make structured output efficient and navigable.
   - Offer field selection such as `--json field1,nested.field2` for commands that can return large records.
   - Use stable field names and shapes. Represent an empty successful result as empty `data`, not an error.
   - Include `breadcrumbs` with safe, relevant next commands when a response naturally leads to follow-up actions.
   Done when an agent can request only needed data and identify the next supported action from the response.

4. Make automation first-class.
   - Provide an explicit agent-mode environment variable and detect common non-interactive contexts such as CI where appropriate.
   - In agent/non-interactive mode, suppress prompts, avoid browser-only flows, default to structured output, and include useful metadata. Missing required input or approval must fail with an actionable error; non-interactive mode never implies consent to destructive actions.
   - Support credentials through documented environment variables or other non-interactive secure mechanisms; never require a browser click for automation.
   - Where auditability helps, support append-only JSONL output to a user-selected file.
   Done when all supported command paths work without terminal input or a browser.

5. Specify failure semantics.
   - Use exit code `0` only for success, including empty successful results; use nonzero codes for failures.
   - On structured-output paths, return a machine-readable error object on stdout with stable fields such as `code` and `message`; keep human explanation on stderr. Document whether failed commands can also return partial data.
   - Avoid ambiguous success messages and undocumented exit-code behavior.
   Done when an automated caller can distinguish success, empty result, and failure from exit status and structured data.

6. Validate both interfaces.
   - Test applicable cases: interactive TTY invocation, piped invocation, explicit JSON invocation, empty result, expected failure, missing non-interactive credentials, and agent mode. Mark unsupported cases not applicable with a reason.
   - Check explicit format precedence, existing caller compatibility, and failure without prompting when required input or approval is missing.
   - Verify stdout parses cleanly and stderr contains no required machine data. Correct failed cases within scope and rerun affected cases; report any blocker or unrun case rather than claiming it passed.
   Done when each case has expected output, exit status, and no interactive dependency, or remaining failures and gaps are explicit.

## Output shape

Report contract choices or review findings, changed paths when applicable, checks run, and blockers or unrun cases. Static review alone does not prove runtime behavior.

Examples:
- “Review this CLI's JSON output”: inspect output paths and callers, report compatibility and parsing issues; do not edit code.
- “Add `--json` to `list`”: define fields, empty results, and errors; implement within agreed scope and test parseable stdout plus exit status.

## Rules

- Treat structured output as a public API; change it deliberately and preserve compatibility where possible.
- Prefer predictable flags, stable command nouns and verbs, and documented environment variables over hidden context.
- Keep credentials out of command output, JSONL capture, and error text.
- Do not require agents to scrape tables, prose, terminal color, or progress indicators.
- Do not add a `--json` dump without considering field selection, empty results, errors, and follow-up guidance.

## Reference

Guidelines adapted from [Building a CLI for Humans and AI Agents](https://dev.to/martakar/building-a-cli-for-humans-and-ai-agents-1lpj).
