#!/usr/bin/env bash
# Build/run bounded macOS C99 GUI bridge smoke. Usage: bash tools/check-gui-bridge.sh [zig|cjit]
# Requires built native GUI stack; no acquisition, installs or project writes.
set -euo pipefail
cd "$(dirname "$0")/.."
compiler=${1:-zig}
if [[ $# -gt 1 || ( "$compiler" != zig && "$compiler" != cjit ) ]]; then
  echo 'Usage: check-gui-bridge.sh [zig|cjit]' >&2; exit 2
fi
if [[ $(uname -s) != Darwin || ! -f build/bin/libcimgui_richmd.dylib ]]; then
  echo 'Requires macOS and built GUI stack: bash tools/build-gui-stack.sh' >&2; exit 1
fi
node --input-type=module -e "import { stackManifest } from './tools/gui-build-manifest.mjs'; stackManifest();"
root=$(pwd)
sdk=${SDKROOT:-$(xcrun --show-sdk-path)}
mkdir -p build/bridge-check
output="build/bridge-check/bridge-$compiler"
includes=(-Ibuild/deps/cimgui -Ibuild/deps/glfw/include -Ivendor/gui)
if [[ $compiler == zig ]]; then
  export ZIG_GLOBAL_CACHE_DIR=${ZIG_GLOBAL_CACHE_DIR:-"$root/build/zig-cache"}
  "${ZIG:-zig}" cc -std=c99 -O2 --sysroot "$sdk" -isystem "$sdk/usr/include" \
    "${includes[@]}" tests/gui/bridge-check.c -Lbuild/bin -lcimgui_richmd \
    -Wl,-rpath,@executable_path/../bin -o "$output" >&2
else
  "${CJIT:-cjit}" -q -C '' -I "$sdk/usr/include" "${includes[@]}" \
    tests/gui/bridge-check.c -Lbuild/bin -l cimgui_richmd -o "$output" >&2
  codesign -s - "$output"
fi
# CJIT emits library install names without an rpath option; scope library lookup
# to this owned test process, never modify user's global loader environment.
DYLD_LIBRARY_PATH="$root/build/bin${DYLD_LIBRARY_PATH:+:$DYLD_LIBRARY_PATH}" \
  "$output" "$root/build/deps/richmd/assets" "$root/build/bridge-check/$compiler.ppm"
