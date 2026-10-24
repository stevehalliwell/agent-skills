# PCG minimal C

- Upstream: https://github.com/imneme/pcg-c-basic
- Pin: `bc39cd76ac3d541e618606bcc6e1e5ba5e5e6aa3`.
- License: Apache-2.0; complete upstream `LICENSE.txt` retained.
- Copyright 2014 Melissa O'Neill. Copyright/license notices remain in source/header.
- Retained source/header/license bytes unchanged. No binaries, build system, demos, or implicit downloads.
- Shared experiment helper calls reentrant seed/draw API only. No global generator state is used. Upstream unbounded rejection function remains unchanged but unused; helper bounds rejection to 128 draws and returns failure on exhaustion.
- Golden vector: https://www.pcg-random.org/using-pcg-c-basic.html (seed state 42, stream 54; first six draws).
- Numerical PRNG only; no cryptographic or player-behavior claim.

SHA-256:

```text
b6582a071a8a090a293621523c063d125a532d772a2a1eb7d60b3e695fe47746  pcg_basic.c
cd823ddc225da9be520a54f13ef2c491506c353800cae04e8b673b2de58f2cc4  pcg_basic.h
b40930bbcf80744c86c46a12bc9da056641d722716c378f5659b9e555ef833e1  LICENSE.txt
```
