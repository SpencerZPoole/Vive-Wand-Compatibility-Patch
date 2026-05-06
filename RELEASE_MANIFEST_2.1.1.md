# Release Manifest 2.1.1

Public mod version: `2.1.1`

Internal F4SE DLL build identifier: `pluginVersion=213`

Version note: `2.1` introduced the scripted dialogue-menu touch-scroll fix. `2.1.1` is the Dave-tested correction for the build `212` regression where interleaved non-right-hand FO4VRTools callback samples reset the right-hand dialogue drag session before it could emit selection steps.

Current Nexus upload archive on disk:

```text
Vive-Wand-Compatibility-Patch-2.1.1.zip
```

Archive size:

```text
311909 bytes
```

Archive SHA256:

```text
C9BE54B0E32D2ACDEE8C43BBDA40F1F9C378E3F7417969E39EF2E1B7E1670451
```

These hashes identify the uploaded Nexus archive and its included runtime files. Local rebuilds from this source may produce different binary hashes depending on the MSVC toolchain version, source checkout path, and linker output details.

## Runtime Zip Contents

The zip contains one top-level folder named `Vive Wand Compatibility Patch\` and exactly these runtime files:

```text
Vive Wand Compatibility Patch/README.md
Vive Wand Compatibility Patch/meta.ini
Vive Wand Compatibility Patch/Install Vanilla OpenVR for Vive Wands.exe
Vive Wand Compatibility Patch/F4SE/Plugins/VirtualHolsters.ini
Vive Wand Compatibility Patch/F4SE/Plugins/ViveWandCompatibilityPatch.dll
```

## Runtime File Hashes

```text
91F7D3E2B8413E0CDC988A8AF2371913A753FD513686296B4064C11FCAB5D31A  README.md
2BF6539962F736DB5F92DED6B36F90C65BCCAB8417966DAA0D88F1A93B91C383  meta.ini
D18D48B92ADB1A9A3088419AE3C8D07B7E5E2D6575ECF259A9110145B310BA3E  Install Vanilla OpenVR for Vive Wands.exe
2E0BD4D78D80241102121B5181A2651FD16F4887519F7FA541141A626C461A84  F4SE\Plugins\VirtualHolsters.ini
3129C5975ACBE5BB4BA47A7D459C2A3B25A9A58F9D16844403CC1ECAA3B60502  F4SE\Plugins\ViveWandCompatibilityPatch.dll
```

## Runtime Binary Sizes

```text
378880  Install Vanilla OpenVR for Vive Wands.exe
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
```

The installer creates `Root\openvr_api.dll` locally by copying the user's own Fallout 4 VR file after install. The archive does not redistribute Valve's DLL.
