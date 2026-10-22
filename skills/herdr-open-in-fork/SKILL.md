---
name: herdr-open-in-fork
description: "In Herdr, open or focus a local Git repository in Fork when asked to open this repo in Fork, bring up Fork, or show this worktree in the Git client. Resolve the requested repo and send its root to Fork. Skip requests to fork a repository, inspect Git state yourself, ordinary repo links, and non-Herdr sessions."
---

# Herdr Open in Fork

1. Check `HERDR_ENV=1`. Outside Herdr, skip automatic use; on explicit invocation, explain scope and stop.
2. Resolve requested repository from conversation; default to current project when user says “this repo” or “bring up Fork.” Use current project context, not another pane's working directory. Ask only when target is ambiguous.
3. Run `git -C "<target-directory>" rev-parse --show-toplevel` and check success. For a file target, use its parent directory. Preserve returned worktree root; do not substitute main checkout or Git common directory. If path is missing or not inside a working tree, report blocker and stop. Do not clone or initialize a repository.
4. Launch Fork with that absolute root as one quoted argument. Use native absolute paths such as `D:/Repos/project` on Windows.

   Windows PowerShell, default installation:

   ```powershell
   Start-Process -FilePath "$env:LOCALAPPDATA\Fork\Fork.exe" -ArgumentList '"D:/Repos/project"' -ErrorAction Stop
   ```

   If default executable is missing, use an existing Fork executable found on `PATH` or a known custom installation. Do not install tools or change settings.

   macOS:

   ```sh
   open -a Fork "/absolute/path/to/project"
   ```

   Let Fork handle existing repository tabs and window activation. Do not close windows, force duplicate instances, or wait for GUI process to exit. No Herdr pane control or UI automation needed.
5. Check launch errors or command exit status. On success, report repository sent to Fork; do not claim visual confirmation, selected tab, or foreground focus without observing it. If launch fails or platform is unsupported, report blocker. Return to prior task.

Opening repository grants no permission to stage, commit, switch branches, fetch, pull, push, or otherwise change Git state.

Examples:
- “Open this repo in Fork.” Send current project’s Git root to Fork.
- “Show this worktree in my Git client.” Send worktree root, not main checkout.
- “Fork this repo.” Do not launch Fork; this means repository creation, not opening Git client.
- “Check the diff.” Inspect diff yourself; do not launch Fork.
