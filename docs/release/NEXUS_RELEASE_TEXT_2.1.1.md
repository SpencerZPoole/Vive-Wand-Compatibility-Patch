# Vive Wand Compatibility Patch 2.1.1

## Short Description

Restores HTC Vive Wand support for Gingas Fallout VR Essentials with a vanilla OpenVR restore, Vive-friendly Virtual Holsters settings, scripted dialogue menu touch-scroll, and Workshop item handling fixes.

## Summary

Unofficial HTC Vive Wand compatibility patch for Gingas' Fallout VR Essentials.

This patch is specifically designed for Gingas' Fallout VR Essentials and the Vive Wand profile. It is not a general-purpose Fallout 4 VR controller overhaul.

Version 2.1.1 starts from a clean baseline: restore Fallout 4 VR's vanilla `openvr_api.dll`, keep the collection's controller auto-detect path, and add only the small fixes still needed for the current tested setup.

Version 2.1 introduced the scripted dialogue-menu touch-scroll bridge. Version 2.1.1 is the Dave-tested correction that keeps the bridge active across interleaved FO4VRTools left-hand callback samples.

HUGE SPECIAL THANKS TO MR. DAVE FOR PROVIDING THE VANILLA openvr_api.dll RESTORE FIX. THAT DISCOVERY IS THE HEART OF THIS PATCH, AND A VERY LARGE SHARE OF THE CREDIT FOR THIS MOD BELONGS TO HIM.

## What This Patch Does

- Restores the vanilla Fallout 4 VR OpenVR loader by copying the user's own `openvr_api.dll` into this mod's `Root` folder.
- Keeps the current Vive Wand baseline on Fallout's controller auto-detect setting: `iVRPlatformControllerOverride=-1`.
- Makes Virtual Holsters use dominant-hand Grip for holster and unholster.
- Ships Virtual Holsters with `DisplaySpheres=false` and `EnableHaptics=false`.
- Restores right-trackpad touch-drag scrolling in scripted vertical dialogue menus.
- Sets Workshop item rotation and held-distance speed values in the active MO2 profile.

This release does not redistribute Valve's `openvr_api.dll`. The included installer copies it locally from your own Fallout 4 VR installation.

## Controls / Behavior After Installation

- Scripted vertical dialogue menus: right-trackpad touch-drag up/down changes the highlighted dialogue option.
- Scripted vertical dialogue menus: right trigger confirms the selected dialogue option.
- Virtual Holsters: dominant-hand Grip holsters and unholsters weapons.
- Virtual Holsters: display spheres are off by default.
- Virtual Holsters: holster-zone haptics are off by default.
- Workshop object handling uses these active-profile values:

```ini
[Workshop]
fItemRotationSpeed=1.0
fItemHoldDistantSpeed=3.0
```

The rest of the controller behavior comes from the vanilla OpenVR restore plus the Fallout VR Essentials control stack.

## Requirements

- Fallout 4 VR.
- Gingas Fallout VR Essentials.
- Mod Organizer 2.
- HTC Vive Wand controllers.
- F4SEVR.
- Fallout4 VR Tools / FO4VRTools.
- Virtual Holsters.

This patch was prepared for the Fallout 4 VR runtime 1.2.72 / F4SEVR 0.6.21 setup used by the collection.

## Installation

1. Download the release archive.
2. Open the zip and confirm it contains a top-level folder named:

```text
Vive Wand Compatibility Patch\
```

3. Extract that whole folder into your Fallout VR Essentials mods folder. Do not extract only the loose folder contents.

The final path should look like:

```text
F:\Gingas Fallout VR Essentials\mods\Vive Wand Compatibility Patch\
```

4. Open Mod Organizer 2 and enable `Vive Wand Compatibility Patch`.
5. Put this mod lower in MO2's left pane than both `Virtual Holsters` and `Root Fix`.
6. Run this executable from inside the extracted `Vive Wand Compatibility Patch` folder:

```text
Install Vanilla OpenVR for Vive Wands.exe
```

MO2 conflict priority matters. This mod must win:

```text
Root\openvr_api.dll
F4SE\Plugins\VirtualHolsters.ini
```

