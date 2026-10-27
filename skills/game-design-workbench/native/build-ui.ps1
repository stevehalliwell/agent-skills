<# Explicit graph UI dependency build. Installed Zig compiles vendored C++ only.
Application remains C99/CJIT. No downloads. Needs matching raylib 5.5 Windows SDK.
Usage: ./build-ui.ps1 -RaylibDirectory PATH [-Zig PATH]
#>
param([string]$RaylibDirectory = $env:RAYLIB_ROOT, [string]$Zig = 'zig')
$ErrorActionPreference = 'Stop'
if (-not $RaylibDirectory) { throw 'Supply -RaylibDirectory PATH (raylib 5.5 win64 SDK).' }
$root = $PSScriptRoot
$library = Join-Path $RaylibDirectory 'lib/libraylibdll.a'
if (-not (Test-Path -LiteralPath $library -PathType Leaf)) { throw "Missing raylib 5.5 import library: $library" }
$output = Join-Path $root 'build'
New-Item -ItemType Directory -Force $output | Out-Null
$arguments = @('c++', '-shared', '-O2', '-fno-exceptions', '-fno-rtti', '-DBUILD_LIBTYPE_SHARED', '-DNO_FONT_AWESOME')
foreach ($include in @('vendor/raylib', 'vendor/cimgui', 'vendor/cimgui/imgui', 'vendor/rlImGui')) { $arguments += @('-I', (Join-Path $root $include)) }
foreach ($source in @('vendor/cimgui/cimgui.cpp', 'vendor/cimgui/imgui/imgui.cpp', 'vendor/cimgui/imgui/imgui_draw.cpp', 'vendor/cimgui/imgui/imgui_tables.cpp', 'vendor/cimgui/imgui/imgui_widgets.cpp', 'vendor/cimgui/imgui/imgui_demo.cpp', 'vendor/rlImGui/rlImGui.cpp')) { $arguments += Join-Path $root $source }
$destination = Join-Path $output 'gd-ui.dll'
$arguments += @((Resolve-Path -LiteralPath $library).Path, '-o', $destination)
& $Zig @arguments
if ($LASTEXITCODE -ne 0) { throw "UI dependency build failed with exit $LASTEXITCODE" }
Write-Output $destination
