<#
Build Windows/CJIT game-design tools. No downloads, installations, or project-data writes.
Usage: ./build.ps1 [-View [-Graph] -RaylibDirectory PATH | -Test | -ViewerTest | -ExplorerTest -RaylibDirectory PATH] [-Cjit PATH] [-OutputDirectory PATH]
Alternate output preserves running views. Stdout returns executable path; no project-data writes.
-Graph needs explicitly prepared gd-ui.dll from build-ui.ps1.
-ViewerTest builds headless protocol/transport fixture; does not launch it.
RaylibDirectory is an extracted raylib 6.0 win64 SDK with lib/raylib.dll.
#>
param(
    [switch]$View,
    [switch]$Graph,
    [switch]$Test,
    [switch]$ViewerTest,
    [switch]$ExplorerTest,
    [string]$UiLibrary = '',
    [string]$RaylibDirectory = $env:RAYLIB_ROOT,
    [string]$Cjit = 'cjit',
    [string]$OutputDirectory = ''
)
$ErrorActionPreference = 'Stop'
if ($ExplorerTest) { $Graph = $true }
if ($Graph) { $View = $true }
if (($View -and ($Test -or $ViewerTest)) -or ($Test -and $ViewerTest)) { throw 'Choose one of -View, -Test or -ViewerTest.' }
$root = $PSScriptRoot
# Alternate output keeps a user's running explorer/draft untouched.
$output = if ($OutputDirectory) { [IO.Path]::GetFullPath($OutputDirectory) } else { Join-Path $root 'build' }
New-Item -ItemType Directory -Force $output | Out-Null
$arguments = @('-q', '-DHAVE_CONFIG_H=1', '-DYAML_DECLARE_STATIC=1',
    '-I', (Join-Path $root 'vendor/libyaml/include'),
    '-I', (Join-Path $root 'vendor/libyaml/src'))
# Shared with native macOS build; paths in manifest are relative to native root.
function Get-BuildSources([string]$Kind) {
    Get-Content -LiteralPath (Join-Path $root 'build-sources.txt') | ForEach-Object {
        if ($_.StartsWith("$Kind ")) { Join-Path $root ($_.Substring($Kind.Length + 1)) }
    }
}
$arguments += @(Get-BuildSources 'core')
if ($Test) { $arguments += Join-Path $root 'test_core.c' }
elseif ($ViewerTest) { $arguments += Join-Path $root 'test_viewer.c' }
elseif ($ExplorerTest) { $arguments += @('-DGD_EXPLORER_TEST=1', (Join-Path $root 'test_explorer_view.c')) }
else { $arguments += Join-Path $root 'gd_cli.c' }
$arguments += @(Get-BuildSources 'yaml')
$arguments += @('-l', 'shell32', '-l', 'advapi32')
$name = if ($Test) { 'test-core.exe' } elseif ($ViewerTest) { 'test-viewer.exe' } else { 'gd.exe' }
if ($View) {
    if (-not $RaylibDirectory) { throw 'View needs -RaylibDirectory PATH or RAYLIB_ROOT (raylib 6.0 win64 SDK). Headless build needs no raylib.' }
    $library = Join-Path $RaylibDirectory 'lib/raylib.dll'
    if (-not (Test-Path -LiteralPath $library -PathType Leaf)) { throw "Missing raylib 6.0 DLL: $library" }
    $header = Join-Path $RaylibDirectory 'include/raylib.h'
    if (-not (Test-Path -LiteralPath $header -PathType Leaf) -or -not (Select-String -LiteralPath $header -Pattern '^#define RAYLIB_VERSION\s+"6\.0"' -Quiet)) { throw 'Supply matching raylib 6.0 SDK headers and libraries; do not reuse 5.5 binaries.' }
    $arguments += @('-DGD_WITH_VIEW=1', '-I', (Join-Path $root 'vendor/raylib'))
    $arguments += @(Get-BuildSources 'view')
    $arguments += (Resolve-Path -LiteralPath $library).Path
    Copy-Item -LiteralPath $library -Destination (Join-Path $output 'raylib.dll') -Force
    if ($Graph) {
        if (-not $UiLibrary) { $UiLibrary = Join-Path $root 'build/gd-ui.dll' }
        if (-not (Test-Path -LiteralPath $UiLibrary -PathType Leaf)) { throw 'Graph view needs prepared gd-ui.dll. Run build-ui.ps1 explicitly first.' }
        $arguments += @('-DGD_WITH_GRAPH=1', '-DNO_FONT_AWESOME=1', '-I', (Join-Path $root 'vendor/cimgui'), '-I', (Join-Path $root 'vendor/rlImGui'))
        $arguments += @(Get-BuildSources 'graph')
        $arguments += (Resolve-Path -LiteralPath $UiLibrary).Path
        $uiDestination = Join-Path $output (Split-Path $UiLibrary -Leaf)
        if ([IO.Path]::GetFullPath($UiLibrary) -ne [IO.Path]::GetFullPath($uiDestination)) { Copy-Item -LiteralPath $UiLibrary -Destination $uiDestination -Force }
    }
    $name = if ($ExplorerTest) { 'test-explorer-view.exe' } else { 'gd-view.exe' }
}
$destination = Join-Path $output $name
$arguments += @('-o', $destination)
& $Cjit @arguments
if ($LASTEXITCODE -ne 0) { throw "CJIT build failed with exit $LASTEXITCODE" }
Write-Output $destination
