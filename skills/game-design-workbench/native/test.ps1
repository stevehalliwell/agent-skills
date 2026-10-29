<# Native/core and CLI regression checks. No downloads or source-record changes.
Usage: ./test.ps1 [-View -RaylibDirectory PATH] [-Cjit PATH]
Fixtures and captures stay in ignored build/. View opens a bounded visible window.
#>
param([switch]$View, [switch]$Graph, [string]$RaylibDirectory = $env:RAYLIB_ROOT, [string]$Cjit = 'cjit', [string]$OutputDirectory = '')
$ErrorActionPreference = 'Stop'
if ($Graph) { $View = $true }
$root = $PSScriptRoot
& (Join-Path $root 'build.ps1') -Test -Cjit $Cjit | Out-Host
$fixtures = Join-Path $root ('build/checks-' + [guid]::NewGuid().ToString('N'))
& (Join-Path $root 'build/test-core.exe') $fixtures
if ($LASTEXITCODE -ne 0) { throw 'Native domain checks failed' }
$draftReport = Get-Content -LiteralPath (Join-Path $fixtures 'draft-changes.json') -Raw -Encoding UTF8 | ConvertFrom-Json
$change = $draftReport.data.changes[0]
if (-not $draftReport.ok -or $draftReport.data.applied -or $draftReport.data.changes.Count -ne 1 -or $change.reference -ne 'atoms/root' -or $change.fields.rank.loaded -ne 1 -or $change.fields.rank.current_disk -ne 9 -or $change.fields.rank.edited -ne 4 -or -not $change.current_disk_available -or -not $draftReport.coverage.disk_changed_since_load) { throw 'Draft three-way rank handoff failed' }
if ($change.fields.prerequisites.loaded.Count -ne 0 -or $change.fields.prerequisites.current_disk.Count -ne 0 -or ($change.fields.prerequisites.edited -join ',') -ne 'atoms/leaf' -or @($draftReport.data.affected | Where-Object { $_.reference -eq 'atoms/child' -and $null -eq $_.authored_rank }).Count -ne 1) { throw 'Draft prerequisite handoff or descendant intent failed' }
foreach ($file in @('draft-unavailable.json', 'draft-identity.json')) {
    $partial = Get-Content -LiteralPath (Join-Path $fixtures $file) -Raw -Encoding UTF8 | ConvertFrom-Json
    if ($partial.ok -or $partial.coverage.complete -or $partial.data.changes[0].current_disk_available -or $null -ne $partial.data.changes[0].fields.rank.current_disk -or $partial.data.changes[0].fields.rank.edited -ne 4 -or $partial.errors.Count -lt 1) { throw "Unsafe partial draft report: $file" }
}
$emptyDraft = Get-Content -LiteralPath (Join-Path $fixtures 'draft-empty.json') -Raw -Encoding UTF8 | ConvertFrom-Json
if (-not $emptyDraft.ok -or $emptyDraft.data.changes.Count -ne 0 -or $emptyDraft.coverage.disk_changed_since_load) { throw 'Return to disk sync point failed' }
& (Join-Path $root 'build.ps1') -Cjit $Cjit | Out-Host
$executable = Join-Path $root 'build/gd.exe'
$example = Join-Path (Split-Path $root -Parent) 'examples/crate-puzzle/game-design'
$inputs = @('--nouns', "nouns=$example/nouns", '--verbs', "verbs=$example/verbs", '--interactions', "interactions=$example/interactions")
$result = (& $executable matrix @inputs --json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or -not $result.ok -or $result.data.cells.Count -ne 4 -or $result.coverage.record_count -ne 16) { throw 'Example matrix mismatch' }
$push = @($result.data.cells | Where-Object { $_.noun -eq 'nouns/crate' -and $_.verb -eq 'verbs/push' })
if ($push.Count -ne 1 -or $push[0].cases.Count -ne 3 -or -not $push[0].active) { throw 'Conditional cases flattened' }
$groups = @($result.data.components)
if ($groups.Count -ne 8 -or @($groups | Where-Object { $_.members.Count -eq 1 }).Count -ne 6) { throw 'Connected grouping changed' }
$crateGroup = $groups | Where-Object { $_.members -contains 'nouns/crate' }
if ($crateGroup.active_pair_count -ne 1 -or ($crateGroup.members -join ',') -ne 'nouns/crate,verbs/push') { throw 'Case count or context inflated component' }
$case = $result.data.cases | Where-Object reference -eq 'interactions/wall-activation'
if ($case.coverage -ne 'not_applicable') { throw 'Non-applicability lost' }
$connectivity = (& $executable connectivity @inputs --json | ConvertFrom-Json)
$crate = $connectivity.data.nouns | Where-Object reference -eq 'nouns/crate'
$wall = $connectivity.data.nouns | Where-Object reference -eq 'nouns/wall'
$pull = $connectivity.data.verbs | Where-Object reference -eq 'verbs/pull'
if ($LASTEXITCODE -ne 0 -or $crate.partner_count -ne 1 -or $crate.case_count -ne 3 -or $wall.partner_count -ne 0 -or $pull.partner_count -ne 0 -or $pull.status -ne 'rejected') { throw 'Connectivity semantics changed' }
$selected = (& $executable matrix @inputs --json --fields cells | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or ($selected.data.PSObject.Properties.Name -join ',') -ne 'cells' -or -not $selected.coverage.complete) { throw 'Field selection lost envelope' }
$onlyGroups = (& $executable matrix @inputs --json --fields components | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or ($onlyGroups.data.PSObject.Properties.Name -join ',') -ne 'components' -or $onlyGroups.data.components.Count -ne 8) { throw 'Component field selection failed' }
$text = & $executable matrix @inputs --format text
if ($LASTEXITCODE -ne 0 -or ($text -join "`n") -notmatch '4 declared cells') { throw 'Text mode failed' }
$skillInputs = @('--skill-atoms', "skill-atoms=$example/skill-atoms")
$skills = (& $executable skill-atoms @skillInputs --json | ConvertFrom-Json)
$coordinate = $skills.data.skills | Where-Object reference -eq 'skill-atoms/coordinate'
$plan = $skills.data.skills | Where-Object reference -eq 'skill-atoms/plan-push'
$late = $skills.data.skills | Where-Object reference -eq 'skill-atoms/read-switch'
if ($LASTEXITCODE -ne 0 -or -not $skills.ok -or -not $skills.coverage.placement_complete -or $skills.data.skills.Count -ne 6 -or $skills.data.links.Count -ne 5) { throw 'Skill graph inventory failed' }
if ($coordinate.authored_rank -ne 2 -or $coordinate.minimum_rank -ne 3 -or $coordinate.display_rank -ne 3 -or $coordinate.prerequisite_depth -ne 3 -or -not $coordinate.rank_conflict) { throw 'Authored rank / prerequisite depth / display rank conflated' }
if ($null -ne $plan.authored_rank -or $plan.display_rank -ne 1 -or -not $late.root -or $late.authored_rank -ne 3 -or $late.prerequisite_depth -ne 0) { throw 'Null rank or late root lost' }
if ($skills.data.findings.Count -ne 1 -or $skills.data.findings[0].code -ne 'skill.rank_conflict' -or $skills.data.findings[0].line -le 0) { throw 'Expected sourced skill finding missing' }
if ($skills.data.components.Count -ne 1 -or $skills.data.components[0].link_count -ne 5) { throw 'Skill weak grouping changed' }
$skillFields = (& $executable skill-atoms @skillInputs --json --fields links | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or ($skillFields.data.PSObject.Properties.Name -join ',') -ne 'links' -or $skillFields.data.links.Count -ne 5) { throw 'Skill field selection failed' }
$skillText = & $executable skill-atoms @skillInputs --format text
if ($LASTEXITCODE -ne 0 -or ($skillText -join "`n") -notmatch 'RANK CONFLICT') { throw 'Skill text output failed' }
$incompleteInputs = @('--skill-atoms', "atoms=$fixtures/skill-incomplete/things", '--related', "context=$fixtures/skill-incomplete/actions")
$incomplete = (& $executable skill-atoms @incompleteInputs --json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or -not $incomplete.ok -or -not $incomplete.coverage.complete -or $incomplete.coverage.placement_complete -or $incomplete.data.links.Count -ne 6 -or @($incomplete.data.skills | Where-Object cycle).Count -ne 3) { throw 'Cycle/unresolved graph semantics failed' }
$unresolved = $incomplete.data.skills | Where-Object reference -eq 'atoms/missing'
if ($unresolved.root -or $unresolved.placement_complete -or $null -ne $unresolved.prerequisite_depth -or $null -ne $unresolved.rank_conflict -or $unresolved.relationships[0].reference -ne 'atoms/absent') { throw 'Missing prerequisite became independent root or vanished' }
$unicode = Join-Path $root ('build/space ' + [char]0x96ea + '-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $unicode | Out-Null
foreach ($collection in @('nouns', 'verbs', 'interactions', 'skill-atoms', 'challenges', 'mechanics', 'systems', 'dynamics', 'aesthetics', 'control-vars')) { Copy-Item -LiteralPath (Join-Path $example $collection) -Destination $unicode -Recurse }
$unicodeInputs = @('--nouns', "nouns=$unicode/nouns", '--verbs', "verbs=$unicode/verbs", '--interactions', "interactions=$unicode/interactions")
$unicodeResult = (& $executable matrix @unicodeInputs --json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or -not $unicodeResult.ok -or $unicodeResult.data.cells.Count -ne 4) { throw 'Unicode/space collection paths failed' }
$unicodeSkills = (& $executable skill-atoms --skill-atoms "skill-atoms=$unicode/skill-atoms" --json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or -not $unicodeSkills.ok -or $unicodeSkills.data.skills.Count -ne 6) { throw 'Unicode skill path failed' }
$emptySkills = Join-Path $unicode 'empty-skills'
New-Item -ItemType Directory -Path $emptySkills | Out-Null
$emptySkillResult = (& $executable skill-atoms --skill-atoms "skills=$emptySkills" --json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or -not $emptySkillResult.ok -or $emptySkillResult.data.skills.Count -ne 0 -or $emptySkillResult.data.links.Count -ne 0) { throw 'Empty skill graph failed' }
$log = Join-Path $root 'build/expected-error.txt'
$previousPreference = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
$failureOutput = & $executable matrix --json 2>$log
$failureExit = $LASTEXITCODE
$ErrorActionPreference = $previousPreference
$failure = $failureOutput | ConvertFrom-Json
if ($failureExit -ne 2 -or $failure.ok -or $failure.coverage.complete -or $failure.errors[0].code -ne 'cli.arguments') { throw 'Structured failure contract changed' }
$routeInputs = @('--skill-atoms', "skill-atoms=$unicode/skill-atoms", '--challenges', "challenges=$unicode/challenges")
$routeResult = (& $executable scaffolding @routeInputs --json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or -not $routeResult.ok -or -not $routeResult.coverage.routes_complete -or $routeResult.data.checks.Count -ne 10 -or $routeResult.data.witnesses.Count -ne 12) { throw 'Scaffolding example/Unicode paths failed' }
$planning = $routeResult.data.checks | Where-Object { $_.challenge -eq 'challenges/combined-room' -and $_.skill -eq 'skill-atoms/plan-push' }
$switch = $routeResult.data.checks | Where-Object { $_.challenge -eq 'challenges/combined-room' -and $_.skill -eq 'skill-atoms/route-switch' }
if ($planning.coverage -ne 'absent' -or $switch.coverage -ne 'some_routes' -or -not $switch.supported_route -or -not $switch.unsupported_route) { throw 'Scaffolding opportunity meaning failed' }
$bypass = $routeResult.data.witnesses | Where-Object { $_.check -eq $switch.index -and -not $_.supported }
$practice = $routeResult.data.witnesses | Where-Object { $_.check -eq $switch.index -and $_.supported }
if (($bypass.path -join ',') -ne 'challenges/move-demo,challenges/push-room,challenges/combined-room' -or $practice.path -notcontains 'challenges/switch-practice') { throw 'Scaffolding route evidence failed' }
$oneShot = @($routeResult.data.findings | Where-Object code -eq 'scaffolding.single_shot')
if ($oneShot.Count -ne 2 -or @($oneShot | Where-Object { $_.severity -ne 'info' -or $_.line -le 0 }).Count -gt 0) { throw 'Single-shot finding severity/location failed' }
$selectedRoutes = (& $executable scaffolding @routeInputs --fields checks,witnesses | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or @($selectedRoutes.data.PSObject.Properties.Name).Count -ne 3 -or $selectedRoutes.data.checks.Count -ne 10) { throw 'Scaffolding field selection failed' }
$routeText = (& $executable scaffolding @routeInputs --format text | Out-String)
if ($LASTEXITCODE -ne 0 -or $routeText -notmatch 'prior opportunity some_routes') { throw 'Scaffolding explicit text failed' }
$emptyRoutes = (& $executable scaffolding --skill-atoms "skills=$emptySkills" --challenges "challenges=$emptySkills" --json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or -not $emptyRoutes.ok -or $emptyRoutes.data.checks.Count -ne 0 -or -not $emptyRoutes.coverage.routes_complete) { throw 'Empty scaffolding failed' }
$previousPreference = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
$badRoutes = (& $executable scaffolding --skill-atoms "skills=$emptySkills" --json 2>$log | ConvertFrom-Json); $badExit = $LASTEXITCODE
$ErrorActionPreference = $previousPreference
if ($badExit -ne 2 -or $badRoutes.ok -or $badRoutes.errors[0].code -ne 'cli.arguments') { throw 'Scaffolding explicit role contract failed' }
$mdaInputs = @('--mechanics', "mechanics=$unicode/mechanics", '--systems', "systems=$unicode/systems", '--dynamics', "dynamics=$unicode/dynamics", '--aesthetics', "aesthetics=$unicode/aesthetics")
$mda = (& $executable mda @mdaInputs --json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or -not $mda.ok -or $mda.data.nodes.Count -ne 9 -or $mda.data.links.Count -ne 6 -or $mda.data.gaps.Count -ne 3 -or -not $mda.coverage.graph_references_complete -or $mda.data.paths.Count -ne 0) { throw 'MDA graph/status/gap scope failed' }
$focused = (& $executable mda @mdaInputs --from mechanics/crate-pushing --direction forward | ConvertFrom-Json)
$intent = $focused.data.paths | Where-Object to -eq 'aesthetics/deliberate-tension'
if ($LASTEXITCODE -ne 0 -or -not $focused.ok -or $focused.data.nodes.Count -ne 4 -or $focused.data.links.Count -ne 6 -or $intent.full_length -ne 3 -or ($intent.records -join ',') -ne 'mechanics/crate-pushing,dynamics/careful-positioning,aesthetics/deliberate-tension' -or @($intent.steps | Where-Object { -not $_.reversed }).Count) { throw 'MDA forward witness or alternatives/boundary links lost' }
$backward = (& $executable mda @mdaInputs --from aesthetics/deliberate-tension --direction backward --json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or $backward.data.nodes.Count -ne 6 -or @($backward.data.paths.steps | Where-Object reversed).Count) { throw 'MDA authored-basis traversal failed' }
$fieldsMda = (& $executable mda @mdaInputs --json --fields gaps | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or ($fieldsMda.data.PSObject.Properties.Name -join ',') -ne 'gaps' -or -not $fieldsMda.coverage.complete) { throw 'MDA field selection lost envelope' }
$mdaText = & $executable mda @mdaInputs --format text
if ($LASTEXITCODE -ne 0 -or ($mdaText -join "`n") -notmatch '9/9 MDA nodes' -or ($mdaText -join "`n") -notmatch 'not causation') { throw 'MDA text output failed' }
$noSystems = @('--mechanics', "mechanics=$unicode/mechanics", '--dynamics', "dynamics=$unicode/dynamics", '--aesthetics', "aesthetics=$unicode/aesthetics")
$unloaded = (& $executable mda @noSystems --from mechanics/crate-pushing --direction forward --json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or -not $unloaded.ok -or $unloaded.coverage.graph_references_complete -or $unloaded.coverage.query_references_complete -or $unloaded.data.nodes.Count -ne 3 -or @($unloaded.data.links | Where-Object resolution -eq 'unloaded').Count -ne 1 -or @($unloaded.data.findings | Where-Object { $_.code -eq 'mda.reference_unloaded' -and $_.severity -eq 'info' }).Count -ne 1) { throw 'MDA unloaded system context confused with missing data' }
$unavailableMda = (& $executable mda @noSystems --systems "systems=$unicode/missing-systems-folder" --json 2>$log | ConvertFrom-Json)
if ($LASTEXITCODE -ne 2 -or $unavailableMda.ok -or $unavailableMda.coverage.complete -or @($unavailableMda.data.links | Where-Object resolution -eq 'unavailable').Count -ne 1 -or @($unavailableMda.data.links | Where-Object resolution -eq 'missing').Count) { throw 'Failed MDA source load became a proven missing-record claim' }
$wrongRole = (& $executable mda @noSystems --related "systems=$unicode/systems" --json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or -not $wrongRole.ok -or @($wrongRole.data.links | Where-Object resolution -eq 'wrong_role').Count -ne 1 -or @($wrongRole.data.nodes | Where-Object aesthetic_trace -eq 'unknown').Count -ne 5) { throw 'MDA roles or conservative unknowns failed' }
$emptyMdaInputs = @('--mechanics', "m=$emptySkills", '--dynamics', "d=$emptySkills", '--aesthetics', "a=$emptySkills")
$emptyMda = (& $executable mda @emptyMdaInputs --json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or -not $emptyMda.ok -or $emptyMda.data.nodes.Count -or $emptyMda.data.links.Count -or -not $emptyMda.coverage.complete) { throw 'MDA empty result failed' }
$badMda = (& $executable mda @mdaInputs --from mechanics/absent --json 2>$log | ConvertFrom-Json)
if ($LASTEXITCODE -ne 2 -or $badMda.ok -or $badMda.coverage.complete -or $badMda.errors[0].code -ne 'mda.root') { throw 'MDA missing root must fail, not return all nodes' }
$badMdaFields = (& $executable mda @mdaInputs --fields invented --json 2>$log | ConvertFrom-Json)
if ($LASTEXITCODE -ne 2 -or $badMdaFields.ok -or $badMdaFields.errors[0].code -ne 'cli.arguments') { throw 'MDA unsupported fields must fail structurally' }
$partialMda = (& $executable mda --mechanics "things=$fixtures/mda-resolution/things" --dynamics "actions=$fixtures/mda-resolution/actions" --aesthetics "cases=$fixtures/mda-resolution/cases" --from cases/intent --direction backward --json 2>$log | ConvertFrom-Json)
if ($LASTEXITCODE -ne 2 -or $partialMda.ok -or $partialMda.coverage.complete -or $partialMda.data.nodes.Count -ne 2 -or @($partialMda.data.links | Where-Object resolution -eq 'ambiguous').Count -ne 1 -or @($partialMda.errors | Where-Object code -eq 'input.identity').Count -ne 1) { throw 'MDA partial ambiguity/identity preservation failed' }
$sharedMda = Get-Content -LiteralPath (Join-Path $fixtures 'mda-forward.json') -Raw -Encoding UTF8 | ConvertFrom-Json
$customMdaInputs = @('--mechanics', "things=$fixtures/mda-traces/things", '--dynamics', "actions=$fixtures/mda-traces/actions", '--aesthetics', "cases=$fixtures/mda-traces/cases", '--systems', "arrangements=$fixtures/mda-traces/arrangements")
$customMda = (& $executable mda @customMdaInputs --from things/push --direction forward --json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or ($sharedMda | ConvertTo-Json -Depth 30 -Compress) -ne ($customMda | ConvertTo-Json -Depth 30 -Compress) -or @($customMda.data.links | Where-Object { $_.source -eq 'actions/drift' -and $_.field -eq 'mechanics' }).Count -ne 2) { throw 'Shared MDA API/CLI parity, aliases, or raw duplicate declarations failed' }
$controlInputs = @('--control-vars', "control-vars=$unicode/control-vars")
$controlSources = @(Get-ChildItem -LiteralPath "$example/control-vars" -File | Get-FileHash | Select-Object Path,Hash)
$controls = (& $executable controls @controlInputs --json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or -not $controls.ok -or $controls.coverage.resolved_count -ne 7 -or ($controls.data.controls | Where-Object reference -eq 'control-vars/walk-duration').value -ne 0.5) { throw 'Control resolution/Unicode sources failed' }
$overrideControls = (& $executable controls @controlInputs --set control-vars/walk-speed=4 --json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or -not $overrideControls.ok -or ($overrideControls.data.controls | Where-Object reference -eq 'control-vars/push-duration').value -ne 0.5 -or ($overrideControls.data.controls | Where-Object reference -eq 'control-vars/walk-speed').authored_value -ne 2) { throw 'Control override propagation/source preservation failed' }
$exportControls = (& $executable controls export @controlInputs --as json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or $exportControls.'initial-energy' -ne 3 -or $exportControls.'push-duration' -ne 1 -or $exportControls.'walk-duration' -ne 0.5 -or $null -ne $exportControls.schema_version) { throw 'Raw typed JSON control export failed' }
$csvControls = (& $executable controls export @controlInputs --as csv | ConvertFrom-Csv)
if ($LASTEXITCODE -ne 0 -or $csvControls.Count -ne 7 -or ($csvControls | Where-Object key -eq 'walk-speed').unit -ne 'm/s') { throw 'Control CSV export failed' }
$iniControls = (& $executable controls export @controlInputs --as ini | Out-String)
if ($LASTEXITCODE -ne 0 -or $iniControls -notmatch '\[controls\]' -or $iniControls -notmatch 'walk-duration=0.5') { throw 'Control INI export failed' }
$controlFields = (& $executable controls @controlInputs --fields controls --from control-vars/walk-duration | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or -not $controlFields.ok -or $controlFields.data.controls.Count -ne 3) { throw 'Control focus/dependency selection failed' }
$emptyControls = (& $executable controls export --control-vars "config=$emptySkills" --as json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or @($emptyControls.PSObject.Properties).Count -ne 0) { throw 'Empty control export failed' }
$badControls = (& $executable controls export @controlInputs --set control-vars/walk-speed=0 --as csv 2>$log | ConvertFrom-Json)
if ($LASTEXITCODE -ne 2 -or $badControls.ok -or $badControls.coverage.complete -or $badControls.errors[0].code -ne 'controls.arithmetic') { throw 'Failed control export must return error envelope, not partial CSV' }
$badControlKey = (& $executable controls @controlInputs --set control-vars/missing=1 --json 2>$log | ConvertFrom-Json)
if ($LASTEXITCODE -ne 2 -or $badControlKey.ok -or $badControlKey.errors[0].code -ne 'controls.override') { throw 'Unknown override key accepted' }
$enumInputs = @('--control-vars', "config=$fixtures/control-loading/things", '--status', 'all')
$enumExport = (& $executable controls export @enumInputs --as json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or $enumExport.flag -isnot [bool] -or $enumExport.flag -or $enumExport.mode -ne '=value') { throw 'Boolean false or enum text lost in export' }
$enumCsv = (& $executable controls export @enumInputs --as csv 2>$log | ConvertFrom-Csv)
if ($LASTEXITCODE -ne 0 -or ($enumCsv | Where-Object key -eq 'mode').value -ne '=value' -or (Get-Content -LiteralPath $log -Raw) -notmatch 'controls.csv_formula') { throw 'Spreadsheet warning changed data or was omitted' }
$selectionInputs = @('--control-vars', "config=$fixtures/control-selection/things")
$namedControls = (& $executable controls export @selectionInputs --include c --as json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or ($namedControls.PSObject.Properties.Name -join ',') -ne 'c' -or $namedControls.c -ne 5) { throw 'Named export included dependency-only controls or lost draft support' }
$allControls = (& $executable controls export @selectionInputs --status all --exclude b --as json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or @($allControls.PSObject.Properties).Count -ne 3 -or $null -ne $allControls.b -or $allControls.c -ne 5 -or $allControls.d -isnot [bool]) { throw 'All-status/exclude combination failed' }
$statusControls = (& $executable controls export @selectionInputs --status accepted --status proposed --exclude c --as json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or ($statusControls.PSObject.Properties.Name -join ',') -ne 'a,b') { throw 'Multiple status/export exclusions failed' }
$excludedControls = (& $executable controls export @selectionInputs --include b --exclude b --as json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or @($excludedControls.PSObject.Properties).Count) { throw 'Exclusion must win over explicit inclusion' }
$acceptedControls = (& $executable controls export @selectionInputs --as json 2>$log | ConvertFrom-Json)
if ($LASTEXITCODE -ne 2 -or $acceptedControls.ok -or @($acceptedControls.errors | Where-Object code -eq 'controls.status').Count -ne 1) { throw 'Accepted formula silently consumed draft dependency' }
$unknownFilter = (& $executable controls export @selectionInputs --exclude absent --as json 2>$log | ConvertFrom-Json)
if ($LASTEXITCODE -ne 2 -or $unknownFilter.ok -or $unknownFilter.errors[0].code -ne 'controls.selection') { throw 'Unknown exclusion key silently ignored' }
$importFile = Join-Path $unicode 'game-tweaks.json'
$gameValues = (& $executable controls export @controlInputs --as json | ConvertFrom-Json)
$gameValues.'walk-speed' = 4
[IO.File]::WriteAllText($importFile,($gameValues | ConvertTo-Json -Compress),[Text.UTF8Encoding]::new($false))
$importPlan = (& $executable controls import @controlInputs --input $importFile --json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or -not $importPlan.ok -or -not $importPlan.data.ready -or $importPlan.data.applied -or $importPlan.data.items.Count -ne 1 -or $importPlan.data.items[0].name -ne 'walk-speed' -or $importPlan.data.items[0].fields.value -ne 4 -or ($importPlan.data.derived | Where-Object reference -eq 'control-vars/walk-duration').candidate -ne 0.25) { throw 'JSON tuning round-trip/formula preservation failed' }
$importFields = (& $executable controls import @controlInputs --input $importFile --fields items | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or ($importFields.data.PSObject.Properties.Name -join ',') -ne 'applied,ready,items,source_digest' -or $importFields.data.items.Count -ne 1) { throw 'Import field selection lost flags or returned unrelated data' }
$csvImportFile = Join-Path $unicode 'game-tweaks.csv'
($csvControls | Where-Object key -eq 'walk-speed').value = '4'
[IO.File]::WriteAllText($csvImportFile,($csvControls | ConvertTo-Csv -NoTypeInformation | Out-String),[Text.UTF8Encoding]::new($false))
$csvPlan = (& $executable controls import @controlInputs --input $csvImportFile --input-format csv | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or -not $csvPlan.ok -or $csvPlan.data.items.Count -ne 1 -or $csvPlan.data.items[0].fields.value -ne 4) { throw 'CSV tuning round-trip failed' }
$iniImportFile = Join-Path $unicode 'game-tweaks.ini'
[IO.File]::WriteAllText($iniImportFile,($iniControls -replace '(?m)^walk-speed=2\r?$', 'walk-speed=4'),[Text.UTF8Encoding]::new($false))
$iniPlan = (& $executable controls import @controlInputs --input $iniImportFile --input-format ini | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or -not $iniPlan.ok -or $iniPlan.data.items.Count -ne 1 -or $iniPlan.data.items[0].fields.value -ne 4) { throw 'INI tuning round-trip failed' }
$gameValues.'walk-duration' = 0.3
[IO.File]::WriteAllText($importFile,($gameValues | ConvertTo-Json -Compress),[Text.UTF8Encoding]::new($false))
$conflictPlan = (& $executable controls import @controlInputs --input $importFile --json 2>$log | ConvertFrom-Json)
if ($LASTEXITCODE -ne 2 -or $conflictPlan.ok -or $conflictPlan.data.ready -or $conflictPlan.data.items.Count -or @($conflictPlan.errors | Where-Object code -eq 'controls.import_formula').Count -ne 1) { throw 'Direct derived tweak must block complete update batch' }
$enumImportFile = Join-Path $unicode 'enum-tweaks.json'
[IO.File]::WriteAllText($enumImportFile,([ordered]@{flag=$true;mode='other'} | ConvertTo-Json -Compress),[Text.UTF8Encoding]::new($false))
$enumPlan = (& $executable controls import @enumInputs --input $enumImportFile --json | ConvertFrom-Json)
if ($LASTEXITCODE -ne 0 -or -not $enumPlan.ok -or $enumPlan.data.items.Count -ne 2 -or ($enumPlan.data.items | Where-Object name -eq 'flag').fields.value -isnot [bool] -or ($enumPlan.data.items | Where-Object name -eq 'mode').fields.value -ne 'other') { throw 'Typed boolean/enum import failed' }
$explorerInputs = @('--skill-atoms', "skill-atoms=$example/skill-atoms", '--challenges', "challenges=$example/challenges")
$headlessExplorer = (& $executable view skill-atoms @explorerInputs --mode challenges --json 2>$log | ConvertFrom-Json)
if ($LASTEXITCODE -ne 2 -or @($headlessExplorer.errors | Where-Object code -eq 'view.unavailable').Count -ne 1) { throw 'Combined explorer inputs rejected before view dispatch' }
$headlessFocus = (& $executable view skill-atoms @explorerInputs --mode relationships --from skill-atoms/coordinate --json 2>$log | ConvertFrom-Json)
if ($LASTEXITCODE -ne 2 -or @($headlessFocus.errors | Where-Object code -eq 'view.unavailable').Count -ne 1) { throw 'Focused explorer arguments rejected before display dispatch' }
$skillRoot = (& $executable view skill-atoms @explorerInputs --from skill-atoms/coordinate --known skill-atoms/push --json 2>$log | ConvertFrom-Json)
if ($LASTEXITCODE -ne 2 -or @($skillRoot.errors | Where-Object code -eq 'view.unavailable').Count -ne 1) { throw 'Skill root/known assumptions rejected before display dispatch' }
$missingChallenges = (& $executable view skill-atoms --skill-atoms "skill-atoms=$example/skill-atoms" --mode challenges --json 2>$log | ConvertFrom-Json)
if ($LASTEXITCODE -ne 2 -or @($missingChallenges.errors | Where-Object code -eq 'cli.arguments').Count -ne 1) { throw 'Challenges mode must require explicit challenge mapping' }
$controlText = (& $executable controls @controlInputs --format text | Out-String)
if ($LASTEXITCODE -ne 0 -or $controlText -notmatch '7/7 resolved') { throw 'Control text inspection failed' }
$controlSourcesAfter = @(Get-ChildItem -LiteralPath "$example/control-vars" -File | Get-FileHash | Select-Object Path,Hash)
if (($controlSources | ConvertTo-Json -Compress) -ne ($controlSourcesAfter | ConvertTo-Json -Compress)) { throw 'Canonical control files changed during tests' }
if ($View) {
    & (Join-Path $root 'build.ps1') -View -Graph:$Graph -RaylibDirectory $RaylibDirectory -Cjit $Cjit -OutputDirectory $OutputDirectory | Out-Host
    $viewExecutable=if ($OutputDirectory) { Join-Path ([IO.Path]::GetFullPath($OutputDirectory)) 'gd-view.exe' } else { Join-Path $root 'build/gd-view.exe' }
    $capture = Join-Path $unicode 'matrix.png'
    & $viewExecutable view matrix @unicodeInputs --frames 3 --screenshot $capture
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $capture -PathType Leaf)) { throw 'Bounded view/Unicode capture failed' }
    $bytes = [IO.File]::ReadAllBytes($capture)
    if ($bytes.Length -lt 8 -or [BitConverter]::ToString($bytes[0..7]) -ne '89-50-4E-47-0D-0A-1A-0A') { throw 'Invalid PNG capture' }
    Write-Output "View capture: $capture"
    if ($Graph) {
        foreach ($explorerMode in @('skills','challenges','relationships')) {
            $capture = Join-Path $unicode "explorer-$explorerMode.png"
            $proposal = (& $viewExecutable view skill-atoms @explorerInputs --mode $explorerMode --frames 4 --screenshot $capture | ConvertFrom-Json)
            if ($LASTEXITCODE -ne 0 -or -not $proposal.ok -or $proposal.data.applied -or $proposal.data.changes.Count -or -not (Test-Path -LiteralPath $capture)) { throw "Combined explorer $explorerMode capture/handoff failed" }
        }
        $focusInputs=@($explorerInputs)+@('--related',"mechanics=$example/mechanics",'--related',"nouns=$example/nouns",'--related',"verbs=$example/verbs",'--related',"resources=$example/resources",'--related',"control-vars=$example/control-vars")
        $focusCapture=Join-Path $unicode 'focused-crate-pushing.png'
        $focusReport=(& $viewExecutable view skill-atoms @focusInputs --mode relationships --from mechanics/crate-pushing --frames 3 --screenshot $focusCapture | ConvertFrom-Json)
        if ($LASTEXITCODE -ne 0 -or -not $focusReport.ok -or $focusReport.data.changes.Count -or -not (Test-Path -LiteralPath $focusCapture)) { throw 'Focused record root/capture/proposal failed' }
        $missingRoot=(& $viewExecutable view skill-atoms @focusInputs --mode relationships --from mechanics/absent --frames 1 --json 2>$log | ConvertFrom-Json)
        if ($LASTEXITCODE -ne 2 -or $missingRoot.ok -or $missingRoot.errors[0].code -ne 'view.root') { throw 'Unknown focused root did not fail without a window' }
        foreach ($layout in @('graph', 'columns')) {
            $graphCapture = Join-Path $unicode "$layout.png"
            [string[]]$extra = @()
            if ($layout -eq 'columns') { $extra += '--show-other-pairs' }
            & $viewExecutable view matrix @unicodeInputs --layout $layout @extra --frames 60 --screenshot $graphCapture
            if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $graphCapture -PathType Leaf)) { throw "Graph $layout capture failed" }
            Write-Output "Graph capture: $graphCapture"
        }
        foreach ($skillLayout in @('prerequisites', 'rank')) {
            foreach ($direction in @('down', 'right')) {
                $skillCapture = Join-Path $unicode "skills-$skillLayout-$direction.png"
                $viewReport = & $viewExecutable view skill-atoms --skill-atoms "skill-atoms=$unicode/skill-atoms" --related "mechanics=$example/mechanics" --layout $skillLayout --direction $direction --frames 60 --screenshot $skillCapture | ConvertFrom-Json
                if ($LASTEXITCODE -ne 0 -or -not $viewReport.ok -or $viewReport.data.applied -or $viewReport.data.changes.Count -ne 0 -or -not (Test-Path -LiteralPath $skillCapture -PathType Leaf)) { throw "Skill $skillLayout/$direction capture/report failed" }
                Write-Output "Skill capture: $skillCapture"
            }
        }
        $viewReport = & $viewExecutable view skill-atoms @incompleteInputs --frames 60 --screenshot (Join-Path $unicode 'skills-incomplete.png') | ConvertFrom-Json
        if ($LASTEXITCODE -ne 0 -or -not $viewReport.ok -or $viewReport.coverage.placement_complete) { throw 'Incomplete skill graph capture/report failed' }
        $reportFile = Join-Path $unicode 'empty-changes.json'
        foreach ($mdaLayout in @('columns', 'graph')) {
            $captureMda = Join-Path $unicode "mda-$mdaLayout.png"
            $mdaStdout = @(& $viewExecutable view mda @mdaInputs --from mechanics/crate-pushing --direction forward --layout $mdaLayout --frames 3 --screenshot $captureMda)
            if ($LASTEXITCODE -ne 0 -or $mdaStdout.Count -or -not (Test-Path -LiteralPath $captureMda -PathType Leaf)) { throw "MDA $mdaLayout view capture/stdout failed" }
            Write-Output "MDA capture: $captureMda"
        }
        $mdaCaptureError = (& $viewExecutable view mda @mdaInputs --frames 1 --screenshot (Join-Path $unicode 'missing-directory/mda.png') 2>$log | ConvertFrom-Json)
        if ($LASTEXITCODE -ne 2 -or $mdaCaptureError.ok -or $mdaCaptureError.errors[0].code -ne 'view.screenshot') { throw 'MDA screenshot failure lost structured error/exit' }
        $fileStdout = @(& $viewExecutable view skill-atoms --skill-atoms "skills=$emptySkills" --frames 3 --screenshot (Join-Path $unicode 'skills-empty.png') --output $reportFile)
        if ($LASTEXITCODE -ne 0 -or $fileStdout.Count -ne 0) { throw 'File-output view leaked stdout or failed' }
        $fileReport = Get-Content -LiteralPath $reportFile -Raw -Encoding UTF8 | ConvertFrom-Json
        if (-not $fileReport.ok -or $fileReport.data.changes.Count -ne 0) { throw 'Empty skill graph file report failed' }
        $empty = Join-Path $unicode 'empty'
        foreach ($collection in @('nouns', 'verbs', 'interactions')) { New-Item -ItemType Directory -Path (Join-Path $empty $collection) -Force | Out-Null }
        & $viewExecutable view matrix --layout graph --nouns "nouns=$empty/nouns" --verbs "verbs=$empty/verbs" --interactions "interactions=$empty/interactions" --frames 3 --screenshot (Join-Path $unicode 'empty.png')
        if ($LASTEXITCODE -ne 0) { throw 'Empty graph failed' }
    }
}
foreach ($resourceName in @('crate-puzzle', 'little-plumber-guy-nes')) {
    $resourceGame = Join-Path (Split-Path $root -Parent) "examples/$resourceName/game-design"
    $resourceInputs = @()
    foreach ($role in @('interactions', 'mechanics', 'nouns', 'verbs', 'axes', 'resources', 'control-vars', 'systems')) {
        $resourceInputs += @("--$role", "$role=$(Join-Path $resourceGame $role)")
    }
    $resourceJson = & $executable resources @resourceInputs --json
    if ($LASTEXITCODE -ne 0) { throw "Resource declaration report failed: $resourceName" }
    $resourceData = $resourceJson | ConvertFrom-Json
    if (-not $resourceData.ok -or $resourceData.coverage.complete -or $resourceData.data.routes.available -or $resourceData.coverage.composed_outcomes_available -or -not $resourceData.data.nodes.Count) {
        throw 'Resource incidence report overstated scope or lost retained nodes'
    }
    $resourceJson | Set-Content -LiteralPath (Join-Path $fixtures "resources-$resourceName.json") -Encoding UTF8
}
Write-Output "Resource declaration evidence: $fixtures/resources-*.json"
Write-Output 'CLI checks passed: Resource direct declarations (routes unavailable), matrix/connectivity, components, cases/dispositions, skill prerequisites/ranks/findings, challenge/route opportunity/witnesses, MDA shared queries/aliases/direction/boundaries/duplicates/unknowns/partial identity errors, typed control resolution/overrides/scoped dependencies/status-name selection/JSON-CSV-INI export-import plans/false-enums/formula protection, cycles/missing targets, empty results, field selection, text, Unicode/space paths, structured failure.'
