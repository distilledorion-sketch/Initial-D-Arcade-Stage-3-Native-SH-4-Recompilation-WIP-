param(
    [string]$Exe = '',
    [string]$GameFilesDirectory = '',
    [switch]$ValidateOnly,
    [ValidateRange(60, 3600)]
    [int]$WatchdogSeconds = 3600,
    [switch]$FramebufferTiming,
    [switch]$VulkanTiming,
    [switch]$AudioSignalTrace,
    [switch]$AicaTrace,
    [switch]$Mute,
    [switch]$CardTrace,
    [switch]$InputTrace,
    [switch]$InputAutotest,
    [switch]$GuestProfile,
    [ValidateSet('60', '90', '120', '144', '165', '240', 'Unlimited')]
    [string]$PresentationFps = '60',
    [ValidateSet('Native', '720p', '1080p', '1440p', '4K', 'Ultrawide1080p')]
    [string]$Resolution = 'Native',
    [ValidateSet('On', 'Off')]
    [string]$VSync = 'On',
    [ValidateSet('Safe', 'Direct')]
    [string]$VulkanPresentation = 'Safe',
    [ValidateSet('On', 'Off')]
    [string]$FpsCounter = 'On',
    [ValidateSet('Off', '2x', '4x')]
    [string]$AntiAliasing = 'Off',
    [ValidateSet('Authentic', 'Bilinear', 'Anisotropic16x')]
    [string]$TextureFiltering = 'Authentic',
    [ValidateSet('On', 'Off')]
    [string]$Fullscreen = 'Off',
    [ValidateRange(0, 16)]
    [int]$Monitor = 0,
    [string]$CardImage = '',
    [switch]$InsertCard,
    [switch]$PresentationTrace,
    [switch]$DeveloperRaceOutcomes,
    [string]$RecordInputs = '',
    [string]$PlaybackInputs = '',
    [switch]$SkipUpdateCheck
)

$ErrorActionPreference = 'Stop'
Import-Module Microsoft.PowerShell.Utility -ErrorAction Stop

$launcherError = Join-Path $PSScriptRoot 'logs\launcher_error.txt'
New-Item -ItemType Directory -Path (Split-Path -Parent $launcherError) `
    -Force | Out-Null
Remove-Item -LiteralPath $launcherError -Force -ErrorAction SilentlyContinue
trap {
    $message = $_.Exception.Message
    [System.IO.File]::WriteAllText(
        $launcherError, $message, [System.Text.UTF8Encoding]::new($false))
    Write-Error $message
    exit 1
}

$handoffRoot = $PSScriptRoot
$buildRoot = $PSScriptRoot
$logRoot = Join-Path $PSScriptRoot 'logs'
$ProductVersion = 2457
$currentExecutable = Join-Path $buildRoot 'demo.exe'
$exe = if ($Exe) {
    (Resolve-Path -LiteralPath $Exe).Path
} else {
    $currentExecutable
}

# A launcher-side updater can replace the product before demo.exe starts and
# therefore does not need to modify a running game process. Source-tree tests
# keep UpdateSupport beside this file; packaged builds place it under tools.
if (-not $ValidateOnly) {
    $updateSupport = @(
        (Join-Path $PSScriptRoot 'tools\UpdateSupport.ps1'),
        (Join-Path $PSScriptRoot 'UpdateSupport.ps1')
    ) | Where-Object {
        Test-Path -LiteralPath $_ -PathType Leaf
    } | Select-Object -First 1
    if ($updateSupport) {
        . $updateSupport
        if (Invoke-Idas3UpdateCheck -CurrentVersion $ProductVersion `
                -BuildRoot $buildRoot -Skip:$SkipUpdateCheck) {
            # The verified external installer waits for a packaged GUI parent
            # when necessary, installs in place, then relaunches once.
            return
        }
    }
}

