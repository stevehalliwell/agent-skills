# Runtime credential handoff

Host credentials can be supplied after container creation. Never require secret values in prompt, command argument, image, build context, repo, or diagnostic output.

## Execute helper

```text
python "<skill-dir>/scripts/pass-auth.py" CHILD_A CHILD_B --provider openai
```

- Default source: host `~/.pi/agent/auth.json`. Use `--source PATH` for different host agent directory; path is not secret value.
- Default provider: `openai`. Only selected provider entry is sent, not other providers, settings, extensions, or sessions.
- Helper validates every target before reading credentials: running, label `purpose=herdr-pi-proof`, private agent-dir tmpfs, node/1000:1000 user. Labels identify workflow resources, not cryptographic authorization; use recorded container identities on trusted Docker host.
- Explicit full-box profile uses container user `0:0`; helper requires `--allow-root`. Do not enable merely to bypass unexpected user mismatch.
- Supports OAuth and literal API keys. Rejects command-backed keys; does not execute host secret commands.
- Credential JSON passes in memory through `docker exec -i` stdin to Node writer. Writer creates mode-0600 runtime auth.json using temporary file then rename. Host auth file untouched; no host temporary credential copy.
- Success: stdout JSON with status `provisioned`, provider, container names, runtime storage, and `host_auth_modified: false`. Exit 0. Failure: JSON error, exit 1; no secret output. If batch fails after earlier handoffs, inspect all owned targets; do not assume none received credentials.

## Check readiness without revealing secrets

```text
docker exec CHILD_A pi auth check --provider openai --no-refresh --json
```

Require `status: ready`. Never add `--credentials` or run credential-printing commands into agent-visible logs. Do not read runtime auth.json back into model context.

## OAuth expiry and safety

Helper requires at least 5 minutes remaining on OAuth credential. This reduces, but does not eliminate, concurrent refresh risk. Copies include access and refresh tokens and are readable by child processes. This is not credential-proxy isolation or a scoped provider token.

For expired host OAuth, explain failure. Refresh through normal host Pi authentication flow, then retry handoff only when host auth refresh is authorized. Never ask user to paste token into chat. If already provisioned children need replacement snapshot, stop their model activity before handoff; writer does not coordinate Pi's refresh lock. Do not repeatedly refresh copied tokens across parallel children as durable design.

Credential copy disappears when container stops because agent dir is tmpfs. Image remains credential-free. Task output, transcripts, and exported files can still leak secrets if child reads them; treat all exports as untrusted and review before retaining.

Network bridge permits model requests but may also reach host/LAN services. Do not describe container as incapable of exfiltration when both credentials and outbound network are available.

## Test helper

```text
python "<skill-dir>/scripts/test-pass-auth.py"
```

Tests use fake credentials only. Cover provider-only stdin transfer with unchanged host source, unowned targets, expired OAuth, and explicit root permission. Missing target must fail without auth read or browser prompt. Credentials remain excluded from argv and success output.
