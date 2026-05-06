# Release Manifest 2.1.4

Public mod version: `2.1.4`

Internal F4SE DLL build identifier: `pluginVersion=213`

Version note: `2.1.4` is a packaging-only update. It keeps the Dave-tested build `213` dialogue fix and repackages the release as a flat MO2-friendly archive.

Current Nexus upload archive:

```text
Vive-Wand-Compatibility-Patch-2.1.4.zip
```

Archive size:

```text
132353 bytes
```

Archive SHA256:

```text
03CA804F78F0D4D34764958151A29198387F7515578F50C90EEE49D517FEC01E
```

These hashes identify the uploaded Nexus archive and its included runtime files. Local rebuilds from this source may produce different binary hashes depending on the MSVC toolchain version, source checkout path, and linker output details.

## Runtime Zip Contents

The zip is a flat MO2-friendly archive. Its root contains exactly these runtime files:

```text
README.md
meta.ini
Root/PUT_OPENVR_API_DLL_HERE.txt
F4SE/Plugins/VirtualHolsters.ini
F4SE/Plugins/ViveWandCompatibilityPatch.dll
```

## Runtime File Hashes

```text
6CD10F6BF4126DBD2B38EC7A39B52A6CF331178D85AD9146A0C1ECD2E4ED1198  README.md
E4D162F4E2B6633E3D77C4B43CD3EF13EE1AC319582D26BA670769C270F99B6F  meta.ini
EF4569C019D3FB13EF34DF5A2D687750C73D35CF1E3CBFAD0311DC318FD4E2F0  Root\PUT_OPENVR_API_DLL_HERE.txt
2E0BD4D78D80241102121B5181A2651FD16F4887519F7FA541141A626C461A84  F4SE\Plugins\VirtualHolsters.ini
99286AF254AF9C640E511B7962E7187F0F2B1ABBC30114141445FFED4A23834D  F4SE\Plugins\ViveWandCompatibilityPatch.dll
```

## Runtime Binary Sizes

```text
254976  F4SE\Plugins\ViveWandCompatibilityPatch.dll
```

## Excluded From Release Zip

The release archive must not include:

```text
Vive Wand Compatibility Patch\
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
