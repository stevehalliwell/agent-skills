<# Synthetic infrastructure tests only. NEVER executes route model/trials.
Requires PowerShell 7. Builds explicit installed CJIT/Zig tools; no downloads.
Usage: pwsh -NoProfile -File ./test-experiment-supervision.ps1 [-Compiler Both|CJIT|Zig]
Artifacts: ignored build/synthetic-<id>/. No Git/checkpoints or canonical writes.
#>
param([ValidateSet('Both','CJIT','Zig')][string]$Compiler = 'Both')
$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'test-experiment.ps1') -Compiler $Compiler | Out-Host
$build = Join-Path $PSScriptRoot 'build/experiment-checks'
$folder = Join-Path $PSScriptRoot ('build/synthetic-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $folder | Out-Null
$checkpoint = '0123456789012345678901234567890123456789'
$buildCommand = 'compiler "quoted argument" C:\path with spaces\'
$toolchain = 'engineering fixture toolchain'
$script:checks = 0
function Assert-Check([bool]$Condition, [string]$Message) {
    if (-not $Condition) { throw $Message }
    $script:checks++
}
function Write-Json([string]$Path, $Value) {
    [IO.File]::WriteAllText($Path, ($Value | ConvertTo-Json -Depth 50 -Compress) + "`n", [Text.UTF8Encoding]::new($false))
}
function Invoke-Tool([string]$Program, [string[]]$Arguments) {
    $start = [Diagnostics.ProcessStartInfo]::new()
    $start.FileName = $Program; $start.UseShellExecute = $false
    $start.RedirectStandardOutput = $true; $start.RedirectStandardError = $true
    foreach ($argument in $Arguments) { $start.ArgumentList.Add($argument) }
    $process = [Diagnostics.Process]::new(); $process.StartInfo = $start
    try {
        if (-not $process.Start()) { throw 'Cannot start engineering tool' }
        $stdout = $process.StandardOutput.ReadToEndAsync(); $stderr = $process.StandardError.ReadToEndAsync()
        if (-not $process.WaitForExit(10000)) { $process.Kill($true); $process.WaitForExit(1000) | Out-Null; throw 'Engineering outer watchdog exceeded 10 seconds' }
        $text = $stdout.GetAwaiter().GetResult(); $err = $stderr.GetAwaiter().GetResult()
        return @{ exit = $process.ExitCode; value = ($text | ConvertFrom-Json -AsHashtable); stdout = $text; stderr = $err }
    } finally { $process.Dispose() }
}
function New-Fixture([string]$Mode) {
    $measures = [ordered]@{}
    foreach ($type in @('number','integer','boolean','enum')) {
        $measures[$type] = [ordered]@{ identity = "engineering#$type"; value_type = $type; unit = $(if ($type -in @('boolean','enum')) { '' } else { 'test-unit' }); boundary = 'Synthetic fixture, not a game measurement' }
        if ($type -eq 'enum') { $measures[$type].allowed_values = @('a','b') }
    }
    return [ordered]@{
        schema_version = 1; model_id = 'engineering-fixture-v1'; recipe_id = 'engineering-only'; configuration_id = 'fixture'
        experiment = @{ id = 'engineering-only'; reference = 'engineering/fixture' }
        controls = @{}; assumptions = @{ synthetic = $true; unicode = '雪😀'; escaped = '\\ud800' }; measures = $measures
        random_generator = 'pcg32-xsh-rr-v1'; seed = '0x000000000000002a'; trials = 2
        trace_trial = $(if ($Mode -eq 'trace') { 0 } else { $null })
        limits = @{ steps_per_trial = 10; wall_seconds = $(if ($Mode -in @('hang','child-parent-exit')) { 0.2 } else { 2 }); output_bytes = 4096 }
        engineering_mode = $Mode
    }
}
foreach ($kind in @('CJIT','Zig')) {
    if ($Compiler -ne 'Both' -and $Compiler -ne $kind) { continue }
    $suffix = $kind.ToLowerInvariant()
    $supervisor = Join-Path $build "gd-experiment-run-$suffix.exe"
    $validator = Join-Path $build "gd-experiment-result-$suffix.exe"
    $emitter = Join-Path $build "gd-experiment-emitter-$suffix.exe"
    $space = Join-Path $folder "owned space $suffix"
    New-Item -ItemType Directory -Path $space | Out-Null
    $ownedEmitter = Join-Path $space 'synthetic emitter.exe'
    Copy-Item -LiteralPath $emitter -Destination $ownedEmitter
    $goodInput = $null; $goodOutput = $null
    foreach ($mode in @('complete','stdin','input-lock','trace','limit','error','preheader-error','hang','stdout-flood','stderr-flood','both-flood','crash','binary','child-parent-exit')) {
        $input = Join-Path $space "$mode.json"; $output = Join-Path $space "$mode-output"
        Write-Json $input (New-Fixture $mode)
        $before = [IO.File]::ReadAllBytes($input)
        $result = Invoke-Tool $supervisor @('--program',$ownedEmitter,'--input',$input,'--output',$output,'--checkpoint',$checkpoint,'--build',$buildCommand,'--toolchain',$toolchain)
        $value = $result.value
        Assert-Check (Test-Path -LiteralPath (Join-Path $output 'supervision.json')) "$kind $mode missing supervision metadata"
        $metadata = Get-Content -LiteralPath (Join-Path $output 'supervision.json') -Raw | ConvertFrom-Json -AsHashtable
        Assert-Check ($metadata.ok -eq $value.ok -and $metadata.supervision_status -eq $value.supervision_status) "$kind $mode metadata/stdout mismatch"
        Assert-Check ([Convert]::ToBase64String($before) -eq [Convert]::ToBase64String([IO.File]::ReadAllBytes((Join-Path $output 'input.json')))) "$kind $mode recorded input bytes changed"
        $bytes = (Get-Item -LiteralPath (Join-Path $output 'stdout.ndjson')).Length + (Get-Item -LiteralPath (Join-Path $output 'stderr.log')).Length
        Assert-Check ($bytes -le 4096 -and $bytes -eq $value.stdout_bytes + $value.stderr_bytes) "$kind $mode byte cap/accounting failed"
        Assert-Check ([Convert]::ToBase64String($before) -eq [Convert]::ToBase64String([IO.File]::ReadAllBytes($input))) "$kind $mode source input modified"
        if ($mode -in @('complete','stdin','input-lock','trace')) {
            Assert-Check ($result.exit -eq 0 -and $value.ok -and $value.validation.protocol_valid -and $value.validation.complete -and $value.validation.trials -eq 2) "$kind $mode rejected synthetic complete output: $($result.stdout)"
            if ($mode -eq 'trace') { Assert-Check ($value.validation.step_records -eq 2) "$kind trace count lost" }
            if ($mode -eq 'complete') {
                $goodInput = $input; $goodOutput = Join-Path $output 'stdout.ndjson'
                $cwd=[IO.File]::ReadAllText((Join-Path $output 'stderr.log'))
                Assert-Check ($cwd.Contains("engineering_cwd=$($value.working_directory)") -and $value.working_directory -eq $output) "$kind child working directory/provenance mismatch"
            }
        } elseif ($mode -eq 'limit') {
            Assert-Check ($result.exit -eq 3 -and -not $value.ok -and $value.validation.protocol_valid -and -not $value.validation.complete -and $value.validation.undefined_measurements -eq 1) "$kind modeled limit lost partial evidence"
        } elseif ($mode -eq 'error') {
            Assert-Check ($result.exit -eq 2 -and -not $value.ok -and $value.validation.protocol_valid -and -not $value.validation.complete -and $value.validation.undefined_measurements -eq 1) "$kind runtime error lost typed partial evidence"
        } elseif ($mode -in @('hang','child-parent-exit')) {
            Assert-Check ($result.exit -eq 3 -and $value.supervision_status -eq 'timeout' -and -not $value.validation.complete -and $value.elapsed_seconds -lt 3) "$kind $mode watchdog failed: $($result.stdout)"
            if ($mode -eq 'child-parent-exit') {
                $log = [IO.File]::ReadAllText((Join-Path $output 'stderr.log'))
                Assert-Check ($log -match 'engineering_child_pid=(\d+)') "$kind child fixture did not spawn"
                $childId = [int]$Matches[1]
                $child = Get-Process -Id $childId -ErrorAction SilentlyContinue
                if ($child) { $exited = $child.WaitForExit(1000); $child.Dispose(); Assert-Check $exited "$kind owned descendant survived watchdog" }
                else { Assert-Check $true "$kind owned child exited" }
            }
        } elseif ($mode -like '*flood') {
            Assert-Check ($result.exit -eq 3 -and $value.supervision_status -eq 'output-limit' -and -not $value.validation.complete -and $bytes -eq 4096) "$kind $mode cap failed"
        } else {
            Assert-Check ($result.exit -eq 2 -and -not $value.ok -and -not $value.validation.complete) "$kind $mode accepted malformed evidence"
            if ($mode -eq 'binary') { Assert-Check ([IO.File]::ReadAllBytes((Join-Path $output 'stdout.ndjson')) -contains 0) "$kind raw NUL was transcoded/lost" }
        }
    }
    $arguments = @('--input',$goodInput,'--results',$goodOutput,'--exit-code','0','--checkpoint',$checkpoint,'--build',$buildCommand,'--toolchain',$toolchain)
    $result = Invoke-Tool $validator $arguments
    Assert-Check ($result.exit -eq 0 -and $result.value.complete) "$kind read-only validator rejected valid fixture"
    $preflight = Invoke-Tool $validator @('--check-input',$goodInput)
    Assert-Check ($preflight.exit -eq 0 -and $preflight.value.input_valid) "$kind input preflight failed"
    $raw = [IO.File]::ReadAllText($goodOutput)
    # Each case starts from independent fixed synthetic records; no route model.
    $cases = @(
        @{name='missing-header'; change={ param($r) return @($r[1],$r[2],$r[3]) }},
        @{name='missing-completion'; change={ param($r) return @($r[0],$r[1],$r[2]) }},
        @{name='duplicate-trial'; change={ param($r) $r[2].trial=0; return $r }},
        @{name='out-of-range'; change={ param($r) $r[2].trial=2; return $r }},
        @{name='wrong-configuration'; change={ param($r) $r[1].configuration_id='other'; return $r }},
        @{name='wrong-protocol'; change={ param($r) $r[0].protocol='other'; return $r }},
        @{name='wrong-commit'; change={ param($r) $r[0].checkpoint_commit='f'*40; return $r }},
        @{name='wrong-build'; change={ param($r) $r[0].build='other'; return $r }},
        @{name='wrong-toolchain'; change={ param($r) $r[0].toolchain='other'; return $r }},
        @{name='wrong-dependency'; change={ param($r) $r[0].dependencies.libyaml='other'; return $r }},
        @{name='wrong-frozen-unit'; change={ param($r) $r[0].input.measures.number.unit='other'; return $r }},
        @{name='wrong-frozen-value'; change={ param($r) $r[0].input.assumptions.synthetic=$false; return $r }},
        @{name='unknown-measure'; change={ param($r) $r[1].measurements.extra=1; return $r }},
        @{name='missing-measure'; change={ param($r) [void]$r[1].measurements.Remove('number'); return $r }},
        @{name='number-as-string'; change={ param($r) $r[1].measurements.number='1.25'; return $r }},
        @{name='integer-fraction'; change={ param($r) $r[1].measurements.integer=1.5; return $r }},
        @{name='boolean-number'; change={ param($r) $r[1].measurements.boolean=1; return $r }},
        @{name='enum-choice'; change={ param($r) $r[1].measurements.enum='unknown'; return $r }},
        @{name='undefined-no-reason'; change={ param($r) $r[1].measurements.number=$null; return $r }},
        @{name='reason-for-defined'; change={ param($r) $r[1].undefined=@{number='unexpected'}; return $r }},
        @{name='wrong-count'; change={ param($r) $r[3].trials_completed=1; return $r }},
        @{name='wrong-partial'; change={ param($r) $r[3].partial_evidence=$true; return $r }},
        @{name='unknown-trial-status'; change={ param($r) $r[1].status='design-failed'; return $r }},
        @{name='error-downgraded-to-limit'; change={ param($r) $r[1].status='error'; $r[3].status='limit'; $r[3].trials_completed=1; $r[3].partial_evidence=$true; return $r }},
        @{name='wrong-frozen-owner'; change={ param($r) $r[0].input.experiment.id='other'; return $r }},
        @{name='wrong-frozen-seed'; change={ param($r) $r[0].input.seed='0x0000000000000000'; return $r }},
        @{name='trailing-record'; change={ param($r) return @($r[0],$r[1],$r[2],$r[3],$r[3]) }}
    )
    foreach ($case in $cases) {
        $r = @($raw.TrimEnd("`r","`n") -split "`n" | ForEach-Object { $_ | ConvertFrom-Json -AsHashtable })
        $modified = & $case.change $r
        $path = Join-Path $space ($case.name + '.ndjson')
        [IO.File]::WriteAllText($path, (($modified | ForEach-Object { $_ | ConvertTo-Json -Depth 50 -Compress }) -join "`n") + "`n", [Text.UTF8Encoding]::new($false))
        $bad = Invoke-Tool $validator @('--input',$goodInput,'--results',$path,'--exit-code','0','--checkpoint',$checkpoint,'--build',$buildCommand,'--toolchain',$toolchain)
        Assert-Check ($bad.exit -eq 2 -and -not $bad.value.complete -and -not $bad.value.protocol_valid) "$kind accepted $($case.name)"
    }
    foreach ($case in @(
        @{name='duplicate-key'; text=$raw.Replace('"trial":0','"trial":0,"trial":0')},
        @{name='nan'; text=$raw.Replace('"number":1.25','"number":NaN')},
        @{name='no-final-newline'; text=$raw.TrimEnd("`r","`n")},
        @{name='blank-line'; text=$raw + "`n"},
        @{name='non-json'; text='not json' + "`n"}
    )) {
        $path = Join-Path $space ($case.name + '.ndjson'); [IO.File]::WriteAllText($path,$case.text,[Text.UTF8Encoding]::new($false))
        $bad = Invoke-Tool $validator @('--input',$goodInput,'--results',$path,'--exit-code','0','--checkpoint',$checkpoint,'--build',$buildCommand,'--toolchain',$toolchain)
        Assert-Check ($bad.exit -eq 2 -and -not $bad.value.complete) "$kind accepted $($case.name)"
    }
    $traceInput=Join-Path $space 'trace.json'
    $traceRaw=[IO.File]::ReadAllText((Join-Path $space 'trace-output/stdout.ndjson'))
    foreach ($variant in @('repeated-step','wrong-trace-trial','over-step-limit','missing-trace','step-after-trial')) {
        $r=@($traceRaw.TrimEnd("`r","`n") -split "`n" | ForEach-Object { $_ | ConvertFrom-Json -AsHashtable })
        switch ($variant) {
            'repeated-step' { $r[2].step=1 }
            'wrong-trace-trial' { $r[1].trial=1 }
            'over-step-limit' { $r[1].step=11 }
            'missing-trace' { $r=@($r[0],$r[3],$r[4],$r[5]) }
            'step-after-trial' { $r=@($r[0],$r[1],$r[2],$r[3],$r[2],$r[4],$r[5]) }
        }
        $path=Join-Path $space "$variant.ndjson"
        [IO.File]::WriteAllText($path,(($r | ForEach-Object { $_ | ConvertTo-Json -Depth 50 -Compress }) -join "`n")+"`n",[Text.UTF8Encoding]::new($false))
        $invalid=Invoke-Tool $validator @('--input',$traceInput,'--results',$path,'--exit-code','0','--checkpoint',$checkpoint,'--build',$buildCommand,'--toolchain',$toolchain)
        Assert-Check ($invalid.exit -eq 2 -and -not $invalid.value.protocol_valid -and -not $invalid.value.complete) "$kind accepted invalid $variant"
    }
    foreach ($variant in @('undefined-goal','object-order','reordered-trials','other-configuration')) {
        $r = @($raw.TrimEnd("`r","`n") -split "`n" | ForEach-Object { $_ | ConvertFrom-Json -AsHashtable })
        $expected = $goodInput
        switch ($variant) {
            'undefined-goal' { $r[1].measurements.number=$null; $r[1].undefined=@{number='Synthetic modeled goal not reached'} }
            'object-order' {
                $reverse = [ordered]@{}; $keys = @($r[0].input.Keys); [Array]::Reverse($keys)
                foreach ($key in $keys) { $reverse[$key]=$r[0].input[$key] }
                $r[0].input=$reverse; $r[0].trailing_metadata='Not part of frozen input subtree'
            }
            'reordered-trials' { $r=@($r[0],$r[2],$r[1],$r[3]) }
            'other-configuration' {
                $r[0].input.configuration_id='other-fixture'; $r[1].configuration_id='other-fixture'; $r[2].configuration_id='other-fixture'
                $expected=Join-Path $space 'other-configuration.json'; Write-Json $expected $r[0].input
            }
        }
        $path=Join-Path $space "$variant.ndjson"
        [IO.File]::WriteAllText($path,(($r | ForEach-Object { $_ | ConvertTo-Json -Depth 50 -Compress }) -join "`n")+"`n",[Text.UTF8Encoding]::new($false))
        $valid=Invoke-Tool $validator @('--input',$expected,'--results',$path,'--exit-code','0','--checkpoint',$checkpoint,'--build',$buildCommand,'--toolchain',$toolchain)
        Assert-Check ($valid.exit -eq 0 -and $valid.value.complete) "$kind rejected valid $variant"
        if ($variant -eq 'undefined-goal') { Assert-Check ($valid.value.undefined_measurements -eq 1) "$kind hid undefined modeled outcome" }
    }
    foreach ($variant in @('max-nodes','max-bytes','max-depth')) {
        $fixture = New-Fixture 'complete'; $fixture.limits.output_bytes=2097152
        $expected=Join-Path $space "$variant.json"; Write-Json $expected $fixture
        if ($variant -eq 'max-nodes') {
            $size=Invoke-Tool $validator @('--check-input',$expected)
            $fixture.padding=@(0) * (4096-$size.value.node_count-1)
        } elseif ($variant -eq 'max-bytes') {
            $fixture.padding=''; Write-Json $expected $fixture
            $fixture.padding='x' * (1048576-(Get-Item -LiteralPath $expected).Length)
        } else {
            $nested=@{leaf=0}; for ($depth=0;$depth -lt 30;$depth++) { $nested=@{nested=$nested} }
            $fixture.padding=$nested
        }
        Write-Json $expected $fixture
        $size=Invoke-Tool $validator @('--check-input',$expected)
        Assert-Check ($size.exit -eq 0 -and $size.value.input_valid) "$kind rejected boundary input $variant"
        $r=@($raw.TrimEnd("`r","`n") -split "`n" | ForEach-Object { $_ | ConvertFrom-Json -AsHashtable }); $r[0].input=$fixture
        $path=Join-Path $space "$variant.ndjson"
        [IO.File]::WriteAllText($path,(($r | ForEach-Object { $_ | ConvertTo-Json -Depth 50 -Compress }) -join "`n")+"`n",[Text.UTF8Encoding]::new($false))
        $valid=Invoke-Tool $validator @('--input',$expected,'--results',$path,'--exit-code','0','--checkpoint',$checkpoint,'--build',$buildCommand,'--toolchain',$toolchain)
        Assert-Check ($valid.exit -eq 0 -and $valid.value.complete) "$kind rejected embedded boundary input $variant"
    }
    foreach ($variant in @('zero-wall','zero-output','bad-count','duplicate-identity','duplicate-choice','bad-trace','too-many-nodes')) {
        $fixture=New-Fixture 'complete'
        switch ($variant) {
            'zero-wall' { $fixture.limits.wall_seconds=0 }
            'zero-output' { $fixture.limits.output_bytes=0 }
            'bad-count' { $fixture.trials=1.5 }
            'duplicate-identity' { $fixture.measures.integer.identity=$fixture.measures.number.identity }
            'duplicate-choice' { $fixture.measures.enum.allowed_values=@('a','a') }
            'bad-trace' { $fixture.trace_trial=-2 }
            'too-many-nodes' { $fixture.padding=@(0)*4096 }
        }
        $path=Join-Path $space "$variant.json"; Write-Json $path $fixture
        $invalid=Invoke-Tool $validator @('--check-input',$path)
        Assert-Check ($invalid.exit -eq 2 -and -not $invalid.value.complete) "$kind accepted bad frozen input $variant"
    }
    $stopped = Invoke-Tool $validator ($arguments + @('--stopped'))
    Assert-Check ($stopped.exit -eq 2 -and $stopped.value.protocol_valid -and -not $stopped.value.complete) "$kind external stop became complete"
    $wrongExit = Invoke-Tool $validator @('--input',$goodInput,'--results',$goodOutput,'--exit-code','9','--checkpoint',$checkpoint,'--build',$buildCommand,'--toolchain',$toolchain)
    Assert-Check ($wrongExit.exit -eq 2 -and -not $wrongExit.value.complete) "$kind ignored process exit mismatch"
    $existing = Join-Path $space 'existing'; New-Item -ItemType Directory -Path $existing | Out-Null
    [IO.File]::WriteAllText((Join-Path $existing 'sentinel'),'keep')
    $refused = Invoke-Tool $supervisor @('--program',$ownedEmitter,'--input',$goodInput,'--output',$existing,'--checkpoint',$checkpoint,'--build',$buildCommand,'--toolchain',$toolchain)
    Assert-Check ($refused.exit -eq 2 -and $refused.value.supervision_status -eq 'output-directory' -and [IO.File]::ReadAllText((Join-Path $existing 'sentinel')) -eq 'keep' -and @(Get-ChildItem -LiteralPath $existing).Count -eq 1) "$kind overwrote/joined existing evidence"
    $unstarted=Join-Path $space 'missing-metadata'
    $refused=Invoke-Tool $supervisor @('--program',$ownedEmitter,'--input',$goodInput,'--output',$unstarted)
    Assert-Check ($refused.exit -eq 2 -and -not $refused.value.launched -and -not (Test-Path -LiteralPath $unstarted)) "$kind missing required provenance started process/wrote evidence"
    $missing = Join-Path $space 'missing-program'
    $failed = Invoke-Tool $supervisor @('--program',(Join-Path $space 'no-program.exe'),'--input',$goodInput,'--output',$missing,'--checkpoint',$checkpoint,'--build',$buildCommand,'--toolchain',$toolchain)
    Assert-Check ($failed.exit -eq 2 -and $failed.value.supervision_status -eq 'start' -and -not $failed.value.launched -and $null -eq $failed.value.process_exit_code -and -not $failed.value.validation.complete) "$kind startup failure became success"
}
[ordered]@{ engineering_checks = $script:checks; experiment_trials_executed = 0; artifact_directory = $folder } | ConvertTo-Json -Compress
