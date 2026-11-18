<# Explicit launcher only; never builds, downloads, validates, or repairs records.
Usage: ./gd.ps1 matrix --nouns nouns=PATH --verbs verbs=PATH --interactions interactions=PATH --json
#>
param([Parameter(ValueFromRemainingArguments = $true)][string[]]$CommandArguments)
$ErrorActionPreference = 'Stop'
$name = if ($CommandArguments.Count -gt 0 -and $CommandArguments[0] -eq 'view') { 'gd-view.exe' } else { 'gd.exe' }
# Use explicit override for fresh view builds without replacing a running binary.
$program = if ($name -eq 'gd-view.exe' -and $env:GD_VIEW_EXECUTABLE) { [IO.Path]::GetFullPath($env:GD_VIEW_EXECUTABLE) } else { Join-Path $PSScriptRoot "build/$name" }
if (-not (Test-Path -LiteralPath $program -PathType Leaf)) {
    [Console]::Error.WriteLine("Missing $program. Run native/build.ps1; for views build -View with -UiLibrary and fresh -OutputDirectory, then run that executable directly or set GD_VIEW_EXECUTABLE.")
    exit 2
}
& $program @CommandArguments
exit $LASTEXITCODE
