param(
    [Parameter(Mandatory = $true)]
    [string] $GameDirectory,
    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[A-Za-z0-9_-]+$')]
    [string] $GameName,
    [ValidateSet('Build', 'Test', 'Package', 'Deploy')]
    [string] $Action = 'Build',
    [ValidateSet('Debug', 'Release')]
    [string] $Configuration = 'Debug'
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$source = (Resolve-Path -LiteralPath $GameDirectory).Path
if (-not (Test-Path -LiteralPath (Join-Path $source 'CMakeLists.txt'))) {
    throw "No CMakeLists.txt found in $source."
}
if ($Action -eq 'Package' -and $Configuration -ne 'Release') {
    throw 'Packaging requires -Configuration Release.'
}
foreach ($tool in @('git', 'cmake', 'ctest')) {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
        throw "$tool was not found. Install Git and CMake and restart your terminal."
    }
}

function Invoke-Tool {
    param([string] $Executable, [string[]] $ToolArguments)
    & $Executable @ToolArguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Executable failed with exit code $LASTEXITCODE."
    }
}

$build = Join-Path $source "build\$Configuration"
$stage = Join-Path $build 'stage'
$gameStage = Join-Path $stage "games\$GameName"
Invoke-Tool 'cmake' @('-S', $source, '-B', $build, '-G', 'Visual Studio 17 2022', '-A', 'x64',
    "-DARCADE_DEPLOYMENT_DIR=$stage", '-DBUILD_TESTING=ON')
Invoke-Tool 'cmake' @('--build', $build, '--config', $Configuration)
if ($Action -in @('Test', 'Package')) {
    Invoke-Tool 'ctest' @('--test-dir', $build, '-C', $Configuration, '--output-on-failure')
}
if (-not (Test-Path -LiteralPath (Join-Path $gameStage "$GameName.exe")) -or
    -not (Test-Path -LiteralPath (Join-Path $gameStage 'meta.json'))) {
    throw "Game deployment is incomplete: $gameStage. Check GameName and the game's CMake deployment."
}

if ($Action -eq 'Package') {
    $output = Join-Path $root 'out\games'
    New-Item -ItemType Directory -Force -Path $output | Out-Null
    $archive = Join-Path $output "$GameName-Windows-Release.zip"
    Compress-Archive -Path (Join-Path $gameStage '*') -DestinationPath $archive -Force
    Write-Warning 'This is a development package, not a distribution-ready release. Review licenses and runtime dependencies before sharing.'
    Write-Host "Package: $archive"
}
if ($Action -eq 'Deploy') {
    $destination = Join-Path $root "Deployment\$Configuration\games\$GameName"
    New-Item -ItemType Directory -Force -Path $destination | Out-Null
    Get-ChildItem -LiteralPath $gameStage -Force | Copy-Item -Destination $destination -Recurse -Force
    Write-Host "Deployed: $destination"
}
Write-Host "Game staging directory: $gameStage"
