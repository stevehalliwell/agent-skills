<!-- wrap-up:start -->
## Pi handoff

Status: Product refinement for native Attendant; no rewrite or GUI implementation approved.

Done:
- Created worktree `D:/Repos/agent-skills-attendant-native`, branch `discuss/attendant-native`, based on `7af183b` from `dev`. Session changes are discussion record and this handoff only; no commits.
- Inventoried current runner and CLI behavior; saved findings, agreed boundaries, and growing checklist in `attendant-native-discussion.md`.
- Agreed C99 + CJIT preference, Node removal, external migration workflows, Git awareness without Git invocation, and visible automatic repair for copied records. Possible viewer is human/agent visual collaboration layer, not primarily a data editor.

Next:
- Refine one concrete visual collaboration loop in discussion record: agent reveals intended edit location, human selects context, CLI returns selection to agent. Define observable behavior without committing to GUI implementation.

Context:
- Attendant tasks not configured; no `.pi/attendant.tables`. Discussion record is source for refinement state.
- Load `task-refinement` for continued shaping and `cli-design` for CLI contract. No implementation authority from wrap-up.
- Viewer value: shared attention in both directions. Selection/highlighting is not implicit approval. Addressing, freshness, communication, and session ownership remain open.
- Keep automatic source repair; expose repair details to agents and warnings to humans. Existing duplicate age rule is `created_at`, then path ordering; copied timestamps make newer-file detection ambiguous.
- Native CLI excludes `migrate check/apply`; human/agent own imports, schema transformations, Git diff, and recovery. No standalone binary requirement; skill flags unavailable C99 compilation path.
- Checks run: CLI `--help`; `node --test skills/attendant/test/attendant.test.js` passed all seven tests on Node.js v24.16.0. Temporary fixtures confirmed source correction during schema inspection, limit-zero behavior, empty column metadata, denied SQL writes, non-strict diagnostic exit status, and differing cached diagnostic shapes. Fixtures removed.
- Native compilation, performance benchmarks, and GUI checks not run; no native implementation exists. Other C99 + CJIT tool location not confirmed or inspected.
- Use this worktree explicitly; original harness working directory was `D:/Repos/agent-skills`. Original worktree's unrelated files remain untouched.

Files:
- `attendant-native-discussion.md`
- `skills/attendant/scripts/attendant.mjs`
- `skills/attendant/scripts/yaml.mjs`
- `skills/attendant/test/attendant.test.js`
- `skills/attendant/references/commands.md`
<!-- wrap-up:end -->
