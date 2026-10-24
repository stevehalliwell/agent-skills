<# Engineering checks and compile-only route model. NEVER runs experiment trials.
Requires PowerShell 7, installed CJIT and/or Zig. No downloads/installations.
Usage: pwsh -NoProfile -File ./test-experiment.ps1 [-Compiler Both|CJIT|Zig]
Outputs stay under ignored native/build/experiment-checks/. Existing viewers untouched.
Each compiler process has 30-second watchdog; helper checks have same watchdog.
#>
param([ValidateSet('Both', 'CJIT', 'Zig')][string]$Compiler = 'Both',
    [string]$Cjit = 'cjit', [string]$Zig = 'zig')
$ErrorActionPreference = 'Stop'
if ($PSVersionTable.PSVersion.Major -lt 7) { throw 'Requires PowerShell 7 for exact process argument handling.' }
$root = $PSScriptRoot
$output = Join-Path $root 'build/experiment-checks'
New-Item -ItemType Directory -Force $output | Out-Null
$fixture = Join-Path $root '../examples/crate-puzzle/experiments/route-cost/frozen-input.json'
$model = Join-Path $root '../examples/crate-puzzle/experiments/route-cost/route_timing.c'
$common = @('-DHAVE_CONFIG_H=1', '-DYAML_DECLARE_STATIC=1', '-I', $root,
    '-I', (Join-Path $root 'vendor/libyaml/include'), '-I', (Join-Path $root 'vendor/libyaml/src'),
    (Join-Path $root 'gd_experiment.c'), (Join-Path $root 'vendor/pcg-basic/pcg_basic.c'))
foreach ($file in @('api.c', 'parser.c', 'reader.c', 'scanner.c')) { $common += Join-Path $root "vendor/libyaml/src/$file" }
function Invoke-Bounded([string]$Program, [string[]]$Arguments, [string]$Log) {
    $start = [Diagnostics.ProcessStartInfo]::new()
    $start.FileName = (Get-Command $Program -ErrorAction Stop).Source
    $start.UseShellExecute = $false
    $start.RedirectStandardOutput = $true
    $start.RedirectStandardError = $true
    $start.Environment['ZIG_GLOBAL_CACHE_DIR'] = Join-Path $root 'build/zig-global'
    $start.Environment['ZIG_LOCAL_CACHE_DIR'] = Join-Path $root 'build/zig-local'
    foreach ($argument in $Arguments) { $start.ArgumentList.Add($argument) }
    $process = [Diagnostics.Process]::new()
    $process.StartInfo = $start
    try {
        if (-not $process.Start()) { throw "Cannot start $Program" }
        $stdout = $process.StandardOutput.ReadToEndAsync()
        $stderr = $process.StandardError.ReadToEndAsync()
        $finished = $process.WaitForExit(30000)
        if (-not $finished) {
            $process.Kill($true)
            $process.WaitForExit()
        }
        $out = $stdout.GetAwaiter().GetResult()
        $err = $stderr.GetAwaiter().GetResult()
        [IO.File]::WriteAllText("$Log.stdout.txt", $out)
        [IO.File]::WriteAllText("$Log.stderr.txt", $err)
        if (-not $finished) { throw "30-second engineering process limit: $Program; logs: $Log" }
        if ($process.ExitCode -ne 0) { throw "Engineering process failed ($($process.ExitCode)); inspect $Log.stderr.txt" }
        return $out
    } finally { $process.Dispose() }
}
$reports = @()
foreach ($kind in @('CJIT', 'Zig')) {
    if ($Compiler -ne 'Both' -and $Compiler -ne $kind) { continue }
    $program = if ($kind -eq 'CJIT') { $Cjit } else { $Zig }
    [string[]]$flags = if ($kind -eq 'CJIT') { @('-q') } else {
        @('cc', '-std=c99', '-Wall', '-Wextra', '-Werror')
    }
    $suffix = $kind.ToLowerInvariant()
    [string[]]$versionArguments = if ($kind -eq 'CJIT') { @('-h') } else { @('version') }
    $versionLog = Join-Path $output "version-$suffix"
    $versionOutput = Invoke-Bounded $program $versionArguments $versionLog
    $versionError = [IO.File]::ReadAllText("$versionLog.stderr.txt")
    $version = (($versionOutput + $versionError).Trim() -split "`r?`n")[0]
    $test = Join-Path $output "test-experiment-$suffix.exe"
    $modelOutput = Join-Path $output "route-timing-$suffix.exe"
    $testArguments = $flags + $common + @((Join-Path $root 'test_experiment.c'), '-o', $test)
    $modelArguments = $flags + $common + @($model, '-o', $modelOutput)
    Invoke-Bounded $program $testArguments (Join-Path $output "compile-test-$suffix") | Out-Null
    Invoke-Bounded $program $modelArguments (Join-Path $output "compile-model-$suffix") | Out-Null
    $infrastructureBuilds = @()
    foreach ($tool in @('result', 'run', 'emitter', 'result-test')) {
        $entry = switch ($tool) { 'result' { 'gd_experiment_result_cli.c' }; 'run' { 'gd_experiment_run.c' }; 'emitter' { 'test_experiment_emitter.c' }; 'result-test' { 'test_experiment_result.c' } }
        $toolOutput = Join-Path $output "gd-experiment-$tool-$suffix.exe"
        $toolArguments = $flags + $common + @((Join-Path $root 'gd_experiment_result.c'), (Join-Path $root $entry), '-o', $toolOutput)
        Invoke-Bounded $program $toolArguments (Join-Path $output "compile-$tool-$suffix") | Out-Null
        $infrastructureBuilds += [ordered]@{ tool = $tool; arguments = $toolArguments }
    }
    $result = Invoke-Bounded $test @($fixture) (Join-Path $output "helpers-$suffix") | ConvertFrom-Json
    $resultHelpers = Invoke-Bounded (Join-Path $output "gd-experiment-result-test-$suffix.exe") @($fixture) (Join-Path $output "result-helpers-$suffix") | ConvertFrom-Json
    if ($resultHelpers.experiment_trials_executed -ne 0) { throw 'Result helper checks must not run model trials.' }
    if ($result.experiment_trials_executed -ne 0) { throw 'Engineering checks must not run route trials.' }
    $reports += [ordered]@{ compiler = $kind; helper_checks = $result.engineering_checks; result_helper_checks = $resultHelpers.engineering_checks;
        experiment_trials_executed = 0; model_compiled_only = $true;
        executable = $modelOutput; compiler_program = (Get-Command $program).Source; compiler_version = $version;
        test_build_arguments = $testArguments; model_build_arguments = $modelArguments; infrastructure_builds = $infrastructureBuilds }
}
$reports | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $output 'engineering-builds.json') -Encoding utf8
$reports | ForEach-Object { Write-Output "$($_.compiler): $($_.helper_checks) input/random checks + $($_.result_helper_checks) result helper checks; model compiled only; 0 experiment trials" }
