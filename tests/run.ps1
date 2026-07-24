# Build and run all AstroCore unit-test suites, then print a summary.
#
#   pwsh tests/run.ps1                 # build + run everything
#   pwsh tests/run.ps1 -Filter gcode*  # only suites whose exe matches tst_<filter>
#   pwsh tests/run.ps1 -NoBuild        # skip build, just run existing exes
#
# Defaults target the Qt 6.11.1 llvm-mingw kit; override with -QtKit / -Toolchain
# or the QT_KIT / QT_TOOLCHAIN env vars if your install differs.

param(
    [string]$QtKit      = $(if ($env:QT_KIT)       { $env:QT_KIT }       else { "C:\Programy\Qt\6.11.1\llvm-mingw_64" }),
    [string]$Toolchain  = $(if ($env:QT_TOOLCHAIN) { $env:QT_TOOLCHAIN } else { "C:\Programy\Qt\Tools\llvm-mingw1706_64" }),
    [string]$BuildDir   = "$PSScriptRoot\..\build\tests",
    [string]$Filter     = "*",
    [switch]$NoBuild
)

$ErrorActionPreference = "Stop"
$env:PATH = "$QtKit\bin;$Toolchain\bin;$env:PATH"

if (-not $NoBuild) {
    New-Item -ItemType Directory -Force $BuildDir | Out-Null
    Push-Location $BuildDir
    try {
        & qmake "$PSScriptRoot\tests.pro"
        if ($LASTEXITCODE -ne 0) { throw "qmake failed" }
        & mingw32-make -j4
        if ($LASTEXITCODE -ne 0) { throw "build failed" }
    } finally {
        Pop-Location
    }
}

$exes = Get-ChildItem -Recurse -Filter "tst_*.exe" $BuildDir |
        Where-Object { $_.BaseName -like "tst_$Filter" }

if (-not $exes) { Write-Host "No test executables found in $BuildDir" -ForegroundColor Yellow; exit 1 }

$anyFailed = $false
foreach ($exe in $exes) {
    # QtTest stdout can be swallowed by some shells, so write results to a file
    # and read them back. Exit code is authoritative (non-zero = failures).
    $out = [System.IO.Path]::GetTempFileName()
    & $exe.FullName -o "$out,txt" | Out-Null
    $code = $LASTEXITCODE
    $totals = (Get-Content $out | Select-String "^Totals:").Line
    $fails  = (Get-Content $out | Select-String "^FAIL!").Line

    $color = if ($code -eq 0) { "Green" } else { "Red"; $anyFailed = $true }
    Write-Host ("{0,-22} {1}" -f $exe.BaseName, $totals) -ForegroundColor $color
    if ($fails) { $fails | ForEach-Object { Write-Host "   $_" -ForegroundColor Red } }
    Remove-Item $out -ErrorAction SilentlyContinue
}

if ($anyFailed) { Write-Host "`nSOME TESTS FAILED" -ForegroundColor Red; exit 1 }
Write-Host "`nALL SUITES PASSED" -ForegroundColor Green
