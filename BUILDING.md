# Building Vive Wand Compatibility Patch 2.1.3

This repository contains the source for the only compiled component in the Vive Wand Compatibility Patch 2.1.3 release:

- `F4SE\Plugins\ViveWandCompatibilityPatch.dll`

The release package does not include an installer EXE or installer script. The OpenVR restore and Workshop INI values are manual installation steps documented in `README.md`.

Built binaries are intentionally not tracked in this source repository.

## Requirements

- Windows
- PowerShell
- Visual Studio 2022 Build Tools or Visual Studio 2022 Community
- MSVC x64 C++ toolchain component: `Microsoft.VisualStudio.Component.VC.Tools.x86.x64`

The build script locates `vcvars64.bat` through `vswhere.exe` first, then falls back to common Visual Studio 2022 install paths.

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
Root\PUT_OPENVR_API_DLL_HERE.txt
F4SE\Plugins\VirtualHolsters.ini
F4SE\Plugins\ViveWandCompatibilityPatch.dll
```

Copy the built DLL from:

```text
Source\ViveWandCompatibilityPatch\package\F4SE\Plugins\ViveWandCompatibilityPatch.dll
```

Do not include:

```text
Root\openvr_api.dll
Root\openvr_api.backup-*.dll
OpenVRInstall.log
Source\
docs\
research\
backups\
release-packages\
*.exe
*.ps1
*.pdb
*.obj
*.exp
*.lib
```

The release package must not redistribute Valve's `openvr_api.dll`. Users manually copy their own Fallout 4 VR `openvr_api.dll` into the installed mod's `Root` folder.

## Reproducibility Note

`RELEASE_MANIFEST_2.1.3.md` records the hashes of the files in the uploaded Nexus archive. A local rebuild of the F4SE DLL may produce a different binary hash depending on the MSVC toolchain version, source checkout path, and linker output details.

The review source and build script are intended to show exactly what code is built and how to rebuild it, not to guarantee byte-identical output on every machine.

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
