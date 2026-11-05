#!/usr/bin/env bash
# Build raylib 6.0 + existing cimgui/rlImGui as one macOS dylib using Zig.
# Based on c-ink/tools/story-player/build.c and native Attendant SDK flags.
# Explicit preparation only: never downloads dependencies or launches a UI.
set -euo pipefail
root=$(cd "$(dirname "$0")" && pwd)
raylib=${RAYLIB_ROOT:-"$root/build/deps/raylib-6.0"}
output="$root/build/mac-ui"
usage() {
  printf '%s\n' 'Usage: bash build-ui.sh [--raylib-directory PATH] [--output-directory PATH]' \
    'macOS only; needs installed Zig/Apple SDK and prepared raylib 6.0 sources (src/rcore.c).' \
    'Defaults: RAYLIB_ROOT or native/build/deps/raylib-6.0; output native/build/mac-ui.' \
    'SDKROOT/ZIG override SDK/compiler. No downloads, application build, or UI launch.' \
    'Stdout: dylib path; diagnostics stderr. Exit 0 success, 1 build/prerequisite failure, 2 usage.'
}
fail() { printf '%s\n' "$*" >&2; exit 1; }
while [[ $# -gt 0 ]]; do
  case "$1" in
    --help) usage; exit 0 ;;
    --raylib-directory|--output-directory)
      [[ $# -ge 2 && -n "$2" ]] || { usage >&2; exit 2; }
      case "$1" in --raylib-directory) raylib=$2 ;; --output-directory) output=$2 ;; esac
      shift 2 ;;
    *) usage >&2; exit 2 ;;
  esac
done
[[ $(uname -s) == Darwin ]] || fail 'This build entry point requires macOS.'
sdk=${SDKROOT:-$(xcrun --show-sdk-path)} || fail 'Cannot locate Apple SDK; set SDKROOT.'
[[ -d "$sdk/usr/include" ]] || fail 'Apple SDK headers unavailable; set SDKROOT.'
[[ -f "$raylib/src/rcore.c" && -f "$raylib/src/rglfw.c" ]] || fail 'Missing raylib 6.0 sources; see README Build for explicit preparation.'
raylib=$(cd "$raylib" && pwd)
# Do not accidentally pair another raylib version with committed application headers.
cmp -s "$raylib/src/raylib.h" "$root/vendor/raylib/raylib.h" || fail 'raylib.h differs from pinned 6.0 application header; use documented unmodified release.'
cmp -s "$raylib/src/rlgl.h" "$root/vendor/raylib/rlgl.h" || fail 'rlgl.h differs from pinned 6.0 application header.'
mkdir -p "$output/objects"
output=$(cd "$output" && pwd)
export ZIG_GLOBAL_CACHE_DIR=${ZIG_GLOBAL_CACHE_DIR:-"$root/build/zig-global-cache"}
export ZIG_LOCAL_CACHE_DIR=${ZIG_LOCAL_CACHE_DIR:-"$root/build/zig-local-cache"}
zig=${ZIG:-zig}
# Treat SDK frameworks as system headers during compilation (Clang 22 CF_ENUM).
sdk_flags=(--sysroot "$sdk" -isystem "$sdk/usr/include" -iframework "$sdk/System/Library/Frameworks")
objects=()
for unit in rcore rshapes rtextures rtext rmodels raudio rglfw; do
  language=c
  [[ "$unit" != rglfw ]] || language=objective-c
  object="$output/objects/$unit.o"
  "$zig" cc -x "$language" -std=gnu99 -O2 -fPIC -D_GNU_SOURCE -DBUILD_LIBTYPE_SHARED \
    -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33 -DMAX_FILEPATH_LENGTH=4096 \
    "${sdk_flags[@]}" -I "$raylib/src" -I "$raylib/src/external/glfw/include" \
    -c "$raylib/src/$unit.c" -o "$object" >&2 || fail "raylib build failed: $unit"
  objects+=("$object")
done
while read -r kind path; do
  [[ "$kind" == ui ]] || continue
  object="$output/objects/$(basename "$path").o"
  "$zig" c++ -std=c++11 -O2 -fPIC -fno-exceptions -fno-rtti -DBUILD_LIBTYPE_SHARED -DNO_FONT_AWESOME \
    "${sdk_flags[@]}" -I "$root/vendor/raylib" -I "$root/vendor/cimgui" \
    -I "$root/vendor/cimgui/imgui" -I "$root/vendor/rlImGui" \
    -c "$root/$path" -o "$object" >&2 || fail "UI dependency build failed: $path"
  objects+=("$object")
done < "$root/build-sources.txt"
destination="$output/libgd-ui.dylib"
"$zig" c++ -dynamiclib --sysroot "$sdk" -F/System/Library/Frameworks -L/usr/lib \
  "${objects[@]}" -Wl,-install_name,@executable_path/libgd-ui.dylib \
  -framework Cocoa -framework OpenGL -framework IOKit -framework CoreAudio -framework CoreVideo -lobjc \
  -o "$destination" >&2 || fail 'UI dependency link failed.'
printf '%s\n' "$destination"
