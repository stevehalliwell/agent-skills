<#
Build Windows/CJIT game-design tools. No downloads, installations, or project-data writes.
Usage: ./build.ps1 [-View | -Test | -ViewerTest | -ExplorerTest | -MatrixTest] [-UiLibrary PATH] [-Cjit PATH] [-OutputDirectory PATH]
-View (also -Graph) builds all windows. View, ExplorerTest and MatrixTest require matching
prepared gd-native-ui.dll and fresh output. Stdout returns executable path.
-ViewerTest builds headless protocol/transport fixture; does not launch it.
#>
param(
    [switch]$View,
    [switch]$Graph,
    [switch]$Test,
    [switch]$ViewerTest,
    [switch]$ExplorerTest,
    [switch]$MatrixTest,
    [string]$UiLibrary = '',
    [string]$Cjit = 'cjit',
    [string]$OutputDirectory = ''
)
$ErrorActionPreference = 'Stop'
if (@($Test,$ViewerTest,$ExplorerTest,$MatrixTest).Where({$_}).Count -gt 1 -or (($Test -or $ViewerTest) -and ($View -or $Graph))) { throw 'Choose one build target.' }
if ($Graph -or $ExplorerTest -or $MatrixTest) { $View = $true }
$root = $PSScriptRoot
# Alternate output keeps a user's running explorer/draft untouched.
$output = if ($OutputDirectory) { [IO.Path]::GetFullPath($OutputDirectory) } else { Join-Path $root 'build' }
if ($View) {
    if (-not $UiLibrary) { $UiLibrary = Join-Path $root 'build/native-ui/gd-native-ui.dll' }
    if (-not (Test-Path -LiteralPath $UiLibrary -PathType Leaf) -or (Split-Path $UiLibrary -Leaf) -ne 'gd-native-ui.dll') { throw 'Supply matching gd-native-ui.dll from build-native-ui.ps1 with -UiLibrary.' }
    if ($root -match '[^\x00-\x7F]' -or $output -match '[^\x00-\x7F]') { throw 'Installed CJIT requires ASCII build source/output paths.' }
    if (-not $OutputDirectory) { throw 'View needs fresh -OutputDirectory to preserve running artifacts.' }
    if ((Test-Path -LiteralPath $output) -and @(Get-ChildItem -LiteralPath $output -Force).Count) { throw 'View output must be new/empty; existing artifacts retained.' }
}
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
elseif ($MatrixTest) { $arguments += Join-Path $root 'test_native_matrix.c' }
else { $arguments += Join-Path $root 'gd_cli.c' }
$arguments += @(Get-BuildSources 'yaml')
$arguments += @('-l', 'shell32', '-l', 'advapi32')
$name = if ($Test) { 'test-core.exe' } elseif ($ViewerTest) { 'test-viewer.exe' } else { 'gd.exe' }
if ($View) {
    $library=Join-Path $output 'gd-native-ui.dll'
    Copy-Item -LiteralPath $UiLibrary -Destination $library
    $arguments += @('-DGD_WITH_VIEW=1','-DGD_WITH_GRAPH=1','-I',(Join-Path $root 'vendor/cimgui'))
    if ($MatrixTest) { $arguments += Join-Path $root 'gd_native_canvas.c' }
    else { $arguments += @(Get-BuildSources 'view') + @(Get-BuildSources 'graph') }
    $arguments += @((Join-Path $root 'gd_native_window.c'),$library,(Join-Path ([Environment]::GetFolderPath('System')) 'user32.dll'))
    $name = if ($ExplorerTest) { 'test-explorer-view.exe' } elseif ($MatrixTest) { 'test-native-matrix.exe' } else { 'gd-view.exe' }
}
$destination = Join-Path $output $name
$arguments += @('-o', $destination)
& $Cjit @arguments
if ($LASTEXITCODE -ne 0) { throw "CJIT build failed with exit $LASTEXITCODE" }
Write-Output $destination
