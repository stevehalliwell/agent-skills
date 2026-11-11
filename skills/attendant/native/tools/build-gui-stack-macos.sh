#!/usr/bin/env bash
# Native macOS GUI dependency library; called by build-gui-stack.sh.
# Requires installed Zig, make and Apple SDK plus explicitly acquired pinned sources.
# No downloads or tool installation. Run from native product root.
set -euo pipefail
root=$(pwd)
zig=${ZIG:-zig}
sdk=${SDKROOT:-$(xcrun --show-sdk-path)}
# First arm64 macOS release; LuaJIT requires explicit deployment target.
export MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET:-11.0}
if [[ ! -d "$sdk/usr/include" || ! -f build/deps/glfw/src/cocoa_window.m ]]; then
  echo 'Missing Apple SDK or pinned GLFW sources; see references/build.md and run bash tools/native-tool.sh prepare explicitly.' >&2
  exit 1
fi
# Mark SDK frameworks as system headers; Clang 22 otherwise promotes Apple's
# older CF_ENUM declarations to errors in the C++ GLFW backend.
sdk_flags=(--sysroot "$sdk" -isystem "$sdk/usr/include" -iframework "$sdk/System/Library/Frameworks" -L/usr/lib)
mkdir -p build/bin build/glfw build/gui-stack-objects
# LuaJIT owns architecture-specific DynASM generation. Keep its native arm64
# recipe rather than reusing the Windows x64/PE generator recipe.
make_flags=(BUILDMODE=static
  "CC=\"$zig\" cc --sysroot \"$sdk\" -isystem \"$sdk/usr/include\""
  "HOST_CC=\"$zig\" cc --sysroot \"$sdk\" -isystem \"$sdk/usr/include\""
  "AR=\"$zig\" ar rcus")
# LuaJIT make does not track compiler/SDK/target flag changes. Rebuild only its
# owned outputs; never remove a user's global compiler cache.
make -C build/deps/luajit/src "${make_flags[@]}" clean
make -C build/deps/luajit/src -j2 "${make_flags[@]}"
cp build/deps/luajit/src/luajit build/bin/luajit
(
  cd build/deps/cimgui/generator
  "$root/build/bin/luajit" generator.lua 'zig cc' 'internal' > "$root/build/generator.log" 2>&1
)
ATTENDANT_TOOL_COMPILER="${ATTENDANT_TOOL_COMPILER:-zig}" bash tools/native-tool.sh fix-generated
cimgui=build/deps/cimgui
imgui="$cimgui/imgui"
richmd=build/deps/richmd
glfw=build/deps/glfw
# Match GLFW 3.4's Cocoa source list, including its shared null/EGL/OSMesa paths.
glfw_sources=(context.c init.c input.c monitor.c platform.c vulkan.c window.c
  egl_context.c osmesa_context.c null_init.c null_monitor.c null_window.c null_joystick.c
  cocoa_time.c posix_module.c posix_thread.c
  cocoa_init.m cocoa_joystick.m cocoa_monitor.m cocoa_window.m nsgl_context.m)
objects=()
for source in "${glfw_sources[@]}"; do
  object="build/glfw/${source%.*}.o"
  "$zig" cc -std=c99 -O3 -fPIC -D_GLFW_COCOA "${sdk_flags[@]}" \
    -I"$glfw/include" -I"$glfw/src" -c "$glfw/src/$source" -o "$object"
  objects+=("$object")
done
# Upstream RichMd applies this same vendor-only SDK OFF_MAX compatibility flag.
"$zig" cc -std=c99 -O3 -fPIC -Wno-macro-redefined "${sdk_flags[@]}" \
  -c "$richmd/external/md4c/src/md4c.c" -o build/md4c.o
sources=(vendor/gui/richmd_c.cpp "$cimgui/cimgui.cpp"
  "$imgui/imgui.cpp" "$imgui/imgui_draw.cpp" "$imgui/imgui_tables.cpp" "$imgui/imgui_widgets.cpp" "$imgui/imgui_demo.cpp"
  "$imgui/backends/imgui_impl_glfw.cpp" "$imgui/backends/imgui_impl_opengl3.cpp"
  "$richmd/imgui_rich_md/rich_md.cpp" "$richmd/imgui_rich_md/narrative_programming.cpp"
  "$richmd/imgui_rich_md/internal/rich_md_renderer.cpp" "$richmd/imgui_rich_md/internal/rich_md_search.cpp"
  "$richmd/imgui_rich_md/third_party/stb_image_impl.cpp")
for source in "${sources[@]}"; do
  object="build/gui-stack-objects/${source//\//_}.o"
  "$zig" c++ -std=c++17 -O3 -fPIC -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS -DGL_SILENCE_DEPRECATION \
    "${sdk_flags[@]}" -I"$imgui" -I"$richmd" -I"$richmd/external/md4c/src" -I"$glfw/include" \
    -c "$source" -o "$object"
  objects+=("$object")
done
# Zig's -F is also a user-header search path. Use it only at link time, after
# compiling SDK framework includes as system headers above.
"$zig" c++ -shared --sysroot "$sdk" -F/System/Library/Frameworks -L/usr/lib \
  "${objects[@]}" build/md4c.o -o build/bin/libcimgui_richmd.dylib \
  -Wl,-install_name,@rpath/libcimgui_richmd.dylib -framework Cocoa -framework OpenGL -framework IOKit
ATTENDANT_TOOL_COMPILER="${ATTENDANT_TOOL_COMPILER:-zig}" bash tools/native-tool.sh stack-manifest
printf '%s\n' 'Built libcimgui_richmd.dylib (GLFW/OpenGL3).'
