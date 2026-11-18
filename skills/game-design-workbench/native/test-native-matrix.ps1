<#
Build/run native standalone matrix qualification; default creates no window.
Usage: ./test-native-matrix.ps1 -UiLibrary PATH [-SourceDirectory PATH] [-Render]
       [-Cjit PATH] [-OutputDirectory NEW-DIRECTORY]
-Render requires separate approval: one owned hidden non-activating matrix,
three frames, PNG in fresh output. No OS input or existing viewer control.
SourceDirectory contains nouns/, verbs/, interactions/; defaults Crate fixture.
Snapshot source SHA256 before/after. Tests preserve loaded model, canonical
axes, count colors and cell hit boundaries. Rendering is not human acceptance.
Stdout: JSON result. Logs: stderr. Exit 0 success, 2 failure.
#>
param([string]$UiLibrary='',[string]$SourceDirectory='',[switch]$Render,[string]$Cjit='cjit',[string]$OutputDirectory='')
$ErrorActionPreference='Stop'
try {
    $root=$PSScriptRoot
    if(-not $SourceDirectory){$SourceDirectory=Join-Path $root '../examples/crate-puzzle/game-design'}
    $source=[IO.Path]::GetFullPath($SourceDirectory)
    function Get-SourceSnapshot {
        $snapshot=@{}
        foreach($name in @('nouns','verbs','interactions')) {
            $directory=Join-Path $source $name
            if(-not(Test-Path -LiteralPath $directory -PathType Container)){throw "Missing fixture collection: $directory"}
            Get-ChildItem -LiteralPath $directory -Filter '*.md' -File -Force | ForEach-Object {$snapshot[$_.FullName]=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash}
        }
        return $snapshot
    }
    $before=Get-SourceSnapshot
    if(!$OutputDirectory){$OutputDirectory=Join-Path $root ('build/native-matrix-check-'+[guid]::NewGuid().ToString('N'))}
    $executable=& (Join-Path $root 'build.ps1') -MatrixTest -UiLibrary $UiLibrary -Cjit $Cjit -OutputDirectory $OutputDirectory
    $output=Split-Path $executable -Parent
    $png=Join-Path $output 'native-matrix-Σ.png'
    [string[]]$arguments=@($source)
    if($Render){$arguments=@($source,$png,'--render')}
    $reply=& $executable @arguments
    if($LASTEXITCODE -ne 0){throw "Native matrix check failed with exit $LASTEXITCODE"}
    $checked=$reply | ConvertFrom-Json
    if(-not $checked.ok -or $checked.rendered -ne [bool]$Render){throw 'Native matrix check did not report expected scope.'}
    $after=Get-SourceSnapshot
    if($before.Count -ne $after.Count){throw 'Source file set changed during check; no source was restored/overwritten.'}
    foreach($path in $before.Keys){if($before[$path] -ne $after[$path]){throw "Source changed during check: $path; no source was restored/overwritten."}}
    if($Render) {
        Add-Type -AssemblyName System.Drawing
        $image=[Drawing.Bitmap]::new($png)
        try {
            if($image.Width -ne 1280 -or $image.Height -ne 800){throw 'Native matrix PNG dimensions differ from requested client area.'}
            $ink=0
            :titleSearch for($y=18;$y -lt 48;$y+=2) {
                for($x=20;$x -lt 320;$x+=2) {
                    $pixel=$image.GetPixel($x,$y)
                    if($pixel.R -gt 200 -and $pixel.G -gt 200 -and $pixel.B -gt 200){$ink++;if($ink -ge 8){break titleSearch}}
                }
            }
            if($ink -lt 8){throw 'Native matrix PNG lacks expected title text ink.'}
        } finally {$image.Dispose()}
    }
    $checked | Add-Member -NotePropertyName source_files_unchanged -NotePropertyValue $true
    $checked | Add-Member -NotePropertyName executable -NotePropertyValue $executable
    $checked | Add-Member -NotePropertyName executable_sha256 -NotePropertyValue (Get-FileHash -LiteralPath $executable -Algorithm SHA256).Hash
    $checked | Add-Member -NotePropertyName library_sha256 -NotePropertyValue (Get-FileHash -LiteralPath (Join-Path $output 'gd-native-ui.dll') -Algorithm SHA256).Hash
    if($Render){$checked | Add-Member -NotePropertyName png -NotePropertyValue $png}
    $checked | ConvertTo-Json -Compress -EscapeHandling EscapeNonAscii
} catch {
    $message=$_.Exception.Message;[Console]::Error.WriteLine($message)
    @{ok=$false;code='native_matrix.check';message=$message} | ConvertTo-Json -Compress -EscapeHandling EscapeNonAscii
    exit 2
}
