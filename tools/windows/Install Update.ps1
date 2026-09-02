param(
    [Parameter(Mandatory = $true)][string]$SourceRoot,
    [Parameter(Mandatory = $true)][string]$DestinationRoot,
    [Parameter(Mandatory = $true)][string]$WorkingRoot,
    [Parameter(Mandatory = $true)][int]$ExpectedVersion,
    [string]$WaitForProcessIds = '',
    [string]$RelaunchPath = '',
    [switch]$NoRelaunch,
    [switch]$KeepWorkingDirectory,
    [switch]$TestMode,
    [ValidateRange(0, 1000)]
    [int]$TestFailAfterCopies = 0
)

$ErrorActionPreference = 'Stop'
$source = [System.IO.Path]::GetFullPath($SourceRoot)
$destination = [System.IO.Path]::GetFullPath($DestinationRoot)
$working = [System.IO.Path]::GetFullPath($WorkingRoot)
$destinationPrefix = $destination.TrimEnd('\', '/') +
    [System.IO.Path]::DirectorySeparatorChar

function Write-InstallerLog {
    param([string]$Message)
    try {
        $logDirectory = Join-Path $destination 'logs'
        New-Item -ItemType Directory -Path $logDirectory -Force | Out-Null
        [System.IO.File]::AppendAllText(
            (Join-Path $logDirectory 'update_install.log'),
            ('[{0}] {1}{2}' -f (Get-Date).ToString('o'), $Message,
                [Environment]::NewLine),
            [System.Text.UTF8Encoding]::new($false))
    } catch {}
}

try {
    foreach ($idText in @($WaitForProcessIds -split ',')) {
        $waitId = 0
        if (-not [int]::TryParse($idText, [ref]$waitId) -or
            $waitId -le 0 -or $waitId -eq $PID) { continue }
        $process = Get-Process -Id $waitId -ErrorAction SilentlyContinue
        if ($process) { $process.WaitForExit(60000) | Out-Null }
    }

    $markerPath = Join-Path $source 'PRODUCT_VERSION.txt'
    $marker = (Get-Content -LiteralPath $markerPath -Raw).Trim()
    if ($marker -notmatch '^v?(?<number>[0-9]+)$' -or
        [int]$matches.number -ne $ExpectedVersion) {
        throw "Staged package version '$marker' does not match v$ExpectedVersion."
    }
    $mainExecutableName = 'Initial D Arcade Stage 3 Recompiled.exe'
    $runtimeExecutableName =
        'Initial D Arcade Stage 3 Recompiled Runtime.exe'
    foreach ($required in @(
        $mainExecutableName, $runtimeExecutableName, 'Play Demo.ps1')) {
        if (-not (Test-Path -LiteralPath (Join-Path $source $required) `
                -PathType Leaf)) {
            throw "Staged package is missing $required."
        }
    }

    $rollback = Join-Path $working 'rollback'
    New-Item -ItemType Directory -Path $rollback -Force | Out-Null
    $createdFiles = New-Object System.Collections.Generic.List[string]
    $protectedDirectories = @('game files', 'card data', 'custom music', 'logs')
    $protectedFiles = @(
        'idas3_user_settings.ini',
        'idas3_launcher_update_settings.json'
    )
    $copiedFiles = 0
    try {
        foreach ($file in Get-ChildItem -LiteralPath $source -Recurse -File) {
            $relative = $file.FullName.Substring($source.Length).TrimStart('\', '/')
            $parts = $relative -split '[\\/]'
            if ($parts[0] -in $protectedDirectories -or
                ($parts.Count -eq 1 -and $parts[0] -in $protectedFiles)) {
                continue
            }
            $target = [System.IO.Path]::GetFullPath(
                (Join-Path $destination $relative))
            if (-not $target.StartsWith(
                    $destinationPrefix, [StringComparison]::OrdinalIgnoreCase)) {
                throw "Unsafe install path: $relative"
            }
            $targetParent = Split-Path -Parent $target
            New-Item -ItemType Directory -Path $targetParent -Force | Out-Null
            if (Test-Path -LiteralPath $target -PathType Leaf) {
                $backup = Join-Path $rollback $relative
                New-Item -ItemType Directory -Path (Split-Path -Parent $backup) `
                    -Force | Out-Null
                Copy-Item -LiteralPath $target -Destination $backup -Force
            } else {
                $createdFiles.Add($target)
            }
            Copy-Item -LiteralPath $file.FullName -Destination $target -Force
            ++$copiedFiles
            if ($TestMode -and $TestFailAfterCopies -gt 0 -and
                $copiedFiles -ge $TestFailAfterCopies) {
                throw "Injected updater test failure after $copiedFiles copies."
            }
        }

        $installedMarkerPath = Join-Path $destination 'PRODUCT_VERSION.txt'
        $installedMarker = (Get-Content -LiteralPath $installedMarkerPath -Raw).Trim()
        if ($installedMarker -notmatch '^v?(?<number>[0-9]+)$' -or
            [int]$matches.number -ne $ExpectedVersion) {
            throw 'Installed version marker verification failed.'
        }
        $installedRuntime = Join-Path $destination $runtimeExecutableName
        if (-not (Test-Path -LiteralPath $installedRuntime `
                -PathType Leaf)) {
            throw 'Installed executable verification failed.'
        }

        # Merge updates never delete unknown destination files.  The sole
        # migration exception is the old product-controlled runtime name, and
        # even that is removed only when it is byte-identical to the newly
        # verified canonical runtime.  Preserve a rollback copy first.
        $legacyRuntime = Join-Path $destination 'demo.exe'
        if (Test-Path -LiteralPath $legacyRuntime -PathType Leaf) {
            $canonicalHash = (Get-FileHash -LiteralPath $installedRuntime `
                -Algorithm SHA256).Hash
            $legacyHash = (Get-FileHash -LiteralPath $legacyRuntime `
                -Algorithm SHA256).Hash
            if ($canonicalHash -eq $legacyHash) {
                $legacyBackup = Join-Path $rollback 'demo.exe'
                if (-not (Test-Path -LiteralPath $legacyBackup `
                        -PathType Leaf)) {
                    Copy-Item -LiteralPath $legacyRuntime `
                        -Destination $legacyBackup -Force
                }
                [System.IO.File]::Delete($legacyRuntime)
            }
        }
    } catch {
        foreach ($backupFile in Get-ChildItem -LiteralPath $rollback -Recurse -File) {
            $relative = $backupFile.FullName.Substring($rollback.Length).TrimStart('\', '/')
            $target = Join-Path $destination $relative
            New-Item -ItemType Directory -Path (Split-Path -Parent $target) `
                -Force | Out-Null
            Copy-Item -LiteralPath $backupFile.FullName -Destination $target -Force
        }
        foreach ($created in $createdFiles) {
            if (Test-Path -LiteralPath $created -PathType Leaf) {
                [System.IO.File]::Delete($created)
            }
        }
        throw
    }

    Write-InstallerLog "Installed verified update v$ExpectedVersion."
    if (-not $NoRelaunch -and $RelaunchPath -and
        (Test-Path -LiteralPath $RelaunchPath -PathType Leaf)) {
        Start-Process -FilePath $RelaunchPath -WorkingDirectory $destination | Out-Null
    }
    if (-not $KeepWorkingDirectory) {
        try {
            $localData = [Environment]::GetFolderPath('LocalApplicationData')
            $localUpdateBase = Join-Path $localData 'InitialDAS3Recomp\updates'
            $allowedRoot = [System.IO.Path]::GetFullPath(
                $localUpdateBase).TrimEnd('\', '/') +
                [System.IO.Path]::DirectorySeparatorChar
            if ($working.StartsWith(
                    $allowedRoot, [StringComparison]::OrdinalIgnoreCase)) {
                [System.IO.Directory]::Delete($working, $true)
            }
        } catch {}
    }
    if ($TestMode) { return }
    exit 0
} catch {
    Write-InstallerLog "Install failed: $($_.Exception.Message)"
    if ($TestMode) { throw }
    try {
        Add-Type -AssemblyName System.Windows.Forms
        [System.Windows.Forms.MessageBox]::Show(
            "The verified update could not be installed. The existing files were restored.`r`n`r`n$($_.Exception.Message)",
            'Initial D Recomp Update Failed', 'OK', 'Error') | Out-Null
    } catch {}
    exit 1
}
