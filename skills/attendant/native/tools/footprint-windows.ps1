<#
.SYNOPSIS
Measure native Windows GUI startup, idle footprint and control round trips.
.DESCRIPTION
Usage: pwsh -File tools/footprint-windows.ps1 [-Seconds 10]
Build both production GUIs first. Honors ATTENDANT_GUI_VARIANT; default output
is build/gui. Creates owned fixtures with real product docs; never edits live
projects. Opens one visible viewer at a time. JSON stdout/report, errors stderr,
exit 0 on success, 1 on failure. No numeric performance pass/fail budget.
Startup ends at successful inspection, not first painted frame. Round trips
include control-process startup; reveal acknowledges receipt, not completed Go.
This is a small-workspace sanity probe, not human input or macOS qualification.
#>
[CmdletBinding()]
param([ValidateRange(3, 120)][int]$Seconds = 10)
$ErrorActionPreference = 'Stop'
$viewer = $null
try {
    if (-not $IsWindows) { throw 'Windows only; run with PowerShell 7 on Windows.' }
    $root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
    $variant = $env:ATTENDANT_GUI_VARIANT
    if ($variant -and $variant -notmatch '^[a-z0-9][a-z0-9-]*$') { throw 'Invalid ATTENDANT_GUI_VARIANT.' }
    $folder = 'gui'
    if ($variant) { $folder += "-$variant" }
    $assets = Join-Path $root 'build/deps/richmd/assets'
    foreach ($compiler in @('cjit', 'zig')) {
        if (-not (Test-Path (Join-Path $root "build/$folder/attendant-gui-$compiler.exe"))) {
            throw "Build production GUI for $compiler first; see references/build.md."
        }
    }
    if (-not (Test-Path $assets)) { throw 'Missing RichMd assets; see references/build.md.' }
    $run = Join-Path $root ('build/footprint/run-' + [guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $run -Force | Out-Null
    $project = Join-Path $run 'workspace'
    Copy-Item (Join-Path $root 'tests/fixtures/project') $project -Recurse
    Copy-Item (Join-Path $root 'README.md') (Join-Path $project 'README.md')
    Copy-Item (Join-Path $root '../references/build.md') (Join-Path $project 'build.md')
    function Inventory {
        @(Get-ChildItem $project -Recurse -File -Force | Sort-Object FullName | ForEach-Object {
            [ordered]@{ path = [IO.Path]::GetRelativePath($project, $_.FullName); bytes = $_.Length; sha256 = (Get-FileHash $_.FullName -Algorithm SHA256).Hash }
        })
    }
    $before = Inventory
    function Start-Child([string]$Executable, [string[]]$Arguments) {
        $info = [Diagnostics.ProcessStartInfo]::new($Executable)
        $info.UseShellExecute = $false
        $info.RedirectStandardOutput = $true
        $info.RedirectStandardError = $true
        foreach ($argument in $Arguments) { $info.ArgumentList.Add($argument) }
        $process = [Diagnostics.Process]::new()
        $process.StartInfo = $info
        if (-not $process.Start()) { throw "Cannot start $Executable" }
        return $process
    }
    function Control([string[]]$Arguments) {
        $watch = [Diagnostics.Stopwatch]::StartNew()
        $child = Start-Child $executable (@('--project', $project) + $Arguments)
        try {
            $output = $child.StandardOutput.ReadToEndAsync()
            $errors = $child.StandardError.ReadToEndAsync()
            if (-not $child.WaitForExit(5000)) { $child.Kill(); throw 'GUI control timed out after 5000 ms.' }
            $watch.Stop()
            if ($child.ExitCode -ne 0) { throw "GUI control failed: $($errors.GetAwaiter().GetResult())" }
            $data = $output.GetAwaiter().GetResult() | ConvertFrom-Json
            [ordered]@{ milliseconds = $watch.Elapsed.TotalMilliseconds; response = $data }
        } finally { $child.Dispose() }
    }
    $results = @()
    foreach ($compiler in @('cjit', 'zig')) {
        $executable = Join-Path $root "build/$folder/attendant-gui-$compiler.exe"
        $watch = [Diagnostics.Stopwatch]::StartNew()
        $viewer = Start-Child $executable @('--project', $project, '--assets', $assets, '--file', 'README.md')
        $viewerOutput = $viewer.StandardOutput.ReadToEndAsync()
        $viewerErrors = $viewer.StandardError.ReadToEndAsync()
        try {
            do {
                if ($viewer.HasExited) { throw "Viewer exited during startup: $($viewerErrors.GetAwaiter().GetResult())" }
                if ($watch.Elapsed.TotalSeconds -gt 15) { throw 'Viewer startup exceeded 15 seconds.' }
                Start-Sleep -Milliseconds 20
                $viewer.Refresh()
            } while ($viewer.MainWindowHandle -eq 0)
            $ready = Control @('--inspect')
            $startup = $watch.Elapsed.TotalMilliseconds
            Start-Sleep -Seconds 2
            $viewer.Refresh()
            $cpu = $viewer.TotalProcessorTime.TotalMilliseconds
            $idleWatch = [Diagnostics.Stopwatch]::StartNew()
            $samples = @()
            for ($i = 0; $i -lt $Seconds; $i++) {
                Start-Sleep -Seconds 1
                $viewer.Refresh()
                if ($viewer.HasExited) { throw 'Viewer exited during idle sampling.' }
                $samples += [ordered]@{ working_set_bytes = $viewer.WorkingSet64; private_bytes = $viewer.PrivateMemorySize64 }
            }
            $elapsed = $idleWatch.Elapsed.TotalMilliseconds
            $cpuUsed = $viewer.TotalProcessorTime.TotalMilliseconds - $cpu
            $inspect = @(); $reveal = @()
            for ($i = 0; $i -lt 5; $i++) {
                $reveal += (Control @('--reveal', $(if ($i % 2) { 'README.md' } else { 'build.md' }))).milliseconds
                $inspect += (Control @('--inspect')).milliseconds
            }
            $results += [ordered]@{
                compiler = $compiler; executable_sha256 = (Get-FileHash $executable -Algorithm SHA256).Hash
                startup_to_inspection_ms = $startup
                idle_seconds = $elapsed / 1000; idle_cpu_ms = $cpuUsed
                idle_cpu_percent_one_core = 100 * $cpuUsed / $elapsed
                idle_samples = $samples; inspect_round_trip_ms = $inspect; reveal_round_trip_ms = $reveal
            }
            if (-not $viewer.CloseMainWindow()) { throw 'Cannot request owned viewer shutdown.' }
            if (-not $viewer.WaitForExit(5000)) { throw 'Owned viewer did not close within 5000 ms.' }
            if ($viewer.ExitCode -ne 0) { throw "Viewer failed: $($viewerErrors.GetAwaiter().GetResult())" }
            $results[-1].completion = $viewerOutput.GetAwaiter().GetResult() | ConvertFrom-Json
        } finally {
            if (-not $viewer.HasExited) { $viewer.Kill(); $viewer.WaitForExit() }
            $viewer.Dispose(); $viewer = $null
        }
    }
    if (($before | ConvertTo-Json -Depth 5 -Compress) -ne ((Inventory) | ConvertTo-Json -Depth 5 -Compress)) {
        throw 'Workspace inventory changed during footprint probe.'
    }
    $report = [ordered]@{
        ok = $true; platform = 'win32'; timestamp_utc = [DateTime]::UtcNow.ToString('o')
        logical_processors = [Environment]::ProcessorCount; run = $run
        workload = 'Small test collections plus current native README and build reference; README initially open.'
        limits = 'Warm sequential single run/compiler. Startup ends at IPC inspection, not first paint. CPU percent uses one logical core. No GPU measurement. Control round trips are not human input/Go completion latency. No numeric acceptance budget.'
        source_unchanged = $true; files = $before; results = $results
    }
    $json = $report | ConvertTo-Json -Depth 12
    [IO.File]::WriteAllText((Join-Path $run 'results.json'), $json + "`n", [Text.UTF8Encoding]::new($false))
    $json
} catch {
    [Console]::Error.WriteLine($_.Exception.Message)
    @{ ok = $false; error = $_.Exception.Message } | ConvertTo-Json -Compress
    exit 1
}
