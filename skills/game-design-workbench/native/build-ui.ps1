<# Explicit graph UI dependency build. Installed Zig compiles vendored C++ only.
Application remains C99/CJIT. No downloads. Needs matching raylib 6.0 Windows SDK.
Usage: ./build-ui.ps1 -RaylibDirectory PATH [-Zig PATH]
#>
param([string]$RaylibDirectory = $env:RAYLIB_ROOT, [string]$Zig = 'zig')
$ErrorActionPreference = 'Stop'
if (-not $RaylibDirectory) { throw 'Supply -RaylibDirectory PATH (raylib 6.0 win64 SDK).' }
$root = $PSScriptRoot
$library = Join-Path $RaylibDirectory 'lib/libraylibdll.a'
if (-not (Test-Path -LiteralPath $library -PathType Leaf)) { throw "Missing raylib 6.0 import library: $library" }
$header = Join-Path $RaylibDirectory 'include/raylib.h'
if (-not (Test-Path -LiteralPath $header -PathType Leaf) -or -not (Select-String -LiteralPath $header -Pattern '^#define RAYLIB_VERSION\s+"6\.0"' -Quiet)) { throw 'Supply matching raylib 6.0 SDK headers and libraries; do not reuse 5.5 binaries.' }
$output = Join-Path $root 'build'
New-Item -ItemType Directory -Force $output | Out-Null
$arguments = @('c++', '-shared', '-O2', '-fno-exceptions', '-fno-rtti', '-DBUILD_LIBTYPE_SHARED', '-DNO_FONT_AWESOME')
foreach ($include in @('vendor/raylib', 'vendor/cimgui', 'vendor/cimgui/imgui', 'vendor/rlImGui')) { $arguments += @('-I', (Join-Path $root $include)) }
Get-Content -LiteralPath (Join-Path $root 'build-sources.txt') | ForEach-Object {
    if ($_.StartsWith('ui ')) { $arguments += Join-Path $root ($_.Substring(3)) }
}
$destination = Join-Path $output 'gd-ui.dll'
$arguments += @((Resolve-Path -LiteralPath $library).Path, '-o', $destination)
& $Zig @arguments
if ($LASTEXITCODE -ne 0) { throw "UI dependency build failed with exit $LASTEXITCODE" }
Write-Output $destination
