---
name: cli-design
description: "Design, build, review, or improve a command-line interface (CLI), especially commands meant for humans, shell scripts, CI, or AI agents. Use before implementing CLI commands, output formats, automation, authentication, errors, or agent workflows. Produce a predictable dual human-and-machine interface; skip ordinary library APIs and graphical interfaces."
---

# CLI Design

Build CLIs that remain usable at a terminal and reliable under automation.

## Workflow

1. Define command intent, required inputs, side effects, and success result before changing code. Keep one command responsible for one clear operation. Done when command behavior and boundaries are stated.

2. Establish the output contract.
   - Reserve stdout for command data.
   - Send human-oriented tables, progress, prompts, warnings, and diagnostics to stderr.
   - Detect whether stdout is a TTY: present readable formatted data for an interactive terminal and JSON when stdout is piped. Support explicit structured-output selection when it improves clarity.
   - Return valid structured output that scripts and agents can consume without scraping prose.
   Done when callers can pipe or parse stdout without handling display text.

3. Make structured output efficient and navigable.
   - Offer field selection such as `--json field1,nested.field2` for commands that can return large records.
   - Use stable field names and shapes. Represent an empty successful result as empty `data`, not an error.
   - Include `breadcrumbs` with safe, relevant next commands when a response naturally leads to follow-up actions.
   Done when an agent can request only needed data and identify the next supported action from the response.

4. Make automation first-class.
   - Provide an explicit agent-mode environment variable and detect common non-interactive contexts such as CI where appropriate.
   - In agent/non-interactive mode, suppress prompts, avoid browser-only flows, default to structured output, and include useful metadata.
   - Support credentials through documented environment variables or other non-interactive secure mechanisms; never require a browser click for automation.
   - Where auditability helps, support append-only JSONL output to a user-selected file.
   Done when all supported command paths work without terminal input or a browser.

5. Specify failure semantics.
   - Use exit code `0` only for success, including empty successful results; use nonzero codes for failures.
   - On structured-output paths, return a machine-readable error object with actionable fields while keeping human explanation on stderr.
   - Avoid ambiguous success messages and undocumented exit-code behavior.
   Done when an automated caller can distinguish success, empty result, and failure from exit status and structured data.

6. Validate both interfaces.
   - Test an interactive TTY invocation, a piped invocation, an explicit JSON invocation, an empty result, an expected failure, missing non-interactive credentials, and agent mode.
   - Verify stdout parses cleanly and stderr contains no required machine data.
   Done when each case has expected output, exit status, and no interactive dependency.

## Rules

- Treat structured output as a public API; change it deliberately and preserve compatibility where possible.
- Prefer predictable flags, stable command nouns and verbs, and documented environment variables over hidden context.
- Keep credentials out of command output, JSONL capture, and error text.
- Do not require agents to scrape tables, prose, terminal color, or progress indicators.
- Do not add a `--json` dump without considering field selection, empty results, errors, and follow-up guidance.

## Reference

Guidelines adapted from [Building a CLI for Humans and AI Agents](https://dev.to/martakar/building-a-cli-for-humans-and-ai-agents-1lpj).
