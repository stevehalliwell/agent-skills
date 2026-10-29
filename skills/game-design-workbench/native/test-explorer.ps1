<#
Verify unified explorer with private copies of both examples. No OS input posted,
source-record writes, downloads, prompts, or existing-window manipulation.
Usage: ./test-explorer.ps1 -RaylibDirectory PATH [-UiLibrary PATH] [-Cjit cjit]
Creates ignored build/explorer-checks-GUID; retains logs, JSON and screenshots.
App-local checks prove state transitions/rendering, not real-pointer acceptance.
#>
param(
    [string]$RaylibDirectory = $env:RAYLIB_ROOT,
    [string]$UiLibrary = '',
    [string]$Cjit = 'cjit',
    [ValidateRange(5, 300)][int]$TimeoutSeconds = 60
)
$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
if (-not $RaylibDirectory -or -not (Test-Path -LiteralPath (Join-Path $RaylibDirectory 'lib/raylib.dll') -PathType Leaf)) {
    throw 'Supply prepared raylib 5.5 SDK with -RaylibDirectory; no download/install attempted.'
}
$examples = Join-Path (Split-Path $root -Parent) 'examples'
$sourceFiles = @(Get-ChildItem -LiteralPath $examples -Recurse -File | Sort-Object FullName | ForEach-Object {
    [pscustomobject]@{ Path = $_.FullName; Hash = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash }
})
$run = Join-Path $root ('build/explorer-checks-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $run | Out-Null
$nativeSources = @(Get-ChildItem -LiteralPath $root -File | Where-Object { $_.Extension -in @('.c', '.h', '.ps1') } | Sort-Object FullName | ForEach-Object {
    [pscustomobject]@{ Path = $_.FullName; Hash = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash }
})
$nativeSources | ConvertTo-Json -Depth 3 | Set-Content -LiteralPath (Join-Path $run 'source-hashes.json') -Encoding UTF8
$build = @{ Graph = $true; RaylibDirectory = $RaylibDirectory; Cjit = $Cjit; OutputDirectory = $run }
if ($UiLibrary) { $build.UiLibrary = $UiLibrary }
& (Join-Path $root 'build.ps1') @build | Out-Host
& (Join-Path $root 'build.ps1') @build -ExplorerTest | Out-Host
function Invoke-OwnedProbe([string]$Executable, [string[]]$Arguments, [string]$Log) {
    # Windows argv: input paths cannot contain quotes; quote every argument.
    if (@($Arguments | Where-Object { $_.Contains('"') }).Count) { throw 'Probe argument contains unsupported quote.' }
    $quoted = @($Arguments | ForEach-Object { '"' + $_ + '"' })
    $process = Start-Process -FilePath $Executable -ArgumentList $quoted -PassThru -RedirectStandardOutput "$Log.json" -RedirectStandardError "$Log.stderr.log"
    try {
        if (-not $process.WaitForExit($TimeoutSeconds * 1000)) { $process.Kill($true); $process.WaitForExit(); throw "Owned probe timed out: $Log" }
        $process.WaitForExit()
        if ($process.ExitCode -ne 0) { throw "Owned probe failed with exit $($process.ExitCode): $Log.stderr.log" }
    } finally { $process.Dispose() }
}
$results = @()
try {
    foreach ($example in @('crate-puzzle', 'little-plumber-guy-nes')) {
        $fixture = Join-Path $run $example
        New-Item -ItemType Directory -Path $fixture | Out-Null
        Copy-Item -LiteralPath (Join-Path $examples "$example/game-design") -Destination $fixture -Recurse
        $game = Join-Path $fixture 'game-design'
        $fixtureSources = @(Get-ChildItem -LiteralPath $game -Recurse -File | ForEach-Object {
            [pscustomobject]@{ Path = $_.FullName; Hash = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash }
        })
        Invoke-OwnedProbe (Join-Path $run 'test-explorer-view.exe') @($game, $fixture) (Join-Path $fixture 'state-checks')
        $checks = Get-Content -LiteralPath (Join-Path $fixture 'state-checks.json') -Raw | ConvertFrom-Json
        $warning = Get-Content -LiteralPath (Join-Path $fixture 'state-checks.stderr.log') -Raw
        if (-not $warning.Contains('Explorer structured stdout: stderr warning probe')) { throw 'Warning did not reach stderr' }
        if (-not $checks.ok -or $checks.allocations_remaining) { throw "State/leak checks failed: $fixture" }
        $inputs = @()
        foreach ($name in @('skill-atoms', 'challenges', 'nouns', 'verbs', 'interactions', 'mechanics', 'systems', 'dynamics', 'aesthetics', 'axes', 'resources', 'control-vars')) {
            $inputs += @("--$name", "$name=$(Join-Path $game $name)")
        }
        foreach ($mode in @('skills', 'challenges', 'relationships', 'matrix', 'mda', 'resources')) {
            $capture = Join-Path $fixture "$mode.png"
            $log = Join-Path $fixture "$mode-draft"
            Invoke-OwnedProbe (Join-Path $run 'gd-view.exe') (@('view', 'explorer') + $inputs + @('--mode', $mode, '--frames', '4', '--screenshot', $capture)) $log
            $draft = Get-Content -LiteralPath "$log.json" -Raw | ConvertFrom-Json
            if (-not $draft.ok -or $draft.data.changes.Count -ne 0 -or -not (Test-Path -LiteralPath $capture)) { throw "Capture/draft check failed: $mode" }
        }
        Invoke-OwnedProbe (Join-Path $run 'gd-view.exe') (@('view', 'explorer') + $inputs + @('--mode', 'matrix', '--layout', 'grid', '--frames', '4', '--screenshot', (Join-Path $fixture 'matrix-grid.png'))) (Join-Path $fixture 'matrix-grid-draft')
        $gridDraft = Get-Content -LiteralPath (Join-Path $fixture 'matrix-grid-draft.json') -Raw | ConvertFrom-Json
        if (-not $gridDraft.ok -or $gridDraft.data.changes.Count -ne 0) { throw 'Grid capture changed draft' }
        if (@(Get-ChildItem -LiteralPath $game -Recurse -File).Count -ne $fixtureSources.Count) { throw "Private fixture inventory changed: $game" }
        foreach ($file in $fixtureSources) {
            if ((Get-FileHash -LiteralPath $file.Path -Algorithm SHA256).Hash -ne $file.Hash) { throw "Private fixture source changed: $($file.Path)" }
        }
        $results += [pscustomobject]@{ example = $example; stateChecks = $checks; productionCaptures = 7; directory = $fixture }
    }
} finally {
    $after = @(Get-ChildItem -LiteralPath $examples -Recurse -File | Sort-Object FullName)
    if ($after.Count -ne $sourceFiles.Count) { throw 'Example source file inventory changed.' }
    foreach ($file in ($sourceFiles + $nativeSources)) {
        if ((Get-FileHash -LiteralPath $file.Path -Algorithm SHA256).Hash -ne $file.Hash) { throw "Source changed during checks: $($file.Path)" }
    }
}
[pscustomobject]@{ ok = $true; directory = $run; examplesUnchanged = $true; nativeSourcesUnchanged = $true; sourceHashes = (Join-Path $run 'source-hashes.json'); results = $results; realPointerAcceptance = 'not tested' } | ConvertTo-Json -Depth 8
