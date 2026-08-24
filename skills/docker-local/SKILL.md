---
name: docker-local
description: "Shared safety and lifecycle baseline for local Docker-backed skills."
disable-model-invocation: true
---

# Local Docker service baseline

Use this baseline for agent skills that start or reuse a local Dockerized service.

## Before work

- Check `docker info` before attempting a container action. If unavailable, report the daemon/client blocker; do not replace the container workflow with host-language package installation.
- Treat an image pull as a material download. State image name, expected size when known, and ask before a first or deliberate updated pull.
- Use the skill's declared lifecycle. A per-run service must get a unique name and always be removed. A per-machine service may use one stable named container and `--restart unless-stopped`; never remove it at the end of a conversion.

## Service boundary

- Publish local APIs only to `127.0.0.1`; do not use host networking or publish all interfaces.
- Keep input and output outside the container unless a bind mount is necessary and explicitly scoped. Do not mount project or home directories by default.
- Pass only service-specific configuration. Do not pass Pi credentials or unrelated host secrets. Keep optional remote/API capabilities disabled unless the user explicitly requests them.
- Use a bounded health check before the service is called. On failure, show a short container-log diagnostic and report the blocker.

## Image and result records

- Prefer a versioned tag or digest. If a skill intentionally uses a moving tag for initial setup, capture the resolved `RepoDigest` after pull and report it; updates require explicit approval.
- Keep generated artifacts in the target project, never in the global skill directory. Report container/image identity, service lifecycle, artifact path, and any cleanup or retained state.
