---
name: herdr-parallel-work
description: "Use for coordinated parallel implementation of one task through Herdr, with a human-reviewed common base, worktree-isolated workers, and explicit integration ownership. Skip fire-and-forget asides, independent reviews or research jobs, unrelated background jobs, concurrent shell commands, and sequential work; use delegate-tasks for self-contained asides."
---

# Herdr Parallel Work

Owner prepares base and handles user review; orchestrator launches workers and integrates results; owner closes created panes and removes integrated worktrees.

## Trigger clarification

Split only independent implementation portions of one task that need a shared checkpoint and coordinated integration. Resolve shared interfaces and prerequisites first; do not spawn agents to discover a split. Use `delegate-tasks` for self-contained asides whose results can be collected later, even inside Herdr; those do not need this workflow's base review or dedicated orchestrator.

“Build API and UI simultaneously” starts base review, not worker launch. “Run tests while server starts” does not invoke this skill.

## Required read

All agents load `herdr` before Herdr operations; follow its environment gate, CLI discovery, targeting, delivery, waits, and safety rules. Outside Herdr, report blocker; do not control another session. This workflow authorizes new orchestrator/worker panes and per-worker worktrees, not guessed CLI syntax.

Load `coding` before code changes and applicable delivery skills for assigned work.

## Workflow

### 1. Prepare base — owner

- Inspect project guidance and repository state, including branch, staged changes, and uncommitted work.
- Establish and validate shared code, interfaces, dependencies, fixtures, acceptance checks, and prerequisites.
- Define worker goals, inputs, owned paths, exclusions, checks, and deliverables. Isolate shared ports, DBs, caches, and services or serialize access. Specify integration target and order.
- Keep unrelated changes out of base commit; never silently stash, reset, or include them. Supply required inputs through approved base or explicit read-only inputs.
- Without Git, ask for approved isolation approach; do not initialize repository automatically.

*Done: validated base and independent worker contracts ready for review.*

### 2. Review and checkpoint — owner

Present base changes, checks, split, write scopes, integration target, and commit scope. Ask structured approval covering base/result commits, required local integration, closure of created panes, and removal of clean, fully integrated worktrees. Honor any separate commit-approval policy. Default: stay in original pane until approved. Do not push or publish.

Revise and recheck requested changes, then repeat review. Commit only reviewed paths with explicit staging, not `git add .`; record SHA. If base already matches a commit, use its SHA without an empty commit. Commit failure blocks launch.

*Done: human approval and reproducible Git checkpoint recorded.*

### 3. Hand off — owner

Create unfocused pane and start named orchestrator. Record live owner/orchestrator IDs. Send complete contract: base SHA, repository, integration target, worker briefs, approval boundaries, owner identity, and shared-input rules below.

Keep project-local run record outside worker write scopes: pane IDs, agent names, worktrees, branches, status, blockers, results, and checks. Orchestrator maintains it; owner uses it for recovery and cleanup.

Owner retains user communication and final closure. Do not duplicate orchestration or edit integration target while orchestrator owns it.

*Done: orchestrator accepts contract; every created resource is trackable.*

### 4. Launch workers — orchestrator

Load this skill and `herdr`. For each portion, create fresh branch/worktree from exact base SHA, unfocused pane with that cwd, and uniquely named worker. Never reuse unrelated dirty worktrees.

Send complete brief, base SHA, worktree/branch, orchestrator identity, reporting route, commit authority, and shared-input rules. Require `herdr` and applicable coding skills. No further agents or cross-branch merges without explicit authority.

Launch all independent workers before waiting.

*Done: every worker running in recorded isolated worktree/pane.*

### 5. Coordinate — orchestrator

Use explicit targets and bounded waits. `idle`/`done` is not accepted completion. Inspect stalled, timed-out, or blocked agents before intervening; never blindly resend prompts.

Workers send progress, findings, questions, and blockers through targeted Herdr messages only when recipient is ready. Otherwise write designated per-worker run-record location; never inject prompts into active turns.

Answer from approved contract and evidence. Send material scope, interface, security, or approval changes to human through owner, or identify exact pane/agent and needed help. Do not switch user focus or answer approval dialogs without user authority.

*Done: all results delivered or unresolved blockers reported.*

### 6. Gather and integrate — orchestrator

Collect summaries, changed paths, result commits, checks, and remaining issues; inspect actual diffs. Workers commit only assigned changes under approved authority. Required outputs must be durable, not solely scrollback or uncommitted work.

For integration, verify target unchanged; stop on unrelated dirty changes. Merge/cherry-pick in planned order, resolve conflicts within scope, and run combined acceptance checks. Never silently drop changes; semantic conflicts changing agreed behavior need human decision.

Without integration, report deliverable locations. Preserve unfinished work and report partial failures. Send owner consolidated outcomes, locations/commits, integration status, checks, issues, and pane inventory. Mark ready for closure only when outputs are durable and no further interaction is needed.

*Done: results gathered, required integration verified, owner has summary and inventory.*

### 7. Close panes and integrated worktrees — owner only

Verify outputs and inventory; confirm agents are no longer writing or awaiting essential help. Close recorded worker panes, then orchestrator pane. Never close original/unrelated panes or whole sessions; children must not close themselves or siblings.

After closing panes, remove each workflow-created worker worktree only when all its result changes are integrated into target, combined acceptance checks pass, and `git status --porcelain --untracked-files=all` in that worktree is empty. Verify integration by commit ancestry for merges or patch/diff comparison for cherry-picks; worker completion alone is not proof. Check for ignored files holding needed outputs or local data before removal.

Run `git worktree remove <recorded-path>` from outside worktree; never use `--force` or delete directories manually. Verify removal with `git worktree list` and update run record. Preserve branches and run record. Preserve worktrees for unintegrated results, failed checks, dirty state, needed local data, or removal failures; report exact cleanup blockers. Failure/cancellation does not authorize data deletion or forced termination of active agents.

Report results, integration, checks, unresolved issues, and cleanup status; resume original task.

*Done: created panes closed and clean, fully integrated worktrees removed, or cleanup blockers recorded; user has consolidated result.*

## Shared-input rules — include in every child contract

- No child may control user's OS-level keyboard, mouse, desktop focus, or global input. No GUI automation assuming exclusive focus.
- Use `agent-browser` as needed; load its skill and use a distinct named browser session per worker. Visible (`--headed`) and headless modes are both allowed. Browser-scoped input does not move user's mouse or send OS-level keystrokes; automation must work while browser is defocused. Do not bring browser to foreground or require exclusive desktop focus. Keep browser skill's trust, secret, and confirmation rules.
- Targeted Herdr input is allowed only for workflow-owned targets, never OS-level input or manipulation of user's focused pane.
- Preserve focus with `--no-focus`; use explicit IDs or unique agent names, never implicit focused targets.
- If OS-level desktop interaction is needed, pause that portion and request human help in named pane through orchestrator. This restriction does not block `agent-browser`.
