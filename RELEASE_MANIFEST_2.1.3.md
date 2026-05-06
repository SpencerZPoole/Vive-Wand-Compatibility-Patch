# Release Manifest 2.1.3

Public mod version: `2.1.3`

Internal F4SE DLL build identifier: `pluginVersion=213`

Version note: `2.1` introduced the scripted dialogue-menu touch-scroll fix. `2.1.1` is the Dave-tested correction for the build `212` regression where interleaved non-right-hand FO4VRTools callback samples reset the right-hand dialogue drag session before it could emit selection steps.

Packaging note: `2.1.2` attempted a no-EXE PowerShell installer repack. `2.1.3` keeps the same game behavior as `2.1.1` and removes all installer/script automation. The OpenVR restore and Workshop INI values are manual installation steps.

Current Nexus upload archive:

```text
Vive-Wand-Compatibility-Patch-2.1.3.zip
```

Archive size:

```text
132620 bytes
```

Archive SHA256:

```text
C9CD16C494736CC797499B55E7FDDF1974D3B8973EF1E2F3D91457107D0D8BA4
```

These hashes identify the uploaded Nexus archive and its included runtime files. Local rebuilds from this source may produce different binary hashes depending on the MSVC toolchain version, source checkout path, and linker output details.

## Runtime Zip Contents

The zip contains one top-level folder named `Vive Wand Compatibility Patch\` and exactly these runtime files:

```text
Vive Wand Compatibility Patch/README.md
Vive Wand Compatibility Patch/meta.ini
Vive Wand Compatibility Patch/Root/PUT_OPENVR_API_DLL_HERE.txt
Vive Wand Compatibility Patch/F4SE/Plugins/VirtualHolsters.ini
Vive Wand Compatibility Patch/F4SE/Plugins/ViveWandCompatibilityPatch.dll
```

## Runtime File Hashes

```text
5FA5055D39CC07D9BD2160DE70A2022ECA7A06BE24FC73D212A683195A0C2F91  README.md
131C1D27FA39992D8334403694D724B2C096D20E09BB9D6CE65312426FC3D03E  meta.ini
EF4569C019D3FB13EF34DF5A2D687750C73D35CF1E3CBFAD0311DC318FD4E2F0  Root\PUT_OPENVR_API_DLL_HERE.txt
2E0BD4D78D80241102121B5181A2651FD16F4887519F7FA541141A626C461A84  F4SE\Plugins\VirtualHolsters.ini
3129C5975ACBE5BB4BA47A7D459C2A3B25A9A58F9D16844403CC1ECAA3B60502  F4SE\Plugins\ViveWandCompatibilityPatch.dll
```

## Runtime Binary Sizes

```text
254976  F4SE\Plugins\ViveWandCompatibilityPatch.dll
```

## Excluded From Release Zip

The release archive must not include:

```text
Root\openvr_api.dll
Root\openvr_api.backup-*.dll
Install Vanilla OpenVR for Vive Wands.exe
Install Vanilla OpenVR for Vive Wands.ps1
Source\
docs\
research\
backups\
release-packages\
OpenVRInstall.log
MCM\
CustomControlMap.txt
*.exe
*.ps1
*.pdb
*.obj
*.exp
*.lib
```

Users manually copy their own Fallout 4 VR `openvr_api.dll` into `Root\openvr_api.dll`. The archive does not redistribute Valve's DLL.
