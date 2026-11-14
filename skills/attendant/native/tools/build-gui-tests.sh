#!/usr/bin/env bash
# Build shared GUI integration runner; does not launch windows or acquire tools.
# Usage: bash tools/build-gui-tests.sh [cjit|zig]. Stdout: executable absolute path.
set -euo pipefail
cd "$(dirname "$0")/.."
compiler=${1:-cjit}
if [[ $# -gt 1 || ( "$compiler" != cjit && "$compiler" != zig ) ]]; then
  echo 'Usage: tools/build-gui-tests.sh [cjit|zig]' >&2; exit 2
fi
mkdir -p build/gui-tests
run=$(mktemp -d build/gui-tests/run-XXXXXX)
source tools/build-inputs.sh
source tools/build-platform.sh
compile() {
  local output=$1; shift
  if [[ "$compiler" == cjit ]]; then
    "${CJIT:-cjit}" -q -C '' ${cjit_platform_flags[@]+"${cjit_platform_flags[@]}"} "${flags[@]:1}" -Isrc/include "$@" -o "$output" ${native_libraries[@]+"${native_libraries[@]}"} || return $?
    if [[ $(uname -s) == Darwin ]]; then codesign -s - "$output" || return $?; fi
  else
    "${ZIG:-zig}" cc -O3 ${zig_platform_flags[@]+"${zig_platform_flags[@]}"} "${flags[@]}" -Isrc/include "$@" -o "$output" ${native_libraries[@]+"${native_libraries[@]}"} || return $?
  fi
}
if ! compile "$run/generator$executable_suffix" tests/gui/generate-integration.c > "$run/generator-build.log" 2>&1 ||
   ! "$run/generator$executable_suffix" > "$run/runner.c" 2> "$run/generator.log" ||
   ! compile "$run/runner$executable_suffix" -DNDEBUG -ULIBPDJSON5_STACK_MAX -DLIBPDJSON5_STACK_MAX=16 "$run/runner.c" "${sources[@]:1}" > "$run/runner-build.log" 2>&1 ||
   ! compile "$run/validator$executable_suffix" -DNDEBUG -ULIBPDJSON5_STACK_MAX -DLIBPDJSON5_STACK_MAX=16 tests/gui/validation-check.c "${sources[@]:1}" > "$run/validator-build.log" 2>&1; then
  echo "GUI test generation/build failed: $run/{generator,runner,validator}-build.log" >&2; exit 2
fi
mkdir "$run/validation"
if ! (cd "$run/validation" && "../validator$executable_suffix" > stdout.log 2> diagnostics.log); then
  echo "GUI validator checks failed: $run/validation/diagnostics.log" >&2; exit 1
fi
if [[ $(uname -s) == Darwin ]]; then printf '%s/%s\n' "$PWD" "$run/runner$executable_suffix";
else cygpath -m "$PWD/$run/runner$executable_suffix"; fi
