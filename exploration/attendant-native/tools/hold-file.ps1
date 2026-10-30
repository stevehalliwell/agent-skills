# Test helper: hold an existing file read-only without delete sharing.
# Usage: powershell -NoProfile -File hold-file.ps1 -Path <file> [-TimeoutSeconds 30]
# Prints LOCKED; release by sending a line/EOF on stdin. Never changes file bytes.
param(
    [Parameter(Mandatory = $true)][string]$Path,
    [ValidateRange(1, 300)][int]$TimeoutSeconds = 30
)
$ErrorActionPreference = 'Stop'
$file = [System.IO.File]::Open((Resolve-Path -LiteralPath $Path).Path,
    [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read, [System.IO.FileShare]::Read)
try {
    [Console]::WriteLine('LOCKED')
    [Console]::Out.Flush()
    $release = [Console]::In.ReadLineAsync()
    [void]$release.Wait($TimeoutSeconds * 1000)
} finally {
    $file.Dispose()
}
