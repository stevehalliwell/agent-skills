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
source tools/build-platform.sh
sources[0]=tests/core/api-probe.c
output="build/api-probe-$compiler$executable_suffix"
if [[ "$compiler" == cjit ]]; then
  "${CJIT:-cjit}" -q -C '' ${cjit_platform_flags[@]+"${cjit_platform_flags[@]}"} -DATTENDANT_TEST_FAULTS "${flags[@]:1}" -o "$output" "${sources[@]}" ${native_libraries[@]+"${native_libraries[@]}"}
  if [[ $(uname -s) == Darwin ]]; then codesign -s - "$output"; fi
else
  export ZIG_GLOBAL_CACHE_DIR=${ZIG_GLOBAL_CACHE_DIR:-"$PWD/build/zig-global-cache"}
  "${ZIG:-zig}" cc -O3 ${zig_platform_flags[@]+"${zig_platform_flags[@]}"} -DATTENDANT_TEST_FAULTS "${flags[@]}" "${sources[@]}" ${native_libraries[@]+"${native_libraries[@]}"} -o "$output"
fi
