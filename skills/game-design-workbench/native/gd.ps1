<# Explicit launcher only; never builds, downloads, validates, or repairs records.
Usage: ./gd.ps1 matrix --nouns nouns=PATH --verbs verbs=PATH --interactions interactions=PATH --json
#>
param([Parameter(ValueFromRemainingArguments = $true)][string[]]$CommandArguments)
$ErrorActionPreference = 'Stop'
$name = if ($CommandArguments.Count -gt 0 -and $CommandArguments[0] -eq 'view') { 'gd-view.exe' } else { 'gd.exe' }
$program = Join-Path $PSScriptRoot "build/$name"
if (-not (Test-Path -LiteralPath $program -PathType Leaf)) {
    [Console]::Error.WriteLine("Missing $program. Run native/build.ps1; add -View -RaylibDirectory PATH for matrix tables, or -Graph for unified explorer, matrix graphs, skill/progression, and MDA views.")
    exit 2
}
& $program @CommandArguments
exit $LASTEXITCODE
