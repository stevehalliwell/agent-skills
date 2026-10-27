<#
Build Windows/CJIT game-design tools. No downloads, installations, or project-data writes.
Usage: ./build.ps1 [-View [-Graph] -RaylibDirectory PATH | -Test] [-Cjit PATH] [-OutputDirectory PATH]
Alternate output preserves running views. Stdout returns executable path; no project-data writes.
-Graph needs explicitly prepared gd-ui.dll from build-ui.ps1.
RaylibDirectory is an extracted raylib 5.5 win64 SDK with lib/raylib.dll.
#>
param(
    [switch]$View,
    [switch]$Graph,
    [switch]$Test,
    [string]$UiLibrary = '',
    [string]$RaylibDirectory = $env:RAYLIB_ROOT,
    [string]$Cjit = 'cjit',
    [string]$OutputDirectory = ''
)
$ErrorActionPreference = 'Stop'
if ($Graph) { $View = $true }
if ($View -and $Test) { throw 'Choose -View or -Test, not both.' }
$root = $PSScriptRoot
# Alternate output keeps a user's running explorer/draft untouched.
$output = if ($OutputDirectory) { [IO.Path]::GetFullPath($OutputDirectory) } else { Join-Path $root 'build' }
New-Item -ItemType Directory -Force $output | Out-Null
$arguments = @('-q', '-DHAVE_CONFIG_H=1', '-DYAML_DECLARE_STATIC=1',
    '-I', (Join-Path $root 'vendor/libyaml/include'),
    '-I', (Join-Path $root 'vendor/libyaml/src'),
    (Join-Path $root 'gd_core.c'), (Join-Path $root 'gd_output.c'), (Join-Path $root 'gd_layout.c'), (Join-Path $root 'gd_focus.c'), (Join-Path $root 'gd_diagnostics.c'), (Join-Path $root 'gd_skills.c'), (Join-Path $root 'gd_draft.c'), (Join-Path $root 'gd_routes.c'), (Join-Path $root 'gd_mda.c'), (Join-Path $root 'gd_controls.c'), (Join-Path $root 'gd_control_import.c'), (Join-Path $root 'gd_rules.c'), (Join-Path $root 'gd_rule_axes.c'), (Join-Path $root 'gd_rule_logic.c'), (Join-Path $root 'gd_rule_quantities.c'), (Join-Path $root 'gd_rule_analysis.c'))
if ($Test) { $arguments += Join-Path $root 'test_core.c' }
else { $arguments += Join-Path $root 'gd_cli.c' }
foreach ($file in @('api.c', 'parser.c', 'reader.c', 'scanner.c')) {
    $arguments += Join-Path $root "vendor/libyaml/src/$file"
}
$arguments += @('-l', 'shell32')
$name = if ($Test) { 'test-core.exe' } else { 'gd.exe' }
if ($View) {
    if (-not $RaylibDirectory) { throw 'View needs -RaylibDirectory PATH or RAYLIB_ROOT (raylib 5.5 win64 SDK). Headless build needs no raylib.' }
    $library = Join-Path $RaylibDirectory 'lib/raylib.dll'
    if (-not (Test-Path -LiteralPath $library -PathType Leaf)) { throw "Missing raylib 5.5 DLL: $library" }
    $arguments += @('-DGD_WITH_VIEW=1', '-I', (Join-Path $root 'vendor/raylib'), (Join-Path $root 'gd_view.c'), (Resolve-Path -LiteralPath $library).Path)
    Copy-Item -LiteralPath $library -Destination (Join-Path $output 'raylib.dll') -Force
    if ($Graph) {
        if (-not $UiLibrary) { $UiLibrary = Join-Path $root 'build/gd-ui.dll' }
        if (-not (Test-Path -LiteralPath $UiLibrary -PathType Leaf)) { throw 'Graph view needs prepared gd-ui.dll. Run build-ui.ps1 explicitly first.' }
        $arguments += @('-DGD_WITH_GRAPH=1', '-DNO_FONT_AWESOME=1', '-I', (Join-Path $root 'vendor/cimgui'), '-I', (Join-Path $root 'vendor/rlImGui'), (Join-Path $root 'gd_graph_view.c'), (Join-Path $root 'gd_mda_view.c'), (Join-Path $root 'gd_diagnostics_ui.c'), (Resolve-Path -LiteralPath $UiLibrary).Path)
        $uiDestination = Join-Path $output (Split-Path $UiLibrary -Leaf)
        if ([IO.Path]::GetFullPath($UiLibrary) -ne [IO.Path]::GetFullPath($uiDestination)) { Copy-Item -LiteralPath $UiLibrary -Destination $uiDestination -Force }
    }
    $name = 'gd-view.exe'
}
$destination = Join-Path $output $name
$arguments += @('-o', $destination)
& $Cjit @arguments
if ($LASTEXITCODE -ne 0) { throw "CJIT build failed with exit $LASTEXITCODE" }
Write-Output $destination
