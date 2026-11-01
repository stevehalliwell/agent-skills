<#
.SYNOPSIS
Build and measure existing shared force solver with deterministic headless fixtures.
.DESCRIPTION
CJIT only. No windows, source-data edits or production solver changes. JSON stdout
contains exact synthetic footprint-overlap counts and bounded-size centerline
crossing counts, plus CPU step timing (measurement loops excluded). Output executable
uses ignored native/build/force-baseline by default, not running viewer directory.
.EXAMPLE
./scripts/measure-force-layout.ps1 -Steps 120 > baseline.json
#>
[CmdletBinding()]
param(
    [ValidateRange(1, 600)][int]$Steps = 120,
    [string]$Cjit = 'cjit',
    [string]$OutputDirectory = ''
)
$ErrorActionPreference = 'Stop'
$native = Join-Path $PSScriptRoot '../skills/game-design-workbench/native'
$native = [IO.Path]::GetFullPath($native)
$output = if ($OutputDirectory) { [IO.Path]::GetFullPath($OutputDirectory) } else { Join-Path $native 'build/force-baseline' }
New-Item -ItemType Directory -Force $output | Out-Null
$executable = Join-Path $output 'test-layout-baseline.exe'
$arguments = @('-q', '-I', $native,
    (Join-Path $native 'gd_layout.c'),
    (Join-Path $native 'gd_layout_routes.c'),
    (Join-Path $native 'test_layout_baseline.c'),
    '-o', $executable)
& $Cjit @arguments
if ($LASTEXITCODE -ne 0) { throw "CJIT baseline build failed: $LASTEXITCODE" }
$json = & $executable $Steps
if ($LASTEXITCODE -ne 0) { throw "Baseline checks failed: $LASTEXITCODE" }
$result = ($json -join "`n") | ConvertFrom-Json
if (-not $result.ok -or $result.fixtures.Count -ne 10) { throw 'Invalid baseline result' }
$json
