# Vive Wand Compatibility Patch 2.1.2

For Gingas' Fallout VR Essentials Mod Collection.

HUGE SPECIAL THANKS TO MR. DAVE FOR PROVIDING THE VANILLA openvr_api.dll RESTORE FIX. THAT DISCOVERY IS THE HEART OF THIS PATCH, AND A VERY LARGE SHARE OF THE CREDIT FOR THIS MOD BELONGS TO HIM.

This patch restores a clean HTC Vive Wand baseline for Fallout VR Essentials while keeping the release payload small and focused.

Version `2.1` introduced the scripted dialogue-menu touch-scroll bridge. Version `2.1.1` is the Dave-tested correction that keeps the bridge active across interleaved FO4VRTools left-hand callback samples. Version `2.1.2` keeps the same game behavior and replaces the compiled installer EXE with a readable PowerShell installer script for easier security review.

## What This Patch Does

- Restores Fallout 4 VR's vanilla OpenVR loader through a local installer.
- Lets the collection's default controller auto-detection continue using `iVRPlatformControllerOverride=-1`.
- Makes Virtual Holsters use dominant-hand Grip for holster and unholster.
- Ships Virtual Holsters defaults with display spheres and holster-zone haptics off.
- Restores right-trackpad touch-drag scrolling in scripted vertical dialogue menus.
- Sets Workshop item rotation and distance speed values in the active MO2 profile.

This patch does not ship Valve's `openvr_api.dll`. The included installer copies that file from your own Fallout 4 VR install.

## Installation

The release zip contains one top-level folder:

```text
Vive Wand Compatibility Patch\
```

Extract that whole folder into the Fallout VR Essentials mods folder. Do not extract only the loose folder contents.

The final path should look like:

```text
F:\Gingas Fallout VR Essentials\mods\Vive Wand Compatibility Patch\
```

Then:

1. Open Mod Organizer 2 and enable `Vive Wand Compatibility Patch`.
2. Put this mod lower in MO2's left pane than both `Virtual Holsters` and `Root Fix`.
3. Run this PowerShell installer script from inside the extracted `Vive Wand Compatibility Patch` folder:

```text
Install Vanilla OpenVR for Vive Wands.ps1
```

If Windows does not offer `Run with PowerShell`, open PowerShell in the mod folder and run:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File ".\Install Vanilla OpenVR for Vive Wands.ps1"
```

MO2 conflict priority matters. This mod needs to win:

```text
Root\openvr_api.dll
F4SE\Plugins\VirtualHolsters.ini
```

The installer copies your vanilla Fallout 4 VR OpenVR DLL into:

```text
Root\openvr_api.dll
```

It also updates the active MO2 profile's `fallout4custom.ini`:

```ini
[Workshop]
fItemRotationSpeed=1.0
fItemHoldDistantSpeed=3.0
```

The installer intentionally does not edit `Documents\My Games\Fallout4VR\Fallout4Custom.ini`. If you switch to or create another MO2 profile, run the installer again while that profile is active.

## Included Files

```text
README.md
meta.ini
Install Vanilla OpenVR for Vive Wands.ps1
F4SE\Plugins\VirtualHolsters.ini
F4SE\Plugins\ViveWandCompatibilityPatch.dll
```

`F4SE\Plugins\VirtualHolsters.ini` sets:

```ini
HolsterButtonID = 2
ConfigRotateButtonID = 2
DisplaySpheres = false
EnableHaptics = false
bDisableActivation = false
```

Button ID `2` is Grip in Virtual Holsters' config comments.

`F4SE\Plugins\ViveWandCompatibilityPatch.dll` is a slim F4SE plugin for one behavior: while `DialogueMenu` is open, right-trackpad touch-drag up/down moves the highlighted dialogue option. Right trigger confirm is left alone.

## Verification

After launching through MO2/F4SE, the F4SE log should include:

```text
pluginVersion=213
Registered Vive Wand DialogueMenu touch-scroll bridge with FO4VRTools controller-state callback
```

During a scripted vertical dialogue menu, right-trackpad touch-drag should move the selected response up/down, and right trigger should confirm the selected response.

The current 2.1.2 baseline uses Fallout's controller auto-detect:

```ini
[VR]
iVRPlatformControllerOverride=-1
```

Leave that setting on auto-detect unless you are diagnosing a confirmed controller-identity regression. The value `7` is only a fallback diagnostic for that specific problem.

## Credits

- HUGE SPECIAL THANKS TO MR. DAVE FOR PROVIDING THE VANILLA openvr_api.dll RESTORE FIX. THAT DISCOVERY IS THE HEART OF THIS PATCH, AND A VERY LARGE SHARE OF THE CREDIT FOR THIS MOD BELONGS TO HIM.
- Gingas and the Fallout VR Essentials collection this patch targets.
- Fallout4 VR Tools / FO4VRTools.
- F4SEVR.
- Virtual Holsters.
