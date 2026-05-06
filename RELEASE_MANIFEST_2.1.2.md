# Release Manifest 2.1.2

Public mod version: `2.1.2`

Internal F4SE DLL build identifier: `pluginVersion=213`

Version note: `2.1` introduced the scripted dialogue-menu touch-scroll fix. `2.1.1` is the Dave-tested correction for the build `212` regression where interleaved non-right-hand FO4VRTools callback samples reset the right-hand dialogue drag session before it could emit selection steps.

Packaging note: `2.1.2` keeps the same game behavior as `2.1.1` and replaces the compiled installer EXE with the readable PowerShell script `Install Vanilla OpenVR for Vive Wands.ps1`.

Current Nexus upload archive on disk:

```text
Vive-Wand-Compatibility-Patch-2.1.2.zip
```

Archive size:

```text
135267 bytes
```

Archive SHA256:

```text
8F6E5E67E92DAE7F95BB6081DC3AD78B7E631A726F4BBB6C232A2B35FFD3C734
```

These hashes identify the uploaded Nexus archive and its included runtime files. Local rebuilds from this source may produce different binary hashes depending on the MSVC toolchain version, source checkout path, and linker output details.

## Runtime Zip Contents

The zip contains one top-level folder named `Vive Wand Compatibility Patch\` and exactly these runtime files:

```text
Vive Wand Compatibility Patch/README.md
Vive Wand Compatibility Patch/meta.ini
Vive Wand Compatibility Patch/Install Vanilla OpenVR for Vive Wands.ps1
Vive Wand Compatibility Patch/F4SE/Plugins/VirtualHolsters.ini
Vive Wand Compatibility Patch/F4SE/Plugins/ViveWandCompatibilityPatch.dll
```

## Runtime File Hashes

```text
4A826677B6377B053FD815CFCCD6B7147649B002B8FB221657D3D4D01703F55C  README.md
7E09DC867F59D3BE67ED7991AC907412C786A08E350722F5D95F892859D52644  meta.ini
489834B8BB8E9958CA1430735F4F9C9DA3D8EDAC386BDBE8717CE010BB60A2AB  Install Vanilla OpenVR for Vive Wands.ps1
2E0BD4D78D80241102121B5181A2651FD16F4887519F7FA541141A626C461A84  F4SE\Plugins\VirtualHolsters.ini
3129C5975ACBE5BB4BA47A7D459C2A3B25A9A58F9D16844403CC1ECAA3B60502  F4SE\Plugins\ViveWandCompatibilityPatch.dll
```

## Runtime Binary Sizes

```text
10626   Install Vanilla OpenVR for Vive Wands.ps1
254976  F4SE\Plugins\ViveWandCompatibilityPatch.dll
```

## Excluded From Release Zip

The release archive must not include:

```text
Root\openvr_api.dll
Root\openvr_api.backup-*.dll
Source\
docs\
research\
backups\
release-packages\
OpenVRInstall.log
MCM\
CustomControlMap.txt
*.pdb
*.obj
*.exp
*.lib
*.exe
```

The installer creates `Root\openvr_api.dll` locally by copying the user's own Fallout 4 VR file after install. The archive does not redistribute Valve's DLL.
