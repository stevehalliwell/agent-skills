# Image maintenance

Execute Docker build from bundled image directory. Keep edited Dockerfile in skill so later rebuilds use same source.

## Build and verify

```text
docker build -t pi-herdr-test:1.1.0 "<skill-dir>/image"
docker run --rm --pull=never --network none pi-herdr-test:1.1.0 --version
docker run --rm --pull=never --network none --entrypoint sh pi-herdr-test:1.1.0 -c "fd --version; test ! -e /home/node/.pi/agent/auth.json"
docker image inspect pi-herdr-test:1.1.0
```

In Git Bash, set `MSYS_NO_PATHCONV=1` to preserve Linux paths. Record resulting image ID, Pi/fd versions, and auth-file absence. Existing containers keep old image until recreated; do not stop them merely because build completed.

## Bundled resources

- [Dockerfile](../image/Dockerfile): Node 24 Bookworm Slim base pinned by digest; Pi pinned to 1.1.0; Bash, CA certificates, fd, Git, ripgrep.
- [Build-context filter](../image/.dockerignore): only Dockerfile enters build context. No repo, auth files, or secret build arguments.
- Debian `fd-find` installs `fdfind`. Dockerfile creates `/usr/local/bin/fd` symlink to prevent Pi startup downloading fd per child.
- Image user `node`, UID/GID 1000; workdir `/workspace`; agent dir `/home/node/.pi/agent`.
- No Herdr server, SSH, desktop, credentials, host mounts, or model configuration baked in.

## Adjust dependencies or Pi

1. Inspect requested change; load coding skill before code edits. Edit bundled Dockerfile, not live container as durable source.
2. Pin requested Pi version and compatible Node base. Digest replacement or uncached base pull requires Docker baseline's download approval. Do not silently update moving tags.
3. Rebuild under deliberate image tag; update SKILL.md launch example if tag changes.
4. Verify changed binary as default image user, Pi startup, absent auth file, and focused helper tests. Correct failures and rerun.
5. Replace live containers only when authorized. Stopping them loses tmpfs credentials, conversations, and work; export wanted non-secret outputs first.

Pi version/base digest are pinned; apt packages and npm transitive dependencies are not lockfile-pinned. Build is not fully reproducible.

## Tested boundary

Windows Herdr 0.9.3 and Docker Desktop Linux engine: two Pi 1.1.0 non-root children authenticate through runtime handoff, execute tools in private workspaces, and exchange parent-relayed messages through pane I/O. Rebuilt image supplies fd 8.6.0. No GUI, full-box root profile, native lifecycle relay, or persistent session restore tested.
