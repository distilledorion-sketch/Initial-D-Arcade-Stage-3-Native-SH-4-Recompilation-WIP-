function Test-Idas3GameFiles {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Root,
        [switch]$FullHash,
        [switch]$Quiet
    )

    try {
        $rootPath = [System.IO.Path]::GetFullPath($Root)
        $mainPath = Join-Path $rootPath 'idas3_main_0C020000.bin'
        $hostfsPath = Join-Path $rootPath 'driveA\HOSTFS'
        $manifestPath = Join-Path $rootPath '.idas3_extraction_complete.json'
        foreach ($required in @($mainPath, $manifestPath)) {
            if (-not (Test-Path -LiteralPath $required -PathType Leaf)) {
                if (-not $Quiet) { Write-Warning "Missing required game file: $required" }
                return $false
            }
        }
        if (-not (Test-Path -LiteralPath $hostfsPath -PathType Container)) {
            if (-not $Quiet) { Write-Warning "Missing HOSTFS directory: $hostfsPath" }
            return $false
        }

        $manifest = Get-Content -LiteralPath $manifestPath -Raw |
            ConvertFrom-Json -ErrorAction Stop
        if ($manifest.format -ne 2 -or -not $manifest.files -or
            @($manifest.files).Count -ne 2196) {
            if (-not $Quiet) { Write-Warning 'The extraction manifest is incomplete or outdated.' }
            return $false
        }

        $main = [System.IO.FileInfo]::new($mainPath)
        if ($main.Length -ne 4194304 -or
            (Get-FileHash -Algorithm SHA256 -LiteralPath $mainPath).Hash -ne
                'EFDA831F1212DB54CC2E4BA53424FE390F91B2DAABB07E93C1E389C3736D0335') {
            if (-not $Quiet) { Write-Warning 'The extracted SH-4 game program failed verification.' }
            return $false
        }

        $hostfsPrefix = $hostfsPath.TrimEnd('\') + '\'
        $unixEpochTicks = [int64]621355968000000000
        foreach ($record in @($manifest.files)) {
            $relative = [string]$record.path
            if (-not $relative -or $relative.Contains('..')) { return $false }
            $candidate = [System.IO.Path]::GetFullPath(
                (Join-Path $hostfsPath ($relative.Replace('/', '\'))))
            if (-not $candidate.StartsWith(
                    $hostfsPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
                return $false
            }
            # This loop covers 2,196 files on every normal launch. Keep it on
            # direct .NET filesystem calls: invoking Test-Path and Get-Item
            # separately for each asset more than doubled PowerShell/provider
            # work and was the largest launcher-side CPU/disk burst.
            if (-not [System.IO.File]::Exists($candidate)) {
                if (-not $Quiet) { Write-Warning "Missing extracted asset: $relative" }
                return $false
            }
            $item = [System.IO.FileInfo]::new($candidate)
            if ($item.Length -ne [int64]$record.size) {
                if (-not $Quiet) { Write-Warning "Wrong asset size: $relative" }
                return $false
            }
            $mtimeNs = ([int64]$item.LastWriteTimeUtc.Ticks - $unixEpochTicks) * 100
            if ($FullHash -or $mtimeNs -ne [int64]$record.mtime_ns) {
                $actual = (Get-FileHash -Algorithm SHA256 -LiteralPath $candidate).Hash
                if ($actual -ne [string]$record.sha256) {
                    if (-not $Quiet) { Write-Warning "Corrupt extracted asset: $relative" }
                    return $false
                }
            }
        }
        return $true
    } catch {
        if (-not $Quiet) { Write-Warning "Game-file verification failed: $($_.Exception.Message)" }
        return $false
    }
}
