#!/usr/bin/env bash
# Current-policy experimental comparisons; normal native executables untouched.
# Usage: bash sqlite-adoption-build.sh <wal-off|full|full-wal-off>
set -euo pipefail
root=$(cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$root"
if [[ $# -ne 1 ]]; then echo 'Usage: sqlite-adoption-build.sh <wal-off|full|full-wal-off>' >&2; exit 2; fi
source ./tools/build-inputs.sh
sources[0]=$(node experiments/sqlite/sqlite-adoption-source.mjs "$1")
export ZIG_GLOBAL_CACHE_DIR=${ZIG_GLOBAL_CACHE_DIR:-"$root/build/zig-global-cache"}
export ZIG_LOCAL_CACHE_DIR=${ZIG_LOCAL_CACHE_DIR:-"$root/build/zig-local-cache"}
"${ZIG:-zig}" cc -O3 -I"$root" "${flags[@]}" "${sources[@]}" -lbcrypt -lshell32 -o "build/projection-zig-adoption-$1.exe"
printf '%s\n' "$root/build/projection-zig-adoption-$1.exe"
