param([string]$HspDir = 'C:\hsp37')
$ErrorActionPreference = 'Stop'
Push-Location (Join-Path $PSScriptRoot 'dist')
try {
$nativeTest=Join-Path $PSScriptRoot 'build/Release/collision_math_test.exe'
if (!(Test-Path -LiteralPath $nativeTest)) { throw 'Run build.ps1 before test.ps1' }
$nativeResult=& $nativeTest
if ($LASTEXITCODE) { throw 'Native collision test failed' }
$nativeResult | Set-Content -LiteralPath collision-math-result.txt -Encoding utf8
Write-Output $nativeResult
foreach ($mode in @('gl','dx')) {
    $suffix = if ($mode -eq 'dx') { '_dx' } else { '' }
    $runtime = if ($mode -eq 'dx') { 'hsp3gpdx.exe' } else { 'hsp3gp.exe' }
    foreach ($file in @('api-result.txt','smoke-result.txt','error.txt','smoke.rgba','backend-info.txt',"api-result-$mode.txt","smoke-result-$mode.txt","backend-info-$mode.txt","smoke-$mode.rgba")) {
        if (Test-Path -LiteralPath $file) { Remove-Item -LiteralPath $file }
    }
    foreach ($file in @('collision-result.txt','collision-smoke-result.txt','collision.rgba',"collision-result-$mode.txt","collision-smoke-result-$mode.txt","collision-$mode.rgba")) {
        if (Test-Path -LiteralPath $file) { Remove-Item -LiteralPath $file }
    }
    foreach ($name in @("api_test$suffix","sample$suffix","collision_test$suffix","collision_sample$suffix")) {
        if (Test-Path -LiteralPath "$name.ax") { Remove-Item -LiteralPath "$name.ax" }
        & "$HspDir\hspcmp.exe" "--compath=$HspDir\common\" -d "$name.hsp"
        if (!(Test-Path -LiteralPath "$name.ax")) { throw "Compile failed: $name" }
        $process = Start-Process -FilePath "$HspDir\$runtime" -ArgumentList "$name.ax --smoke" -WorkingDirectory (Get-Location).Path -WindowStyle Hidden -PassThru
        if (!$process.WaitForExit(20000)) {
            Stop-Process -Id $process.Id
            throw "Runtime timed out: $name"
        }
        $process.Refresh()
        if ($process.ExitCode -ne 0) { throw "$name failed: exit $($process.ExitCode). See *-result.txt / error.txt" }
    }
    foreach ($name in @('api-result','smoke-result','collision-result','collision-smoke-result')) {
        $result = Get-Content -LiteralPath "$name.txt" -Raw
        if (!$result.StartsWith('PASS:')) { throw $result }
        Copy-Item -LiteralPath "$name.txt" -Destination "$name-$mode.txt"
        Write-Output "$mode : $result"
    }
    if ((Get-Item -LiteralPath smoke.rgba).Length -ne 1920000) { throw 'Framebuffer capture size mismatch' }
    Copy-Item -LiteralPath smoke.rgba -Destination "smoke-$mode.rgba"
    if ((Get-Item -LiteralPath collision.rgba).Length -ne 2340000) { throw 'Collision capture size mismatch' }
    Copy-Item -LiteralPath collision.rgba -Destination "collision-$mode.rgba"
    Copy-Item -LiteralPath backend-info.txt -Destination "backend-info-$mode.txt"
    Get-Content -LiteralPath "backend-info-$mode.txt"
}
} finally { Pop-Location }
