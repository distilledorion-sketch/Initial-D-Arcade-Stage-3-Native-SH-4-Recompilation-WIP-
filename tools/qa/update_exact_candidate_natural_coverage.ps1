param(
    [Parameter(Mandatory = $true)]
    [string]$CandidateExe,
    [Parameter(Mandatory = $true)]
    [string]$ExpectedSha256,
    [string]$EvidenceRoot = '',
    [string]$Output = ''
)

$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if (-not $EvidenceRoot) {
    $EvidenceRoot = Join-Path $projectRoot 'runtime\v2250_lead_day'
}
if (-not $Output) {
    $Output = Join-Path $EvidenceRoot 'exact_candidate_natural_coverage.csv'
}

$exe = (Resolve-Path -LiteralPath $CandidateExe).Path
$expectedHash = $ExpectedSha256.ToUpperInvariant()
$actualHash = (Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash
if ($actualHash -ne $expectedHash) {
    throw "Candidate SHA-256 mismatch: expected $expectedHash, got $actualHash"
}

$live = @(Get-CimInstance Win32_Process | Where-Object {
    $_.ExecutablePath -eq $exe
})
if ($live.Count) {
    throw "Refusing to summarize a live candidate: $($live.Count) process(es)"
}
$marker = Join-Path (Split-Path -Parent $exe) `
    'idas3_direct_vulkan_unclean.marker'
if (Test-Path -LiteralPath $marker) {
    throw "Refusing to summarize candidate with an unclean marker: $marker"
}

function Test-ExactNaturalEvidence {
    param([object]$Summary)

    $failures = [Collections.Generic.List[string]]::new()
    $target = [string]$Summary.NaturalTargetRival
    $logPath = [string]$Summary.Log
    if (-not $target) { $failures.Add('natural target is missing') }
    if (-not (Test-Path -LiteralPath $logPath -PathType Leaf)) {
        $failures.Add('log is missing')
        $lines = @()
    } else {
        $lines = @(Get-Content -LiteralPath $logPath)
    }

    $escapedTarget = [regex]::Escape($target)
    $armIndices = @()
    for ($index = 0; $index -lt $lines.Count; ++$index) {
        if ($lines[$index] -match
            "\[NATIVE-DEVELOPER-RACE-OUTCOME-NATURAL\].*rival=$escapedTarget(?:\s|$)") {
            $armIndices += $index
        }
    }
    if ($armIndices.Count -ne 1) {
        $failures.Add("natural arm count is $($armIndices.Count), expected 1")
        $postArm = @()
    } else {
        $postArm = @($lines[$armIndices[0]..($lines.Count - 1)])
    }

    $maxTravel = 0.0
    $displayFps = @()
    $vulkanFps = @()
    foreach ($line in $postArm) {
        if ($line -match 'player_travel=([0-9.]+)') {
            $value = [double]::Parse(
                $Matches[1], [Globalization.CultureInfo]::InvariantCulture)
            $maxTravel = [Math]::Max($maxTravel, $value)
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

    $resultAssets = @($postArm | Where-Object {
        $_ -match "\[LOAD2APP\] path='/driveA/sound/pack/RESULT\.bin\.nz'"
    })
    $developerWrites = @($postArm | Where-Object {
        $_ -match '\[NATIVE-DEVELOPER-RACE-OUTCOME\] (?:requested|applied)='
    })
    $faults = @($lines | Where-Object {
        $_ -match '(?i)fatal|exception|crash|access violation' -or
        $_ -match '\[UNIMPLEMENTED\]|\[NSEQ-PARSER-FAILSOFT\]'
    })

    if ($maxTravel -lt 10.0) { $failures.Add('natural travel is below 10 m') }
    if ($resultAssets.Count -eq 0) { $failures.Add('natural RESULT asset is missing') }
    if ($developerWrites.Count) { $failures.Add('developer writes occurred after arm') }
    if ($faults.Count) { $failures.Add('runtime fault signals are present') }
    if (-not ($lines | Where-Object { $_ -match '^\[VULKAN\] device:' } |
            Select-Object -First 1)) {
        $failures.Add('Direct Vulkan device evidence is missing')
    }
    if (-not $displayFps.Count) { $failures.Add('post-arm display FPS is missing') }
    if (-not $vulkanFps.Count) { $failures.Add('post-arm Vulkan FPS is missing') }
    if ([int]$Summary.Errors -ne 0) { $failures.Add('summary errors are nonzero') }
    if ([int]$Summary.RuntimeFaults -ne 0) {
        $failures.Add('summary runtime faults are nonzero')
    }
    if ([int]$Summary.NaturalResults -lt 1) {
        $failures.Add('summary natural-result count is zero')
    }
    if ([string]$Summary.NativeVulkan -ne '1' -or
        [string]$Summary.VulkanOffscreen -ne '0') {
        $failures.Add('run was not Direct Vulkan')
    }

    $displayStats = $displayFps | Measure-Object -Minimum -Average
    $vulkanStats = $vulkanFps | Measure-Object -Minimum -Average
    [pscustomobject]@{
        TargetRival = $target
        Status = if ($failures.Count) { 'FAIL' } else { 'PASS' }
        ExecutableSha256 = $actualHash
        EvidenceGeneratedAtUtc = $Summary.EvidenceGeneratedAtUtc
        Route = $Summary.Route
        MaxTargetTravel = '{0:F3}' -f $maxTravel
        NaturalResultAssetLoads = $resultAssets.Count
        DeveloperWritesAfterArm = $developerWrites.Count
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
        Log = $logPath
        Failures = $failures -join '; '
    }
}

$evaluated = @(Get-ChildItem -LiteralPath $EvidenceRoot -File `
    -Filter 'ai_profile_summary_*.csv' | ForEach-Object {
        foreach ($summary in Import-Csv -LiteralPath $_.FullName) {
            if ($summary.ExecutableSha256 -eq $expectedHash -and
                $summary.NaturalTargetRival) {
                Test-ExactNaturalEvidence -Summary $summary
            }
        }
    })

# Keep the newest strict PASS for each target. If a target has no passing run,
# retain its newest failed attempt so the ledger exposes the gap.
$rows = @($evaluated | Group-Object TargetRival | ForEach-Object {
    $passes = @($_.Group | Where-Object Status -eq 'PASS' |
        Sort-Object EvidenceGeneratedAtUtc -Descending)
    if ($passes.Count) {
        $passes[0]
    } else {
        $_.Group | Sort-Object EvidenceGeneratedAtUtc -Descending |
            Select-Object -First 1
    }
} | Sort-Object TargetRival)

$rows | Export-Csv -LiteralPath $Output -NoTypeInformation
$passCount = @($rows | Where-Object Status -eq 'PASS').Count
$failCount = @($rows | Where-Object Status -eq 'FAIL').Count

[pscustomobject]@{
    Candidate = $exe
    ExecutableSha256 = $actualHash
    ExactNaturalTargetsPassed = $passCount
    ExactNaturalTargetsFailed = $failCount
    RivalTargetTotal = 32
    Output = (Resolve-Path -LiteralPath $Output).Path
}

if ($failCount) { exit 1 }
