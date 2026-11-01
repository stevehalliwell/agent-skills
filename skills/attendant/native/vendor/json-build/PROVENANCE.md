# json-build provenance

- Upstream: <https://github.com/lcsmuller/json-build>.
- Commit: `f4fdbf2e738a2c0dee5aeae3a89ac0e90f71b223`; no release version established.
- Imported: root `json-build.h` and `LICENSE`; license unchanged. Header locally narrowed to fixed-buffer surface.
- License: MIT; see `LICENSE`.
- Excluded: examples, tests, build files, binaries, archives.

Pinned files retrieved from upstream raw GitHub URLs. Original SHA-256:

```text
42a042f047db2293cb1db9749769e5d2952903d1b307606674c45329fd01b154  json-build.h
49481105b2592528e6ca7bf6dc0c1865e01aa6f86fb0a2ec03e26036900e604d  LICENSE
```

## Bounded adoption constraints

- Use fixed-buffer functions, not `_auto` dynamic-buffer functions.
- Physical output capacity includes one trailing NUL byte; wire length excludes it.
- Reject zero capacity before calling upstream: exhaustion path still writes NUL.
- Set `JSONB_MAX_DEPTH` consistently for declarations/implementation.
- `jsonb_token` accepts trusted raw tokens, not validated arbitrary JSON.
- `jsonb_number` does not establish required exact numeric/nonfinite transport;
  native binding must supply checked numeric tokens and raw-bit tags.

## Local changes

Removed `_auto` declarations/documentation/implementations, realloc copy macros,
dynamic escape helper, unused `<stdlib.h>` and corresponding macro cleanup.
Added guard around `JSONB_API` so dependency unit can supply Windows C-export
decoration. Fixed-buffer functions unchanged. No replacement JSON syntax/string
escaping. `deps/json_backend.c` compiles vendor implementation separately;
`include/json.h` binds application-native data against prepared C exports.
Local header SHA-256:

```text
20a95741624dd363caf79361b4e6ec4e4e3e8b4414d535bb4a0ac54d9bd20ffb  json-build.h
```

`include/json.h` uses fixed-buffer surface for native command results/examples.
Normal tests cover escaping, exact capacity, integer tokens and depth; shared
binding checks also cover exact numeric round-trips and atomic decode/overrides.
Spike evidence: [`docs/json-library-spike-results.md`](../../docs/json-library-spike-results.md).
