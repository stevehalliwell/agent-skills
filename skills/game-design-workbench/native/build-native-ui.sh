#!/usr/bin/env bash
# Build macOS native GUI dependency only. No downloads or application execution.
set -euo pipefail
root=$(cd "$(dirname "$0")" && pwd)
glfw=${GLFW_ROOT:-"$root/build/deps/glfw-3.4"}
output="$root/build/mac-native-ui"
zig=${ZIG:-zig}
usage() {
  printf '%s\n' 'Usage: bash build-native-ui.sh [--glfw-directory PATH] [--output-directory NEW-DIRECTORY]' \
    'macOS only. Needs installed Zig and Apple SDK plus prepared upstream GLFW 3.4 source.' \
    'Defaults: GLFW_ROOT or build/deps/glfw-3.4; output build/mac-native-ui.' \
    'Stdout: absolute libgd-native-ui.dylib path. Diagnostics: stderr; no window.'
}
fail() { printf '%s\n' "$*" >&2; exit 1; }
while [[ $# -gt 0 ]]; do
  case "$1" in
    --help) usage; exit 0 ;;
    --glfw-directory|--output-directory)
      [[ $# -ge 2 && -n "$2" ]] || { usage >&2; exit 2; }
      case "$1" in --glfw-directory) glfw=$2 ;; --output-directory) output=$2 ;; esac
      shift 2 ;;
    *) usage >&2; exit 2 ;;
  esac
done
[[ $(uname -s) == Darwin ]] || fail 'Native GLFW/OpenGL3 dependency build requires macOS.'
sdk=${SDKROOT:-$(xcrun --show-sdk-path)} || fail 'Cannot locate Apple SDK; set SDKROOT.'
[[ -d "$sdk/usr/include" ]] || fail 'Apple SDK headers unavailable.'
header="$glfw/include/GLFW/glfw3.h"
[[ -f "$header" && -f "$glfw/src/cocoa_window.m" ]] || fail 'Supply prepared GLFW 3.4 source with --glfw-directory.'
for pair in 'MAJOR 3' 'MINOR 4' 'REVISION 0'; do
  read -r version value <<< "$pair"
  grep -Eq "^#define[[:space:]]+GLFW_VERSION_${version}[[:space:]]+$value([[:space:]]|$)" "$header" || fail 'Expected GLFW 3.4.0 headers and sources.'
done
if [[ -d "$output" && -n $(ls -A "$output") ]]; then fail 'Output must be new/empty; existing artifacts retained.'; fi
mkdir -p "$output/objects" "$output/include/GLFW"
output=$(cd "$output" && pwd)
export MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET:-11.0}
export ZIG_GLOBAL_CACHE_DIR=${ZIG_GLOBAL_CACHE_DIR:-"$root/build/zig-global-cache"}
export ZIG_LOCAL_CACHE_DIR=${ZIG_LOCAL_CACHE_DIR:-"$root/build/zig-local-cache"}
sdk_flags=(--sysroot "$sdk" -isystem "$sdk/usr/include" -iframework "$sdk/System/Library/Frameworks" -L/usr/lib)
# GLFW 3.4 Cocoa source list. Compile into owned objects, never edit checkout.
glfw_sources=(context.c init.c input.c monitor.c platform.c vulkan.c window.c
  egl_context.c osmesa_context.c null_init.c null_monitor.c null_window.c null_joystick.c
  cocoa_time.c posix_module.c posix_thread.c
  cocoa_init.m cocoa_joystick.m cocoa_monitor.m cocoa_window.m nsgl_context.m)
objects=()
for source in "${glfw_sources[@]}"; do
  object="$output/objects/glfw-${source%.*}.o"
  "$zig" cc -std=c99 -O2 -fPIC -D_GLFW_COCOA "${sdk_flags[@]}" -I "$glfw/include" -I "$glfw/src" -c "$glfw/src/$source" -o "$object" >&2
  objects+=("$object")
done
sources=(gd_native_ui_macos.cpp vendor/cimgui/imgui/backends/imgui_impl_glfw.cpp vendor/cimgui/imgui/backends/imgui_impl_opengl3.cpp)
while read -r kind path; do [[ "$kind" != ui ]] || sources+=("$path"); done < "$root/build-sources.txt"
for source in "${sources[@]}"; do
  object="$output/objects/${source//\//_}.o"
  "$zig" c++ -std=c++17 -O2 -fPIC -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS -DGL_SILENCE_DEPRECATION \
    "${sdk_flags[@]}" -I "$root/vendor/cimgui/imgui" -I "$glfw/include" -c "$root/$source" -o "$object" >&2
  objects+=("$object")
done
library="$output/libgd-native-ui.dylib"
"$zig" c++ -shared --sysroot "$sdk" -F/System/Library/Frameworks -L/usr/lib "${objects[@]}" \
  -Wl,-install_name,@executable_path/libgd-native-ui.dylib -framework Cocoa -framework IOKit -framework OpenGL \
  -framework CoreGraphics -framework CoreFoundation -framework ImageIO -o "$library" >&2
cp "$header" "$output/include/GLFW/glfw3.h"
cp "$glfw/LICENSE.md" "$output/GLFW-LICENSE.md"
printf '%s\n' "$library"
