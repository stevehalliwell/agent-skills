---
name: herdr-open-in-vscode
description: "In Herdr, open local files for the user in VS Code when asked to open that file, show those images, let me read that Markdown, or bring up code for review. Open files in editor tabs with their project context. Skip requests to inspect files yourself, ordinary file links, URLs, and non-Herdr sessions."
---

# Herdr Open in VS Code

1. Check `HERDR_ENV=1`. Outside Herdr, skip automatic use; on explicit invocation, explain scope and stop.
2. Resolve requested files from conversation to existing absolute paths. Use current project context, not another pane's working directory. Ask only when target is ambiguous. Do not pass missing paths: VS Code can create new files instead.
3. Identify owning project root from known workspace context, otherwise `git -C "<file-parent>" rev-parse --show-toplevel`. Preserve worktree root. For non-Git files, use known project folder or containing folder. Use an existing `.code-workspace` only when context identifies it as intended workspace; do not create one. Group files by project.
4. Run one command per project through available shell. Supply explicit root or workspace plus absolute file paths. Quote each argument; use native absolute paths such as `D:/Repos/project/file.md` on Windows.

   ```powershell
   code "D:/Repos/project" "D:/Repos/project/docs/design.md" "D:/Repos/project/images/example.png"
   ```

   For a specific line or column:

   ```powershell
   code "D:/Repos/project" --goto "D:/Repos/project/src/main.ts:42:7"
   ```

   Let VS Code match an existing project window or open one. Do not use `--reuse-window` (last active window may belong to another project), `--new-window` (forces duplicates), `--add` (changes workspace), or `--wait` (blocks agent). Do not inherit terminal-specific VS Code routing: if `VSCODE_IPC_HOOK_CLI` is set, omit it only for this child command.
5. Check command exit status. On success, report files sent to VS Code; do not claim visual confirmation. If CLI is unavailable or fails, report blocker without installing tools or changing settings. Return to prior task.

Files open in editor tabs, including Markdown source and supported image previews. Explorer reveal depends on existing `explorer.autoReveal` setting; do not promise Explorer selection or change settings. No Herdr pane control, extension, or URL handler needed.

Examples:
- “Open that design doc so I can read it.” Resolve recent document and open with its project root.
- “Show me both screenshots.” Open both images, grouped by owning project.
- “Read that file and explain the bug.” Inspect file yourself; do not launch VS Code.
