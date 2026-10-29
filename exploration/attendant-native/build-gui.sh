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
mkdir -p build/gui
source build-inputs.sh
sources=(gui/main.c gui/workspace.c gui/reveal.c gui/inspect.c native-api.c "${sources[@]:1}")
includes=(-I"$stack/build/deps/cimgui" -I"$stack")
suffix=""
if [[ "${2:-}" == --check ]]; then includes+=(-DATTENDANT_GUI_TESTING); suffix=-check; fi
if [[ "$compiler" == cjit ]]; then
  "${CJIT:-cjit}" -q -C '' "${flags[@]:1}" "${includes[@]}" -o build/gui/attendant-gui-cjit${suffix}.exe "${sources[@]}" \
    -L"$stack/build/bin" -lcimgui_richmd -lbcrypt -lshell32 -luser32
else
  export ZIG_GLOBAL_CACHE_DIR=${ZIG_GLOBAL_CACHE_DIR:-"$PWD/build/zig-global-cache"}
  "${ZIG:-zig}" cc -O3 "${flags[@]}" "${includes[@]}" "${sources[@]}" \
    -L"$stack/build/bin" -lcimgui_richmd -lbcrypt -lshell32 -luser32 -o build/gui/attendant-gui-zig${suffix}.exe
fi
if [[ "${2:-}" == --check ]]; then
  sources[0]=gui/model-check.c
  if [[ "$compiler" == cjit ]]; then
    "${CJIT:-cjit}" -q -C '' "${flags[@]:1}" -DATTENDANT_MODEL_TESTING -DATTENDANT_TEST_FAULTS -o build/gui/model-check-cjit.exe "${sources[@]}" -lbcrypt -lshell32 -luser32
  else
    "${ZIG:-zig}" cc -O3 "${flags[@]}" -DATTENDANT_MODEL_TESTING -DATTENDANT_TEST_FAULTS "${sources[@]}" -lbcrypt -lshell32 -luser32 -o build/gui/model-check-zig.exe
  fi
fi
# Keep running viewer's identical mapped DLL untouched.
if ! cmp -s "$stack/build/bin/cimgui_richmd.dll" build/gui/cimgui_richmd.dll; then
  cp "$stack/build/bin/cimgui_richmd.dll" build/gui/
fi
node gui/build-manifest.mjs "$compiler" ${2:+"$2"}
printf '%s\n' "$PWD/build/gui/attendant-gui-$compiler$suffix.exe"
