#!/usr/bin/env bash
# Windows GUI, separate from CLI. Requires explicitly prepared/built GUI spike DLL.
# Usage: bash build-gui.sh [cjit|zig] [--check]
set -euo pipefail
cd "$(dirname "$0")"
compiler=${1:-cjit}
if [[ $# -gt 2 || ( "$compiler" != cjit && "$compiler" != zig ) || ( $# == 2 && "$2" != --check ) ]]; then
  echo 'Usage: build-gui.sh [cjit|zig] [--check]' >&2; exit 2
fi
stack=../../spikes/cimgui-richmd-c99
if [[ ! -f "$stack/build/bin/cimgui_richmd.dll" ]]; then
  echo 'Missing GUI DLL. Explicitly run spike prepare.mjs and build.sh first; GUI build never downloads.' >&2; exit 1
fi
variant=${ATTENDANT_GUI_VARIANT:-}
if [[ -n "$variant" && ! "$variant" =~ ^[a-z0-9][a-z0-9-]*$ ]]; then
  echo 'ATTENDANT_GUI_VARIANT must contain lowercase letters, digits or hyphens, starting with a letter or digit.' >&2; exit 2
fi
output="build/gui${variant:+-$variant}"
mkdir -p "$output"
source tools/build-inputs.sh
sources=(apps/gui/main.c apps/gui/workspace.c apps/gui/reveal.c apps/gui/inspect.c src/native-api.c "${sources[@]:1}")
includes=(-I"$stack/build/deps/cimgui" -I"$stack")
suffix=""
if [[ "${2:-}" == --check ]]; then includes+=(-DATTENDANT_GUI_TESTING); suffix=-check; fi
if [[ "$compiler" == cjit ]]; then
  "${CJIT:-cjit}" -q -C '' "${flags[@]:1}" "${includes[@]}" -o "$output/attendant-gui-cjit${suffix}.exe" "${sources[@]}" \
    -L"$stack/build/bin" -lcimgui_richmd -lbcrypt -lshell32 -luser32
else
  export ZIG_GLOBAL_CACHE_DIR=${ZIG_GLOBAL_CACHE_DIR:-"$PWD/build/zig-global-cache"}
  "${ZIG:-zig}" cc -O3 "${flags[@]}" "${includes[@]}" "${sources[@]}" \
    -L"$stack/build/bin" -lcimgui_richmd -lbcrypt -lshell32 -luser32 -o "$output/attendant-gui-zig${suffix}.exe"
fi
if [[ "${2:-}" == --check ]]; then
  sources[0]=tests/gui/model-check.c
  if [[ "$compiler" == cjit ]]; then
    "${CJIT:-cjit}" -q -C '' "${flags[@]:1}" -DATTENDANT_MODEL_TESTING -DATTENDANT_TEST_FAULTS -o "$output/model-check-cjit.exe" "${sources[@]}" -lbcrypt -lshell32 -luser32
  else
    "${ZIG:-zig}" cc -O3 "${flags[@]}" -DATTENDANT_MODEL_TESTING -DATTENDANT_TEST_FAULTS "${sources[@]}" -lbcrypt -lshell32 -luser32 -o "$output/model-check-zig.exe"
  fi
fi
# Keep running viewer's identical mapped DLL untouched.
if ! cmp -s "$stack/build/bin/cimgui_richmd.dll" "$output/cimgui_richmd.dll"; then
  cp "$stack/build/bin/cimgui_richmd.dll" "$output/"
fi
node tools/gui-build-manifest.mjs "$compiler" ${2:+"$2"}
printf '%s\n' "$PWD/$output/attendant-gui-$compiler$suffix.exe"
