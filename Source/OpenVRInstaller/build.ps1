param(
    [ValidateSet('Release', 'Debug')]
    [string]$Configuration = 'Release'
)

$ErrorActionPreference = 'Stop'

$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$ModRoot = Split-Path -Parent (Split-Path -Parent $ProjectRoot)
$Source = Join-Path $ProjectRoot 'src\main.cpp'
$Dist = Join-Path $ProjectRoot 'dist'
$PackageDir = Join-Path $ProjectRoot 'package'
$ToolBaseName = 'Install Vanilla OpenVR for Vive Wands'
$OutExe = Join-Path $Dist "$ToolBaseName.exe"
$OutObj = Join-Path $Dist "$ToolBaseName.obj"
$CompilerPdb = Join-Path $Dist "$ToolBaseName.vc.pdb"

New-Item -ItemType Directory -Force -Path $Dist | Out-Null
New-Item -ItemType Directory -Force -Path $PackageDir | Out-Null

function Find-VcVars {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path -LiteralPath $vswhere) {
        $installPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($installPath) {
            $candidate = Join-Path $installPath 'VC\Auxiliary\Build\vcvars64.bat'
            if (Test-Path -LiteralPath $candidate) {
                return $candidate
            }
        }
    }

    $fallbacks = @(
        "$env:ProgramFiles\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat",
        "$env:ProgramFiles\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    )

    foreach ($fallback in $fallbacks) {
        if (Test-Path -LiteralPath $fallback) {
            return $fallback
        }
    }

    return $null
}

$vcvars = Find-VcVars
if (!$vcvars) {
    throw "MSVC x64 toolchain not found. Install the Visual Studio C++ workload/component Microsoft.VisualStudio.Component.VC.Tools.x86.x64, then rerun this script."
}

$runtime = if ($Configuration -eq 'Debug') { '/MTd' } else { '/MT' }
$opt = if ($Configuration -eq 'Debug') { '/Od /Zi' } else { '/O2' }

$clCommand = @(
    'cl',
    '/nologo',
    '/std:c++17',
    '/EHsc',
    '/W4',
    '/DUNICODE',
    '/D_UNICODE',
    $runtime,
    $opt,
    "/Fo:`"$OutObj`"",
    "/Fd:`"$CompilerPdb`"",
    "`"$Source`"",
    "/Fe:`"$OutExe`"",
    '/link',
    '/NOLOGO',
    '/SUBSYSTEM:WINDOWS',
    'User32.lib',
    'Shell32.lib',
    'Ole32.lib',
    'Advapi32.lib',
    'Version.lib'
) -join ' '

$cmd = "`"$vcvars`" && $clCommand"
cmd.exe /c $cmd
if ($LASTEXITCODE -ne 0) {
    throw "cl.exe failed with exit code $LASTEXITCODE"
}

$PackageExe = Join-Path $PackageDir "$ToolBaseName.exe"
$ModRootExe = Join-Path $ModRoot "$ToolBaseName.exe"
Copy-Item -LiteralPath $OutExe -Destination $PackageExe -Force
Copy-Item -LiteralPath $OutExe -Destination $ModRootExe -Force

Write-Host "Built $OutExe"
Write-Host "Packaged EXE at $PackageExe"
Write-Host "Installed EXE at $ModRootExe"
