# Game Design Workbench

One Markdown game-design model, selected framework reviews, and local query/report helpers. Current delivery includes collection contracts and provisional framework guides. Guides need framework-by-framework source/completeness review before Slice 2; they are not comprehensive references.

## Start

1. Agent: load [skill](SKILL.md). Direct CLI user: complete [Runtime](references/runtime.md#runtime), then read [CLI contract](references/cli.md).
2. Try [crate-and-switch walkthrough](examples/crate-puzzle/README.md), or use [setup](references/setup.md) for a project. Setup creates no game facts.
3. For field ownership and shared record rules, use [model contract](references/model.md).

## Delivery status

- Available: guarded setup/update, declared-data queries, term resolution, selected agent review, dated report storage.
- Deferred: C99 semantics/control exports, graphical model inspection (Slice 4), simulation and result views (Slice 5).
- Offline handoff report describes delivery evidence; it is not a model viewer.

## Maintenance

Canonical source owns generators and tests. Installed snapshots exclude maintainer code. See [CLI maintainer tools](references/cli.md#maintainer-tools) for commands.
