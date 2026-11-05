# Local message pipe provenance

Copied unchanged from `c-ink/tools/story-player/{native.h,pipe.inc}` at c-ink revision `27eacf311c8667158fa446f5c07357515c4f5411`.

- Original helpers: proto-framework `include/base.h`, revision `f0b848e2541903ac99696f1f8a71d93e72ef4c5b`.
- c-ink extraction permission: owner explicitly authorized copying/adapting pipe helpers under MIT. User authorized reuse here for Mac port. Retain copied MIT `LICENSE`.
- c-ink adaptations: explicit native C99 headers/types, Linux UID/SIGPIPE policy, invalid Windows UTF-16 rejection, native executable lookup. No framework allocator/collections imported.
- Workbench uses this backend on macOS only. Existing Windows viewer transport remains separate.
- Retain same-user peer checks, private `/tmp/proto-pipes-<uid>/`, framed partial I/O, bounded waits, cancellation and owned-inode cleanup. Never remove stale or foreign endpoints automatically.
- Workbench adapter: `../../gd_viewer_macos.h`. Preserves JSON protocol, PID-plus-nonce session identity, exact targeting and unknown mutation outcomes. No c-ink player protocol imported.

SHA-256:

```text
36c74004c948711ab2b211a6fdfaf3d7b2af04fef036b0e2e84911924b526d43  native.h
0c52bcc0d0958fe1f6584ccd2edb7d7a45d6065ec94f45051c3df653a511f1d0  pipe.inc
```
