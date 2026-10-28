#!/usr/bin/env bash
# Build experimental Zig profiles, never overwrite normal native executables.
# Usage: bash sqlite-tuning-build.sh [control|cache|mmap|combined|large|vacuum]
set -euo pipefail
root=$(cd -- "$(dirname -- "$0")" && pwd)
cd "$root"
if [[ $# -ne 1 ]]; then echo 'Usage: sqlite-tuning-build.sh [control|cache|mmap|combined|large|vacuum]' >&2; exit 2; fi
case "$1" in
    control) profile=0 ;; cache) profile=1 ;; mmap) profile=2 ;;
    combined) profile=3 ;; large) profile=4 ;; vacuum) profile=5 ;;
    *) echo "Unknown tuning profile: $1" >&2; exit 2 ;;
esac
mkdir -p build
source ./build-inputs.sh
sources[0]=sqlite-tuning.c
export ZIG_GLOBAL_CACHE_DIR=${ZIG_GLOBAL_CACHE_DIR:-"$root/build/zig-global-cache"}
export ZIG_LOCAL_CACHE_DIR=${ZIG_LOCAL_CACHE_DIR:-"$root/build/zig-local-cache"}
"${ZIG:-zig}" cc -O3 -I"$root" -DSQLITE_TUNING_PROFILE="$profile" "${flags[@]}" "${sources[@]}" -lbcrypt -lshell32 -o "build/projection-zig-tuning-$1.exe"
printf '%s\n' "$root/build/projection-zig-tuning-$1.exe"
