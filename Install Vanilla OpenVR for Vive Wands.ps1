[CmdletBinding()]
param(
    [switch]$DryRun,
    [switch]$NoPause
)

$ErrorActionPreference = 'Stop'

$InstallerTitle = 'Vive Wand Compatibility Patch OpenVR Installer'
$WorkshopValues = [ordered]@{
    fItemRotationSpeed = '1.0'
    fItemHoldDistantSpeed = '3.0'
}

function Write-InstallLog {
    param([string]$Message)

    $line = '[{0}] {1}' -f (Get-Date -Format 'yyyy-MM-dd HH:mm:ss'), $Message
    Write-Host $Message
    Add-Content -LiteralPath $script:LogPath -Value $line -Encoding UTF8
}

function Finish-Install {
    param([int]$ExitCode)

    if (-not $NoPause) {
        Write-Host ''
        Read-Host 'Press Enter to exit' | Out-Null
    }

    exit $ExitCode
}

function Decode-MoIniValue {
    param([string]$Value)

    if ([string]::IsNullOrWhiteSpace($Value)) {
        return ''
    }

    $decoded = $Value.Trim()
    if ($decoded.StartsWith('@ByteArray(') -and $decoded.EndsWith(')')) {
        $decoded = $decoded.Substring(11, $decoded.Length - 12)
    }
    if ($decoded.Length -ge 2 -and $decoded.StartsWith('"') -and $decoded.EndsWith('"')) {
        $decoded = $decoded.Substring(1, $decoded.Length - 2)
    }

    $decoded = $decoded.Replace('\\', '\')
    $decoded = $decoded.Replace('\/', '/')
    return $decoded
}

function Find-IniValue {
    param(
        [string]$Text,
        [string]$Key
    )

    $pattern = '^\s*' + [regex]::Escape($Key) + '\s*=\s*(.*)$'
    foreach ($line in ($Text -split "`r?`n")) {
        if ($line -match $pattern) {
            return $Matches[1].Trim()
        }
    }

    return ''
}

function Find-MoRoot {
    param([string]$StartPath)

    $current = Get-Item -LiteralPath $StartPath
    for ($i = 0; $i -lt 8 -and $null -ne $current; ++$i) {
        $candidate = Join-Path $current.FullName 'ModOrganizer.ini'
        if (Test-Path -LiteralPath $candidate) {
            return $current.FullName
        }

        $current = $current.Parent
    }

    return ''
}

function Get-FalloutVrPath {
    param(
        [string]$MoIniText,
        [string]$MoRoot
    )

    $gamePath = Decode-MoIniValue (Find-IniValue -Text $MoIniText -Key 'gamePath')
    if ($gamePath -and (Test-Path -LiteralPath (Join-Path $gamePath 'openvr_api.dll'))) {
        return $gamePath
    }

    $fallbacks = @(
        'F:\Steam\steamapps\common\Fallout 4 VR',
        'C:\Program Files (x86)\Steam\steamapps\common\Fallout 4 VR',
        (Join-Path $MoRoot 'Fallout 4 VR')
    )

    foreach ($fallback in $fallbacks) {
        if ($fallback -and (Test-Path -LiteralPath (Join-Path $fallback 'openvr_api.dll'))) {
            return $fallback
        }
    }

    while ($true) {
        Write-Host ''
        Write-Host 'Could not automatically find Fallout 4 VR openvr_api.dll.'
        $manualPath = Read-Host 'Enter your Fallout 4 VR install folder path'
        $manualPath = $manualPath.Trim('" ')
        if ($manualPath -and (Test-Path -LiteralPath (Join-Path $manualPath 'openvr_api.dll'))) {
            return $manualPath
        }

        Write-Host 'That folder does not contain openvr_api.dll.'
    }
}

function Set-WorkshopIniValues {
    param(
        [string]$Path,
        [switch]$DryRun
    )

    $encoding = New-Object System.Text.UTF8Encoding($false)
    $original = ''
    if (Test-Path -LiteralPath $Path) {
        $original = [System.IO.File]::ReadAllText($Path)
    }

    $lines = New-Object 'System.Collections.Generic.List[string]'
    if ($original.Length -gt 0) {
        foreach ($line in ($original -split "`r`n|`n|`r")) {
            $lines.Add($line)
        }
        if ($lines.Count -gt 0 -and $lines[$lines.Count - 1] -eq '') {
            $lines.RemoveAt($lines.Count - 1)
        }
    }

    $sectionStart = -1
    for ($i = 0; $i -lt $lines.Count; ++$i) {
        if ($lines[$i] -match '^\s*\[Workshop\]\s*$') {
            $sectionStart = $i
            break
        }
    }

    if ($sectionStart -lt 0) {
        if ($lines.Count -gt 0 -and $lines[$lines.Count - 1].Trim().Length -gt 0) {
            $lines.Add('')
        }
        $lines.Add('[Workshop]')
        foreach ($entry in $WorkshopValues.GetEnumerator()) {
            $lines.Add(('{0}={1}' -f $entry.Key, $entry.Value))
        }
    } else {
        $sectionEnd = $lines.Count
        for ($i = $sectionStart + 1; $i -lt $lines.Count; ++$i) {
            if ($lines[$i] -match '^\s*\[.+\]\s*$') {
                $sectionEnd = $i
                break
            }
        }

        $insertIndex = $sectionEnd
        foreach ($entry in $WorkshopValues.GetEnumerator()) {
            $keyPattern = '^\s*' + [regex]::Escape($entry.Key) + '\s*='
            $foundIndex = -1
            for ($i = $sectionStart + 1; $i -lt $sectionEnd; ++$i) {
                if ($lines[$i] -match $keyPattern) {
                    $foundIndex = $i
                    break
                }
            }

            $newLine = '{0}={1}' -f $entry.Key, $entry.Value
            if ($foundIndex -ge 0) {
                $lines[$foundIndex] = $newLine
            } else {
                $lines.Insert($insertIndex, $newLine)
                ++$insertIndex
                ++$sectionEnd
            }
        }
    }

    $updated = ($lines -join "`r`n") + "`r`n"
    if ($updated -eq $original) {
        Write-InstallLog "Workshop INI values already correct: $Path"
        return
    }

    if ($DryRun) {
        Write-InstallLog "Dry run: would update Workshop INI values in $Path"
        return
    }

    $parent = Split-Path -Parent $Path
    if (!(Test-Path -LiteralPath $parent)) {
        New-Item -ItemType Directory -Force -Path $parent | Out-Null
    }

    if (Test-Path -LiteralPath $Path) {
        $backupPath = '{0}.backup-vivewand-{1}' -f $Path, (Get-Date -Format 'yyyyMMdd-HHmmss')
        Copy-Item -LiteralPath $Path -Destination $backupPath -Force
        Write-InstallLog "Backed up active profile INI to $backupPath"
    }

    [System.IO.File]::WriteAllText($Path, $updated, $encoding)
    Write-InstallLog "Updated Workshop INI values in $Path"
}

function Test-ModPriority {
    param([string]$ModListPath)

    if (!(Test-Path -LiteralPath $ModListPath)) {
        Write-InstallLog "Could not find modlist.txt for priority check: $ModListPath"
        return
    }

    $lines = Get-Content -LiteralPath $ModListPath
    $patchIndex = -1
    $rootFixIndex = -1
    $holstersIndex = -1

    for ($i = 0; $i -lt $lines.Count; ++$i) {
        $line = $lines[$i].Trim()
        if ($line -eq '+Vive Wand Compatibility Patch') { $patchIndex = $i }
        if ($line -eq '+Root Fix') { $rootFixIndex = $i }
        if ($line -eq '+Virtual Holsters') { $holstersIndex = $i }
    }

    if ($patchIndex -lt 0) {
        Write-InstallLog 'Warning: Vive Wand Compatibility Patch is not enabled in the active MO2 profile modlist.'
        return
    }

    if ($rootFixIndex -ge 0 -and $patchIndex -gt $rootFixIndex) {
        Write-InstallLog 'Warning: this mod may not be lower than Root Fix in MO2 left-pane priority.'
    }
    if ($holstersIndex -ge 0 -and $patchIndex -gt $holstersIndex) {
        Write-InstallLog 'Warning: this mod may not be lower than Virtual Holsters in MO2 left-pane priority.'
    }
}

try {
    $script:ModRoot = Split-Path -Parent $PSCommandPath
    $script:LogPath = Join-Path $script:ModRoot 'OpenVRInstall.log'

    if (Test-Path -LiteralPath $script:LogPath) {
        Remove-Item -LiteralPath $script:LogPath -Force
    }

    Write-Host $InstallerTitle
    Write-Host ''
    if ($DryRun) {
        Write-InstallLog 'Dry run enabled; no files will be copied or edited.'
    }

    if (!(Test-Path -LiteralPath (Join-Path $script:ModRoot 'F4SE\Plugins\VirtualHolsters.ini'))) {
        throw "This script must be run from inside the installed Vive Wand Compatibility Patch mod folder: $script:ModRoot"
    }

    $moRoot = Find-MoRoot -StartPath $script:ModRoot
    if (!$moRoot) {
        throw 'Could not find ModOrganizer.ini by walking up from the script folder.'
    }
    Write-InstallLog "Detected MO2 root: $moRoot"

    $moIniPath = Join-Path $moRoot 'ModOrganizer.ini'
    $moIniText = Get-Content -LiteralPath $moIniPath -Raw
    $selectedProfile = Decode-MoIniValue (Find-IniValue -Text $moIniText -Key 'selected_profile')
    if (!$selectedProfile) {
        throw 'Could not read selected_profile from ModOrganizer.ini.'
    }
    Write-InstallLog "Detected active MO2 profile: $selectedProfile"

    $profileDir = Join-Path (Join-Path $moRoot 'profiles') $selectedProfile
    $profileCustomIni = Join-Path $profileDir 'fallout4custom.ini'
    $profileModList = Join-Path $profileDir 'modlist.txt'

    Test-ModPriority -ModListPath $profileModList

    $falloutVrPath = Get-FalloutVrPath -MoIniText $moIniText -MoRoot $moRoot
    $sourceOpenVr = Join-Path $falloutVrPath 'openvr_api.dll'
    $targetRoot = Join-Path $script:ModRoot 'Root'
    $targetOpenVr = Join-Path $targetRoot 'openvr_api.dll'

    Write-InstallLog "Detected Fallout 4 VR path: $falloutVrPath"
    if ($DryRun) {
        Write-InstallLog "Dry run: would copy $sourceOpenVr to $targetOpenVr"
    } else {
        if (!(Test-Path -LiteralPath $targetRoot)) {
            New-Item -ItemType Directory -Force -Path $targetRoot | Out-Null
            Write-InstallLog "Created Root folder: $targetRoot"
        }

        if (Test-Path -LiteralPath $targetOpenVr) {
            $sourceHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $sourceOpenVr).Hash
            $targetHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $targetOpenVr).Hash
            if ($sourceHash -ne $targetHash) {
                $backupOpenVr = Join-Path $targetRoot ('openvr_api.backup-{0}.dll' -f (Get-Date -Format 'yyyyMMdd-HHmmss'))
                Copy-Item -LiteralPath $targetOpenVr -Destination $backupOpenVr -Force
                Write-InstallLog "Backed up previous Root openvr_api.dll to $backupOpenVr"
            }
        }

        Copy-Item -LiteralPath $sourceOpenVr -Destination $targetOpenVr -Force
        Write-InstallLog "Copied vanilla Fallout 4 VR openvr_api.dll to $targetOpenVr"
    }

    Set-WorkshopIniValues -Path $profileCustomIni -DryRun:$DryRun

    Write-InstallLog 'Done.'
    Write-Host ''
    Write-Host 'Success. Launch through MO2/F4SEVR after confirming this mod wins Root\openvr_api.dll and F4SE\Plugins\VirtualHolsters.ini.'
    Finish-Install 0
} catch {
    Write-Host ''
    Write-Host "ERROR: $($_.Exception.Message)" -ForegroundColor Red
    try {
        if ($script:LogPath) {
            Write-InstallLog "ERROR: $($_.Exception.Message)"
        }
    } catch {
    }
    Finish-Install 1
}
