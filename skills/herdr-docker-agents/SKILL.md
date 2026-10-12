---
name: herdr-docker-agents
description: "Use for parallel Herdr child panes only when assigned work needs full control of an isolated machine environment or may perform dangerous or damaging filesystem operations. Run those children in private Docker containers, attached to the existing Herdr session. Skip routine parallel coding, reviews, research, ordinary delegation, and generic Docker or credential setup unless maintaining this workflow explicitly."
compatibility: "Docker Linux engine; Herdr-managed parent pane; Python 3 for credential handoff. Tested with Windows Herdr 0.9.3 and Pi 1.1.0."
---

# Herdr Docker agents

Run each child Pi inside private Linux container. Attach container terminal to sibling pane in parent's existing Herdr session. No guest Herdr server or SSH required.

## Trigger clarification

Invoke only when parallel child task requires isolation for full-box control or risky filesystem actions, or user explicitly asks to maintain this workflow. Ordinary parallel work uses Herdr/delegation skills instead. Do not infer danger merely because task writes code.

Start with requested child count, task, and destructive boundary. For connection-only proof, use one child. Keep each task self-contained; child does not inherit parent conversation, files, skills, or extensions.

This workflow provides container isolation, not separate microVMs. Docker Desktop Linux containers share its Linux VM kernel. Image has no GUI desktop or computer-use tools.

## Required read

- Load [Herdr](../herdr/SKILL.md) before controlling panes. Require `HERDR_ENV=1`; otherwise stop session-control work.
- Load [Docker baseline](../docker-local/SKILL.md) before container actions. Follow image-download approval and ownership rules.
- Read [Image maintenance](references/image.md) when building, changing dependencies, or updating Pi.
- Read [Credential handoff](references/credentials.md) before supplying host credentials or handling authentication failure.

Resolve `<skill-dir>` to directory containing this SKILL.md. Execute bundled resources there; do not regenerate or copy helpers into projects. Keep task outputs and resource inventory project-local.

## Workflow

1. **Inspect and bound.** Check Docker engine, inherited Herdr context, installed Herdr status, caller geometry, and cached image. Use requested scope; default to sibling panes, preserve focus. Record planned child names, tasks, resource limits, network needs, and credential provider. Do not inspect credential values or dump environment variables.
   *Done: Linux engine reachable; session confirmed; launch scope known.*

2. **Prepare image.** Reuse `pi-herdr-test:1.1.0` if compatible. Otherwise follow Image maintenance, obtain required download approval, build, and verify Pi/fd versions and absent auth file. Do not rebuild merely because skill was invoked.
   *Done: verified image identity recorded; no credentials in image.*

3. **Create owned containers.** Use unique names, one per child. Start with command below; replace `CHILD_NAME` and image only with validated launch values. Label identifies disposable test resources; also record exact container IDs. Create all required containers before handing off credentials. No host workspace, home, desktop, Docker socket, or Herdr socket mounts. Choose least guest authority that fits task: default below allows destructive operations in private workspace, not system changes.

   ```text
   docker run --pull=never -d --name CHILD_NAME --label purpose=herdr-pi-proof --cap-drop ALL --security-opt no-new-privileges=true --read-only --tmpfs /tmp:rw,nosuid,nodev,size=64m,uid=1000,gid=1000 --tmpfs /workspace:rw,nosuid,nodev,size=64m,uid=1000,gid=1000 --tmpfs /home/node/.pi/agent:rw,nosuid,nodev,size=32m,uid=1000,gid=1000,mode=0700 --memory 1g --cpus 1 --pids-limit 128 --entrypoint sleep pi-herdr-test:1.1.0 infinity
   ```

   For explicitly required **full-box control**, add `--user 0:0` and omit `--read-only`. Child can change its own container system as root; image layers and host remain separate. Keep capability drop, no-new-privileges, limits, no mounts, and no published ports. Never add `--privileged`, host namespaces, devices, or daemon sockets. This profile is not unrestricted VM/hardware control; report operations Docker blocks rather than weakening host boundary. Use auth helper's explicit `--allow-root` flag for this profile. Root profile is documented, not runtime-tested here.

   PowerShell accepts this one-line command. In Git Bash, set `MSYS_NO_PATHCONV=1` for Docker commands containing Linux paths. Default bridge permits outbound network and may reach host/LAN services; it is not provider-only egress isolation. Change network/mount boundary only with explicit authority. Container root is read-only; tmpfs workspace/auth/session data disappears when container stops.
   *Done: owned containers running with expected user, tmpfs, limits, and no mounts/ports.*

