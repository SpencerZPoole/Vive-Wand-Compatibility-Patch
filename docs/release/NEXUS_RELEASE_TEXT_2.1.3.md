# Vive Wand Compatibility Patch 2.1.3

## Short Description

Restores HTC Vive Wand support for Gingas Fallout VR Essentials with a manual vanilla OpenVR restore, Vive-friendly Virtual Holsters settings, scripted dialogue menu touch-scroll, and recommended Workshop item handling values.

## Summary

Unofficial HTC Vive Wand compatibility patch for Gingas' Fallout VR Essentials.

This patch is specifically designed for Gingas' Fallout VR Essentials and the Vive Wand profile. It is not a general-purpose Fallout 4 VR controller overhaul.

Version 2.1 introduced the scripted dialogue-menu touch-scroll bridge. Version 2.1.1 is the Dave-tested correction that keeps the bridge active across interleaved FO4VRTools left-hand callback samples. Version 2.1.2 attempted a no-EXE PowerShell installer repack. Version 2.1.3 removes all installer/script automation and uses manual setup.

HUGE SPECIAL THANKS TO MR. DAVE FOR PROVIDING THE VANILLA openvr_api.dll RESTORE FIX. THAT DISCOVERY IS THE HEART OF THIS PATCH, AND A VERY LARGE SHARE OF THE CREDIT FOR THIS MOD BELONGS TO HIM.

## What This Patch Does

- Restores the vanilla Fallout 4 VR OpenVR loader by having the user manually copy their own `openvr_api.dll` into this mod's `Root` folder.
- Keeps the current Vive Wand baseline on Fallout's controller auto-detect setting: `iVRPlatformControllerOverride=-1`.
- Makes Virtual Holsters use dominant-hand Grip for holster and unholster.
- Ships Virtual Holsters with `DisplaySpheres=false` and `EnableHaptics=false`.
- Restores right-trackpad touch-drag scrolling in scripted vertical dialogue menus.
- Documents recommended Workshop item rotation and held-distance speed values for the active MO2 profile.

This release does not redistribute Valve's `openvr_api.dll`.

## Controls / Behavior After Installation

- Scripted vertical dialogue menus: right-trackpad touch-drag up/down changes the highlighted dialogue option.
- Scripted vertical dialogue menus: right trigger confirms the selected dialogue option.
- Virtual Holsters: dominant-hand Grip holsters and unholsters weapons.
- Virtual Holsters: display spheres are off by default.
- Virtual Holsters: holster-zone haptics are off by default.
- Workshop object handling should use these active-profile values:

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
6. Find your vanilla Fallout 4 VR install folder. It is usually under:

```text
...\Steam\steamapps\common\Fallout 4 VR\
```

7. Copy this file from that Fallout 4 VR folder:

```text
openvr_api.dll
```

8. Paste it into this patch mod folder:

```text
Vive Wand Compatibility Patch\Root\
```

The final copied file should be:

```text
Vive Wand Compatibility Patch\Root\openvr_api.dll
```

MO2 conflict priority matters. This mod must win:

```text
Root\openvr_api.dll
F4SE\Plugins\VirtualHolsters.ini
```

## Recommended Workshop INI Values

In Mod Organizer 2:

1. Select the MO2 profile you intend to play.
2. Click the Tools icon at the top of MO2.
3. Open `INI Editor`.
4. Select the middle tab, `Fallout4Custom.ini`.
5. Scroll down to the `[Workshop]` section.
6. Set these values:

```ini
[Workshop]
fItemRotationSpeed=1.0
fItemHoldDistantSpeed=3.0
```

7. Click `Save` before closing the INI Editor.

These settings are profile-specific. Repeat this step if you switch to or create another MO2 profile.

The current 2.1.3 baseline uses:

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
Root\PUT_OPENVR_API_DLL_HERE.txt
F4SE\Plugins\VirtualHolsters.ini
F4SE\Plugins\ViveWandCompatibilityPatch.dll
```

The release archive intentionally does not include `Root\openvr_api.dll`; you copy that file locally from your own Fallout 4 VR installation.

## Troubleshooting

If the vanilla OpenVR restore did not apply, confirm your copied file exists here:

```text
Vive Wand Compatibility Patch\Root\openvr_api.dll
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

If Workshop object rotation or held-distance movement feels wrong, repeat the MO2 INI Editor step while your intended MO2 profile is active and check that the active profile's `Fallout4Custom.ini` contains the `[Workshop]` values above.

## Credits

- HUGE SPECIAL THANKS TO MR. DAVE FOR PROVIDING THE VANILLA openvr_api.dll RESTORE FIX. THAT DISCOVERY IS THE HEART OF THIS PATCH, AND A VERY LARGE SHARE OF THE CREDIT FOR THIS MOD BELONGS TO HIM.
- Gingas and the Fallout VR Essentials collection this patch targets.
- Fallout4 VR Tools / FO4VRTools.
- F4SEVR.
- Virtual Holsters.

## Changelog 2.1.3

- Removes all installer/script automation from the release package.
- Replaces the attempted PowerShell installer route with manual OpenVR copy instructions.
- Adds `Root\PUT_OPENVR_API_DLL_HERE.txt` so the OpenVR copy destination is visible after extraction.
- Keeps the same build `213` dialogue-menu behavior from 2.1.1.

## Changelog 2.1.2

- Repackaged the release without a compiled installer EXE.
- Replaced `Install Vanilla OpenVR for Vive Wands.exe` with the readable PowerShell installer script `Install Vanilla OpenVR for Vive Wands.ps1`.
- Kept the same build `213` dialogue-menu behavior from 2.1.1.

## Changelog 2.1.1

- Fixes the build 212 dialogue-scroll regression where interleaved non-right-hand FO4VRTools callback samples reset the active right-hand drag session before it could move the selected dialogue option.
- Keeps the internal F4SE DLL identifier at build `213` for log verification.
- Confirmed by Dave's follow-up test as the dialogue-menu fix that actually restores scripted dialogue navigation.

## Changelog 2.1

- Restores the vanilla Fallout 4 VR `openvr_api.dll` path.
- Adds a slim F4SE plugin that restores right-trackpad touch-drag scrolling in scripted vertical dialogue menus.
- Updates Virtual Holsters defaults for Vive Wands:
  - Grip holster/unholster.
  - `DisplaySpheres=false`.
  - `EnableHaptics=false`.
- Documents Workshop item manipulation speed values for the active MO2 profile:
  - `fItemRotationSpeed=1.0`.
  - `fItemHoldDistantSpeed=3.0`.
- Removes the old broad controller-patching approach; 2.1 keeps only the dialogue-menu bridge plus config and OpenVR restore behavior.
