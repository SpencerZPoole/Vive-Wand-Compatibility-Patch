# Vive Wand Compatibility Patch 2.1.4

For Gingas' Fallout VR Essentials Mod Collection.

HUGE SPECIAL THANKS TO MR. DAVE FOR PROVIDING THE VANILLA openvr_api.dll RESTORE FIX. THAT DISCOVERY IS THE HEART OF THIS PATCH, AND A VERY LARGE SHARE OF THE CREDIT FOR THIS MOD BELONGS TO HIM.

This patch restores a clean HTC Vive Wand baseline for Fallout VR Essentials while keeping the release payload small, focused, and easy to inspect.

Version `2.1.4` is a packaging-only update. It keeps the Dave-tested build `213` dialogue fix and changes the release zip back to a flat MO2-friendly archive.

## What This Patch Does

- Restores Fallout 4 VR's vanilla OpenVR loader through a manual local file copy.
- Lets the collection's default controller auto-detection continue using `iVRPlatformControllerOverride=-1`.
- Makes Virtual Holsters use dominant-hand Grip for holster and unholster.
- Ships Virtual Holsters defaults with display spheres and holster-zone haptics off.
- Restores right-trackpad touch-drag scrolling in scripted vertical dialogue menus.
- Documents the recommended Workshop item rotation and distance speed values for the active MO2 profile.

This patch does not ship Valve's `openvr_api.dll`. You copy that file from your own Fallout 4 VR install.

## Installation

Recommended MO2 install:

1. In Mod Organizer 2, click `Install a new mod from an archive`.
2. Select `Vive-Wand-Compatibility-Patch-2.1.4.zip`.
3. If MO2 asks for the mod name, use:

```text
Vive Wand Compatibility Patch
```

4. Enable `Vive Wand Compatibility Patch`.
5. Put this mod lower in MO2's left pane than both `Virtual Holsters` and `Root Fix`.

Manual extraction fallback:

If you do not install through MO2's archive installer, create or use this folder:

```text
F:\Gingas Fallout VR Essentials\mods\Vive Wand Compatibility Patch\
```

Then extract the loose archive contents into that folder. The installed mod folder should directly contain `README.md`, `meta.ini`, `F4SE\`, and `Root\`.

OpenVR restore:

1. Find your vanilla Fallout 4 VR install folder. It is usually under:

```text
...\Steam\steamapps\common\Fallout 4 VR\
```

2. Copy this file from that Fallout 4 VR folder:

```text
openvr_api.dll
```

3. Paste it into this patch mod folder:

```text
Vive Wand Compatibility Patch\Root\
```

The final copied file should be:

```text
Vive Wand Compatibility Patch\Root\openvr_api.dll
```

MO2 conflict priority matters. This mod needs to win:

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

## Included Files

```text
README.md
meta.ini
Root\PUT_OPENVR_API_DLL_HERE.txt
F4SE\Plugins\VirtualHolsters.ini
F4SE\Plugins\ViveWandCompatibilityPatch.dll
```

`Root\PUT_OPENVR_API_DLL_HERE.txt` is only a placeholder so the manual copy destination is visible. Replace nothing; just paste your own `openvr_api.dll` into the same `Root` folder.

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

The current 2.1.4 baseline uses Fallout's controller auto-detect:

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