4. **Pass host auth after startup.** Obtain permission to expose selected provider credential to children unless already explicit. Execute bundled helper on host:

   ```text
   python "<skill-dir>/scripts/pass-auth.py" CHILD_A CHILD_B --provider openai
   docker exec CHILD_A pi auth check --provider openai --no-refresh --json
   ```

   Repeat readiness check for each child. Helper sends only selected provider through stdin into private runtime auth.json; no secret arguments, host copies, or image changes. Do not print credentials or use `--credentials`. On failure, follow Credential handoff; never dump auth.json.
   *Done: every child reports authentication ready; host auth remains untouched by handoff.*

5. **Attach sibling panes.** Follow Herdr geometry rule; use `--no-focus` and caller's cwd. Parse each returned pane ID, label it, record pane/container mapping, then submit command with `herdr pane run PANE_ID "..."`:

   ```text
   docker exec -it CHILD_NAME pi --model openai/MODEL_ID --thinking low --session-id CHILD_SESSION --no-extensions --no-skills --no-context-files --no-prompt-templates --no-themes --no-approve
   ```

   Choose available model from child's `pi --list-models`; do not assume host catalogue transferred. Confirm TUI ready before sending task. Use explicit named/allowlisted resources only when task requires them.
   *Done: each child TUI visible in mapped pane; parent focus unchanged.*

6. **Communicate through panes.** On tested Windows path, use raw terminal surface:

   ```text
   herdr pane run PANE_ID "SELF_CONTAINED_TASK"
   herdr pane read PANE_ID --source recent-unwrapped --lines 80
   herdr pane wait-output PANE_ID --regex "^\s*UNIQUE_REPLY_MARKER\s*$" --timeout 60000
   ```

   `pane run` sends text and Enter to foreground Pi, not a host shell. Inspect foreground/readiness first. Prefer a unique standalone reply marker; substring matching can match echoed prompt before child answers. Parent may relay messages between children. Children need no host Herdr API access.

   **Windows limitation:** Docker attachment hides guest Pi from native foreground-process detection. In tested Herdr 0.9.3, manual `pane report-agent` adds identity but `agent prompt` still fails with `agent_not_ready`. Do not fabricate lifecycle state, promise native prompt/wait support, or mount host socket as workaround. Use pane I/O unless a separate integration is implemented and verified.
   *Done: child answers actual task; parent reads response rather than only echoed prompt.*

7. **Verify and finish.** For first setup, test two children: each writes own workspace marker and cannot see other's; relay acknowledgment through parent; verify original pane remains focused. Report image, container IDs, panes, auth method, checks, and native-agent limitations. Default when child work finishes: export wanted non-secret outputs, terminate and remove each recorded owned Docker container with `docker rm -f CONTAINER_NAME`, then close its recorded owned Herdr pane with `herdr pane close PANE_ID`. Verify both removed; preserve original session and unrelated resources. Retain child panes/containers only when user explicitly requests continued interaction or later pickup; report retained resources and ephemeral-data rule. Keep reusable image unless user requests image removal. On partial launch failure, clean this attempt's resources after preserving requested work; never prune unrelated Docker resources.
   *Done: child panes and containers removed by default, or retained at explicit user request; resource disposition reported.*

## Acceptance examples

- “Run two agents in Herdr to test destructive cleanup and disk-formatting scripts.” Invoke; keep filesystem experiments inside private container storage. Disk/device formatting may need VM instead; never expose host devices.
- “Start two agents to review these files.” Skip; ordinary Herdr delegation needs no Docker workflow.
- “Add fd to child image.” Edit bundled Dockerfile, rebuild and verify fd; leave existing containers unchanged unless replacement requested.
- “Give each agent a private desktop.” Explain image lacks GUI; scope desktop tooling separately, not pretend terminal isolation proves computer-use isolation.
