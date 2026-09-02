$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
. (Join-Path $repoRoot 'tools\windows\UpdateSupport.ps1')

function Assert-True {
    param([bool]$Condition, [string]$Message)
    if (-not $Condition) { throw "FAIL: $Message" }
}

$newRelease = [pscustomobject]@{
    draft = $false
    prerelease = $true
    tag_name = 'v2455-test'
    name = 'Public Early Demo v2455'
    published_at = '2026-09-02T03:04:05Z'
    html_url = 'https://example.invalid/release'
    assets = @([pscustomobject]@{
        name = 'Public.Early.Demo.v2455.zip'
        size = 1234
        browser_download_url = 'https://example.invalid/update.zip'
        digest = 'sha256:' + ('a' * 64)
    })
}
$olderRelease = [pscustomobject]@{
    draft = $false
    prerelease = $true
    tag_name = 'v2454-old'
    name = 'Old'
    published_at = '2026-09-01T00:00:00Z'
    html_url = 'https://example.invalid/old'
    assets = @([pscustomobject]@{
        name = 'old.zip'; size = 1
        browser_download_url = 'https://example.invalid/old.zip'
        digest = 'sha256:' + ('b' * 64)
    })
}
$available = Get-Idas3AvailableUpdate -CurrentVersion 2454 `
    -Releases @($olderRelease, $newRelease)
Assert-True ($available.Version -eq 2455) `
    'release parser selects the highest newer prerelease'
