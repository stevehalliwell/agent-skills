#!/usr/bin/env bash
# Execute C99 host against Zig-built DLL. No compilation/download/install of dependencies.
set -euo pipefail
if [[ "${1:-}" == --help ]]; then
  echo 'Usage: bash run-cjit.sh [--frames 1..6000 --shot FILE.ppm] [--no-preview] [--exercise]'
  echo 'No options: open interactive source editor and Markdown preview; close window to exit.'
  exit 0
fi
cd "$(dirname "$0")"
if [[ ! -f build/bin/cimgui_richmd.dll ]]; then
  echo 'Missing DLL: run node prepare.mjs, then bash build.sh.' >&2; exit 1
fi
system_root="$(cygpath -m "${SystemRoot:-${SYSTEMROOT:-C:/Windows}}")"
assets="$(cygpath -m "$PWD/build/deps/richmd/assets")"
# CJIT 1.2 needs the system DLL search directory explicitly for user32 exports.
exec "${CJIT:-cjit}" -q -DCIMGUI_DEFINE_ENUMS_AND_STRUCTS -DCIMGUI_NO_EXPORT \
  -Ibuild/deps/cimgui -Lbuild/bin -lcimgui_richmd \
  -L "$system_root/System32" -luser32 host.c -- --assets "$assets" "$@"
