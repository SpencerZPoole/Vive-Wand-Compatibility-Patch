# Building Vive Wand Compatibility Patch 2.1.1

This repository contains the source for the two executable components in the Vive Wand Compatibility Patch 2.1.1 release:

- `Install Vanilla OpenVR for Vive Wands.exe`
- `F4SE\Plugins\ViveWandCompatibilityPatch.dll`

The release package itself is runtime-only. Built binaries are intentionally not tracked in this source repository.

## Requirements

- Windows
- PowerShell
- Visual Studio 2022 Build Tools or Visual Studio 2022 Community
- MSVC x64 C++ toolchain component: `Microsoft.VisualStudio.Component.VC.Tools.x86.x64`

Both build scripts locate `vcvars64.bat` through `vswhere.exe` first, then fall back to common Visual Studio 2022 install paths.

## Build the Installer EXE

From the repository root:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Source\OpenVRInstaller\build.ps1 -Configuration Release
```

The script builds:

```text
Source\OpenVRInstaller\dist\Install Vanilla OpenVR for Vive Wands.exe
```

It also copies the EXE to:

```text
Source\OpenVRInstaller\package\Install Vanilla OpenVR for Vive Wands.exe
Install Vanilla OpenVR for Vive Wands.exe
```

Those generated files are ignored by git.

## Build the F4SE DLL

From the repository root:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Source\ViveWandCompatibilityPatch\build.ps1 -Configuration Release
```

The script builds:

```text
Source\ViveWandCompatibilityPatch\dist\ViveWandCompatibilityPatch.dll
```

It also copies the runtime DLL to:

```text
Source\ViveWandCompatibilityPatch\package\F4SE\Plugins\ViveWandCompatibilityPatch.dll
```

That generated output is ignored by git.

## Assemble the Runtime Package

The Nexus release zip should contain one top-level folder:

```text
Vive Wand Compatibility Patch\
```

Inside that folder, include exactly:

```text
README.md
meta.ini
Install Vanilla OpenVR for Vive Wands.exe
F4SE\Plugins\VirtualHolsters.ini
F4SE\Plugins\ViveWandCompatibilityPatch.dll
```

Copy the built EXE from:

```text
Source\OpenVRInstaller\package\Install Vanilla OpenVR for Vive Wands.exe
```

Copy the built DLL from:

```text
Source\ViveWandCompatibilityPatch\package\F4SE\Plugins\ViveWandCompatibilityPatch.dll
```

Do not include:

```text
Root\openvr_api.dll
OpenVRInstall.log
Source\
docs\
research\
backups\
release-packages\
*.pdb
*.obj
*.exp
*.lib
```

The release package must not redistribute Valve's `openvr_api.dll`. The installer copies the user's own Fallout 4 VR OpenVR DLL locally after installation.

## Reproducibility Note

`RELEASE_MANIFEST_2.1.1.md` records the hashes of the binaries in the uploaded Nexus archive. A local rebuild from this source may produce different binary hashes depending on the MSVC toolchain version, source checkout path, and linker output details.

The review source and build scripts are intended to show exactly what code is built and how to rebuild it, not to guarantee byte-identical output on every machine.

## Verification Commands

Confirm the F4SE DLL exports only the two F4SE entry points:

```powershell
dumpbin /EXPORTS .\F4SE\Plugins\ViveWandCompatibilityPatch.dll
```

Expected exports:

```text
F4SEPlugin_Load
F4SEPlugin_Query
```

Confirm the internal plugin build identifier:

```powershell
Select-String -Path .\Source\ViveWandCompatibilityPatch\src\main.cpp -Pattern 'kPluginVersion'
```

Expected value:

```text
kPluginVersion = 213
```
