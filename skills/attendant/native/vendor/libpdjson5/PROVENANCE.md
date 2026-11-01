# libpdjson5 provenance

- Upstream: <https://github.com/boris-kolpackov/libpdjson5>.
- Version: 1.0.1; commit `ed602821f2929930e5179cd9240d468abf210d60`.
- Imported: `libpdjson5/pdjson5.c`, `libpdjson5/pdjson5.h`, root `UNLICENSE`.
- Source/header flattened into this directory; bytes unchanged.
- License: Unlicense; see `UNLICENSE`.
- Excluded: build system, examples, tests, binaries, archives.

Pinned files retrieved from upstream raw GitHub URLs. SHA-256:

```text
a5972bb41f8591a728a417d2f599a9faf7ef19e6dbacf0a214f15809db46346c  pdjson5.c
90e419c4588ae57eea4e5ef857adc84995f1b1ffe9573abb338f4c6c66a5d1c5  pdjson5.h
7e12e5df4bae12cb21581ba157ced20e1986a0508dd10d0e8a4ab9a4cf94e85c  UNLICENSE
```

`include/json.h` selects strict JSON, disables streaming, bounds depth,
and installs caller-backed arena allocator before parsing. Upstream default allocator
uses heap; vendoring does not authorize that path. Raw number events support
exact native conversion without passing integers through floating point.

Production flat binding permits two containers (root object and f64 scalar tag),
using zero-based `LIBPDJSON5_STACK_MAX=1` in `deps/json_backend.c` and standalone
checks. Vendor compiles separately into prepared C exports; binding compiles
with application. Normal dependency tests exercise exact
depth/one-over and prepared backing. Framework command route uses this binding.
Spike evidence:
[`docs/json-library-spike-results.md`](../../docs/json-library-spike-results.md).
