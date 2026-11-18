param(
    [ValidateSet('Build', 'Test', 'Package')]
    [string] $Action = 'Build',
    [ValidateSet('Debug', 'Release')]
    [string] $Configuration = 'Debug',
    [string] $QtPath = $env:QT_ROOT_DIR
)

$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot

function Invoke-Tool {
    param([string] $Executable, [string[]] $ToolArguments)
    & $Executable @ToolArguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Executable failed with exit code $LASTEXITCODE."
    }
}

if ($Action -eq 'Package' -and $Configuration -ne 'Release') {
    throw 'Packaging requires -Configuration Release.'
}

foreach ($tool in @('git', 'cmake', 'ctest')) {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
        throw "$tool was not found. Install Git and CMake and restart your terminal."
    }
}

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path -LiteralPath $vswhere)) {
    throw 'Install Visual Studio 2022 with the Desktop development with C++ workload.'
}
$visualStudio = & $vswhere -version '[17.0,18.0)' -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if ($LASTEXITCODE -ne 0 -or -not $visualStudio) {
    throw 'Visual Studio 2022 C++ tools were not found. Install the Desktop development with C++ workload.'
}

if (-not $QtPath) {
    $qtBase = 'C:\Qt'
    if (Test-Path -LiteralPath $qtBase) {
        $kits = @(Get-ChildItem -LiteralPath $qtBase -Directory |
            ForEach-Object { Join-Path $_.FullName 'msvc2022_64' } |
            Where-Object { Test-Path -LiteralPath (Join-Path $_ 'lib\cmake\Qt6\Qt6Config.cmake') })
        if ($kits.Count -eq 1) { $QtPath = $kits[0] }
        elseif ($kits.Count -gt 1) { throw 'Multiple Qt kits found. Specify -QtPath explicitly.' }
    }
}
if (-not $QtPath -or -not (Test-Path -LiteralPath (Join-Path $QtPath 'lib\cmake\Qt6\Qt6Config.cmake'))) {
    throw 'Qt 6 MSVC 2022 64-bit kit not found. Install it and specify -QtPath or set QT_ROOT_DIR.'
}
$QtPath = (Resolve-Path -LiteralPath $QtPath).Path
$deployQt = Join-Path $QtPath 'bin\windeployqt.exe'
if (-not (Test-Path -LiteralPath $deployQt)) {
    throw "windeployqt.exe not found in $QtPath."
}

$buildRoot = Join-Path $root "build\windows\$Configuration"
$mainBuild = Join-Path $buildRoot 'main'
$sfml2Build = Join-Path $buildRoot 'sfml2'
$deployment = Join-Path $root "Deployment\$Configuration"
$generator = @('-G', 'Visual Studio 17 2022', '-A', 'x64')

Invoke-Tool 'cmake' (@('-S', $root, '-B', $mainBuild) + $generator +
    @("-DCMAKE_PREFIX_PATH=$QtPath", "-DARCADE_DEPLOYMENT_DIR=$deployment", '-DBUILD_TESTING=ON'))
Invoke-Tool 'cmake' @('--build', $mainBuild, '--config', $Configuration)
Invoke-Tool 'cmake' (@('-S', (Join-Path $root 'Source\Games\TemplateSFML2'), '-B', $sfml2Build) + $generator +
    @("-DARCADE_DEPLOYMENT_DIR=$deployment", '-DBUILD_TESTING=ON'))
Invoke-Tool 'cmake' @('--build', $sfml2Build, '--config', $Configuration)

if ($Action -in @('Test', 'Package')) {
    Invoke-Tool 'ctest' @('--test-dir', $mainBuild, '-C', $Configuration, '--output-on-failure')
    Invoke-Tool 'ctest' @('--test-dir', $sfml2Build, '-C', $Configuration, '--output-on-failure')
}

$qtConfiguration = if ($Configuration -eq 'Debug') { '--debug' } else { '--release' }
Invoke-Tool $deployQt @($qtConfiguration, '--qmldir', (Join-Path $root 'Source\Launcher'),
    (Join-Path $deployment 'ArcadeLauncher.exe'))
Write-Host "Launcher: $(Join-Path $deployment 'ArcadeLauncher.exe')"

if ($Action -eq 'Package') {
    $output = Join-Path $root 'out'
    New-Item -ItemType Directory -Force -Path $output | Out-Null
    $archive = Join-Path $output 'ArcadePortfolio-Windows-Release.zip'
    Compress-Archive -Path (Join-Path $deployment '*') -DestinationPath $archive -Force
    Write-Warning 'This is a development package, not a distribution-ready release. Review Qt, SFML, third-party licenses, source availability, and runtime dependencies before sharing.'
    Write-Host "Package: $archive"
}
