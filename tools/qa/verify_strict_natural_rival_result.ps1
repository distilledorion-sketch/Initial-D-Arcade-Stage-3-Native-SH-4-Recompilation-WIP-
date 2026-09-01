param(
    [Parameter(Mandatory=$true)]
    [string]$Log,
    [Parameter(Mandatory=$true)]
    [ValidatePattern('^r_[A-Za-z0-9]+$')]
    [string]$TargetRival,
    [Parameter(Mandatory=$true)]
    [string]$Exe,
    [Parameter(Mandatory=$true)]
    [ValidatePattern('^[0-9A-Fa-f]{64}$')]
    [string]$ExpectedExeSha256,
    [ValidateRange(0.01, 100000.0)]
    [double]$MinimumTravel = 10.0
)

$ErrorActionPreference = 'Stop'
$resolvedLog = (Resolve-Path -LiteralPath $Log).Path
$resolvedExe = (Resolve-Path -LiteralPath $Exe).Path
$actualHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $resolvedExe).Hash
$lines = @(Get-Content -LiteralPath $resolvedLog)
$failures = [Collections.Generic.List[string]]::new()

if ($actualHash -ne $ExpectedExeSha256.ToUpperInvariant()) {
    $failures.Add('executable SHA-256 does not match the requested candidate')
}

$escapedTarget = [regex]::Escape($TargetRival)
$armMatches = @()
for ($index = 0; $index -lt $lines.Count; ++$index) {
    if ($lines[$index] -match
        "\[NATIVE-DEVELOPER-RACE-OUTCOME-NATURAL\].*rival=$escapedTarget(?:\s|$)") {
        $armMatches += $index
    }
}
if ($armMatches.Count -ne 1) {
    $failures.Add(
        "expected exactly one natural arm for $TargetRival; found $($armMatches.Count)")
    $postArm = @()
} else {
    $postArm = @($lines[$armMatches[0]..($lines.Count - 1)])
}

$maxTravel = 0.0
$displayFps = @()
$vulkanFps = @()
foreach ($line in $postArm) {
    if ($line -match 'player_travel=([0-9.]+)') {
        $travel = [double]::Parse(
            $Matches[1], [Globalization.CultureInfo]::InvariantCulture)
        $maxTravel = [Math]::Max($maxTravel, $travel)
    }
    if ($line -match '\[DISPLAY-FPS\] instant=([0-9.]+)') {
        $displayFps += [double]::Parse(
            $Matches[1], [Globalization.CultureInfo]::InvariantCulture)
    }
    if ($line -match '\[VULKAN-PRESENT-FPS\] instant=([0-9.]+)') {
        $vulkanFps += [double]::Parse(
            $Matches[1], [Globalization.CultureInfo]::InvariantCulture)
    }
}
if ($maxTravel -lt $MinimumTravel) {
    $failures.Add(
        ('target travel {0:F3} m is below required {1:F3} m' -f
            $maxTravel, $MinimumTravel))
}

$resultAssets = @($postArm | Where-Object {
    $_ -match "\[LOAD2APP\] path='/driveA/sound/pack/RESULT\.bin\.nz'"
})
if ($resultAssets.Count -eq 0) {
    $failures.Add('no genuine RESULT asset load occurred after the natural arm')
}

$postArmDeveloperWrites = @($postArm | Where-Object {
    $_ -match '\[NATIVE-DEVELOPER-RACE-OUTCOME\] (?:requested|applied)='
})
if ($postArmDeveloperWrites.Count -ne 0) {
    $failures.Add(
        "developer outcome writes occurred after arm: $($postArmDeveloperWrites.Count)")
}

$faults = @($lines | Where-Object {
    $_ -match '(?i)fatal|exception|crash|access violation' -or
    $_ -match '\[UNIMPLEMENTED\]|\[NSEQ-PARSER-FAILSOFT\]'
})
if ($faults.Count -ne 0) {
    $failures.Add("runtime fault/error signals found: $($faults.Count)")
}
if (-not ($lines | Where-Object { $_ -match '^\[VULKAN\] device:' } |
        Select-Object -First 1)) {
    $failures.Add('Direct Vulkan device initialization is missing')
}
if ($displayFps.Count -eq 0) {
    $failures.Add('no display FPS samples after natural arm')
}
if ($vulkanFps.Count -eq 0) {
    $failures.Add('no Vulkan present FPS samples after natural arm')
}

$processCount = @(Get-CimInstance Win32_Process | Where-Object {
    $_.ExecutablePath -eq $resolvedExe
}).Count
if ($processCount -ne 0) {
    $failures.Add("candidate still has $processCount live process(es)")
}
$marker = Join-Path (Split-Path -Parent $resolvedExe) `
    'idas3_direct_vulkan_unclean.marker'
$markerPresent = Test-Path -LiteralPath $marker
if ($markerPresent) {
    $failures.Add('Direct Vulkan unclean-session marker is still present')
}

$displayStats = $displayFps | Measure-Object -Minimum -Average
$vulkanStats = $vulkanFps | Measure-Object -Minimum -Average
$result = [pscustomobject]@{
    Status = if ($failures.Count) { 'FAIL' } else { 'PASS' }
    TargetRival = $TargetRival
    Executable = $resolvedExe
    ExecutableSha256 = $actualHash
    Log = $resolvedLog
    NaturalArmCount = $armMatches.Count
    MaxTargetTravel = '{0:F3}' -f $maxTravel
    NaturalResultAssetLoads = $resultAssets.Count
    DeveloperWritesAfterArm = $postArmDeveloperWrites.Count
    DisplayFpsMin = if ($displayFps.Count) {
        '{0:F1}' -f $displayStats.Minimum
    } else { '' }
    DisplayFpsAverage = if ($displayFps.Count) {
        '{0:F1}' -f $displayStats.Average
    } else { '' }
    VulkanFpsMin = if ($vulkanFps.Count) {
        '{0:F1}' -f $vulkanStats.Minimum
    } else { '' }
    VulkanFpsAverage = if ($vulkanFps.Count) {
        '{0:F1}' -f $vulkanStats.Average
    } else { '' }
    RuntimeFaultSignals = $faults.Count
    LiveCandidateProcesses = $processCount
    UncleanMarkerPresent = $markerPresent
    Failures = $failures -join '; '
}
$result | Format-List
if ($failures.Count) { exit 1 }
