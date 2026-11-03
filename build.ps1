param([string]$CMake = "cmake")
$ErrorActionPreference = "Stop"
Push-Location $PSScriptRoot
try {
if (!(Get-Command $CMake -ErrorAction SilentlyContinue)) {
    $bundled = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    if (Test-Path -LiteralPath $bundled) { $CMake = $bundled }
    else { throw 'CMake is required. Pass -CMake with its absolute path.' }
}
if (!(Test-Path vendor/Effekseer/CMakeLists.txt)) {
    git clone --depth 1 --branch 1808 https://github.com/effekseer/Effekseer.git vendor/Effekseer
    if ($LASTEXITCODE) { throw 'Effekseer download failed' }
}
$expected = 'b5700082b49bc5f919610880dafddc90211a9a4e'
$actual = git -C vendor/Effekseer rev-parse HEAD
if ($LASTEXITCODE -or $actual -ne $expected) { throw "Expected Effekseer 1808 commit $expected" }
& $CMake -S . -B build -G 'Visual Studio 17 2022' -A Win32
if ($LASTEXITCODE) { throw 'CMake configure failed' }
& $CMake --build build --config Release --target effekseer_hgimg4 effekseer_hgimg4dx collision_math_test --parallel -- /nr:false
if ($LASTEXITCODE) { throw 'DLL build failed' }
Write-Output 'Built dist/effekseer_hgimg4.dll and dist/effekseer_hgimg4dx.dll'
} finally { Pop-Location }
