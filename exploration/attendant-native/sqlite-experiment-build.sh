#!/usr/bin/env bash
# Compile isolated disposable-cache experiments. Never overwrite normal binaries.
# Usage: bash sqlite-experiment-build.sh <profile>; generator --help lists profiles.
set -euo pipefail
root=$(cd -- "$(dirname -- "$0")" && pwd)
cd "$root"
if [[ $# -ne 1 ]]; then echo 'Usage: sqlite-experiment-build.sh <profile>' >&2; exit 2; fi
source ./build-inputs.sh
# Node generates experimental sources outside measured CLI runtime.
source_file=$(node sqlite-experiment-source.mjs "$1")
sources[0]="$source_file"
export ZIG_GLOBAL_CACHE_DIR=${ZIG_GLOBAL_CACHE_DIR:-"$root/build/zig-global-cache"}
export ZIG_LOCAL_CACHE_DIR=${ZIG_LOCAL_CACHE_DIR:-"$root/build/zig-local-cache"}
"${ZIG:-zig}" cc -O3 -I"$root" "${flags[@]}" "${sources[@]}" -lbcrypt -lshell32 -o "build/projection-zig-experiment-$1.exe"
printf '%s\n' "$root/build/projection-zig-experiment-$1.exe"
