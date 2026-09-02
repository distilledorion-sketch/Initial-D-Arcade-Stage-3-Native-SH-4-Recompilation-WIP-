$script:Idas3UpdaterModuleRoot = $PSScriptRoot
$script:Idas3UpdateRepository =
    'distilledorion-sketch/Initial-D-Arcade-Stage-3-Native-SH-4-Recompilation-WIP-'
$script:Idas3UpdateSettingsName = 'idas3_launcher_update_settings.json'

function Write-Idas3UpdateLog {
    param(
        [Parameter(Mandatory = $true)][string]$BuildRoot,
        [Parameter(Mandatory = $true)][string]$Message
    )
    try {
        $logDirectory = Join-Path $BuildRoot 'logs'
        New-Item -ItemType Directory -Path $logDirectory -Force | Out-Null
        $line = '[{0}] {1}{2}' -f (Get-Date).ToString('o'), $Message,
            [Environment]::NewLine
        [System.IO.File]::AppendAllText(
            (Join-Path $logDirectory 'update_check.log'), $line,
            [System.Text.UTF8Encoding]::new($false))
    } catch {
        # Update telemetry must never prevent the game from starting.
    }
}

function Get-Idas3AutomaticUpdatePreference {
    param([Parameter(Mandatory = $true)][string]$BuildRoot)
    $path = Join-Path $BuildRoot $script:Idas3UpdateSettingsName
    try {
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { return $false }
        $saved = Get-Content -LiteralPath $path -Raw | ConvertFrom-Json
        return $saved.AutomaticUpdates -eq $true
    } catch {
        Write-Idas3UpdateLog -BuildRoot $BuildRoot `
            -Message "Ignoring unreadable update preference: $($_.Exception.Message)"
        return $false
    }
}

function Set-Idas3AutomaticUpdatePreference {
    param(
        [Parameter(Mandatory = $true)][string]$BuildRoot,
        [Parameter(Mandatory = $true)][bool]$Enabled
    )
    $path = Join-Path $BuildRoot $script:Idas3UpdateSettingsName
    $temporary = "$path.new"
    $json = [ordered]@{
        AutomaticUpdates = $Enabled
        UpdatedUtc = [DateTime]::UtcNow.ToString('o')
    } | ConvertTo-Json
    [System.IO.File]::WriteAllText(
        $temporary, $json + [Environment]::NewLine,
        [System.Text.UTF8Encoding]::new($false))
    Move-Item -LiteralPath $temporary -Destination $path -Force
}

function Get-Idas3AvailableUpdate {
    param(
        [Parameter(Mandatory = $true)][int]$CurrentVersion,
        [object[]]$Releases,
        [string]$Repository = $script:Idas3UpdateRepository
    )
    if (-not $PSBoundParameters.ContainsKey('Releases')) {
        [Net.ServicePointManager]::SecurityProtocol =
            [Net.ServicePointManager]::SecurityProtocol -bor
            [Net.SecurityProtocolType]::Tls12
        $headers = @{
            Accept = 'application/vnd.github+json'
            'User-Agent' = 'IDAS3-Native-Recomp-Updater'
            'X-GitHub-Api-Version' = '2022-11-28'
        }
        $uri = "https://api.github.com/repos/$Repository/releases?per_page=30"
        # Windows PowerShell can preserve a top-level JSON array as one object
        # whose properties are arrays. Sending it through the pipeline expands
        # the individual release records before the selector combines fields.
        $response = Invoke-RestMethod -Uri $uri -Headers $headers `
            -Method Get -TimeoutSec 6
        $Releases = @($response | ForEach-Object { $_ })
    }

    $candidates = @()
    foreach ($release in @($Releases)) {
        if ($release.draft -eq $true) { continue }
        $tag = [string]$release.tag_name
        if ($tag -notmatch '^v(?<number>[0-9]+)(?:[^0-9].*)?$') { continue }
        $version = [int]$matches.number
        $zipAssets = @($release.assets | Where-Object {
            [string]$_.name -match '\.zip$' -and [int64]$_.size -gt 0
        })
        if ($zipAssets.Count -eq 0) { continue }
        $asset = @($zipAssets | Sort-Object @{
            Expression = {
                if ([string]$_.name -match '(?i)public.*early.*demo') { 0 } else { 1 }
            }
        }, name)[0]
        $published = [DateTimeOffset]::MinValue
        if ($release.published_at) {
            try { $published = [DateTimeOffset]$release.published_at } catch {}
        }
        $candidates += [pscustomobject]@{
            Version = $version
            VersionLabel = "v$version"
            Name = [string]$release.name
            Tag = $tag
            PublishedAt = $published
            ReleaseDateText = if ($published -ne [DateTimeOffset]::MinValue) {
                $published.ToLocalTime().ToString('MMMM d, yyyy')
            } else { 'an unknown date' }
            ReleaseUrl = [string]$release.html_url
            AssetName = [string]$asset.name
            AssetUrl = [string]$asset.browser_download_url
            AssetDigest = [string]$asset.digest
            AssetSize = [int64]$asset.size
        }
    }
    $available = @($candidates | Where-Object {
        $_.Version -gt $CurrentVersion
    } | Sort-Object Version, PublishedAt -Descending | Select-Object -First 1)
    if ($available.Count -eq 0) { return $null }
    return $available[0]
}

