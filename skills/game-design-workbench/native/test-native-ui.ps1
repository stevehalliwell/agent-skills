<#
Default: headless native GUI dependency check. No OS input or source writes.
Usage: ./test-native-ui.ps1 -UiLibrary PATH [-Cjit PATH] [-OutputDirectory NEW-DIRECTORY] [-Gpu]
-Gpu explicitly creates one owned hidden non-activating window for device/render/
resize/shutdown/PNG checks. PNG decode uses installed .NET System.Drawing,
not another compiler. Requires authorization when window checks are deferred.
Requires matching ImGui 1.93 gd-native-ui.dll from build-native-ui.ps1. Builds C99
check with CJIT, copies library beside owned executable and runs ABI/error checks.
Stdout: one JSON result. Logs: stderr. Exit 0 success, 2 failure.
#>
param([string]$UiLibrary='', [string]$Cjit='cjit', [string]$OutputDirectory='', [switch]$Gpu)
$ErrorActionPreference='Stop'
try {
    if(-not $IsWindows){throw 'This check covers Windows native backend; macOS check remains pending.'}
    if(-not $UiLibrary -or -not(Test-Path -LiteralPath $UiLibrary -PathType Leaf)){throw 'Supply prepared gd-native-ui.dll with -UiLibrary.'}
    if((Split-Path $UiLibrary -Leaf) -ne 'gd-native-ui.dll'){throw 'Expected gd-native-ui.dll; do not rename the dependency.'}
    $root=$PSScriptRoot
    $output=if($OutputDirectory){[IO.Path]::GetFullPath($OutputDirectory)}else{Join-Path $root ('build/native-ui-checks-'+[guid]::NewGuid().ToString('N'))}
    if((Test-Path -LiteralPath $output) -and @(Get-ChildItem -LiteralPath $output -Force).Count){throw 'Output must be new/empty; existing files retained.'}
    New-Item -ItemType Directory -Force $output | Out-Null
    $library=Join-Path $output 'gd-native-ui.dll';Copy-Item -LiteralPath $UiLibrary -Destination $library
    $executable=Join-Path $output 'test-native-ui.exe'
    $arguments=@('-q','-I',(Join-Path $root 'vendor/cimgui'),(Join-Path $root 'test_native_ui.c'),(Join-Path $root 'gd_native_window.c'),(Join-Path $root 'gd_native_canvas.c'),$library,(Join-Path ([Environment]::GetFolderPath('System')) 'user32.dll'),'-o',$executable)
    & $Cjit @arguments | ForEach-Object {[Console]::Error.WriteLine($_)}
    if($LASTEXITCODE -ne 0){throw "C99 ABI check build failed with exit $LASTEXITCODE"}
    [string[]]$testArguments=@()
    if($Gpu){$testArguments=@('--gpu')}
    $previousPng=[Environment]::GetEnvironmentVariable('GD_NATIVE_UI_TEST_PNG','Process')
    $png=Join-Path $output 'native-canvas-Σ.png'
    try {
        if($Gpu){[Environment]::SetEnvironmentVariable('GD_NATIVE_UI_TEST_PNG',$png,'Process')}
        $reply=& $executable @testArguments
        if($LASTEXITCODE -ne 0){throw "C99 ABI check failed with exit $LASTEXITCODE"}
    } finally {[Environment]::SetEnvironmentVariable('GD_NATIVE_UI_TEST_PNG',$previousPng,'Process')}
    $checked=$reply | ConvertFrom-Json
    if(-not $checked.ok){throw 'C99 ABI check did not report success.'}
    if($Gpu) {
        Add-Type -AssemblyName System.Drawing
        $image=[Drawing.Bitmap]::new($png)
        try {
            $pixel=$image.GetPixel($image.Width-1,$image.Height-1)
            if([math]::Abs($pixel.R-20) -gt 1 -or [math]::Abs($pixel.G-23) -gt 1 -or [math]::Abs($pixel.B-31) -gt 1 -or $pixel.A -ne 255){throw 'PNG decoded background differs from native RGBA clear; channel conversion failed.'}
            $white=0
            :glyphSearch for($y=80;$y -lt 150;$y+=2) {
                for($x=32;$x -lt 400;$x+=2) {
                    $ink=$image.GetPixel($x,$y)
                    if($ink.R -gt 220 -and $ink.G -gt 220 -and $ink.B -gt 220 -and $ink.A -gt 220){$white++;if($white -ge 8){break glyphSearch}}
                }
            }
            if($white -lt 8){throw 'PNG lacks expected canvas text ink; font/vertex rendering failed.'}
        } finally {$image.Dispose()}
        $checked | Add-Member -NotePropertyName png_pixel_verified -NotePropertyValue $true
        $checked | Add-Member -NotePropertyName png_canvas_text_verified -NotePropertyValue $true
        $checked.checks+=2
    }
    $checked | Add-Member -NotePropertyName executable_sha256 -NotePropertyValue (Get-FileHash -LiteralPath $executable -Algorithm SHA256).Hash
    $checked | Add-Member -NotePropertyName library_sha256 -NotePropertyValue (Get-FileHash -LiteralPath $library -Algorithm SHA256).Hash
    $checked | ConvertTo-Json -Compress -EscapeHandling EscapeNonAscii
} catch {
    $message=$_.Exception.Message;[Console]::Error.WriteLine($message)
    @{ok=$false;code='native_ui.check';message=$message} | ConvertTo-Json -Compress -EscapeHandling EscapeNonAscii
    exit 2
}
