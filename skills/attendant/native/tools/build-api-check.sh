#!/usr/bin/env bash
# Headless embedded-source API probe, not installed CLI. No downloads.
set -euo pipefail
cd "$(dirname "$0")/.."
compiler=${1:-cjit}
if [[ $# -gt 1 || ( "$compiler" != cjit && "$compiler" != zig ) ]]; then
  echo 'Usage: tools/build-api-check.sh [cjit|zig]' >&2; exit 2
fi
mkdir -p build
source tools/build-inputs.sh
sources[0]=tests/core/api-probe.c
if [[ "$compiler" == cjit ]]; then
  "${CJIT:-cjit}" -q -C '' -DATTENDANT_TEST_FAULTS "${flags[@]:1}" -o build/api-probe-cjit.exe "${sources[@]}" -lbcrypt -lshell32
else
  export ZIG_GLOBAL_CACHE_DIR=${ZIG_GLOBAL_CACHE_DIR:-"$PWD/build/zig-global-cache"}
  "${ZIG:-zig}" cc -O3 -DATTENDANT_TEST_FAULTS "${flags[@]}" "${sources[@]}" -lbcrypt -lshell32 -o build/api-probe-zig.exe
fi
