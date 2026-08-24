# Maintain an OKF bundle

Use after code, schemas, interfaces, operations, or reliable domain knowledge changes. Read specification §§5–9 as applicable, in addition to the sections required by `SKILL.md`.

1. Map the changed files and facts to existing concepts through `resource`, `sources`, links, filenames, and `index.md`. Identify material components without a concept and concept documents whose claims no longer reflect the source. Done when each change is classified as covered, needs update, needs a new concept, or is outside the bundle's scope.
2. Update affected concepts or create a new one. Every concept needs parseable frontmatter with a non-empty `type`; use descriptive, consistent types. Keep the body readable and link related concepts with bundle-relative or relative Markdown links. Add sources only when known. Done when each in-scope claim reflects evidence and every new concept is structurally conformant.
3. Apply lifecycle and provenance carefully. Record `generated` only when actor and current UTC time are known; preserve `verified`, `status`, `stale_after`, and unknown extension keys unless the evidence warrants a change. Done when metadata has not overstated freshness, trust, or provenance.
4. Maintain reserved files when present or deliberately introduced. Update the containing directory's `index.md` with title and description entries; add a newest-first dated `log.md` entry describing the change. The root `index.md` may declare `okf_version: "0.2"`; do not add arbitrary concept frontmatter to index or log files. Done when navigation and history match the concepts.
5. If reorganization is warranted, propose it before moving files. After approval, update all affected links and indexes. Done when relocated concepts are reachable through their intended paths.
6. Run the validation workflow. Done when its conformance result and non-blocking warnings are known.
