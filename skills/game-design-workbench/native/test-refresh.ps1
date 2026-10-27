<#
.SYNOPSIS
Capture live stale/reload/failure/recovery behavior on private example copies.
.DESCRIPTION
Windows desktop and Windows PowerShell 5.1 required. Uses an already-built graph-enabled gd-view.exe; never
builds/downloads or edits shipped records. Runs matrix, skill/progression, or MDA views.
Posts F5 only to owned test window. Never moves pointer, clicks, types text,
or injects global keyboard input. Rank/prerequisite controls and Ctrl+Z remain
manual checks. Arranges owned window for visible client-area captures, not whole
desktop captures. Temporary edits are restored byte-for-byte in finally;
only the owned child process is closed. Copies/logs/PNGs stay in ignored build/.
Screenshots require visual review; this is not an OCR/asserted UI-state test.
.EXAMPLE
./test-refresh.ps1
.EXAMPLE
./test-refresh.ps1 -Mode skills -SettleMilliseconds 2500
#>
param(
    [ValidateSet('both', 'matrix', 'matrix-graph', 'skills', 'challenges', 'relationships', 'mda')][string]$Mode = 'both',
    [ValidateRange(1500, 10000)][int]$SettleMilliseconds = 1800,
    [string]$ViewExecutable = (Join-Path $PSScriptRoot 'build/gd-view.exe')
)
$ErrorActionPreference = 'Stop'
if ($env:OS -ne 'Windows_NT') { throw 'This visible desktop check requires Windows.' }
if ($PSVersionTable.PSVersion.Major -ne 5) { throw "Desktop capture helper requires Windows PowerShell 5.1. Run powershell.exe -NoProfile -File `"$PSCommandPath`" -Mode $Mode" }
if (-not (Test-Path -LiteralPath $ViewExecutable -PathType Leaf)) {
    throw 'Missing view executable. Run build.ps1 -Graph with an explicitly prepared UI DLL and raylib SDK first.'
}
Add-Type -AssemblyName System.Drawing, System.Windows.Forms
Add-Type -ReferencedAssemblies System.Drawing, System.Windows.Forms -TypeDefinition @'
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.Threading;
using System.Text;
using System.Windows.Forms;
public static class GdRefreshDesktop {
    [StructLayout(LayoutKind.Sequential)] public struct Rect { public int Left, Top, Right, Bottom; }
    [StructLayout(LayoutKind.Sequential)] public struct Point { public int X, Y; }
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    delegate bool WindowCallback(IntPtr window, IntPtr data);
    [DllImport("user32.dll")] static extern bool EnumWindows(WindowCallback callback, IntPtr data);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] static extern int GetWindowText(IntPtr window, StringBuilder text, int capacity);
    [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr window, out uint process);
    [DllImport("user32.dll")] static extern bool GetClientRect(IntPtr window, out Rect rectangle);
    [DllImport("user32.dll")] static extern bool ClientToScreen(IntPtr window, ref Point point);
    [DllImport("user32.dll")] static extern bool ShowWindow(IntPtr window, int command);
    [DllImport("user32.dll")] static extern bool SetForegroundWindow(IntPtr window);
    [DllImport("user32.dll")] static extern bool SetWindowPos(IntPtr window, IntPtr after, int x, int y, int width, int height, uint flags);
    [DllImport("user32.dll", CharSet=CharSet.Unicode, SetLastError=true)] static extern bool PostMessage(IntPtr window, uint message, IntPtr key, IntPtr flags);
    [DllImport("user32.dll")] static extern uint MapVirtualKey(uint key, uint map);
    public static IntPtr FindOwnedWindow(int process, string title) {
        IntPtr found = IntPtr.Zero;
        EnumWindows(delegate(IntPtr window, IntPtr data) {
            uint owner; GetWindowThreadProcessId(window, out owner);
            if (owner != (uint)process) return true;
            StringBuilder text = new StringBuilder(256); GetWindowText(window, text, text.Capacity);
            if (text.ToString() != title) return true;
            found = window; return false;
        }, IntPtr.Zero);
        return found;
    }
    public static void Arrange(IntPtr window) {
        Rectangle area = Screen.FromHandle(window).WorkingArea;
        if (area.Width < 1024 || area.Height < 720) throw new Exception("Desktop working area is too small for this check.");
        ShowWindow(window, 9);
        if (!SetWindowPos(window, new IntPtr(-1), area.X+12, area.Y+12, Math.Min(1456,area.Width-24), Math.Min(939,area.Height-24), 0x40))
            throw new Exception("Cannot arrange owned test window.");
        SetForegroundWindow(window);
    }
    public static void Refresh(IntPtr window) {
        uint scan = MapVirtualKey(0x74, 0) << 16;
        if (!PostMessage(window, 0x100, new IntPtr(0x74), new IntPtr((int)(1u|scan)))) throw new Exception("Cannot post F5 down.");
        Thread.Sleep(120);
        if (!PostMessage(window, 0x101, new IntPtr(0x74), new IntPtr(unchecked((int)(0xc0000001u|scan))))) throw new Exception("Cannot post F5 up.");
    }
    public static void Capture(IntPtr window, string path) {
        Rect bounds; Point origin = new Point();
        if (!GetClientRect(window, out bounds) || !ClientToScreen(window, ref origin)) throw new Exception("Cannot locate test client area.");
        int width = bounds.Right-bounds.Left, height = bounds.Bottom-bounds.Top;
        if (width <= 0 || height <= 0) throw new Exception("Empty test client area.");
        using (Bitmap image = new Bitmap(width, height)) {
            using (Graphics graphics = Graphics.FromImage(image)) graphics.CopyFromScreen(origin.X, origin.Y, 0, 0, image.Size);
            image.Save(path, ImageFormat.Png);
        }
    }
    public static void Close(IntPtr window) { PostMessage(window, 0x10, IntPtr.Zero, IntPtr.Zero); }
}
'@
[void][GdRefreshDesktop]::SetProcessDPIAware()
$example = Join-Path (Split-Path $PSScriptRoot -Parent) 'examples/crate-puzzle/game-design'
$artifacts = Join-Path $PSScriptRoot ('build/refresh-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $artifacts | Out-Null
$utf8 = New-Object System.Text.UTF8Encoding($false, $true)
$reports = @()
$modes = if ($Mode -eq 'both') { @('matrix', 'skills') } else { @($Mode) }
foreach ($current in $modes) {
    $directory = Join-Path $artifacts $current
    $inputDirectory = Join-Path $directory 'input'
    New-Item -ItemType Directory -Path $inputDirectory -Force | Out-Null
    $roles = if ($current -in @('matrix','matrix-graph')) { @('nouns', 'verbs', 'interactions', 'mechanics') } elseif ($current -eq 'mda') { @('mechanics', 'systems', 'dynamics', 'aesthetics') } elseif ($current -eq 'challenges') { @('skill-atoms', 'challenges', 'mechanics') } else { @('skill-atoms', 'mechanics') }
    foreach ($role in $roles) { Copy-Item -LiteralPath (Join-Path $example $role) -Destination $inputDirectory -Recurse }
    if ($current -in @('matrix','matrix-graph')) {
        $relativeTarget = 'interactions/free-push.md'
        $pattern = '(?m)^  "coverage": "defined"(?=\r?$)'
        $changed = '  "coverage": "unknown"'; $invalid = '  "coverage": ["invalid"]'
        $arguments = @('view', 'matrix', '--nouns', "nouns=$inputDirectory/nouns", '--verbs', "verbs=$inputDirectory/verbs", '--interactions', "interactions=$inputDirectory/interactions")
        $title = 'Game Design Workbench ' + [char]0x2014 + ' interaction matrix'
        if ($current -eq 'matrix-graph') { $arguments += @('--layout','graph'); $title='Game Design Workbench - matrix graph' }
    } elseif ($current -eq 'mda') {
        $relativeTarget = 'aesthetics/deliberate-tension.md'
        $pattern = '(?m)^  "dynamics": \[\r?\n    "dynamics/careful-positioning"\r?\n  \],(?=\r?$)'
        $changed = '  "dynamics": [],'; $invalid = '  "dynamics": [123],'
        $arguments = @('view', 'mda', '--mechanics', "mechanics=$inputDirectory/mechanics", '--systems', "systems=$inputDirectory/systems", '--dynamics', "dynamics=$inputDirectory/dynamics", '--aesthetics', "aesthetics=$inputDirectory/aesthetics")
        $title = 'Game Design Workbench - MDA explorer'
    } elseif ($current -eq 'relationships') {
        $relativeTarget = 'skill-atoms/coordinate.md'
        $pattern = '(?m)^prerequisites:\r?\n  - skill-atoms/plan-push\r?\n  - skill-atoms/route-switch(?=\r?$)'
        $changed = 'prerequisites: []'; $invalid = 'prerequisites: [123]'
        $arguments = @('view', 'skill-atoms', '--skill-atoms', "skill-atoms=$inputDirectory/skill-atoms", '--mode', 'relationships', '--from', 'skill-atoms/coordinate')
        $title = 'Game Design Workbench - skill and progression explorer'
    } elseif ($current -eq 'challenges') {
        $relativeTarget = 'challenges/push-room.md'
        $pattern = '(?m)^next:\r?\n  - challenges/switch-practice\r?\n  - challenges/combined-room(?=\r?$)'
        $changed = 'next: [challenges/switch-practice]'; $invalid = 'next: [123]'
        $arguments = @('view', 'skill-atoms', '--skill-atoms', "skill-atoms=$inputDirectory/skill-atoms", '--challenges', "challenges=$inputDirectory/challenges", '--mode', 'challenges')
        $title = 'Game Design Workbench - skill and progression explorer'
    } else {
        $relativeTarget = 'skill-atoms/coordinate.md'
        $pattern = '(?m)^rank: 2(?=\r?$)'
        $changed = 'rank: 4'; $invalid = 'rank: invalid-refresh-test'
        $arguments = @('view', 'skill-atoms', '--skill-atoms', "skill-atoms=$inputDirectory/skill-atoms", '--layout', 'rank')
        $title = 'Game Design Workbench - skill and progression explorer'
    }
    if ($current -ne 'mda') { $arguments += @('--related', "mechanics=$inputDirectory/mechanics") }
    $target = Join-Path $inputDirectory $relativeTarget
    $source = Join-Path $example $relativeTarget
    $sourceHash = (Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash
    $inputHashes = @{}
    Get-ChildItem -LiteralPath $inputDirectory -Recurse -File | ForEach-Object { $inputHashes[$_.FullName] = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash }
    $original = [IO.File]::ReadAllBytes($target)
    $originalText = $utf8.GetString($original)
    if ([regex]::Matches($originalText, $pattern).Count -ne 1) { throw "Expected one baseline field in $target" }
    $process = $null; $window = [IntPtr]::Zero; $captures = @()
    try {
        # Start-Process joins arguments without quoting; quote complete arguments
        # so explicit collection mappings also work under paths containing spaces.
        $quoted = @($arguments | ForEach-Object { '"' + $_ + '"' })
        $process = Start-Process -FilePath (Resolve-Path -LiteralPath $ViewExecutable).Path -ArgumentList $quoted -WorkingDirectory $PSScriptRoot -RedirectStandardOutput (Join-Path $directory 'stdout.txt') -RedirectStandardError (Join-Path $directory 'stderr.txt') -PassThru
        # Keep process handle open so Windows PowerShell can read ExitCode after close.
        $null = $process.Handle
        $deadline = [DateTime]::UtcNow.AddSeconds(12)
        do {
            if ($process.HasExited) { throw "View exited before test: $($process.ExitCode). See $directory/stderr.txt" }
            $window = [GdRefreshDesktop]::FindOwnedWindow($process.Id, $title)
            if ($window -ne [IntPtr]::Zero) { break }
            Start-Sleep -Milliseconds 100
        } while ([DateTime]::UtcNow -lt $deadline)
        if ($window -eq [IntPtr]::Zero) { throw "Cannot find owned view window: $title" }
        [GdRefreshDesktop]::Arrange($window)
        $stages = @('01-before', '02-stale-valid', '03-refreshed-valid', '04-stale-invalid', '05-refresh-failed')
        if ($current -eq 'relationships') { $stages += @('05b-root-removed', '05c-root-reload-blocked') }
        $stages += @('06-restored-stale', '07-restored-refreshed')
        foreach ($stage in $stages) {
            switch ($stage) {
                '02-stale-valid' { [IO.File]::WriteAllText($target, [regex]::Replace($originalText, $pattern, $changed), $utf8) }
                '04-stale-invalid' { [IO.File]::WriteAllText($target, [regex]::Replace($originalText, $pattern, $invalid), $utf8) }
                '05b-root-removed' { [IO.File]::WriteAllText($target, [regex]::Replace($originalText, '(?m)^name: coordinate(?=\r?$)', 'name: coordinate-renamed'), $utf8) }
                '06-restored-stale' { [IO.File]::WriteAllBytes($target, $original) }
                { $_ -in @('03-refreshed-valid', '05-refresh-failed', '05c-root-reload-blocked', '07-restored-refreshed') } { [GdRefreshDesktop]::Refresh($window) }
            }
            Start-Sleep -Milliseconds $SettleMilliseconds
            if ($process.HasExited) { throw "View exited during $stage" }
            $capture = Join-Path $directory "$stage.png"
            [GdRefreshDesktop]::Capture($window, $capture)
            $captures += $capture
        }
    } finally {
        [IO.File]::WriteAllBytes($target, $original)
        if ($process -and -not $process.HasExited) {
            if ($window -ne [IntPtr]::Zero) { [GdRefreshDesktop]::Close($window) }
            if (-not $process.WaitForExit(5000)) { Stop-Process -Id $process.Id -Force }
        }
    }
    $restoredHash = (Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash
    if ($restoredHash -ne $sourceHash) { throw "Temporary record did not restore exactly: $target" }
    if ((Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash -ne $sourceHash) { throw "Shipped source changed during check: $source" }
    if ($process.ExitCode -ne 0) { throw "View closed with exit $($process.ExitCode)" }
    if ($current -eq 'mda' -and [IO.File]::ReadAllText((Join-Path $directory 'stdout.txt')).Length) { throw 'Read-only MDA view wrote unexpected stdout data.' }
    foreach ($path in $inputHashes.Keys) { if ((Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash -ne $inputHashes[$path]) { throw "Input changed: $path" } }
    $reports += [ordered]@{ view = $current; captures = $captures; restored_sha256 = $restoredHash; source_unchanged = $true; process_exit = $process.ExitCode }
}
$report = [ordered]@{ artifacts = $artifacts; visual_review_required = $true; checks = $reports } | ConvertTo-Json -Depth 5
[IO.File]::WriteAllText((Join-Path $artifacts 'captures.json'), $report, $utf8)
Write-Output $report
