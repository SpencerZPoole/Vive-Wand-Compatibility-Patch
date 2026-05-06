# OpenVR Installer Utility

Builds `Install Vanilla OpenVR for Vive Wands.exe`, a small Win32 helper that copies the user's own Fallout 4 VR `openvr_api.dll` into this mod's `Root` folder.

It intentionally does not ship Valve's DLL and does not modify `Root Fix`.

Release packaging note: the mod should be zipped with `Vive Wand Compatibility Patch\` as the top-level folder. Users should extract that whole folder into `F:\Gingas Fallout VR Essentials\mods\`, set MO2 priority, and then run this executable from inside the extracted mod folder.

MO2 priority note: lower in MO2's left pane means higher priority. In `modlist.txt`, that high-priority bottom mod appears earlier in the file, so this utility treats `Vive Wand Compatibility Patch` appearing before `Root Fix` as a passing priority check.

## INI Updates

The installer also enforces the Workshop object manipulation speed fix in `Fallout4Custom.ini`:

```ini
[Workshop]
fItemRotationSpeed=1.0
fItemHoldDistantSpeed=3.0
```

It targets only the active MO2 profile path when it can be resolved:

- active MO2 profile `profiles\<selected profile>\fallout4custom.ini`

This covers the normal MO2 profile-local launch path while avoiding edits to the user's Documents `Fallout4Custom.ini`. Existing INIs are backed up only when the values need to be changed. The result is written to `OpenVRInstall.log`. If the user changes or creates MO2 profiles, run the installer again while that profile is active.

## Build

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Source\OpenVRInstaller\build.ps1 -Configuration Release
```

The build script writes:

- `Source\OpenVRInstaller\dist\Install Vanilla OpenVR for Vive Wands.exe`
- `Source\OpenVRInstaller\package\Install Vanilla OpenVR for Vive Wands.exe`
- `Install Vanilla OpenVR for Vive Wands.exe` at the mod root

## Silent Test Mode

For automated local testing:

```powershell
.\Install Vanilla OpenVR for Vive Wands.exe /silent
```

Silent mode suppresses message boxes and folder browsing. Normal double-click usage shows success/error message boxes.

Silent mode still copies the OpenVR DLL, updates the Workshop INI values, and writes `OpenVRInstall.log`.
