param()

$ErrorActionPreference = 'Stop'

$repoRoot = [System.IO.Path]::GetFullPath(
    (Join-Path $PSScriptRoot '..\..'))
$launcherSource = Join-Path $repoRoot 'tools\windows\Play Demo.ps1'
if (-not (Test-Path -LiteralPath $launcherSource -PathType Leaf)) {
    throw "Launcher source is missing: $launcherSource"
}

$tempBase = [System.IO.Path]::GetFullPath([System.IO.Path]::GetTempPath())
$testRoot = Join-Path $tempBase (
    'idas3_launcher_relocation_' + [Guid]::NewGuid().ToString('N'))
$resolvedTestRoot = [System.IO.Path]::GetFullPath($testRoot)
if (-not $resolvedTestRoot.StartsWith(
        $tempBase, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Refusing unsafe test path: $resolvedTestRoot"
}

try {
    $toolsRoot = Join-Path $resolvedTestRoot 'tools'
    $gameRoot = Join-Path $resolvedTestRoot 'game files'
    $hostfsRoot = Join-Path $gameRoot 'driveA\HOSTFS'
    New-Item -ItemType Directory -Path $toolsRoot, $hostfsRoot -Force |
        Out-Null
    Copy-Item -LiteralPath $launcherSource `
        -Destination (Join-Path $resolvedTestRoot 'Play Demo.ps1')

    foreach ($path in @(
        (Join-Path $resolvedTestRoot `
            'Initial D Arcade Stage 3 Recompiled Runtime.exe'),
        (Join-Path $resolvedTestRoot 'demo.exe'),
        (Join-Path $gameRoot 'idas3_main_0C020000.bin'),
        (Join-Path $gameRoot '.idas3_extraction_complete.json'))) {
        [System.IO.File]::WriteAllBytes($path, [byte[]]::new(1))
    }

    $tracePath = Join-Path $resolvedTestRoot 'integrity_calls.txt'
    $escapedTrace = $tracePath.Replace("'", "''")
    $integrityFixture = @"
function Test-Idas3GameFiles {
    param(
        [Parameter(Mandatory = `$true)][string]`$Root,
        [switch]`$FullHash,
        [switch]`$Quiet
    )
    [System.IO.File]::AppendAllText('$escapedTrace', "fast`n")
    `$calls = @(Get-Content -LiteralPath '$escapedTrace' -ErrorAction SilentlyContinue)
    return `$calls.Count -ge 2
}
"@
    [System.IO.File]::WriteAllText(
        (Join-Path $toolsRoot 'GameFilesIntegrity.ps1'),
        $integrityFixture,
        [System.Text.UTF8Encoding]::new($false))

    $result = & (Join-Path $resolvedTestRoot 'Play Demo.ps1') `
        -GameFilesDirectory $gameRoot -ValidateOnly
    if ($result.Status -ne 'READY') {
        throw "Relocated launcher did not recover to READY: $($result.Status)"
    }
    if (Test-Path -LiteralPath (Join-Path $resolvedTestRoot 'demo.exe')) {
        throw 'Launcher did not retire the byte-identical legacy runtime.'
    }
    $calls = @(Get-Content -LiteralPath $tracePath)
    if ($calls.Count -ne 2 -or $calls[0] -ne 'fast' -or
        $calls[1] -ne 'fast') {
        throw "Unexpected integrity retry sequence: $($calls -join ',')"
    }

    [pscustomobject]@{
        Status = 'PASS'
        RelocatedBuild = $true
        IntegritySequence = $calls -join ' -> '
        LegacyRuntimeMigrated = $true
        GameProcessStarted = $false
    }
} finally {
    if (Test-Path -LiteralPath $resolvedTestRoot) {
        $resolvedAgain = [System.IO.Path]::GetFullPath($resolvedTestRoot)
        if (-not $resolvedAgain.StartsWith(
                $tempBase, [System.StringComparison]::OrdinalIgnoreCase) -or
            -not ([System.IO.Path]::GetFileName($resolvedAgain)).StartsWith(
                'idas3_launcher_relocation_',
                [System.StringComparison]::Ordinal)) {
            throw "Refusing unsafe cleanup path: $resolvedAgain"
        }
        Remove-Item -LiteralPath $resolvedAgain -Recurse -Force
    }
}
