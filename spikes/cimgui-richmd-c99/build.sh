#!/usr/bin/env bash
# Windows x64 spike build. Requires fetched/pinned sources, Zig on PATH; never downloads.
set -euo pipefail
cd "$(dirname "$0")"
export ZIG_GLOBAL_CACHE_DIR="${ZIG_GLOBAL_CACHE_DIR:-$(pwd)/build/zig-cache}"
zig="${ZIG:-zig}"
mkdir -p build/bin
root="$(pwd)"
if [[ ! -f build/deps/luajit/src/ljamalg.c ]]; then
  echo 'Missing sources: run node prepare.mjs first.' >&2; exit 1
fi
node prepare.mjs --verify
# Rebuild generation tool; Zig's private content-addressed cache avoids stale tool reuse.
  (
    cd build/deps/luajit/src
    "$zig" cc -O2 host/minilua.c -o minilua.exe
    ./minilua.exe ../dynasm/dynasm.lua -LN -D WIN -D JIT -D FFI -D ENDIAN_LE -D FPU -D P64 -o host/buildvm_arch.h vm_x64.dasc
    git show -s --format=%ct > luajit_relver.txt
    ./minilua.exe host/genversion.lua
    "$zig" cc -O2 -I. -I../dynasm host/buildvm*.c -o buildvm.exe
    ./buildvm.exe -m peobj -o lj_vm.obj
    libs=(lib_base.c lib_math.c lib_bit.c lib_string.c lib_table.c lib_io.c lib_os.c lib_package.c lib_debug.c lib_jit.c lib_ffi.c lib_buffer.c)
    for mode in bcdef ffdef libdef recdef; do
      ./buildvm.exe -m "$mode" -o "lj_${mode}.h" "${libs[@]}"
    done
    ./buildvm.exe -m vmdef -o jit/vmdef.lua "${libs[@]}"
    ./buildvm.exe -m folddef -o lj_folddef.h lj_opt_fold.c
    "$zig" cc -O2 ljamalg.c luajit.c lj_vm.obj -o "$root/build/bin/luajit.exe"
  )
(
  cd build/deps/cimgui/generator
  "$root/build/bin/luajit.exe" generator.lua 'zig cc' 'internal' > "$root/build/generator.log" 2>&1
)
node fix-generated.mjs
echo 'Generated matching cimgui wrappers.'
cimgui=build/deps/cimgui
imgui="$cimgui/imgui"
richmd=build/deps/richmd
sources=(richmd_c.cpp "$cimgui/cimgui.cpp"
  "$imgui/imgui.cpp" "$imgui/imgui_draw.cpp" "$imgui/imgui_tables.cpp" "$imgui/imgui_widgets.cpp" "$imgui/imgui_demo.cpp"
  "$imgui/backends/imgui_impl_win32.cpp" "$imgui/backends/imgui_impl_dx11.cpp"
  "$richmd/imgui_rich_md/rich_md.cpp" "$richmd/imgui_rich_md/narrative_programming.cpp"
  "$richmd/imgui_rich_md/internal/rich_md_renderer.cpp" "$richmd/imgui_rich_md/internal/rich_md_search.cpp"
  "$richmd/imgui_rich_md/third_party/stb_image_impl.cpp")
# Keep MD4C compiled as C99 rather than pretending its .c file is C++.
"$zig" cc -std=c99 -O3 -c "$richmd/external/md4c/src/md4c.c" -o build/md4c.obj
"$zig" c++ -std=c++17 -O3 -shared -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS \
  -I"$imgui" -I"$richmd" -I"$richmd/external/md4c/src" \
  "${sources[@]}" build/md4c.obj -o build/bin/cimgui_richmd.dll \
  -ld3d11 -ldxgi -ld3dcompiler_47 -ldwmapi -lgdi32 -limm32 -lole32 -luuid
"$zig" cc -std=c99 -pedantic-errors -O3 -Wall -Wextra -Werror -DCIMGUI_DEFINE_ENUMS_AND_STRUCTS -DCIMGUI_NO_EXPORT \
  -I"$cimgui" host.c -Lbuild/bin -lcimgui_richmd -lgdi32 -o build/bin/host-zig.exe
node build-manifest.mjs
echo 'Built cimgui_richmd.dll and C99 Zig host.'
