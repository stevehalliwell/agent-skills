<#
Build isolated no-raylib Win32/D3D11 GUI dependency from vendored ImGui 1.93.
Application stays C99/CJIT. Installed Zig compiles C++ library only.
Usage: ./build-native-ui.ps1 [-Zig PATH] [-OutputDirectory PATH]
No downloads, installs, GUI launches or project-data writes. Existing output
DLL is refused; choose fresh output rather than replacing a running library. Stdout: one JSON result; compiler logs: stderr.
Exit 0 success, 2 invalid input/build failure. macOS recipe remains pending.
#>
param([string]$Zig = 'zig', [string]$OutputDirectory = '')
$ErrorActionPreference = 'Stop'
try {
    if (-not $IsWindows) { throw 'This first native-backend build supports Windows only; macOS GLFW/OpenGL3 recipe remains pending.' }
    $root = $PSScriptRoot
    $output = if ($OutputDirectory) { [IO.Path]::GetFullPath($OutputDirectory) } else { Join-Path $root 'build/native-ui' }
    $bindings = Join-Path $root 'vendor/cimgui'
    $imgui = Join-Path $bindings 'imgui'
    if (-not (Select-String -LiteralPath (Join-Path $imgui 'imgui.h') -Pattern '^#define IMGUI_VERSION\s+"1\.93\.0"' -Quiet) -or
        -not (Select-String -LiteralPath (Join-Path $bindings 'cimgui.h') -Pattern 'version "1\.93\.0" 19300' -Quiet)) {
        throw 'Expected matching pinned ImGui/cimgui 1.93.0 headers.'
    }
    $destination = Join-Path $output 'gd-native-ui.dll'
    if (Test-Path -LiteralPath $destination) { throw 'Output library already exists; choose fresh -OutputDirectory. Existing library retained.' }
    New-Item -ItemType Directory -Force $output | Out-Null
    $arguments = @('c++', '-target', 'x86_64-windows-gnu', '-shared', '-O2', '-fno-exceptions', '-fno-rtti', '-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS',
        '-I', $bindings, '-I', $imgui, (Join-Path $root 'gd_native_ui.cpp'), (Join-Path $bindings 'cimgui.cpp'))
    foreach ($name in @('imgui.cpp', 'imgui_draw.cpp', 'imgui_tables.cpp', 'imgui_widgets.cpp', 'imgui_demo.cpp',
        'backends/imgui_impl_win32.cpp', 'backends/imgui_impl_dx11.cpp')) { $arguments += Join-Path $imgui $name }
    $arguments += @('-ld3d11', '-ldxgi', '-ld3dcompiler_47', '-ldwmapi', '-lgdi32', '-limm32', '-lole32', '-luuid', '-lwindowscodecs', '-o', $destination)
    & $Zig @arguments | ForEach-Object { [Console]::Error.WriteLine($_) }
    if ($LASTEXITCODE -ne 0) { throw "Native GUI dependency build failed with exit $LASTEXITCODE" }
    @{ ok = $true; backend = 'Win32/D3D11'; imgui_version = '1.93.0'; library = $destination; library_sha256 = (Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash; binding_profile = 'IMGUI_DISABLE_OBSOLETE_FUNCTIONS'; raylib = $false; application_compiler = 'CJIT (separate build)' } | ConvertTo-Json -Compress -EscapeHandling EscapeNonAscii
} catch {
    $message = $_.Exception.Message
    [Console]::Error.WriteLine($message)
    @{ ok = $false; code = 'native_ui.build'; message = $message } | ConvertTo-Json -Compress -EscapeHandling EscapeNonAscii
    exit 2
}
