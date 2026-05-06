# Security Review Notes

This source repository is provided so Nexus Mods can review the executable components in Vive Wand Compatibility Patch 2.1.1.

## Executable Components

The release package contains two executable components:

- `Install Vanilla OpenVR for Vive Wands.exe`
- `F4SE\Plugins\ViveWandCompatibilityPatch.dll`

Both are built from the source under `Source\`.

## Installer Behavior

`Install Vanilla OpenVR for Vive Wands.exe` is a local Win32 utility. It is intended to be run from inside the installed MO2 mod folder.

It does:

- locate the installed patch mod folder from the EXE path
- locate the Fallout 4 VR install path through local Steam/registry paths or a user-selected folder
- copy the user's own Fallout 4 VR `openvr_api.dll` into this mod's `Root\openvr_api.dll`
- create `Root\` if needed
- back up an existing `Root\openvr_api.dll` before replacing it
- locate the active Mod Organizer 2 profile when possible
- update only the active MO2 profile `fallout4custom.ini` with:

```ini
[Workshop]
fItemRotationSpeed=1.0
fItemHoldDistantSpeed=3.0
```

It does not:

- download files
- contact the network
- bundle or redistribute Valve's `openvr_api.dll`
- edit `Documents\My Games\Fallout4VR\Fallout4Custom.ini`
- modify SteamVR bindings
- install services
- create scheduled tasks
- add autorun entries
- write outside the installed mod folder and active MO2 profile path, except for standard Windows file picker interaction when the user manually selects a Fallout 4 VR folder

The installer writes a local `OpenVRInstall.log` beside the EXE. That log is generated at runtime and is not included in the Nexus package.

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
- ship custom control maps
- modify Pip-Boy, QuickContainer, Workshop, gameplay movement, sprint, sneak, favorites, or MCM behavior
- redistribute third-party DLLs

## Third-Party Header

The DLL source includes `Source\ViveWandCompatibilityPatch\vendor\openvr\openvr.h`, the OpenVR SDK header needed to compile against OpenVR controller-state types. The release package does not include Valve's `openvr_api.dll`.
