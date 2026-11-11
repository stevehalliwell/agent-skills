#!/usr/bin/env bash
# Generate/build/run native and framework self-check suites. Usage: bash test.sh [cjit|zig]
# Requires installed compiler/Bash only. No Node, downloads or dependency installs.
# Stdout: retained run-directory path. TSV/results/logs stay below that directory.
set -euo pipefail
root=$(cd -- "$(dirname -- "$0")" && pwd)
cd "$root"
compiler=${1:-cjit}
if [[ $# -gt 1 || ( "$compiler" != cjit && "$compiler" != zig ) ]]; then
  echo 'Usage: test.sh [cjit|zig]' >&2; exit 2
fi
mkdir -p build/native-tests
run=$(mktemp -d build/native-tests/run-XXXXXX)
source tools/build-inputs.sh
source tools/build-platform.sh
compile() {
  local output=$1
  shift
  if [[ "$compiler" == cjit ]]; then
    "${CJIT:-cjit}" -q -C '' ${cjit_platform_flags[@]+"${cjit_platform_flags[@]}"} "${flags[@]:1}" -Isrc/include "$@" -o "$output" ${native_libraries[@]+"${native_libraries[@]}"} || return $?
    if [[ $(uname -s) == Darwin ]]; then codesign -s - "$output" || return $?; fi
  else
    "${ZIG:-zig}" cc -O3 ${zig_platform_flags[@]+"${zig_platform_flags[@]}"} "${flags[@]}" -Isrc/include "$@" -o "$output" ${native_libraries[@]+"${native_libraries[@]}"} || return $?
  fi
}
generator="$run/generator$executable_suffix"
runner="$run/runner$executable_suffix"
if ! compile "$generator" tests/core/generate.c > "$run/generator-build.log" 2>&1; then
  echo "Generator build failed: $run/generator-build.log" >&2; exit 2
fi
if ! "$generator" > "$run/runner.c" 2> "$run/generator.log"; then
  echo "Generator failed: $run/generator.log" >&2; exit 2
fi
if ! compile "$runner" -DATTENDANT_TEST_FAULTS "$run/runner.c" "${sources[@]:1}" > "$run/runner-build.log" 2>&1; then
  echo "Test build failed: $run/runner-build.log" >&2; exit 2
fi
printf 'compiler=%s\ntest_header=' "$compiler" > "$run/metadata.txt"
if [[ $(uname -s) == Darwin ]]; then
  shasum -a 256 src/include/test.h >> "$run/metadata.txt"
else
  sha256sum src/include/test.h >> "$run/metadata.txt"
fi
for zone in UTC Pacific/Auckland; do
  directory="$run/${zone//\//-}"
  mkdir "$directory"
  if TZ="$zone" ATTENDANT_TEST_DIRECTORY="$directory" "$runner" -tsv > "$directory/results.tsv" 2> "$directory/diagnostics.log"; then
    :
  else
    status=$?
    echo "Native tests failed ($compiler, $zone): $directory/results.tsv and diagnostics.log" >&2
    if [[ "$status" -eq 1 ]]; then exit 1; else exit 2; fi
  fi
  awk -F '\t' -v zone="$zone" '$1 == "complete" { printf "%s: %s passed, %s failed\n", zone, $4, $2 }' "$directory/results.tsv" >&2
done
source tests/framework/check.sh
check_test_framework
printf '%s\n' "$root/$run"
