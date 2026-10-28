<#
.SYNOPSIS
Capture six native graph projections without pointer/keyboard automation or example writes.
.DESCRIPTION
Usage: pwsh -NoProfile -File scripts/capture-workbench-graph-review.ps1 -RaylibDirectory PATH
  [-Example little-plumber-guy-nes|crate-puzzle] [-Layers overview]
Default captures all five layers. For multiple overrides invoke from PowerShell with -Layers @('default','overview').
  [-Executable PATH] [-UiLibrary PATH] [-OutputParent PATH] [-Frames 4] [-Cjit cjit]
Requires existing CJIT, prepared gd-ui.dll and raylib SDK. No downloads/installs.
Creates unique ignored artifact directory; never overwrites an earlier run. Non-default layers
compile PRIVATE copies with initialized control state; these are rendering probes, NOT evidence
of real UI activation, input isolation or usability. Original production sources stay unchanged.
.OUTPUTS
Stdout: one JSON result. Logs/screenshots/seeded sources stay in artifact directory. Exit 0 pass,
1 capture/build/data-integrity failure; PowerShell argument binding errors are nonzero. No prompts.
#>
param(
    [string]$RaylibDirectory='',
    [ValidateSet('little-plumber-guy-nes','crate-puzzle')][string]$Example='little-plumber-guy-nes',
    [ValidateSet('default','foundations','typed','shapes','overview')][string[]]$Layers=@('default','foundations','typed','shapes','overview'),
    [string]$Executable='', [string]$UiLibrary='', [string]$OutputParent='',
    [ValidateRange(1,600)][int]$Frames=4, [string]$Cjit='cjit'
)
$ErrorActionPreference='Stop'
$repository=Split-Path $PSScriptRoot -Parent
$native=Join-Path $repository 'skills/game-design-workbench/native'
$game=Join-Path $repository "skills/game-design-workbench/examples/$Example/game-design"
$directory=''; $captures=@(); $process=$null
function Replace-Once([string]$Text,[string]$Old,[string]$New) {
    $first=$Text.IndexOf($Old,[StringComparison]::Ordinal)
    if ($first -lt 0 -or $first -ne $Text.LastIndexOf($Old,[StringComparison]::Ordinal)) { throw "Seed patch must match once: $Old" }
    return $Text.Replace($Old,$New)
}
function Tree-Hash([string]$Path) {
    $manifest=@(Get-ChildItem -LiteralPath $Path -File -Recurse | Sort-Object FullName | ForEach-Object { "$($_.FullName):$((Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash)" }) -join "`n"
    return [Convert]::ToHexString([Security.Cryptography.SHA256]::HashData([Text.Encoding]::UTF8.GetBytes($manifest)))
}
try {
    if (!$RaylibDirectory) { throw 'Require -RaylibDirectory pointing at prepared raylib SDK; no interactive prompt.' }
    if (!$Executable) { $Executable=Join-Path $native 'build/graph-independent-review/gd-view.exe' }
    if (!$UiLibrary) { $UiLibrary=Join-Path $native 'build/gd-ui.dll' }
    if (!$OutputParent) { $OutputParent=Join-Path $native 'build' }
    foreach ($path in @($Executable,$UiLibrary,(Join-Path $RaylibDirectory 'lib/raylib.dll'))) {
        if (!(Test-Path -LiteralPath $path -PathType Leaf)) { throw "Missing prepared input: $path" }
    }
    $Executable=[IO.Path]::GetFullPath($Executable); $UiLibrary=[IO.Path]::GetFullPath($UiLibrary)
    $RaylibDirectory=[IO.Path]::GetFullPath($RaylibDirectory)
    $before=Tree-Hash $game
    $sourceMatchesHead=@(& git -C $repository diff --name-only HEAD -- skills/game-design-workbench/native).Count -eq 0
    $sourceFiles=@('gd_graph_view.c','gd_mda_view.c','gd_resource_view.c')
    $sourceHashes=@{}; foreach ($file in $sourceFiles) { $sourceHashes[$file]=(Get-FileHash (Join-Path $native $file)).Hash }
    $directory=Join-Path ([IO.Path]::GetFullPath($OutputParent)) ('graph-captures-'+[guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $directory | Out-Null
    foreach ($layer in ($Layers | Select-Object -Unique)) {
        $viewExecutable=$Executable
        if ($layer -ne 'default') {
            $seed=Join-Path $directory $layer; New-Item -ItemType Directory -Path $seed | Out-Null
            foreach ($file in $sourceFiles) {
                $text=[IO.File]::ReadAllText((Join-Path $native $file))
                if ($file -eq 'gd_resource_view.c') {
                    $text=Replace-Once $text 'int shared=1,branches=0' 'int shared=0,branches=0'
                    $tab=@{foundations=2;typed=3;shapes=4;overview=5}[$layer]
                    $text=Replace-Once $text 'int path_tab=0;' "int path_tab=$tab;"
                    if ($layer -eq 'typed') { $text=Replace-Once $text 'reach_controls={0,-1,0,-1,-1}' 'reach_controls={1,-1,0,-1,-1}' }
                } else {
                    $text=Replace-Once $text 'shared=true' 'shared=false'
                    if ($layer -eq 'foundations') { $text=Replace-Once $text 'foundation_enabled=false' 'foundation_enabled=true' }
                    if ($layer -eq 'typed') {
                        if ($file -eq 'gd_graph_view.c') { $text=Replace-Once $text 'reach_controls[3]={{0,-1,0,-1,-1},{0,-1,0,-1,-1},{0,-1,0,-1,-1}}' 'reach_controls[3]={{1,-1,1,-1,-1},{1,-1,0,-1,-1},{1,-1,0,-1,-1}}' }
                        else { $text=Replace-Once $text 'reach_controls={0,-1,0,-1,-1}' 'reach_controls={1,-1,1,-1,-1}' }
                    }
                    if ($file -eq 'gd_graph_view.c') {
                        $text=Replace-Once $text '    if (draft) SetExitKey(KEY_NULL);' @'
    /* PRIVATE render probe: select illustrative identity, never alter known set. */
    if (!focused && model->skill_mode && !challenges) {
        for (int record=0;record<model->record_count;++record)
            if (!strcmp(model->records[record].name,"jump-stomp") || !strcmp(model->records[record].name,"coordinate")) { select_record(layout,&selection,record); break; }
    }
    if (challenges) for (int node=0;node<layout->node_count;++node)
        if (model->records[layout->nodes[node].key].entry) { select_record(layout,&selection,layout->nodes[node].key); break; }
    if (selection.node<0 && layout->node_count) selection.node=0;
    if (draft) SetExitKey(KEY_NULL);
'@
                    }
                }
                if ($layer -eq 'shapes') {
                    if ($file -eq 'gd_resource_view.c') { $text=Replace-Once $text 'int shape_enabled=0' 'int shape_enabled=1' }
                    else {
                        $text=Replace-Once $text 'shape_enabled=false' 'shape_enabled=true'
                        if ($file -eq 'gd_mda_view.c') {
                            $text=Replace-Once $text '        const GD_GraphShapes *shapes=shape_enabled ? gd_graph_model_shapes(model,analysis):NULL;' @'
        const GD_GraphShapes *shapes=shape_enabled ? gd_graph_model_shapes(model,analysis):NULL;
        /* PRIVATE render probe: illustrate first repeated bucket, not UI activation. */
        if (!frame && shapes && shapes->complete) for (int family=0;family<shapes->family_count;++family)
            if (shapes->size[family]>1) { display.record=layout->nodes[shapes->representative[family]].key; break; }
'@
                        }
                    }
                }
                if ($layer -eq 'overview') { $text=Replace-Once $text 'overview_controls={0,-1,-1}' 'overview_controls={1,-1,-1}' }
                [IO.File]::WriteAllText((Join-Path $seed $file),$text,[Text.UTF8Encoding]::new($false))
            }
            $wrapper=Join-Path $seed 'cjit-seeded.ps1'
            $escapedNative=$native.Replace("'","''"); $escapedSeed=$seed.Replace("'","''"); $escapedCjit=$Cjit.Replace("'","''")
            $wrapperText=@'
$files=@('gd_graph_view.c','gd_mda_view.c','gd_resource_view.c')
$items=@($args | ForEach-Object { if ([IO.Path]::GetFileName($_) -in $files) { Join-Path '__SEED__' ([IO.Path]::GetFileName($_)) } else { $_ } })
& '__CJIT__' -I '__NATIVE__' @items
exit $LASTEXITCODE
'@
            [IO.File]::WriteAllText($wrapper,$wrapperText.Replace('__SEED__',$escapedSeed).Replace('__CJIT__',$escapedCjit).Replace('__NATIVE__',$escapedNative))
            & (Join-Path $native 'build.ps1') -Graph -Cjit $wrapper -UiLibrary $UiLibrary -RaylibDirectory $RaylibDirectory -OutputDirectory $seed *> (Join-Path $seed 'build.log')
            if ($LASTEXITCODE -ne 0) { throw "Seeded $layer build failed; see $seed/build.log" }
            $viewExecutable=Join-Path $seed 'gd-view.exe'
        }
        foreach ($view in @('skills','challenges','relationships','matrix','mda','resources')) {
            $arguments=@('view')
            switch ($view) {
                {$_ -in 'skills','challenges','relationships'} {
                    $arguments+=@('skill-atoms','--skill-atoms',"skill-atoms=$game/skill-atoms",'--challenges',"challenges=$game/challenges",'--related',"mechanics=$game/mechanics")
                    if ($view -eq 'challenges') { $arguments+=@('--mode','challenges') }
                    if ($view -eq 'relationships') { $root=if ($Example -eq 'crate-puzzle') {'coordinate'} else {'jump-stomp'}; $arguments+=@('--mode','relationships','--from',"skill-atoms/$root") }
                }
                'matrix' { $arguments+=@('matrix','--nouns',"nouns=$game/nouns",'--verbs',"verbs=$game/verbs",'--interactions',"interactions=$game/interactions",'--layout','graph') }
                'mda' { $arguments+=@('mda','--mechanics',"mechanics=$game/mechanics",'--systems',"systems=$game/systems",'--dynamics',"dynamics=$game/dynamics",'--aesthetics',"aesthetics=$game/aesthetics") }
                'resources' { $arguments+=@('resources','--interactions',"interactions=$game/interactions",'--mechanics',"mechanics=$game/mechanics",'--nouns',"nouns=$game/nouns",'--axes',"axes=$game/axes",'--control-vars',"control-vars=$game/control-vars",'--resources',"resources=$game/resources") }
            }
            $name="$layer-$view"; $screenshot=Join-Path $directory "$name.png"
            $arguments+=@('--frames',"$Frames",'--screenshot',$screenshot)
            $process=[Diagnostics.Process]::new(); $process.StartInfo=[Diagnostics.ProcessStartInfo]::new($viewExecutable)
            $process.StartInfo.UseShellExecute=$false; $process.StartInfo.RedirectStandardOutput=$true; $process.StartInfo.RedirectStandardError=$true; $process.StartInfo.WorkingDirectory=$repository
            foreach ($argument in $arguments) { $process.StartInfo.ArgumentList.Add($argument) }
            $process.Start() | Out-Null; $stdout=$process.StandardOutput.ReadToEndAsync(); $stderr=$process.StandardError.ReadToEndAsync()
            if (!$process.WaitForExit(30000)) { if (!$process.HasExited) { $process.Kill() }; throw "Owned render timed out: $name" }
            $stdoutText=$stdout.GetAwaiter().GetResult()
            [IO.File]::WriteAllText((Join-Path $directory "$name.stdout.log"),$stdoutText)
            [IO.File]::WriteAllText((Join-Path $directory "$name.stderr.log"),$stderr.GetAwaiter().GetResult())
            if ($process.ExitCode -ne 0 -or !(Test-Path -LiteralPath $screenshot -PathType Leaf) -or (Get-Item -LiteralPath $screenshot).Length -lt 128) { throw "Render failed: $name; exit $($process.ExitCode); see capture logs" }
            $draftVerified=$null
            if ($view -in @('skills','challenges','relationships')) {
                $report=$stdoutText | ConvertFrom-Json
                if (!$report.ok -or $report.data.changes.Count -ne 0 -or $report.coverage.modified_records -ne 0 -or $report.coverage.disk_changed_since_load) { throw "Nonempty/unsafe draft capture: $name" }
                $draftVerified=$true
            }
            $captures+=@{view=$view;layer=$layer;seeded=($layer -ne 'default');screenshot=$screenshot;arguments=$arguments;empty_draft_verified=$draftVerified}
            $process.Dispose(); $process=$null
        }
    }
    if ((Tree-Hash $game) -ne $before) { throw 'Example tree changed during capture' }
    foreach ($file in $sourceFiles) { if ((Get-FileHash (Join-Path $native $file)).Hash -ne $sourceHashes[$file]) { throw "Production source changed: $file" } }
    $result=@{ok=$true;example=$Example;production_commit=(& git -C $repository rev-parse HEAD);directory=$directory;captures=$captures;example_tree_unchanged=$true;production_sources_unchanged=$true;source_hashes=$sourceHashes;source_revision_matches_head=$sourceMatchesHead;seed_note='Private layer switches and illustrative selections; typed Skills/MDA reverse traversal; known set unchanged';manual_activation_verified=$false;input_isolation_verified=$false}
    $json=$result | ConvertTo-Json -Depth 8; [IO.File]::WriteAllText((Join-Path $directory 'captures.json'),$json); Write-Output $json
} catch {
    if ($process -and !$process.HasExited) { $process.Kill() }
    [Console]::Error.WriteLine($_.Exception.Message)
    @{ok=$false;code='capture.failed';message=$_.Exception.Message;directory=$directory;completed=$captures.Count} | ConvertTo-Json -Compress
    exit 1
}