function Find-GameInputFile {
    param([string]$Root, [string]$FileName)
    foreach ($candidate in @(
        (Join-Path $Root $FileName),
        (Join-Path (Join-Path $Root 'inputs') $FileName))) {
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }
    $found = Get-ChildItem -LiteralPath $Root -Recurse -File `
        -Filter $FileName -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($found) { return $found.FullName }
    return ''
}

function Find-GameDriveA {
    param([string]$Root)
    foreach ($candidate in @($Root, (Join-Path $Root 'driveA'))) {
        if ((Split-Path -Leaf $candidate) -ieq 'driveA' -and
            (Test-Path -LiteralPath (Join-Path $candidate 'HOSTFS') `
                -PathType Container)) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }
    $found = Get-ChildItem -LiteralPath $Root -Recurse -Directory `
        -Filter 'driveA' -ErrorAction SilentlyContinue | Where-Object {
            Test-Path -LiteralPath (Join-Path $_.FullName 'HOSTFS') `
                -PathType Container
        } | Select-Object -First 1
    if ($found) { return $found.FullName }
    return ''
}

function Get-VulkanOffscreenFlag {
    param([ValidateSet('Safe', 'Direct')][string]$Mode)
    if ($Mode -eq 'Direct') { return '0' }
    return '1'
}

if ($GameFilesDirectory) {
    $gameFilesRoot = [System.IO.Path]::GetFullPath($GameFilesDirectory)
    New-Item -ItemType Directory -Path $gameFilesRoot -Force | Out-Null
    $main = Find-GameInputFile -Root $gameFilesRoot `
        -FileName 'idas3_main_0C020000.bin'
    $driveARoot = Find-GameDriveA -Root $gameFilesRoot
} else {
    $gameFilesRoot = Join-Path $PSScriptRoot 'game files'
    $main = Find-GameInputFile -Root $gameFilesRoot `
        -FileName 'idas3_main_0C020000.bin'
    $driveARoot = Find-GameDriveA -Root $gameFilesRoot
}

