#!/usr/bin/env bash
# Build the projection-only exploration. Usage: ./build.sh [cjit|zig]
# Requires installed compiler; no downloads or dependency installation.
set -euo pipefail
root=$(cd -- "$(dirname -- "$0")" && pwd)
cd "$root"
compiler=${1:-cjit}
if [[ $# -gt 1 || ( "$compiler" != cjit && "$compiler" != zig ) ]]; then
    echo 'Usage: build.sh [cjit|zig]' >&2
    exit 2
fi
mkdir -p build
source ./tools/build-inputs.sh
if [[ "$compiler" == cjit ]]; then
    # CJIT is not GCC-compatible; omit -std and pass an empty compiler flag set.
    "${CJIT:-cjit}" -q -C '' "${flags[@]:1}" -o build/projection-cjit.exe "${sources[@]}" -l bcrypt -l shell32
else
    # Isolate the observed broken global cache without touching the user's cache.
    export ZIG_GLOBAL_CACHE_DIR=${ZIG_GLOBAL_CACHE_DIR:-"$root/build/zig-global-cache"}
    export ZIG_LOCAL_CACHE_DIR=${ZIG_LOCAL_CACHE_DIR:-"$root/build/zig-local-cache"}
    "${ZIG:-zig}" cc -O3 "${flags[@]}" "${sources[@]}" -lbcrypt -lshell32 -o build/projection-zig.exe
fi
printf '%s\n' "$root/build/projection-$compiler.exe"
