# Set up a design workspace

## Establish source owners

1. Inspect project instructions, `.pi/attendant.tables`, live schemas/usages, and existing design documents. Read loaded Attendant skill's collection workflow. Existing Markdown stays in place. Done: existing collections and design sources are known.
2. Identify one live concept owner; default `game-design/concept.md`, or explicitly bind an existing document, including README. Identify current project mode from that document. Ask only when mode or ownership is unresolved. Done: concept location and mode are explicit.
3. Propose concrete collection paths/aliases and source changes. Default is `game-design/<role>` as `<role>` for the 15 roles in distribution templates. Reuse compatible existing collections after reading their contracts. Keep existing records in place. Use Attendant's setup guidance; do not initialize empty collections over existing records. Done: user can see additions, conflicts, and preserved sources.
4. Follow Attendant's confirmation and empty-collection workflow. Copy approved schema/usage/template files afterward from shipped templates; adapt references to actual aliases. No tutorial facts enter live design. Done: approved collections exist and match the proposed contracts.
5. Link concept near README's beginning without replacing existing navigation. In project `AGENTS.md`, identify concept path, any non-default role aliases, and use of installed `game-design-workbench` and `attendant` skills. Keep `.pi/attendant.tables` authoritative for collection discovery. Done: another agent can find actual owners without a manifest.
6. Inspect diff and use non-correcting Attendant validation. Commit completed authorized concept changes under project Git policy. Report unresolved contracts and checks not implemented. Done: changes are reviewable and no unrelated source was altered.

Partial models remain useful without setup. Empty optional collections are valid. Missing/incompatible contracts are not empty query results.

## Use installed guidance

Use the installed skill directly. If the project needs local copies, copy only the guidance, templates, or tool sources it uses. Record their location in project instructions. Do not create a second configured tutorial model, installer, or hash manifest.
