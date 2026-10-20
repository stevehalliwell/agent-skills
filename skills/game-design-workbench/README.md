# Game Design Workbench

One Markdown game-design model, selected framework reviews, and agent workflows using Attendant and Git directly. Current delivery includes collection contracts and provisional framework guides. Guides still need individual source/completeness review before Slice 2; they are not comprehensive references.

## Start

1. Load [skill](SKILL.md); use separately installed Attendant for its record operations.
2. Read [concept and working mode](workflows/concept.md) and [model contract](workflows/model.md).
3. Try [crate-and-switch walkthrough](examples/crate-puzzle/README.md), or follow [setup](workflows/setup.md). Setup adds no game facts.
4. Direct commands, experiment checkpoint workflow, and maintainer tools: [existing tools](workflows/commands.md).

## Delivery status

- Available: guided setup/update, direct record/relationship inspection, declared term matching, selected agent review, ordinary dated review documents when requested.
- Deferred: C99 semantics/control-variable import/export (Slice 3), graphical model inspection (Slice 4), simulation and result views (Slice 5).
- No custom setup, query, snapshot, report, or Git lifecycle subsystem. Skill instructions use existing tools.
- Historical offline delivery report remains evidence of an older version, not a current model viewer or verification run.

## Layout

- `workflows/`: agent procedures and shared model/intent contracts.
- `frameworks/`: selected framework guides and discovery catalog.
- `references/<framework>/`: supporting source material, separate from application guidance. Thirteen folders are reserved; `.gitkeep` retains empty targets in Git. No local source copies supplied yet.
- `templates/`: authoritative starting collection files.
- `examples/`: authored game-design example and historical evidence.

Each framework guide ends with References. Add local source Markdown only after checking source adequacy and reuse terms; public access is not redistribution permission. Otherwise use an original summary with citation rather than full transcription. Book-based material remains bibliographic unless suitable reuse permission exists.

## Maintenance

Edit templates, example Markdown, framework guides, and catalog directly. No generators or JavaScript content mirrors. One test file validates shipped assets and temporary copies through external Attendant; it is not a project runtime requirement.