function Show-Idas3UpdatePrompt {
    param([Parameter(Mandatory = $true)]$Update)
    Add-Type -AssemblyName System.Windows.Forms
    Add-Type -AssemblyName System.Drawing

    $form = New-Object System.Windows.Forms.Form
    $form.Text = 'Initial D Arcade Stage 3 Recomp Update'
    $form.StartPosition = 'CenterScreen'
    $form.FormBorderStyle = 'FixedDialog'
    $form.MaximizeBox = $false
    $form.MinimizeBox = $false
    $form.ClientSize = New-Object System.Drawing.Size(540, 245)
    $form.Font = New-Object System.Drawing.Font('Segoe UI', 10)

    $heading = New-Object System.Windows.Forms.Label
    $heading.Location = New-Object System.Drawing.Point(24, 20)
    $heading.Size = New-Object System.Drawing.Size(492, 32)
    $heading.Font = New-Object System.Drawing.Font('Segoe UI Semibold', 14)
    $heading.Text = "Update $($Update.VersionLabel) is available"
    $form.Controls.Add($heading)

    $details = New-Object System.Windows.Forms.Label
    $details.Location = New-Object System.Drawing.Point(26, 62)
    $details.Size = New-Object System.Drawing.Size(488, 52)
    $details.Text = "Released $($Update.ReleaseDateText).`r`nDownload the verified update before starting the game?"
    $form.Controls.Add($details)

    $releaseLink = New-Object System.Windows.Forms.LinkLabel
    $releaseLink.Location = New-Object System.Drawing.Point(26, 119)
    $releaseLink.Size = New-Object System.Drawing.Size(200, 25)
    $releaseLink.Text = 'View release notes on GitHub'
    $releaseUrl = [string]$Update.ReleaseUrl
    $releaseLink.Add_LinkClicked({
        if ($releaseUrl) { Start-Process $releaseUrl }
    })
    $form.Controls.Add($releaseLink)

    $automatic = New-Object System.Windows.Forms.CheckBox
    $automatic.Location = New-Object System.Drawing.Point(29, 153)
    $automatic.Size = New-Object System.Drawing.Size(360, 28)
    $automatic.Text = 'Automatically install future updates'
    $form.Controls.Add($automatic)

    $yes = New-Object System.Windows.Forms.Button
    $yes.Location = New-Object System.Drawing.Point(330, 196)
    $yes.Size = New-Object System.Drawing.Size(88, 32)
    $yes.Text = 'Yes'
    $yes.Add_Click({ $form.Tag = 'yes'; $form.Close() })
    $form.Controls.Add($yes)
    $form.AcceptButton = $yes

    $no = New-Object System.Windows.Forms.Button
    $no.Location = New-Object System.Drawing.Point(428, 196)
    $no.Size = New-Object System.Drawing.Size(88, 32)
    $no.Text = 'No'
    $no.Add_Click({ $form.Tag = 'no'; $form.Close() })
    $form.Controls.Add($no)
    $form.CancelButton = $no

    $form.ShowDialog() | Out-Null
    return [pscustomobject]@{
        Install = $form.Tag -eq 'yes'
        AutomaticUpdates = $form.Tag -eq 'yes' -and $automatic.Checked
    }
}

