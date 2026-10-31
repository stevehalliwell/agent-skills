# Human inspection UX

User authorized implementation after full automated validation and attempted visual review. Most new functions were not discoverable/useful to human; CLI semantic correctness is not UI or design-usefulness acceptance.

## Contract

- Loaded-data selection never requires typing/copying internal identities. Add shared dropdown with fuzzy search at top; readable labels retain exact qualified identity/source where needed. Search is UI filtering, not semantic analysis scope.
- Graph selection supplies explicit context actions for supported fields (from/to/avoid/start/comparison entry, case/object/axis as applicable). Do not silently retarget query when selection changes. Unavailable/ambiguous mappings explain why; never pick first same-noun occurrence.
- Select structural identities, roots, relation kinds and statuses from native loaded context. Select MDS features/entries from native catalogue; weights remain numeric controls. Multi-selection supports visible remove/clear and discloses native caps.
- Actor/case/participant/axis/value and rule/binding selectors use same searchable interaction. Known enum values and context operators use pickers. Numeric samples/literals retain typed numeric entry; arbitrary authored prose remains free text. Keep raw identity/assumption/list syntax only under explicit Advanced controls; human route must not require it.
- Preserve native evaluators, CLI contracts, typed scope, local identities, unknowns, budgets, source navigation, pending/applied distinction, independent panes, drafts/Undo and atomic last-good reload. No source records, gameplay facts or resource pools added.

## Actor initial state

- Actors opens with declared object/actor inventory, not blank state-query canvas. Show loaded noun definitions, including objects without known axes, with source-backed labels and counts. Do not present case-local binding count as runtime actor count.
- Selecting object reveals its declared axes and exact case/local-binding occurrences. State analysis is explicit drill-down through selected local occurrence and typed axis; browsing alone never evaluates applicability or merges local bindings.
- Keep clear return to inventory, state analysis and cross-binding coverage. Real empty data, no matches and unconfigured analysis have distinct messages. Default inventory needs no root/axis/assumption entry.

## Challenge readability

- Cause: `gd_route_layout()` retained orthogonal routing while Skills enabled `direct_routes`.
- Use existing direct inter-band and rounded same-band/loop routing for Challenges. Preserve authored `next` edges, direction, branches, cycles and shortest-entry-hop display bands. No inferred teaching/action order or geometry changes to MDA/Resources.
- Picking, highlights, arrows and framing use same route geometry as drawing. Tests cover both orientations, branches and cycles.

## Checks

- Current constraint: no further GUI launches, window-based tests, restarts or focus changes. User objected to automated windows stealing focus. Remaining verification is headless/compile-only; existing app/draft stays untouched. Earlier owned-window results below are historical evidence, not authorization to open more windows.
- Fuzzy picker empty/prefix/subsequence/no-match; exact identities remain distinct, caps fail without partial adoption, search changes no query scope.
- Both examples: Actors nonempty immediately when definitions exist; object inspection/drill-down/return; empty/no-axis/unknown cases; default/reset/reload/Find and failed candidate allocation.
- Supported graph-context actions populate pending controls only; visible applied results remain until successful run. Source roles/occurrences must map exactly.
- Earlier owned-window checks rendered pickers, inventory and Challenge routes without OS input automation; they still stole focus. Do not rerun them under current constraint. Native/CLI regressions preserve source hashes. Build to alternate output; never replace running review app/draft.
- Human checks: complete ordinary selection and state/MDS/structural inspection without internal-ID entry; follow Challenge action route at branches. Automated tests do not establish comprehension.

## Implemented result and evidence

- Actors opens with all loaded noun definitions; case-local counts remain separate from runtime identity. Browse object/graph without deleting pending inputs. Explicit case/local occurrence and typed-axis drill-down; return to inventory; typed state/context builders and numeric samples. Exact newline mode preserves semicolons/value bytes; Advanced retains legacy shorthand.
- Shared ASCII-case-insensitive subsequence search covers loaded structural endpoints/starts/kinds/statuses, MDS feature catalogue/entries/weights, Actor/Effect and rule/binding selectors. Graph actions populate pending controls only. Native IDs remain exact; layout indices never substitute for keys. Scoped Relationships results do not restrict subsequent root choices.
- Challenges uses direct inter-band and rounded same-band/loop routing. Existing native routes, branch/cycle/self-loop direction and hit geometry retained. Resource/MDA geometry unchanged.
- Fresh CJIT native **128/128**, CLI, plain/graph builds/checks pass. Node **59 tests: 58 passed, 0 failed, 1 skipped** desktop-authored draft-report integration. Logs: `native/build/human-ux-full.log`, `human-ux-node.log` beneath skill.
- Before focus prohibition, both-example owned-window suites passed: immediate inventory, fuzzy popup, fresh unrun selector context, noncontiguous keys, independent Relationships roots, all nine MDS scopes, rule/binding modes and atomic reload/reset. Zero counted allocations remain; largest observed fixture peak **573,963,310 bytes**, not a universal bound. Original/private examples and native sources unchanged during suite. Evidence: `native/build/explorer-checks-6aa8f82e70cc4fb3af4fd2d05640ff19/`; summary `human-ux-explorer.log`.
- Independent reviewer reproduced four defects, then closed them with **54 private headless checks**, zero semantic bug observations/leaks: bad fresh-context operation name, layout-index/native-key confusion, narrowed root catalogue, browse deleting pending inputs. Findings fixed before final checks. Private evidence: `native/build/human-ux-review/`.
- After focus prohibition: headless-only CJIT suite rerun **128/128 plus CLI**; no windows launched. Zig C99 compile-only checks pass for four changed UI units, with existing vendor typedef warnings. Logs: `native/build/human-ux-headless-final.log`, `human-ux-zig-ui-compile.log`. No alternate-compiler UI runtime claim.
- New review executable: `skills/game-design-workbench/native/build/human-ux/gd-view.exe`. Built separately; not launched for human. No acceptance, real-pointer usability or design-usefulness claim. User authorized local commit after implementation report. No push; human usability acceptance remains open.

## Comments

- Zig `-fsyntax-only` hit compiler cache rename failure; explicit object compilation with project-local cache succeeded. No vendor changes.
- Current plumber data verified: 366 case-local participant bindings across 164 cases; default blank Actors was UI problem, not absence of definitions.
- Human review app remains open at prior tested build (PID 52204); do not close/replace it or discard draft. New build uses separate directory.
- Resource layout and IPC remain separate deferred work.
