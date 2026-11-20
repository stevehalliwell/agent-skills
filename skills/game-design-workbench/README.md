# Game Design Workbench

Model and investigate a game design in Markdown. Record nouns, rules, skills, challenges, and resources; explore what the records say without turning them into a game simulation.

## Start

1. Load the [Workbench skill](SKILL.md) and install [native Attendant](../attendant/) for record editing, queries, and validation.
2. Walk through [Crate Puzzle](examples/crate-puzzle/README.md), explore the larger [Little Plumber Guy NES example](examples/little-plumber-guy-nes/README.md), or trace a resource conversion in [Forge Chain](examples/forge-chain/README.md). To start your own design, follow [setup](workflows/setup.md) and the [model contract](workflows/model.md).
3. Follow the [native build and explorer guide](native/README.md) to build the CLI and optional desktop viewer. Windows native core and CLI checks pass on the release candidate; macOS runtime is not yet validated on this snapshot.

## What it does

- Query declared relationships, prerequisites, resource routes, and conditional actor/effect outcomes with the native CLI.
- Use `gd view explorer` for linked graphs and grids, exact case and source inspection, and State Charts. Run analyses explicitly; switching views does not apply pending inputs.
- Let an agent inspect selection, navigate, stage inputs, and run supported investigations through [local viewer IPC](native/controls.md). Source changes stay outside IPC; use Attendant for authoritative record updates.
- Apply [framework guides](frameworks/index.md) and [source notes](references/README.md) from local Markdown. Source links are optional further reading. Schell’s card JSON and derived table are bundled for offline use.
- [Tune declared controls](workflows/tune-controls.md) or prepare an [authorized experiment](workflows/experiment.md) when a design question needs measurement.

Graphs and results describe authored declarations under a loaded scope. They do not prove runtime feasibility, difficulty, causation, or what players experience.

## Maintain

Edit Markdown in `templates/`, `examples/`, `frameworks/`, and `workflows/` directly. See [native build and test commands](native/README.md#build) and [dependency notices](native/THIRD-PARTY.md) for C99 tooling. Do not bundle source PDFs or saved webpages for core use.