function Expand-Idas3VerifiedUpdatePackage {
    param(
        [Parameter(Mandatory = $true)][string]$ZipPath,
        [Parameter(Mandatory = $true)][string]$ExpectedDigest,
        [Parameter(Mandatory = $true)][int]$ExpectedVersion,
        [Parameter(Mandatory = $true)][string]$ExtractRoot
    )
    if ($ExpectedDigest -notmatch '^(?i)sha256:(?<hash>[0-9a-f]{64})$') {
        throw 'GitHub did not provide a usable SHA-256 digest for this update.'
    }
    $expectedHash = $matches.hash.ToUpperInvariant()
    $actualHash = (Get-FileHash -LiteralPath $ZipPath -Algorithm SHA256).Hash
    if ($actualHash -ne $expectedHash) {
        throw "Update checksum mismatch (expected $expectedHash, received $actualHash)."
    }

    Add-Type -AssemblyName System.IO.Compression.FileSystem
    New-Item -ItemType Directory -Path $ExtractRoot -Force | Out-Null
    $rootFull = [System.IO.Path]::GetFullPath($ExtractRoot)
    $rootPrefix = $rootFull.TrimEnd('\', '/') +
        [System.IO.Path]::DirectorySeparatorChar
    $archive = [System.IO.Compression.ZipFile]::OpenRead($ZipPath)
    try {
        foreach ($entry in $archive.Entries) {
            if (-not $entry.FullName) { continue }
            $relative = $entry.FullName.Replace('/',
                [System.IO.Path]::DirectorySeparatorChar)
            $destination = [System.IO.Path]::GetFullPath(
                (Join-Path $rootFull $relative))
            if (-not $destination.StartsWith(
                    $rootPrefix, [StringComparison]::OrdinalIgnoreCase)) {
                throw "Unsafe path in update package: $($entry.FullName)"
            }
            if ($entry.FullName.EndsWith('/') -or
                $entry.FullName.EndsWith('\')) {
                New-Item -ItemType Directory -Path $destination -Force | Out-Null
                continue
            }
            $parent = Split-Path -Parent $destination
            New-Item -ItemType Directory -Path $parent -Force | Out-Null
            try {
                [System.IO.Compression.ZipFileExtensions]::ExtractToFile(
                    $entry, $destination, $true)
            } catch {
                throw "Could not extract '$($entry.FullName)' to '$destination': $($_.Exception.Message)"
            }
        }
    } finally {
        $archive.Dispose()
    }

    $packageRoots = @(Get-ChildItem -LiteralPath $rootFull -Recurse -File `
        -Filter 'PRODUCT_VERSION.txt' | ForEach-Object {
            $candidate = $_.Directory.FullName
            $canonicalRuntime = Join-Path $candidate `
                'Initial D Arcade Stage 3 Recompiled Runtime.exe'
            $legacyRuntime = Join-Path $candidate 'demo.exe'
            $mainLauncher = Join-Path $candidate `
                'Initial D Arcade Stage 3 Recompiled.exe'
            if (((Test-Path -LiteralPath $canonicalRuntime -PathType Leaf) -or
                 (Test-Path -LiteralPath $legacyRuntime -PathType Leaf)) -and
                (Test-Path -LiteralPath $mainLauncher -PathType Leaf) -and
                (Test-Path -LiteralPath (Join-Path $candidate 'Play Demo.ps1') `
                    -PathType Leaf)) { $candidate }
        } | Select-Object -Unique)
    if ($packageRoots.Count -ne 1) {
        throw 'The verified ZIP does not contain exactly one complete demo package.'
    }
    $markerPath = Join-Path $packageRoots[0] 'PRODUCT_VERSION.txt'
    $marker = (Get-Content -LiteralPath $markerPath -Raw).Trim()
    if ($marker -notmatch '^v?(?<number>[0-9]+)$' -or
        [int]$matches.number -ne $ExpectedVersion) {
        throw "The ZIP version marker '$marker' does not match v$ExpectedVersion."
    }
    return $packageRoots[0]
}

