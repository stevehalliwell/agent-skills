#!/usr/bin/env bash
# macOS build only. No downloads, application execution, or UI launch.
# Stdout: absolute executable path. Diagnostics: stderr. Exit 0/1/2.
set -euo pipefail
root=$(cd "$(dirname "$0")" && pwd)
compiler=cjit
target=cli
graph=0
view=0
output="$root/build/mac"
ui_library=''
usage() {
  printf '%s\n' 'Usage: bash build.sh [--compiler cjit|zig] [--test|--viewer-test|--explorer-test] [--view|--graph] [--ui-library PATH] [--output-directory PATH]' \
    'macOS only. CLI default; --view (also --graph) builds all windows; --explorer-test never launches.' \
    'Prepare native UI library with build-native-ui.sh. View output must be new/empty.' \
    'SDKROOT overrides xcrun SDK. CJIT/ZIG override compiler executable. No downloads or implicit dependency builds.' \
    'Stdout: executable path; diagnostics stderr. Exit 0 success, 1 build/prerequisite failure, 2 usage.'
}
fail() { printf '%s\n' "$*" >&2; exit 1; }
while [[ $# -gt 0 ]]; do
  case "$1" in
    --help) usage; exit 0 ;;
    --compiler|--output-directory|--ui-library)
      [[ $# -ge 2 && -n "$2" ]] || { usage >&2; exit 2; }
      case "$1" in
        --compiler) compiler=$2 ;;
        --output-directory) output=$2 ;;
        --ui-library) ui_library=$2 ;;
      esac
      shift 2 ;;
    --test|--viewer-test|--explorer-test)
      [[ "$target" == cli ]] || { usage >&2; exit 2; }
      target=${1#--}; shift ;;
    --view|--graph) graph=1; view=1; shift ;;
    *) usage >&2; exit 2 ;;
  esac
done
[[ "$compiler" == cjit || "$compiler" == zig ]] || { usage >&2; exit 2; }
[[ "$target" != explorer-test ]] || { graph=1; view=1; }
[[ "$view" == 0 || "$target" == cli || "$target" == explorer-test ]] || { usage >&2; exit 2; }
[[ -z "$ui_library" || "$view" == 1 ]] || { usage >&2; exit 2; }
[[ $(uname -s) == Darwin ]] || fail 'This build entry point requires macOS.'
sdk=${SDKROOT:-$(xcrun --show-sdk-path)} || fail 'Cannot locate Apple SDK; set SDKROOT.'
[[ -d "$sdk/usr/include" ]] || fail 'Apple SDK headers unavailable; set SDKROOT.'
if [[ "$view" == 1 ]]; then
  ui_library=${ui_library:-"$root/build/mac-native-ui/libgd-native-ui.dylib"}
  [[ -f "$ui_library" && $(basename "$ui_library") == libgd-native-ui.dylib ]] || fail 'Supply libgd-native-ui.dylib from build-native-ui.sh with --ui-library.'
  include=$(cd "$(dirname "$ui_library")" && pwd)/include
  [[ -f "$include/GLFW/glfw3.h" ]] || fail 'Keep the prepared GLFW header beside native UI library in include/GLFW/.'
  if [[ -d "$output" && -n $(ls -A "$output") ]]; then fail 'View output must be new/empty; choose --output-directory.'; fi
fi
mkdir -p "$output"
output=$(cd "$output" && pwd)
arguments=(-DHAVE_CONFIG_H=1 -DYAML_DECLARE_STATIC=1 -I "$root/vendor/libyaml/include" -I "$root/vendor/libyaml/src")
while read -r kind path; do
  case "$kind" in
    core|yaml) arguments+=("$root/$path") ;;
    view) [[ "$view" == 0 ]] || arguments+=("$root/$path") ;;
    graph) [[ "$graph" == 0 ]] || arguments+=("$root/$path") ;;
  esac
done < "$root/build-sources.txt"
name=gd
case "$target" in
  cli) entry=gd_cli.c ;;
  test) entry=test_core.c; name=test-core ;;
  viewer-test) entry=test_viewer.c; name=test-viewer ;;
  explorer-test) entry=test_explorer_view.c; name=test-explorer-view; arguments+=(-DGD_EXPLORER_TEST=1) ;;
esac
arguments+=("$root/$entry")
if [[ "$view" == 1 ]]; then
  [[ "$target" != cli ]] || name=gd-view
  arguments+=(-DGD_WITH_VIEW=1 -DGD_WITH_GRAPH=1 -I "$root/vendor/cimgui" -I "$include" "$root/gd_native_window_macos.c")
  cp "$ui_library" "$output/libgd-native-ui.dylib"
  if [[ -f "$(dirname "$ui_library")/GLFW-LICENSE.md" ]]; then cp "$(dirname "$ui_library")/GLFW-LICENSE.md" "$output/GLFW-LICENSE.md"; fi
  arguments+=("$output/libgd-native-ui.dylib")
fi
destination="$output/$name-$compiler"
if [[ "$compiler" == cjit ]]; then
  "${CJIT:-cjit}" -q -C '' -I "$sdk/usr/include" "${arguments[@]}" -o "$destination" >&2 || fail 'CJIT build failed.'
  codesign -s - "$destination" >&2 || fail 'Ad-hoc signing failed.'
else
  export ZIG_GLOBAL_CACHE_DIR=${ZIG_GLOBAL_CACHE_DIR:-"$root/build/zig-global-cache"}
  export ZIG_LOCAL_CACHE_DIR=${ZIG_LOCAL_CACHE_DIR:-"$root/build/zig-local-cache"}
  "${ZIG:-zig}" cc -std=c99 -O2 --sysroot "$sdk" -isystem "$sdk/usr/include" "${arguments[@]}" -o "$destination" >&2 || fail 'Zig build failed.'
fi
printf '%s\n' "$destination"
