#!/usr/bin/env bash
# macOS C99 GLFW host and optional headless model. No downloads/installations.
# Usage: bash tools/build-gui-macos.sh [zig|cjit] [--check]
set -euo pipefail
cd "$(dirname "$0")/.."
compiler=${1:-cjit}
if [[ $(uname -s) != Darwin || $# -gt 2 || ( "$compiler" != zig && "$compiler" != cjit ) || ( $# == 2 && "$2" != --check ) ]]; then
  echo 'Usage on macOS: build-gui-macos.sh [zig|cjit] [--check]' >&2; exit 2
fi
if [[ ! -f build/bin/libcimgui_richmd.dylib ]]; then
  echo 'Missing GUI library. Explicitly prepare dependencies, then run tools/build-gui-stack.sh. Build never downloads.' >&2; exit 1
fi
variant=${ATTENDANT_GUI_VARIANT:-}
if [[ -n "$variant" && ! "$variant" =~ ^[a-z0-9][a-z0-9-]*$ ]]; then
  echo 'ATTENDANT_GUI_VARIANT must contain lowercase letters, digits or hyphens, starting with a letter or digit.' >&2; exit 2
fi
output="build/gui${variant:+-$variant}"
mkdir -p "$output"
source tools/build-inputs.sh
source tools/build-platform.sh
sources=(apps/gui/main.c apps/gui/workspace.c apps/gui/reveal.c apps/gui/inspect.c apps/gui/ipc-macos.c src/native-api.c "${sources[@]:1}")
includes=(-Ibuild/deps/cimgui -Ibuild/deps/glfw/include -Ivendor/gui)
suffix=""
if [[ ${2:-} == --check ]]; then includes+=(-DATTENDANT_GUI_TESTING); suffix=-check; fi
executable="$output/attendant-gui-$compiler$suffix"
if [[ $compiler == zig ]]; then
  "${ZIG:-zig}" cc -O3 "${zig_platform_flags[@]}" "${flags[@]}" "${includes[@]}" "${sources[@]}" \
    -Lbuild/bin -lcimgui_richmd -Wl,-rpath,@executable_path -o "$executable"
else
  # CJIT emits Mach-O that install_name_tool cannot rewrite. Give its linker a
  # private library copy with executable-relative identity; runtime library stays original.
  link_directory="$output/link-cjit$suffix"
  mkdir -p "$link_directory"
  cp build/bin/libcimgui_richmd.dylib "$link_directory/"
  install_name_tool -id @executable_path/libcimgui_richmd.dylib "$link_directory/libcimgui_richmd.dylib"
  "${CJIT:-cjit}" -q -C '' "${cjit_platform_flags[@]}" "${flags[@]:1}" "${includes[@]}" "${sources[@]}" \
    -L"$link_directory" -lcimgui_richmd -o "$executable"
  codesign -s - "$executable"
fi
if [[ ${2:-} == --check ]]; then
  sources[0]=tests/gui/model-check.c
  model="$output/model-check-$compiler"
  if [[ $compiler == zig ]]; then
    "${ZIG:-zig}" cc -O3 "${zig_platform_flags[@]}" "${flags[@]}" -DATTENDANT_MODEL_TESTING -DATTENDANT_TEST_FAULTS "${sources[@]}" -o "$model"
  else
    "${CJIT:-cjit}" -q -C '' "${cjit_platform_flags[@]}" "${flags[@]:1}" -DATTENDANT_MODEL_TESTING -DATTENDANT_TEST_FAULTS "${sources[@]}" -o "$model"
    codesign -s - "$model"
  fi
fi
if ! cmp -s build/bin/libcimgui_richmd.dylib "$output/libcimgui_richmd.dylib"; then
  cp build/bin/libcimgui_richmd.dylib "$output/"
fi
ATTENDANT_TOOL_COMPILER="${ATTENDANT_TOOL_COMPILER:-$compiler}" bash tools/native-tool.sh gui-manifest "$compiler" ${2:+"$2"} >&2
printf '%s\n' "$PWD/$executable"
