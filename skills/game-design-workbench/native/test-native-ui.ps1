<#
Default: headless native GUI dependency check. No OS input or source writes.
Usage: ./test-native-ui.ps1 -UiLibrary PATH [-Cjit PATH] [-OutputDirectory NEW-DIRECTORY] [-Gpu]
-Gpu explicitly creates one owned hidden non-activating window for device/render/
resize/shutdown checks. Requires authorization when window checks are deferred.
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
    $arguments=@('-q','-I',(Join-Path $root 'vendor/cimgui'),(Join-Path $root 'test_native_ui.c'),$library,'-l','user32','-o',$executable)
    & $Cjit @arguments | ForEach-Object {[Console]::Error.WriteLine($_)}
    if($LASTEXITCODE -ne 0){throw "C99 ABI check build failed with exit $LASTEXITCODE"}
    [string[]]$testArguments=@()
    if($Gpu){$testArguments=@('--gpu')}
    $reply=& $executable @testArguments
    if($LASTEXITCODE -ne 0){throw "C99 ABI check failed with exit $LASTEXITCODE"}
    $checked=$reply | ConvertFrom-Json
    if(-not $checked.ok){throw 'C99 ABI check did not report success.'}
    $checked | Add-Member -NotePropertyName library_sha256 -NotePropertyValue (Get-FileHash -LiteralPath $library -Algorithm SHA256).Hash
    $checked | ConvertTo-Json -Compress
} catch {
    $message=$_.Exception.Message;[Console]::Error.WriteLine($message)
    @{ok=$false;code='native_ui.check';message=$message} | ConvertTo-Json -Compress
    exit 2
}