function Start-Idas3StagedUpdate {
    param(
        [Parameter(Mandatory = $true)]$Update,
        [Parameter(Mandatory = $true)][string]$BuildRoot
    )
    $localData = [Environment]::GetFolderPath('LocalApplicationData')
    if (-not $localData) { throw 'Windows did not provide a Local AppData folder.' }
    $workingRoot = Join-Path (Join-Path $localData 'InitialDAS3Recomp\updates') `
        (('{0}-{1}' -f $Update.VersionLabel, [Guid]::NewGuid().ToString('N')))
    New-Item -ItemType Directory -Path $workingRoot -Force | Out-Null
    try {
        $assetFileName = [System.IO.Path]::GetFileName([string]$Update.AssetName)
        if (-not $assetFileName -or $assetFileName -ne [string]$Update.AssetName) {
            throw 'GitHub returned an unsafe update asset name.'
        }
        $zipPath = Join-Path $workingRoot $assetFileName
        $oldProgress = $ProgressPreference
        $ProgressPreference = 'SilentlyContinue'
        try {
            Invoke-WebRequest -Uri $Update.AssetUrl -OutFile $zipPath `
                -UseBasicParsing -TimeoutSec 120 -Headers @{
                    'User-Agent' = 'IDAS3-Native-Recomp-Updater'
                }
        } finally {
            $ProgressPreference = $oldProgress
        }
        $extractRoot = Join-Path $workingRoot 'package'
        $packageRoot = Expand-Idas3VerifiedUpdatePackage -ZipPath $zipPath `
            -ExpectedDigest $Update.AssetDigest `
            -ExpectedVersion $Update.Version -ExtractRoot $extractRoot

        $installerSource = Join-Path $script:Idas3UpdaterModuleRoot `
            'Install Update.ps1'
        if (-not (Test-Path -LiteralPath $installerSource -PathType Leaf)) {
            throw "The update installer helper is missing: $installerSource"
        }
        $installer = Join-Path $workingRoot 'Install Update.ps1'
        Copy-Item -LiteralPath $installerSource -Destination $installer -Force

        $waitIds = @()
        try {
            $current = Get-CimInstance Win32_Process -Filter "ProcessId=$PID"
            $parent = Get-CimInstance Win32_Process `
                -Filter "ProcessId=$($current.ParentProcessId)"
            $buildPrefix = [System.IO.Path]::GetFullPath($BuildRoot).TrimEnd('\', '/') +
                [System.IO.Path]::DirectorySeparatorChar
            if ($parent.ExecutablePath -and
                [System.IO.Path]::GetFullPath($parent.ExecutablePath).StartsWith(
                    $buildPrefix, [StringComparison]::OrdinalIgnoreCase)) {
                # The packaged GUI frontend is inside the product folder and
                # Windows locks that executable while it waits for this script.
                $waitIds += [int]$parent.ProcessId
            }
        } catch {}
        $relaunch = Join-Path $BuildRoot `
            'Initial D Arcade Stage 3 Recompiled.exe'
        if (-not (Test-Path -LiteralPath $relaunch -PathType Leaf)) {
            $relaunch = Join-Path $BuildRoot 'Play Demo.cmd'
        }
        $quote = { param([string]$Value) '"' + $Value.Replace('"', '""') + '"' }
        $arguments = @(
            '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', (& $quote $installer),
            '-SourceRoot', (& $quote $packageRoot),
            '-DestinationRoot', (& $quote $BuildRoot),
            '-WorkingRoot', (& $quote $workingRoot),
            '-ExpectedVersion', [string]$Update.Version,
            '-WaitForProcessIds', (& $quote ($waitIds -join ',')),
            '-RelaunchPath', (& $quote $relaunch)
        ) -join ' '
        Start-Process -FilePath 'powershell.exe' -ArgumentList $arguments `
            -WindowStyle Hidden | Out-Null
        Write-Idas3UpdateLog -BuildRoot $BuildRoot `
            -Message "Verified $($Update.VersionLabel); staged installer dispatched."
        return $true
    } catch {
        Write-Idas3UpdateLog -BuildRoot $BuildRoot `
            -Message "Update staging failed: $($_.Exception.Message)"
        throw
    }
}

function Invoke-Idas3UpdateCheck {
    param(
        [Parameter(Mandatory = $true)][int]$CurrentVersion,
        [Parameter(Mandatory = $true)][string]$BuildRoot,
        [switch]$Skip
    )
    if ($Skip -or $env:IDAS3_DISABLE_UPDATE_CHECK -eq '1') { return $false }
    try {
        $update = Get-Idas3AvailableUpdate -CurrentVersion $CurrentVersion
    } catch {
        Write-Idas3UpdateLog -BuildRoot $BuildRoot `
            -Message "Update check unavailable; continuing launch: $($_.Exception.Message)"
        return $false
    }
    if (-not $update) { return $false }

    $automatic = Get-Idas3AutomaticUpdatePreference -BuildRoot $BuildRoot
    $install = $automatic
    if (-not $automatic) {
        $choice = Show-Idas3UpdatePrompt -Update $update
        $install = $choice.Install
        if ($choice.AutomaticUpdates) {
            Set-Idas3AutomaticUpdatePreference -BuildRoot $BuildRoot -Enabled $true
        }
    }
    if (-not $install) {
        Write-Idas3UpdateLog -BuildRoot $BuildRoot `
            -Message "User deferred $($update.VersionLabel)."
        return $false
    }

    try {
        return Start-Idas3StagedUpdate -Update $update -BuildRoot $BuildRoot
    } catch {
        try {
            Add-Type -AssemblyName System.Windows.Forms
            [System.Windows.Forms.MessageBox]::Show(
                "The update could not be installed, so the current version will start normally.`r`n`r`n$($_.Exception.Message)",
                'Initial D Recomp Update Failed', 'OK', 'Error') | Out-Null
        } catch {}
        return $false
    }
}
