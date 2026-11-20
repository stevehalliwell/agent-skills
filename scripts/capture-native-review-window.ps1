<#
.SYNOPSIS
Capture visible pixels inside one authorized native review window.
.DESCRIPTION
Usage: pwsh -NoProfile -File scripts/capture-native-review-window.ps1 -ProcessId PID [-OutputDirectory PATH] [-BringToFront]
Default: read-only screen capture; no focus, geometry, close or input changes.
-BringToFront explicitly focuses only this authorized window before capture; never restores a minimized window.
Window must be visible and unobscured. Output uses unique PNG name; no overwrite.
Default artifacts: skills/game-design-workbench/native/build/manual-review-captures/.
#>
param(
    [Parameter(Mandatory=$true)][ValidateRange(1,2147483647)][int]$ProcessId,
    [string]$OutputDirectory='',
    [switch]$BringToFront
)
$ErrorActionPreference='Stop'
if (![OperatingSystem]::IsWindows()) { throw 'Windows desktop required.' }
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class NativeReviewCapture {
    [StructLayout(LayoutKind.Sequential)] public struct Bounds { public int Left, Top, Right, Bottom; }
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr window, out Bounds bounds);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr window);
    [DllImport("user32.dll")] public static extern bool IsIconic(IntPtr window);
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr window);
    [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
}
'@
[NativeReviewCapture]::SetProcessDPIAware() | Out-Null
$process=Get-Process -Id $ProcessId
$window=$process.MainWindowHandle
if (!$window -or ![NativeReviewCapture]::IsWindowVisible($window) -or [NativeReviewCapture]::IsIconic($window)) { throw 'Process has no visible, non-minimized main window.' }
if ($BringToFront -and [NativeReviewCapture]::GetForegroundWindow() -ne $window) {
    if (![NativeReviewCapture]::SetForegroundWindow($window)) { throw 'Windows refused requested focus; bring review window forward manually.' }
    Start-Sleep -Milliseconds 200
    if ([NativeReviewCapture]::GetForegroundWindow() -ne $window) { throw 'Review window did not become foreground; no capture taken.' }
}
$bounds=[NativeReviewCapture+Bounds]::new()
if (![NativeReviewCapture]::GetWindowRect($window,[ref]$bounds)) { throw 'Cannot read window bounds.' }
$width=$bounds.Right-$bounds.Left; $height=$bounds.Bottom-$bounds.Top
if ($width -le 0 -or $height -le 0 -or [long]$width*$height -gt 16777216) { throw 'Invalid window bounds or capture exceeds 16777216 pixels.' }
if (!$OutputDirectory) { $OutputDirectory=Join-Path (Split-Path $PSScriptRoot -Parent) 'skills/game-design-workbench/native/build/manual-review-captures' }
[IO.Directory]::CreateDirectory([IO.Path]::GetFullPath($OutputDirectory)) | Out-Null
$output=Join-Path $OutputDirectory ("review-$ProcessId-"+[guid]::NewGuid().ToString('N')+'.png')
$bitmap=$null; $graphics=$null
try {
    $bitmap=[Drawing.Bitmap]::new($width,$height); $graphics=[Drawing.Graphics]::FromImage($bitmap)
    $graphics.CopyFromScreen($bounds.Left,$bounds.Top,0,0,[Drawing.Size]::new($width,$height))
    $bitmap.Save($output,[Drawing.Imaging.ImageFormat]::Png)
} finally {
    if ($graphics) { $graphics.Dispose() }; if ($bitmap) { $bitmap.Dispose() }
}
@{ok=$true;process_id=$ProcessId;image=[IO.Path]::GetFullPath($output);width=$width;height=$height;focus_requested=[bool]$BringToFront;scope='visible window pixels only; no pointer/keyboard input or geometry changes; occlusion not removed'} | ConvertTo-Json -Compress
