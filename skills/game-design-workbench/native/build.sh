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
    'macOS only. CLI default; --explorer-test builds graph fixture, never launches it.' \
    'Prepare graphics library explicitly with build-ui.sh. No downloads or implicit dependency builds.' \
    'SDKROOT overrides xcrun SDK. CJIT/ZIG override compiler executable. Outputs may replace earlier builds; use separate directory for a running viewer.' \
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
    --view) view=1; shift ;;
    --graph) graph=1; view=1; shift ;;
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
  if [[ -z "$ui_library" ]]; then ui_library="$root/build/mac-ui/libgd-ui.dylib"; fi
  [[ -f "$ui_library" ]] || fail 'Missing prepared graphics library. Run build-ui.sh explicitly, or supply --ui-library PATH.'
  ui_library=$(cd "$(dirname "$ui_library")" && pwd)/$(basename "$ui_library")
  [[ $(basename "$ui_library") == libgd-ui.dylib ]] || fail 'Graphics library must be named libgd-ui.dylib.'
  arguments+=(-DGD_WITH_VIEW=1 -I "$root/vendor/raylib")
  if [[ "$graph" == 1 ]]; then
    arguments+=(-DGD_WITH_GRAPH=1 -DNO_FONT_AWESOME=1 -I "$root/vendor/cimgui" -I "$root/vendor/rlImGui")
  fi
  # Backend identity is @executable_path/libgd-ui.dylib for CJIT and Zig.
  [[ "$ui_library" == "$output/libgd-ui.dylib" ]] || cp "$ui_library" "$output/libgd-ui.dylib"
  arguments+=("$output/libgd-ui.dylib")
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
