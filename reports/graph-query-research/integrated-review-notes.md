# Integrated graph review notes

## Capture scope

Notes captured, then discussed against remaining todo work. User requests coverage of new feedback in existing planned tasks and explicit review against these notes at next checkpoint after next implementation batch. User later authorized bounded unified-explorer implementation under Review 06. Interaction choices and technical contract live in `todo.md`; user accepted bounded unified-shell checkpoint, authorized commit and requested continuation through known steps until next review point unless material questions arise. Broader integrated acceptance remains open.

## User observations and questions

- Skills right-angle/squared connections are much harder to read than previous node-to-node lines. User expected node placement/layout optimization to reduce crossings, not routing changes that make connections worse. Older node-to-node lines were better.
- Rounded group backgrounds are acceptable in principle, but lines and potentially labels extend beyond them. User prefers no rounded background over content escaping its bounds.
- MDA curved connections and Resource straight connections seem unchanged and remain fine. Objection targets new Skills connections, not those renderers.
- Resource tag-cloud layout, mostly individual nodes floating in empty space, is not very usable. User is unsure whether dataset contributes to problem. Wants arrangement/layout modes with root nodes along top or side axis and connections extending from them. Meaning of roots in Resource incidence needs later discussion; no root policy chosen.
- Explain what report section “Dense overview with source-backed cells” actually shows and how it helps inspection.
- Return to question statements from previous feedback that triggered this work. Review whether tool can now show answers and whether analysis can communicate those answers to agent. This is main assessment, not merely whether new layers run.
- User could not find Resources or MDA in running tool. Navigation/discoverability gap needs joint review.
- All views need to become one tool. User qualifies this request if unification is already planned for a later step; check existing plan during discussion.
- Add anti-aliasing if not already present. Zoomed-out lines become a mess; user points to MDA explorer at default zoom as concrete example. SDF-based lines are a possible approach suggested by user, not a selected implementation. Current anti-aliasing state remains unverified.

- Add optional selection-focused layout mode for connected graph views. Pull selected set into alignment; push non-selected nodes away from selected connected set according to graph distance or disconnection. Move surrounding clusters as clusters, not only individual nodes. Aim: clear crossings around current inspection without untangling entire graph. User analogy: move tangled Christmas lights aside to extract ornaments caught inside. Exact selection scope, alignment axis, distance/disconnection measure and cluster definition remain unspecified; no algorithm chosen.

- User likes free graph mode described as “with settling off.” Nodes jitter near final positions; consider more drag/damping or force tuning. Preserve positive feedback on mode. Exact toggle state meant by “settling off” needs checking during discussion; no tuning values or mechanism chosen.

- User defers Resource graph review until integrated with other tools. Current Resource UI is too hard to read and behavior differs from other views; it should behave consistently with them. Resource usefulness/interaction review is deferred, not accepted.

## Session facts

- Initial live launch opened plumber Skills explorer. Explore menu currently offers Skills, Challenges and Relationships, not MDA/Resources.
- Agent explained MDA/Resources still require separate CLI launches with their respective collections; unified six-view navigation remains unfinished.
- Separate unseeded MDA and Resource windows now run on same plumber dataset. Original Skills window remains open. No source data or knowledge assumptions changed.

## Planned coverage

- `todo.md`, One explorer, multiple views: unified application, consistent MDA/Resource controls; Resource review deferred until integration/readability.
- Draw directed structures with readable paths and Readable graphs and usable controls: rejected Skills squared routing, node-placement crossing reduction, group backgrounds, AA, jitter, optional selection-focused layout and Resource arrangements. Preserve accepted aspects of other renderers and free graph mode.
- Reusable design questions, coordinated with discovery/gap/actor-state/resource tasks: original-question audit and matching visual/agent-usable analysis, not algorithm badges as substitute answers.
- `todo.md`, Next integrated review checkpoint — current feedback coverage: account for every note at next post-batch review; demonstrate addressed items or explicitly retain outstanding ownership/status. Include dense-overview explanation/usefulness. Existing final gate remains.

## Current implementation checkpoint

- Review 06 first slice brings six existing projections into one window with shared controls/source inspection, independent view state, explicit cross-view Find and current-view reset. Ordinary switching does not follow selection. Find frames all visible source-associated matches, keeps destination filters and reports hidden matches. Reset never discards drafts or changes other views.
- Source: `todo.md`, Review 06; operation guide: `skills/game-design-workbench/native/README.md#unified-explorer`. Legacy commands remain compatible.
- Integration delivery addresses discoverability/control inconsistency for review; it does not accept Resource graph readability/usefulness. MDA full-graph Fit remains tiny; Resource cards still overlap in dense default layout.
- Skills routing/group backgrounds, AA, jitter, selection-focused layout and Resource arrangement modes remain in Readable graphs and usable controls. Existing drawing retained in this slice, including rejected Skills squared routing.
- Dense-overview usefulness and original design-question/agent-output audit remain pending. Reusing existing layers in one shell does not answer those questions by itself.
- Engineering: 87/87 native + CLI regression; Node 24 passed / 4 external integrations skipped; strict C99 compilation of seven changed production units, two test units and test-enabled shell; graph and plain-view builds passed. Both examples pass app-local state/reload checks with zero owned allocations left and 322,213,476-byte measured peak during reload overlap. Original/private example hashes unchanged. Independent static review found cross-snapshot centering, stale-edge, rank-buffer and stdout-log issues; fixes include focused regressions and deferred active Undo repair. Focused static rechecks confirm reported issues resolved; no broad independent execution/acceptance claim.
- Evidence under `skills/game-design-workbench/native/build/`: `unified-explorer-native-final.log`, `unified-explorer-node-final.log`, `unified-explorer-plain.log`, `unified-explorer-checks-final.log`; full state JSON/stderr/captures and source hashes in `explorer-checks-56ef2be86e6242f8a3658585fe2eb0a1/`. Twelve production default captures total, plus layered app-local captures. Strict objects in `unified-explorer-strict/`. Build artifacts remain ignored; check script reproduces checks with prepared SDK.
- Next checkpoint must assess every note, including remaining objections and explicit outstanding ownership. App-local checks/captures are engineering evidence only; no real-pointer or user acceptance claimed.

## Comments

- Preserve uncertainty about dataset quality and potentially escaping labels.
- Do not treat rendering checks, report examples or successful launches as usefulness acceptance.
- Original-question audit and integrated user acceptance remain outstanding.