The installer will copy your own vanilla Fallout 4 VR OpenVR DLL into:

```text
Root\openvr_api.dll
```

It will also update the active MO2 profile's `fallout4custom.ini`:

```ini
[Workshop]
fItemRotationSpeed=1.0
fItemHoldDistantSpeed=3.0
```

The installer intentionally does not edit:

```text
Documents\My Games\Fallout4VR\Fallout4Custom.ini
```

If you switch to or create another MO2 profile, run the installer again while that profile is active.

The current 2.1.1 baseline uses:

```ini
[VR]
iVRPlatformControllerOverride=-1
```

Leave that setting on auto-detect unless you are diagnosing a confirmed controller-identity regression. The value `7` is only a fallback diagnostic for that specific problem.

Launch through the MO2/F4SEVR launch option. Do not launch directly through `Fallout4VR.exe`.

## Included Files

```text
README.md
meta.ini
Install Vanilla OpenVR for Vive Wands.exe
F4SE\Plugins\VirtualHolsters.ini
F4SE\Plugins\ViveWandCompatibilityPatch.dll
```

The release archive intentionally does not include `Root\openvr_api.dll`; that file is created locally by the installer.

## Troubleshooting

If the vanilla OpenVR restore did not apply, rerun:

```text
Install Vanilla OpenVR for Vive Wands.exe
```

Then confirm this mod wins `Root\openvr_api.dll` over `Root Fix`.

If scripted dialogue menus do not scroll with right-trackpad touch-drag, confirm the F4SE plugin is installed:

```text
F4SE\Plugins\ViveWandCompatibilityPatch.dll
```

Expected log clues:

```text
pluginVersion=213
Registered Vive Wand DialogueMenu touch-scroll bridge with FO4VRTools controller-state callback
```

If Virtual Holsters does not holster with Grip, confirm this mod wins:

```text
F4SE\Plugins\VirtualHolsters.ini
```

Expected values:

```ini
HolsterButtonID = 2
ConfigRotateButtonID = 2
DisplaySpheres = false
EnableHaptics = false
bDisableActivation = false
```

If Workshop object rotation or held-distance movement feels wrong, run the installer again while your intended MO2 profile is active and check that the active profile's `fallout4custom.ini` contains the `[Workshop]` values above.

## Credits

- HUGE SPECIAL THANKS TO MR. DAVE FOR PROVIDING THE VANILLA openvr_api.dll RESTORE FIX. THAT DISCOVERY IS THE HEART OF THIS PATCH, AND A VERY LARGE SHARE OF THE CREDIT FOR THIS MOD BELONGS TO HIM.
- Gingas and the Fallout VR Essentials collection this patch targets.
- Fallout4 VR Tools / FO4VRTools.
- F4SEVR.
- Virtual Holsters.

## Changelog 2.1.1

- Fixes the build 212 dialogue-scroll regression where interleaved non-right-hand FO4VRTools callback samples reset the active right-hand drag session before it could move the selected dialogue option.
- Keeps the internal F4SE DLL identifier at build `213` for log verification.
- Confirmed by Dave's follow-up test as the dialogue-menu fix that actually restores scripted dialogue navigation.

## Changelog 2.1

- Restores the vanilla Fallout 4 VR `openvr_api.dll` path using the included installer.
- Adds a slim F4SE plugin that restores right-trackpad touch-drag scrolling in scripted vertical dialogue menus.
- Updates Virtual Holsters defaults for Vive Wands:
  - Grip holster/unholster.
  - `DisplaySpheres=false`.
  - `EnableHaptics=false`.
- Installer now sets Workshop item manipulation speed values in the active MO2 profile:
  - `fItemRotationSpeed=1.0`.
  - `fItemHoldDistantSpeed=3.0`.
- Keeps the patch self-contained: the installer does not edit the user's Documents `Fallout4Custom.ini`.
- Release package now ships as a runtime-only mod folder for manual extraction into `Gingas Fallout VR Essentials\mods`.
- Removes the old broad controller-patching approach; 2.1 keeps only the dialogue-menu bridge plus config and installer fixes.