Assert-True (-not (Get-Idas3AvailableUpdate -CurrentVersion 2455 `
    -Releases @($olderRelease, $newRelease))) `
    'current version does not update to itself or an older release'

$testRoot = Join-Path $env:TEMP ('idas3_updater_test_' + [Guid]::NewGuid().ToString('N'))
$sourceRoot = Join-Path $testRoot 'source'
$destinationRoot = Join-Path $testRoot 'destination'
$workingRoot = Join-Path $testRoot 'working'
$extractRoot = Join-Path $testRoot 'extract'
foreach ($path in @($sourceRoot, $destinationRoot, $workingRoot)) {
    New-Item -ItemType Directory -Path $path -Force | Out-Null
}
try {
    Set-Idas3AutomaticUpdatePreference -BuildRoot $destinationRoot -Enabled $true
    Assert-True (Get-Idas3AutomaticUpdatePreference `
        -BuildRoot $destinationRoot) 'automatic update preference round-trips'

    foreach ($path in @(
        (Join-Path $sourceRoot 'card data'),
        (Join-Path $sourceRoot 'game files'),
        (Join-Path $sourceRoot 'custom music'),
        (Join-Path $destinationRoot 'card data'),
        (Join-Path $destinationRoot 'game files'),
        (Join-Path $destinationRoot 'custom music'))) {
        New-Item -ItemType Directory -Path $path -Force | Out-Null
    }
    [System.IO.File]::WriteAllText((Join-Path $sourceRoot 'PRODUCT_VERSION.txt'), '2455')
    [System.IO.File]::WriteAllText((Join-Path $sourceRoot 'demo.exe'), 'new-demo')
    [System.IO.File]::WriteAllText((Join-Path $sourceRoot 'Play Demo.ps1'), 'new-launcher')
    [System.IO.File]::WriteAllText((Join-Path $sourceRoot 'README.txt'), 'new-readme')
    [System.IO.File]::WriteAllText((Join-Path $sourceRoot 'idas3_user_settings.ini'), 'bad-default')
    [System.IO.File]::WriteAllText((Join-Path $sourceRoot 'card data\player.card'), 'bad-card')
    [System.IO.File]::WriteAllText((Join-Path $destinationRoot 'demo.exe'), 'old-demo')
    [System.IO.File]::WriteAllText((Join-Path $destinationRoot 'idas3_user_settings.ini'), 'user-settings')
    [System.IO.File]::WriteAllText((Join-Path $destinationRoot 'card data\player.card'), 'user-card')

    & (Join-Path $repoRoot 'tools\windows\Install Update.ps1') `
        -SourceRoot $sourceRoot -DestinationRoot $destinationRoot `
        -WorkingRoot $workingRoot -ExpectedVersion 2455 `
        -NoRelaunch -KeepWorkingDirectory -TestMode
    $installedDemoPath = Join-Path $destinationRoot 'demo.exe'
    $installedDemo = Get-Content -LiteralPath $installedDemoPath -Raw
    Assert-True ($installedDemo -eq 'new-demo') `
        'installer replaces product-controlled files'
    $installedSettingsPath = Join-Path $destinationRoot 'idas3_user_settings.ini'
    $installedSettings = Get-Content -LiteralPath $installedSettingsPath -Raw
    Assert-True ($installedSettings -eq 'user-settings') `
        'installer preserves user settings'
    $installedCardPath = Join-Path $destinationRoot 'card data\player.card'
    $installedCard = Get-Content -LiteralPath $installedCardPath -Raw
    Assert-True ($installedCard -eq 'user-card') `
        'installer preserves card data'

    $rollbackSource = Join-Path $testRoot 'rollback-source'
    $rollbackDestination = Join-Path $testRoot 'rollback-destination'
    $rollbackWorking = Join-Path $testRoot 'rollback-working'
    foreach ($path in @(
        $rollbackSource, $rollbackDestination, $rollbackWorking)) {
        New-Item -ItemType Directory -Path $path -Force | Out-Null
    }
    [System.IO.File]::WriteAllText(
        (Join-Path $rollbackSource 'PRODUCT_VERSION.txt'), '2455')
    [System.IO.File]::WriteAllText(
        (Join-Path $rollbackSource 'demo.exe'), 'new-rollback-demo')
    [System.IO.File]::WriteAllText(
        (Join-Path $rollbackSource 'Play Demo.ps1'), 'new-rollback-launcher')
    [System.IO.File]::WriteAllText(
        (Join-Path $rollbackDestination 'demo.exe'), 'old-rollback-demo')
    $rollbackFailed = $false
    try {
        & (Join-Path $repoRoot 'tools\windows\Install Update.ps1') `
            -SourceRoot $rollbackSource `
            -DestinationRoot $rollbackDestination `
            -WorkingRoot $rollbackWorking -ExpectedVersion 2455 `
            -NoRelaunch -KeepWorkingDirectory -TestMode `
            -TestFailAfterCopies 2
    } catch {
        $rollbackFailed = $true
    }
    Assert-True $rollbackFailed 'injected mid-install failure is reported'
    Assert-True ((Get-Content -LiteralPath `
            (Join-Path $rollbackDestination 'demo.exe') -Raw) -eq
        'old-rollback-demo') 'failed install restores replaced product file'
    Assert-True (-not (Test-Path -LiteralPath `
            (Join-Path $rollbackDestination 'PRODUCT_VERSION.txt'))) `
        'failed install removes newly created product file'

    $zipPath = Join-Path $testRoot 'verified.zip'
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [System.IO.Compression.ZipFile]::CreateFromDirectory($sourceRoot, $zipPath)
    $digest = 'sha256:' + (Get-FileHash -LiteralPath $zipPath `
        -Algorithm SHA256).Hash.ToLowerInvariant()
    $expanded = Expand-Idas3VerifiedUpdatePackage -ZipPath $zipPath `
        -ExpectedDigest $digest -ExpectedVersion 2455 `
        -ExtractRoot $extractRoot
    $expandedFull = [System.IO.Path]::GetFullPath([string]$expanded).
        TrimEnd('\', '/')
    $extractFull = [System.IO.Path]::GetFullPath($extractRoot).
        TrimEnd('\', '/')
    Assert-True ($expandedFull -ieq $extractFull) `
        'verified package extracts and locates its versioned product root'

    $unsafeZip = Join-Path $testRoot 'unsafe.zip'
    $unsafeExtract = Join-Path $testRoot 'unsafe-extract'
    $escapedPath = Join-Path $testRoot 'escaped.txt'
    $archive = [System.IO.Compression.ZipFile]::Open(
        $unsafeZip, [System.IO.Compression.ZipArchiveMode]::Create)
    try {
        $entry = $archive.CreateEntry('../escaped.txt')
        $writer = New-Object System.IO.StreamWriter($entry.Open())
        try { $writer.Write('must-not-escape') } finally { $writer.Dispose() }
    } finally {
        $archive.Dispose()
    }
    $unsafeRejected = $false
    try {
        $unsafeDigest = 'sha256:' + (Get-FileHash -LiteralPath $unsafeZip `
            -Algorithm SHA256).Hash.ToLowerInvariant()
        [void](Expand-Idas3VerifiedUpdatePackage -ZipPath $unsafeZip `
            -ExpectedDigest $unsafeDigest -ExpectedVersion 2455 `
            -ExtractRoot $unsafeExtract)
    } catch {
        $unsafeRejected = $_.Exception.Message -match 'Unsafe path'
    }
    Assert-True $unsafeRejected 'archive traversal path is rejected'
    Assert-True (-not (Test-Path -LiteralPath $escapedPath)) `
        'archive traversal writes nothing outside extraction root'
} finally {
    if (Test-Path -LiteralPath $testRoot) {
        [System.IO.Directory]::Delete($testRoot, $true)
    }
}

Write-Output 'windows updater tests: PASS'