$integrityTool = Join-Path $PSScriptRoot 'tools\GameFilesIntegrity.ps1'
if (-not (Test-Path -LiteralPath $integrityTool -PathType Leaf)) {
    throw "The game-file integrity checker is missing: $integrityTool"
}
. $integrityTool
$gameFilesReady = Test-Idas3GameFiles -Root $gameFilesRoot -Quiet
if (-not $gameFilesReady) {
    # A just-started GUI launcher, antivirus scanner, or indexer can briefly
    # disturb the metadata-only pass.  If the extracted layout is present,
    # retry against file content before incorrectly falling back to CHD setup.
    $manifest = Join-Path $gameFilesRoot '.idas3_extraction_complete.json'
    $hostfs = Join-Path $gameFilesRoot 'driveA\HOSTFS'
    if ($main -and (Test-Path -LiteralPath $main -PathType Leaf) -and
        (Test-Path -LiteralPath $manifest -PathType Leaf) -and
        (Test-Path -LiteralPath $hostfs -PathType Container)) {
        Start-Sleep -Milliseconds 200
        $gameFilesReady = Test-Idas3GameFiles -Root $gameFilesRoot `
            -FullHash -Quiet
    }
}
if (-not $ValidateOnly -and -not $gameFilesReady) {
    $setup = Join-Path $PSScriptRoot 'Setup Game Files.ps1'
    if (-not (Test-Path -LiteralPath $setup -PathType Leaf)) {
        throw "The game files are missing and the one-time setup script is unavailable: $setup"
    }
    Write-Host 'Game files are not extracted yet. Starting one-time CHD + PIC setup...'
    & $setup -OutputDirectory $gameFilesRoot
    $main = Find-GameInputFile -Root $gameFilesRoot `
        -FileName 'idas3_main_0C020000.bin'
    $driveARoot = Find-GameDriveA -Root $gameFilesRoot
    if (-not (Test-Idas3GameFiles -Root $gameFilesRoot)) {
        throw 'Automatic setup completed, but the game files are still invalid.'
    }
}

# Choices made through the in-game F1 menu persist here. Explicit command-line
# parameters always win, which keeps diagnostic launches deterministic.
$settingsPath = Join-Path $buildRoot 'idas3_user_settings.ini'
if (Test-Path -LiteralPath $settingsPath) {
    $saved = @{}
    foreach ($line in Get-Content -LiteralPath $settingsPath) {
        if ($line -match '^([^=]+)=(.*)$') { $saved[$matches[1]] = $matches[2] }
    }
    if (-not $PSBoundParameters.ContainsKey('Resolution') -and $saved.Resolution) {
        $Resolution = $saved.Resolution
    }
    if (-not $PSBoundParameters.ContainsKey('PresentationFps') -and $saved.PresentationFps) {
        $PresentationFps = $saved.PresentationFps
    }
    if (-not $PSBoundParameters.ContainsKey('VSync') -and $saved.VSync) {
        $VSync = $(if ($saved.VSync -eq '0') { 'Off' } else { 'On' })
    }
    if (-not $PSBoundParameters.ContainsKey('VulkanPresentation') -and
        $saved.VulkanPresentation) {
        $VulkanPresentation = $saved.VulkanPresentation
    }
    if (-not $PSBoundParameters.ContainsKey('FpsCounter') -and $saved.FpsCounter) {
        $FpsCounter = $(if ($saved.FpsCounter -eq '0') { 'Off' } else { 'On' })
    }
    if (-not $PSBoundParameters.ContainsKey('AntiAliasing') -and
        $saved.AntiAliasing) {
        $AntiAliasing = $saved.AntiAliasing
    }
    if (-not $PSBoundParameters.ContainsKey('TextureFiltering') -and
        $saved.TextureFiltering) {
        $TextureFiltering = $saved.TextureFiltering
    }
    if (-not $PSBoundParameters.ContainsKey('Fullscreen') -and
        $saved.Fullscreen) {
        $Fullscreen = $(if ($saved.Fullscreen -eq '0') { 'Off' } else { 'On' })
    }
    if (-not $PSBoundParameters.ContainsKey('CardImage') -and
        $saved.CardImage) {
        $CardImage = $saved.CardImage
    }
}

if (-not (Test-Path -LiteralPath $exe -PathType Leaf)) {
    throw "The native recomp executable is missing: $exe"
}
if (-not $main -or -not (Test-Path -LiteralPath $main -PathType Leaf)) {
    throw "Could not find idas3_main_0C020000.bin under '$gameFilesRoot'."
}
if (-not $driveARoot -or
    -not (Test-Path -LiteralPath (Join-Path $driveARoot 'HOSTFS') `
        -PathType Container)) {
    throw "Could not find a driveA folder containing HOSTFS under '$gameFilesRoot'."
}

if ($ValidateOnly) {
    [pscustomobject]@{
        Executable = $exe
        MainProgram = $main
        Platform = 'Native NAOMI 2 services'
        VulkanPresentation = $VulkanPresentation
        VulkanOffscreenFlag = Get-VulkanOffscreenFlag $VulkanPresentation
        DriveA = $driveARoot
        Status = $(if ($gameFilesReady) { 'READY' } else { 'NEEDS SETUP' })
    }
    return
}

$processName = [System.IO.Path]::GetFileNameWithoutExtension($exe)
$existing = Get-Process -Name $processName -ErrorAction SilentlyContinue
if ($existing) {
    throw "The native recomp is already running (PID $($existing.Id -join ', '))."
}

foreach ($name in @(
    'IDAS3_NATIVE_JVS',
    'IDAS3_JVS_EEPROM',
    'IDAS3_NATIVE_INPUT_AUTOTEST',
    'IDAS3_NATIVE_INPUT_AUTOTEST_COIN_FRAME',
    'IDAS3_NATIVE_INPUT_AUTOTEST_START_FRAME',
    'IDAS3_NATIVE_INPUT_AUTOTEST_SECOND_START_FRAME',
    'IDAS3_NATIVE_INPUT_AUTOTEST_PULSE_POLLS',
    'IDAS3_NATIVE_INPUT_AUTOTEST_EXTENDED',
    'IDAS3_NATIVE_INPUT_AUTOTEST_EXTENDED_PERIOD_FRAMES',
    'IDAS3_NATIVE_INPUT_RECORD',
    'IDAS3_NATIVE_INPUT_PLAYBACK',
    'IDAS3_NATIVE_INPUT_TRACE',
    'IDAS3_NATIVE_AUDIO_NULL_SINK',
    'IDAS3_NATIVE_FRAME_SEQUENCE_DIR',
    'IDAS3_NATIVE_FRAME_SEQUENCE_FROM',
    'IDAS3_NATIVE_FRAME_SEQUENCE_STRIDE',
    'IDAS3_NATIVE_FRAME_SEQUENCE_MAX',
    'IDAS3_NATIVE_CARD_IMAGE',
    'IDAS3_NATIVE_CARD_INSERT',
    'IDAS3_CUSTOM_MUSIC',
    'IDAS3_CUSTOM_MUSIC_VOLUME',
    'IDAS3_DIAG_CARD_MANAGER',
    'IDAS3_DIAG_CARD_SERIAL',
    'IDAS3_DIAG_CARD_STATE_MACHINE',
    'IDAS3_DIAG_CARD_FLOW',
    # These automatic result controls belong exclusively to disposable-card
    # branch coverage. A normal user launch must never inherit them from a
    # diagnostic shell; manual F5/F6 support remains separately opt-in below.
    'IDAS3_DEVELOPER_RACE_OUTCOME_AUTO',
    'IDAS3_DEVELOPER_RACE_OUTCOME_AUTO_DELAY_CALLS',
    'IDAS3_DEVELOPER_RACE_OUTCOME_AUTO_WIN_PREFIX',
    'IDAS3_DEVELOPER_RACE_OUTCOME_AUTO_LOSS_RIVAL',
    'IDAS3_DEVELOPER_RACE_OUTCOME_AUTO_MIN_TARGET_TRAVEL',
    'IDAS3_DEVELOPER_RIVAL_PROGRESS_COMPLETE_BASE',
    'IDAS3_DEVELOPER_RIVAL_PROGRESS_COMPLETE_BASE_EAGER',
    'IDAS3_DEVELOPER_RIVAL_PROGRESS_COMPLETE_THROUGH_TAKUMI2')) {
    Remove-Item -LiteralPath "Env:$name" -ErrorAction SilentlyContinue
}
foreach ($slot in 1..13) {
    Remove-Item -LiteralPath ('Env:IDAS3_CUSTOM_MUSIC_{0:D2}' -f $slot) `
        -ErrorAction SilentlyContinue
}

if ($InputAutotest) {
    # Internal deterministic launcher regression route. This remains opt-in
    # and cannot affect an ordinary user launch.
    $env:IDAS3_NATIVE_INPUT_AUTOTEST = '1'
    $env:IDAS3_NATIVE_INPUT_AUTOTEST_COIN_FRAME = '1400'
    $env:IDAS3_NATIVE_INPUT_AUTOTEST_START_FRAME = '100000000'
    $env:IDAS3_NATIVE_INPUT_AUTOTEST_PULSE_POLLS = '8'
}

if ($RecordInputs -and $PlaybackInputs) {
    throw '-RecordInputs and -PlaybackInputs cannot be used together.'
}
if ($RecordInputs) {
    $inputRecordPath = if ([System.IO.Path]::IsPathRooted($RecordInputs)) {
        [System.IO.Path]::GetFullPath($RecordInputs)
    } else {
        [System.IO.Path]::GetFullPath(
            (Join-Path (Get-Location).Path $RecordInputs))
    }
    if (Test-Path -LiteralPath $inputRecordPath) {
        throw "Input recordings are never overwritten: $inputRecordPath"
    }
    $inputRecordParent = Split-Path -Parent $inputRecordPath
    if (-not (Test-Path -LiteralPath $inputRecordParent -PathType Container)) {
        New-Item -ItemType Directory -Path $inputRecordParent -Force | Out-Null
    }
    $env:IDAS3_NATIVE_INPUT_RECORD = $inputRecordPath
}
if ($PlaybackInputs) {
    $inputPlaybackPath = if ([System.IO.Path]::IsPathRooted($PlaybackInputs)) {
        [System.IO.Path]::GetFullPath($PlaybackInputs)
    } else {
        [System.IO.Path]::GetFullPath(
            (Join-Path (Get-Location).Path $PlaybackInputs))
    }
    if (-not (Test-Path -LiteralPath $inputPlaybackPath -PathType Leaf)) {
        throw "Input recording does not exist: $inputPlaybackPath"
    }
    $env:IDAS3_NATIVE_INPUT_PLAYBACK = $inputPlaybackPath
}

# User music is host-side and never changes the original game files. The
# native F1 Music page can import WAV/MP3/FLAC files here and map each of the
# game's thirteen selectable race-song slots.
$customMusicRoot = Join-Path $buildRoot 'custom music'
New-Item -ItemType Directory -Path $customMusicRoot -Force | Out-Null
$env:IDAS3_CUSTOM_MUSIC = $(
    if ($saved -and $saved.ContainsKey('CustomMusic')) {
        $saved.CustomMusic
    } else { '1' })
$env:IDAS3_CUSTOM_MUSIC_VOLUME = $(
    if ($saved -and $saved.ContainsKey('CustomMusicVolume')) {
        $saved.CustomMusicVolume
    } else { '85' })
foreach ($slot in 1..13) {
    $setting = 'CustomMusicSlot{0:D2}' -f $slot
    if (-not $saved -or -not $saved.ContainsKey($setting) -or
        -not $saved[$setting]) { continue }
    $musicPath = if ([System.IO.Path]::IsPathRooted($saved[$setting])) {
        [System.IO.Path]::GetFullPath($saved[$setting])
    } else {
        [System.IO.Path]::GetFullPath(
            (Join-Path $customMusicRoot $saved[$setting]))
    }
    Set-Item -LiteralPath ('Env:IDAS3_CUSTOM_MUSIC_{0:D2}' -f $slot) `
        -Value $musicPath
}

$cardDataRoot = Join-Path $buildRoot 'card data'
New-Item -ItemType Directory -Path $cardDataRoot -Force | Out-Null
$cardPath = if ($CardImage) {
    if ([System.IO.Path]::IsPathRooted($CardImage)) {
        $absoluteCardPath = [System.IO.Path]::GetFullPath($CardImage)
        $portableCardPath = [System.IO.Path]::GetFullPath(
            (Join-Path $cardDataRoot ([System.IO.Path]::GetFileName($CardImage))))
        $absoluteParent = Split-Path -Parent $absoluteCardPath
        if (-not (Test-Path -LiteralPath $absoluteCardPath -PathType Leaf) -and
            ((Test-Path -LiteralPath $portableCardPath -PathType Leaf) -or
             -not (Test-Path -LiteralPath $absoluteParent -PathType Container))) {
            # Keep this diagnostic off the success-output stream.  This block
            # is an expression assigned to $cardPath, so Write-Output would
            # turn the result into an array whose first item is this message.
            Write-Verbose "Repaired moved card setting: $portableCardPath"
            $portableCardPath
        } else {
            $absoluteCardPath
        }
    } else {
        [System.IO.Path]::GetFullPath(
            (Join-Path $cardDataRoot $CardImage))
    }
} else {
    [System.IO.Path]::GetFullPath(
        (Join-Path $cardDataRoot 'InitialD_player_card.card'))
}
$cardParent = Split-Path -Parent $cardPath
if (-not (Test-Path -LiteralPath $cardParent -PathType Container)) {
    throw "Card-image directory does not exist: $cardParent"
}
if (Test-Path -LiteralPath $cardPath) {
    $cardFile = Get-Item -LiteralPath $cardPath
    if ($cardFile.PSIsContainer -or $cardFile.Length -ne 207) {
        throw "Existing Initial D card images must be exactly 207 bytes: $cardPath"
    }
}
$selectedCardExists = Test-Path -LiteralPath $cardPath -PathType Leaf
if ($InsertCard -and -not $selectedCardExists) {
    throw "The configured card image does not exist yet: $cardPath"
}
$env:IDAS3_NATIVE_CARD_IMAGE = $cardPath
# A selected, valid slot is automatically queued before guest startup. This
# avoids asking the user to race the card-check screen with a second F1 action.
# An empty slot remains cardless so choosing NO can issue a new card into it.
$env:IDAS3_NATIVE_CARD_INSERT = $(if ($selectedCardExists) { '1' } else { '0' })
if ($RecordInputs) {
    Write-Output "input_recording=$inputRecordPath"
}
if ($PlaybackInputs) {
    Write-Output "input_playback=$inputPlaybackPath"
}

$env:IDAS3_HOST_DRIVEA = $driveARoot
$env:IDAS3_NATIVE_ARM_AICA = '1'
$env:IDAS3_NATIVE_AUDIO = $(if ($Mute) { '0' } else { '1' })
# Execute the authentic translated boot/device sequence at an accelerated
# cabinet clock while its audio and window output are hidden.  The presenter
# reveals only after ASegaLogo tick 130 and its first complete Vulkan frame,
# which starts the visible game at the fully formed Sega Rosso logo without
# bypassing guest menus, save state, device initialization, or game logic.
$env:IDAS3_NATIVE_BOOT_FAST_FORWARD_FRAME = '1000000'
$env:IDAS3_NATIVE_BOOT_FAST_FORWARD_HZ = '240'
$env:IDAS3_NATIVE_BOOT_SEGA_LOGO_REVEAL_TICK = '130'
# The old mailbox shim and the Flycast-derived ARM7/AICA implementation must
# never own sound RAM together. The ARM/AICA device is the production path.
$env:IDAS3_NATIVE_AICA = '0'
$env:IDAS3_NATIVE_ELAN = '1'
$env:IDAS3_NATIVE_ELAN_PORT_IMMEDIATE = '1'
$env:IDAS3_NATIVE_ELAN_ALL_GENERATIONS = '1'
$env:IDAS3_NATIVE_VULKAN = '1'
$env:IDAS3_MAIN_RAM_FAST_READ = '1'
$env:IDAS3_MAIN_RAM_FAST_WRITE = '1'
$env:IDAS3_NATIVE_MOVE_SCENE_HANDOFF = '1'
$env:IDAS3_ASSET_GUEST_ALLOC = '1'
# PACK2x/result sound packs now use their guarded transient aperture instead
# of fragmenting the overlapping guest heaps.  Keep 1 MiB in reserve for the
# remaining guest allocations; 4 MiB incorrectly forced the large name/kana
# font into that transient aperture and stalled the rival-to-race transition.
$env:IDAS3_ASSET_HEAP_RESERVE = '0x100000'
$env:IDAS3_WATCHDOG_SECONDS = $WatchdogSeconds.ToString()
$env:IDAS3_NATIVE_WINDOW = '1'
$env:IDAS3_NATIVE_WINDOW_FPS = $(
    if ($PresentationFps -eq 'Unlimited') { '0' } else { $PresentationFps })
# Display refresh is independent from the authentic 60 Hz cabinet/game clock.
# Vulkan generates the additional visual phases; never let a stale developer
# guest-120 experiment alter gameplay, physics, timers, input, or audio.
Remove-Item -LiteralPath 'Env:IDAS3_EXPERIMENTAL_GUEST_HZ' `
    -ErrorAction SilentlyContinue
Remove-Item -LiteralPath 'Env:IDAS3_DEV_ALLOW_GUEST_120' `
    -ErrorAction SilentlyContinue
$env:IDAS3_NATIVE_WINDOW_POS = '100,100'
$env:IDAS3_NATIVE_WINDOW_BACKGROUND = '0'
$env:IDAS3_NATIVE_WINDOW_TOPMOST = '0'
$env:IDAS3_NATIVE_FPS_COUNTER = $(if ($FpsCounter -eq 'On') { '1' } else { '0' })
$env:IDAS3_DEVELOPER_RACE_OUTCOME_KEYS = $(
    if ($DeveloperRaceOutcomes) { '1' } else { '0' })
if ($DeveloperRaceOutcomes) {
    Write-Output 'developer_race_outcomes=F5 player win; F6 player loss'
}
$env:IDAS3_NATIVE_ANTIALIASING = $AntiAliasing
$env:IDAS3_NATIVE_SUPERSAMPLE = switch ($AntiAliasing) {
    '2x' { '1.41421356237' }
    '4x' { '2.0' }
    default { '1.0' }
}
$env:IDAS3_NATIVE_TEXTURE_FILTERING = $TextureFiltering
$env:IDAS3_NATIVE_INPUT_DEVICE = $(
    if ($saved -and $saved.InputDevice) { $saved.InputDevice } else { '0' })
$env:IDAS3_NATIVE_FFB_STRENGTH = $(
    if ($saved -and $saved.ForceFeedbackStrength) {
        $saved.ForceFeedbackStrength
    } else { '60' })
$env:IDAS3_NATIVE_STEERING_SMOOTHING = $(
    if ($saved -and $null -ne $saved.SteeringSmoothing -and
        $saved.SteeringSmoothing -ne '') {
        $saved.SteeringSmoothing
    } else { '0' })
foreach ($binding in @{
    BindSteerLeft = 'IDAS3_NATIVE_BIND_STEER_LEFT'
    BindSteerRight = 'IDAS3_NATIVE_BIND_STEER_RIGHT'
    BindAccel = 'IDAS3_NATIVE_BIND_ACCEL'
    BindBrake = 'IDAS3_NATIVE_BIND_BRAKE'
    BindShiftDown = 'IDAS3_NATIVE_BIND_SHIFT_DOWN'
    BindShiftUp = 'IDAS3_NATIVE_BIND_SHIFT_UP'
    BindStart = 'IDAS3_NATIVE_BIND_START'
    BindView = 'IDAS3_NATIVE_BIND_VIEW'
    BindCoin = 'IDAS3_NATIVE_BIND_COIN'
    XInputBindSteerLeft = 'IDAS3_NATIVE_XINPUT_BIND_STEER_LEFT'
    XInputBindSteerRight = 'IDAS3_NATIVE_XINPUT_BIND_STEER_RIGHT'
    XInputBindAccel = 'IDAS3_NATIVE_XINPUT_BIND_ACCEL'
    XInputBindBrake = 'IDAS3_NATIVE_XINPUT_BIND_BRAKE'
    XInputBindShiftDown = 'IDAS3_NATIVE_XINPUT_BIND_SHIFT_DOWN'
    XInputBindShiftUp = 'IDAS3_NATIVE_XINPUT_BIND_SHIFT_UP'
    XInputBindStart = 'IDAS3_NATIVE_XINPUT_BIND_START'
    XInputBindView = 'IDAS3_NATIVE_XINPUT_BIND_VIEW'
    XInputBindCoin = 'IDAS3_NATIVE_XINPUT_BIND_COIN'
    DirectInputBindSteerLeft = 'IDAS3_NATIVE_DINPUT_BIND_STEER_LEFT'
    DirectInputBindSteerRight = 'IDAS3_NATIVE_DINPUT_BIND_STEER_RIGHT'
    DirectInputBindAccel = 'IDAS3_NATIVE_DINPUT_BIND_ACCEL'
    DirectInputBindBrake = 'IDAS3_NATIVE_DINPUT_BIND_BRAKE'
    DirectInputBindShiftDown = 'IDAS3_NATIVE_DINPUT_BIND_SHIFT_DOWN'
    DirectInputBindShiftUp = 'IDAS3_NATIVE_DINPUT_BIND_SHIFT_UP'
    DirectInputBindStart = 'IDAS3_NATIVE_DINPUT_BIND_START'
    DirectInputBindView = 'IDAS3_NATIVE_DINPUT_BIND_VIEW'
    DirectInputBindCoin = 'IDAS3_NATIVE_DINPUT_BIND_COIN'
}.GetEnumerator()) {
    if ($saved -and $saved[$binding.Key]) {
        Set-Item -LiteralPath "Env:$($binding.Value)" `
            -Value $saved[$binding.Key]
    } else {
        Remove-Item -LiteralPath "Env:$($binding.Value)" `
            -ErrorAction SilentlyContinue
    }
}
$env:IDAS3_NATIVE_PRESENT_FPS = $PresentationFps
$env:IDAS3_NATIVE_VSYNC = $(if ($VSync -eq 'On') { '1' } else { '0' })
$env:IDAS3_NATIVE_VULKAN_OFFSCREEN =
    Get-VulkanOffscreenFlag $VulkanPresentation
$env:IDAS3_NATIVE_FULLSCREEN = $(if ($Fullscreen -eq 'On') { '1' } else { '0' })
$env:IDAS3_NATIVE_WINDOW_MONITOR = $Monitor.ToString()
$resolutionSizes = @{
    '720p' = '1280x720'
    '1080p' = '1920x1080'
    '1440p' = '2560x1440'
    '4K' = '3840x2160'
    'Ultrawide1080p' = '2560x1080'
}
if ($Resolution -eq 'Native') {
    Remove-Item -LiteralPath 'Env:IDAS3_NATIVE_RENDER_SIZE' -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath 'Env:IDAS3_NATIVE_RENDER_SCALE' -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath 'Env:IDAS3_NATIVE_WINDOW_SIZE' -ErrorAction SilentlyContinue
} else {
    $selectedSize = $resolutionSizes[$Resolution]
    $env:IDAS3_NATIVE_RENDER_SIZE = $selectedSize
    # SIZE derives the correct native projection scale automatically. Leaving
    # the old diagnostic SCALE set would override that and can crop the view.
    Remove-Item -LiteralPath 'Env:IDAS3_NATIVE_RENDER_SCALE' -ErrorAction SilentlyContinue
    $env:IDAS3_NATIVE_WINDOW_SIZE = $selectedSize
}
$env:IDAS3_NATIVE_120FPS_TRACE = $(if ($PresentationTrace) { '1' } else { '0' })
$env:IDAS3_RASTER_WORKER = '1'
$env:IDAS3_VBLANK_WALL = '1'
$env:IDAS3_DIAG_FPS = '1'
if ($CardTrace) {
    $env:IDAS3_DIAG_CARD_MANAGER = '1'
    $env:IDAS3_DIAG_CARD_SERIAL = '1'
    $env:IDAS3_DIAG_CARD_STATE_MACHINE = '1'
    $env:IDAS3_DIAG_CARD_FLOW = '1'
} else {
    foreach ($name in @(
        'IDAS3_DIAG_CARD_MANAGER',
        'IDAS3_DIAG_CARD_SERIAL',
        'IDAS3_DIAG_CARD_STATE_MACHINE',
        'IDAS3_DIAG_CARD_FLOW')) {
        Remove-Item -LiteralPath "Env:$name" -ErrorAction SilentlyContinue
    }
}
$env:IDAS3_NATIVE_FRAMEBUFFER_TIMING_TRACE = $(if ($FramebufferTiming) { '1' } else { '0' })
$env:IDAS3_NATIVE_VULKAN_TIMING_TRACE = $(if ($VulkanTiming) { '1' } else { '0' })
$env:IDAS3_NATIVE_AUDIO_SIGNAL_TRACE = $(if ($AudioSignalTrace) { '1' } else { '0' })
$env:IDAS3_NATIVE_FRAME_SERVICE_TIMING_TRACE = '0'
$env:IDAS3_NATIVE_INPUT_TRACE = $(if ($InputTrace) { '1' } else { '0' })
$env:IDAS3_GUEST_PROFILE = $(if ($GuestProfile) { '1' } else { '0' })
$env:IDAS3_NATIVE_PIXEL_OWNER_TRACE = '0'
$env:IDAS3_NATIVE_AICA_STREAM_FRONTIER_TRACE = $(if ($AicaTrace) { '1' } else { '0' })
$env:IDAS3_NATIVE_AICA_API_TRACE = $(if ($AicaTrace) { '1' } else { '0' })
$env:IDAS3_NATIVE_AICA_INIT_TRACE = '0'
$env:IDAS3_NATIVE_AICA_ALLOCATOR_TRACE = '0'

New-Item -ItemType Directory -Path $logRoot -Force | Out-Null
$stamp = Get-Date -Format 'yyyyMMdd_HHmmss'
$stdout = Join-Path $logRoot "native_user_$stamp.stdout.log"
$stderr = Join-Path $logRoot "native_user_$stamp.stderr.log"

$mainArgument = '"' + $main + '"'
$process = Start-Process -FilePath $exe -ArgumentList @($mainArgument) `
    -WorkingDirectory $buildRoot -RedirectStandardOutput $stdout `
    -RedirectStandardError $stderr -PassThru

# A successful process creation is not enough: missing Vulkan/runtime support
# and malformed inputs can make the child close immediately. Give native boot
# enough time to establish its window, then turn an early exit into a launcher
# error that the Windows front-end can display.
Start-Sleep -Seconds 3
$process.Refresh()
if ($process.HasExited) {
    $details = @()
    foreach ($path in @($stderr, $stdout)) {
        if (Test-Path -LiteralPath $path -PathType Leaf) {
            $details += Get-Content -LiteralPath $path -Tail 12 `
                -ErrorAction SilentlyContinue
        }
    }
    $detailText = ($details | Where-Object { $_ } | Select-Object -Last 12) `
        -join [Environment]::NewLine
    if (-not $detailText) { $detailText = "Exit code $($process.ExitCode)" }
    throw "The game closed during startup. Please check the game files and Vulkan graphics driver.`n`n$detailText"
}

Write-Output "pid=$($process.Id)"
Write-Output "stdout=$stdout"
Write-Output "stderr=$stderr"
Write-Output "card_image=$cardPath"
Write-Output "card_inserted_at_start=$selectedCardExists"
if (-not $selectedCardExists) {
    Write-Output 'card_hint=The selected slot is empty. Choose NO at the card prompt to issue and save a new card here automatically.'
}
