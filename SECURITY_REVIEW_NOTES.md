# Security Review Notes

This source repository is provided so Nexus Mods can review the compiled component in Vive Wand Compatibility Patch 2.1.3.

## Executable Components

The release package contains one compiled binary component:

- `F4SE\Plugins\ViveWandCompatibilityPatch.dll`

It is built from the source under `Source\`.

The release package does not include:

- installer EXE files
- PowerShell installer scripts
- Valve's `openvr_api.dll`
- custom control maps
- MCM files
- ESP/ESL plugins

## Manual OpenVR Restore

The OpenVR restore is now a manual installation step. Users copy their own vanilla Fallout 4 VR `openvr_api.dll` into:

```text
Vive Wand Compatibility Patch\Root\openvr_api.dll
```

The package includes only:

```text
Root\PUT_OPENVR_API_DLL_HERE.txt
```

That placeholder makes the destination folder visible without redistributing Valve's DLL.

## Manual Workshop INI Values

The Workshop settings are also manual. Users open MO2's INI Editor for the active profile, select `Fallout4Custom.ini`, and set:

```ini
[Workshop]
fItemRotationSpeed=1.0
fItemHoldDistantSpeed=3.0
```

The mod does not edit `Documents\My Games\Fallout4VR\Fallout4Custom.ini` or any MO2 profile INI automatically.

## F4SE DLL Behavior

`F4SE\Plugins\ViveWandCompatibilityPatch.dll` is a small F4SEVR plugin.

It does:

- load through F4SEVR
- register a FO4VRTools controller-state callback
- detect when `DialogueMenu` is open
- convert right Vive Wand trackpad touch-drag up/down into `DialogueMenu` selection movement
- leave right trigger confirm untouched
- write a low-noise plugin log for load, callback registration, menu detection, and dialogue scroll steps

It does not:

- download files
- contact the network
- patch SteamVR bindings
- copy or move files
- edit INI files
- modify Pip-Boy, QuickContainer, Workshop, gameplay movement, sprint, sneak, favorites, or MCM behavior
- redistribute third-party DLLs

## Third-Party Header

The DLL source includes `Source\ViveWandCompatibilityPatch\vendor\openvr\openvr.h`, the OpenVR SDK header needed to compile against OpenVR controller-state types. The release package does not include Valve's `openvr_api.dll`.
