<!-- wrap-up:start -->
## Pi handoff

Status: Actor/Effect Free implementation checkpoint saved; user review found unresolved defaults. Not visually accepted. Stop here; no fixes attempted after feedback.

Done:
- Shared Neighborhood seed committed as `549d7bd`; reviewed Skills/Challenges/Relationships Free choices committed as `a99c028`.
- Actor/Effect native/free controls, shared Native/Neighborhood, pause/drag and guarded scene replacement implemented. Intentional stacked variant cards move together; typed identities/evidence stay separate. Geometry changes do not Run/Display or apply pending inputs.
- Recorded Mario feedback: Effects pulls into centre and vibrates against cards; Actors shows root-node grid with no visible connecting edges. User requested checkpoint and later resume, not acceptance.

Next:
- Resume open Step 5 in `todo.md`: investigate Actor/Effect default applied scenes/topology and rendered settling, then refine approach for more visually communicative defaults before coding. No root cause or replacement selected. Do not fabricate edges or silently evaluate staged queries.

Context:
- Attendant tasks not configured; `todo.md` remains task authority.
- CJIT core 136/136; Node 21/21, zero skipped. Each example: 254 IPC +2232 controls +105 configuration +10 feedback; presentation 562 Crate /1953 Mario. CLI, protocol helper and graph builds pass. CPU checks do not prove readability, every-control or real-pointer acceptance. No tests rerun for wrap-up-only doc edits.
- CJIT only until final Step 6. Crossing experiments removed; crossing reduction remains unresolved and must be revisited before final review. Nine broader UX follow-ups and gameplay-loop backlog remain open.
- Preserve native defaults/restoration, applied query and pending inputs, typed selection, contributor context, source drafts/Undo, independent panes and last-good results. User chose to keep stacked variants together; do not silently change this policy.
- Query scene owns inline typed layout. Private spatial projection maps compound stacks to shared solver, without merging typed scene nodes/edges. Native pose retained on scene. Geometry implementation: `native/gd_query_scene_free.h`; lifecycle: `native/gd_explorer_query_view.c`; shared controls: `native/gd_explorer_presentation.h` and `native/gd_explorer_view.c` (all under `skills/game-design-workbench/`).
- Fresh build outputs: `skills/game-design-workbench/native/build/query-free-work{,-ui}`. Evidence: `native/build/control-work/query-free-*`. Review receipts: `query-free-review-{layout,initial_arrangement}.json` and timestamped logs.
- Authorized Mario review viewer PID 15832 opened from `query-free-work-ui/gd-view.exe`; liveness not checked at wrap-up. Discover current viewers before targeting; use exact session/fresh tokens, no unknown mutation retry. Do not rebuild a running executable or DLL in place. Do not close/manipulate windows without authorization; no OS pointer/global-key automation or focus stealing.
- Full Actor/Effect review findings, storage/work bounds and test limits live in linked implementation report. No new dependency, crossing logic or per-frame allocation added.

Files:
- `todo.md`
- `reports/graph-query-research/shared-force-layout-query.md`
- `reports/graph-query-research/shared-force-layout-plan.md`
- `skills/game-design-workbench/native/gd_query_scene_free.h`
- `skills/game-design-workbench/native/gd_explorer_query_view.c`
<!-- wrap-up:end -->
