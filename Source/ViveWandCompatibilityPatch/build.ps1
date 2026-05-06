param(
    [ValidateSet('Release', 'Debug')]
    [string]$Configuration = 'Release'
)

$ErrorActionPreference = 'Stop'

$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$OpenVrInclude = Join-Path $ProjectRoot 'vendor\openvr'
$Source = Join-Path $ProjectRoot 'src\main.cpp'
$Dist = Join-Path $ProjectRoot 'dist'
$PluginBaseName = 'ViveWandCompatibilityPatch'
$OutDll = Join-Path $Dist "$PluginBaseName.dll"
$OutObj = Join-Path $Dist "$PluginBaseName.obj"
$CompilerPdb = Join-Path $Dist "$PluginBaseName.vc.pdb"
$DefFile = Join-Path $ProjectRoot 'exports.def'

if (!(Test-Path -LiteralPath $OpenVrInclude)) {
    throw "Missing local OpenVR header directory at $OpenVrInclude."
}

New-Item -ItemType Directory -Force -Path $Dist | Out-Null

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
    '/LD',
    $runtime,
    $opt,
    "/Fo:`"$OutObj`"",
    "/Fd:`"$CompilerPdb`"",
    "`"$Source`"",
    "/I`"$OpenVrInclude`"",
    "/Fe:`"$OutDll`"",
    '/link',
    '/NOLOGO',
    '/SUBSYSTEM:WINDOWS',
    "/DEF:`"$DefFile`"",
    'Shell32.lib',
    'User32.lib'
) -join ' '

$cmd = "`"$vcvars`" && $clCommand"
cmd.exe /c $cmd
if ($LASTEXITCODE -ne 0) {
    throw "cl.exe failed with exit code $LASTEXITCODE"
}

$PackagePluginDir = Join-Path $ProjectRoot 'package\F4SE\Plugins'
New-Item -ItemType Directory -Force -Path $PackagePluginDir | Out-Null
Copy-Item -LiteralPath $OutDll -Destination (Join-Path $PackagePluginDir "$PluginBaseName.dll") -Force
$OutPdb = [System.IO.Path]::ChangeExtension($OutDll, '.pdb')
$PackagePdb = Join-Path $PackagePluginDir "$PluginBaseName.pdb"
if (Test-Path -LiteralPath $PackagePdb) {
    Remove-Item -LiteralPath $PackagePdb -Force
}

Write-Host "Built $OutDll"
Write-Host "Packaged DLL at $(Join-Path $PackagePluginDir "$PluginBaseName.dll")"
Write-Host "PDB retained in dist only when generated; release package output is runtime DLL only."
