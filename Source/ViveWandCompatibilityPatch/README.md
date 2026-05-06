# ViveWandCompatibilityPatch Source

This folder contains the source for the small F4SE DLL shipped by Vive Wand Compatibility Patch 2.1.4.

The game loads:

```text
F4SE\Plugins\ViveWandCompatibilityPatch.dll
```

## Current DLL

The current installed DLL is build `213`:

```text
SHA256: 99286AF254AF9C640E511B7962E7187F0F2B1ABBC30114141445FFED4A23834D
Size: 254976 bytes
```

Build `213` is intentionally dialogue-only. It registers a FO4VRTools controller-state callback, detects `DialogueMenu`, and converts right-trackpad touch-drag into Scaleform list movement for `moveSelectionUp` / `moveSelectionDown` with `ScrollUp` / `ScrollDown` fallback. It preserves the right-hand drag session across interleaved left-hand callback samples.

Build `213` is the `2.1.1` correction after build `212` loaded but failed to emit dialogue scroll steps during follow-up testing.

It does not contain the older broad input bridge paths.

## Build

From the mod root:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Source\ViveWandCompatibilityPatch\build.ps1 -Configuration Release
```

The build writes the runtime DLL to:

```text
Source\ViveWandCompatibilityPatch\package\F4SE\Plugins\ViveWandCompatibilityPatch.dll
```

Copy that file to the live payload path:

```text
F4SE\Plugins\ViveWandCompatibilityPatch.dll
```
