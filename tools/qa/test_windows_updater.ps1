$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
. (Join-Path $repoRoot 'tools\windows\UpdateSupport.ps1')
$mainExecutableName = 'Initial D Arcade Stage 3 Recompiled.exe'
$runtimeExecutableName = 'Initial D Arcade Stage 3 Recompiled Runtime.exe'
$productFolderName = 'Initial D Arcade Stage 3 Recompiled'

function Assert-True {
    param([bool]$Condition, [string]$Message)
    if (-not $Condition) { throw "FAIL: $Message" }
}

$newRelease = [pscustomobject]@{
    draft = $false
    prerelease = $true
    tag_name = 'v2458-test'
    name = 'Initial D Arcade Stage 3 Recompiled v2458'
    published_at = '2026-09-02T03:04:05Z'
    html_url = 'https://example.invalid/release'
    assets = @([pscustomobject]@{
        name = 'Initial.D.Arcade.Stage.3.Recompiled.v2458.zip'
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
$available = Get-Idas3AvailableUpdate -CurrentVersion 2457 `
    -Releases @($olderRelease, $newRelease)
Assert-True ($available.Version -eq 2458) `
    'release parser selects the highest newer prerelease'
Assert-True (-not (Get-Idas3AvailableUpdate -CurrentVersion 2458 `
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
    [System.IO.File]::WriteAllText((Join-Path $sourceRoot 'PRODUCT_VERSION.txt'), '2458')
    [System.IO.File]::WriteAllText(
        (Join-Path $sourceRoot $runtimeExecutableName), 'new-runtime')
    [System.IO.File]::WriteAllText(
        (Join-Path $sourceRoot $mainExecutableName), 'new-main')
    [System.IO.File]::WriteAllText((Join-Path $sourceRoot 'Play Demo.ps1'), 'new-launcher')
    [System.IO.File]::WriteAllText((Join-Path $sourceRoot 'README.txt'), 'new-readme')
    [System.IO.File]::WriteAllText((Join-Path $sourceRoot 'new-product.txt'), 'new-product')
    [System.IO.File]::WriteAllText((Join-Path $sourceRoot 'idas3_user_settings.ini'), 'bad-default')
    [System.IO.File]::WriteAllText((Join-Path $sourceRoot 'card data\player.card'), 'bad-card')
    [System.IO.File]::WriteAllText(
        (Join-Path $destinationRoot $runtimeExecutableName), 'old-runtime')
    [System.IO.File]::WriteAllText(
        (Join-Path $destinationRoot $mainExecutableName), 'old-main')
    [System.IO.File]::WriteAllText(
        (Join-Path $destinationRoot 'demo.exe'), 'new-runtime')
    [System.IO.File]::WriteAllText(
        (Join-Path $destinationRoot 'user notes.txt'), 'keep-user-file')
    [System.IO.File]::WriteAllText((Join-Path $destinationRoot 'idas3_user_settings.ini'), 'user-settings')
    [System.IO.File]::WriteAllText((Join-Path $destinationRoot 'card data\player.card'), 'user-card')

    & (Join-Path $repoRoot 'tools\windows\Install Update.ps1') `
        -SourceRoot $sourceRoot -DestinationRoot $destinationRoot `
        -WorkingRoot $workingRoot -ExpectedVersion 2458 `
        -NoRelaunch -KeepWorkingDirectory -TestMode
    $installedRuntimePath = Join-Path $destinationRoot $runtimeExecutableName
    $installedRuntime = Get-Content -LiteralPath $installedRuntimePath -Raw
    Assert-True ($installedRuntime -eq 'new-runtime') `
        'installer replaces product-controlled files'
    Assert-True ((Get-Content -LiteralPath `
            (Join-Path $destinationRoot $mainExecutableName) -Raw) -eq
        'new-main') 'installer retains the canonical main executable name'
    Assert-True (-not (Test-Path -LiteralPath `
            (Join-Path $destinationRoot 'demo.exe'))) `
        'installer removes a byte-identical legacy runtime name'
    Assert-True ((Get-Content -LiteralPath `
            (Join-Path $destinationRoot 'user notes.txt') -Raw) -eq
        'keep-user-file') 'merge preserves destination-only files'
    Assert-True ((Get-Content -LiteralPath `
            (Join-Path $destinationRoot 'new-product.txt') -Raw) -eq
        'new-product') 'merge adds new product files'
    $installedSettingsPath = Join-Path $destinationRoot 'idas3_user_settings.ini'
    $installedSettings = Get-Content -LiteralPath $installedSettingsPath -Raw
    Assert-True ($installedSettings -eq 'user-settings') `
        'installer preserves user settings'
    $installedCardPath = Join-Path $destinationRoot 'card data\player.card'
    $installedCard = Get-Content -LiteralPath $installedCardPath -Raw
    Assert-True ($installedCard -eq 'user-card') `
        'installer preserves card data'

    $modifiedWorking = Join-Path $testRoot 'modified-working'
    New-Item -ItemType Directory -Path $modifiedWorking -Force | Out-Null
    [System.IO.File]::WriteAllText(
        (Join-Path $destinationRoot 'demo.exe'), 'locally-modified-runtime')
    & (Join-Path $repoRoot 'tools\windows\Install Update.ps1') `
        -SourceRoot $sourceRoot -DestinationRoot $destinationRoot `
        -WorkingRoot $modifiedWorking -ExpectedVersion 2458 `
        -NoRelaunch -KeepWorkingDirectory -TestMode
    Assert-True ((Get-Content -LiteralPath `
            (Join-Path $destinationRoot 'demo.exe') -Raw) -eq
        'locally-modified-runtime') `
        'merge preserves a non-identical legacy filename'

    $rollbackSource = Join-Path $testRoot 'rollback-source'
    $rollbackDestination = Join-Path $testRoot 'rollback-destination'
    $rollbackWorking = Join-Path $testRoot 'rollback-working'
    foreach ($path in @(
        $rollbackSource, $rollbackDestination, $rollbackWorking)) {
        New-Item -ItemType Directory -Path $path -Force | Out-Null
    }
    [System.IO.File]::WriteAllText(
        (Join-Path $rollbackSource 'PRODUCT_VERSION.txt'), '2458')
    [System.IO.File]::WriteAllText(
        (Join-Path $rollbackSource $runtimeExecutableName),
        'new-rollback-runtime')
    [System.IO.File]::WriteAllText(
        (Join-Path $rollbackSource $mainExecutableName), 'new-rollback-main')
    [System.IO.File]::WriteAllText(
        (Join-Path $rollbackSource 'Play Demo.ps1'), 'new-rollback-launcher')
    [System.IO.File]::WriteAllText(
        (Join-Path $rollbackDestination $runtimeExecutableName),
        'old-rollback-runtime')
    [System.IO.File]::WriteAllText(
        (Join-Path $rollbackDestination $mainExecutableName),
        'old-rollback-main')
    $rollbackFailed = $false
    try {
        & (Join-Path $repoRoot 'tools\windows\Install Update.ps1') `
            -SourceRoot $rollbackSource `
            -DestinationRoot $rollbackDestination `
            -WorkingRoot $rollbackWorking -ExpectedVersion 2458 `
            -NoRelaunch -KeepWorkingDirectory -TestMode `
            -TestFailAfterCopies 2
    } catch {
        $rollbackFailed = $true
    }
    Assert-True $rollbackFailed 'injected mid-install failure is reported'
    Assert-True ((Get-Content -LiteralPath `
            (Join-Path $rollbackDestination $runtimeExecutableName) -Raw) -eq
        'old-rollback-runtime') 'failed install restores replaced product file'
    Assert-True (-not (Test-Path -LiteralPath `
            (Join-Path $rollbackDestination 'PRODUCT_VERSION.txt'))) `
        'failed install removes newly created product file'

    $zipPath = Join-Path $testRoot 'verified.zip'
    $zipSource = Join-Path $testRoot 'zip-source'
    $zipProductRoot = Join-Path $zipSource $productFolderName
    New-Item -ItemType Directory -Path $zipProductRoot -Force | Out-Null
    Copy-Item -Path (Join-Path $sourceRoot '*') `
        -Destination $zipProductRoot -Recurse -Force
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [System.IO.Compression.ZipFile]::CreateFromDirectory($zipSource, $zipPath)
    $digest = 'sha256:' + (Get-FileHash -LiteralPath $zipPath `
        -Algorithm SHA256).Hash.ToLowerInvariant()
    $expanded = Expand-Idas3VerifiedUpdatePackage -ZipPath $zipPath `
        -ExpectedDigest $digest -ExpectedVersion 2458 `
        -ExtractRoot $extractRoot
    $expandedFull = [System.IO.Path]::GetFullPath([string]$expanded).
        TrimEnd('\', '/')
    $extractFull = [System.IO.Path]::GetFullPath($extractRoot).
        TrimEnd('\', '/')
    $expectedExpanded = [System.IO.Path]::GetFullPath(
        (Join-Path $extractRoot $productFolderName)).TrimEnd('\', '/')
    Assert-True ($expandedFull -ieq $expectedExpanded -and
        $expandedFull -ine $extractFull) `
        'verified package locates the canonical nested product folder'

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
            -ExpectedDigest $unsafeDigest -ExpectedVersion 2458 `
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
