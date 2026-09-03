# v2489 verified Vulkan checkpoint — 2026-09-03

v2489 is the newest accepted Windows x64 executable. It remains an unfinished
public early demo, not a claim that every game path or hardware combination is
complete.

## Product boundary

- Native static-AOT SH-4 execution; no interpreter, JIT, or NAOMI 2 BIOS
  fallback is part of the product path.
- Native Vulkan is the normal fresh-install renderer. Safe presentation remains
  a manually selected troubleshooting option and is not forced by the public
  launcher.
- The public archive contains no CHD, PIC, BIOS, extracted game asset, card,
  custom music, log, capture, generated guest-code source, or private path.
- Users must supply their own matching `gds-0033.chd` and
  `317-0384-com.pic`; setup verifies and extracts them locally.

## Accepted executable evidence

- Runtime SHA-256:
  `558468793CCF52D79D3A1858B267CFBAD53030C158D79E11269075C3D62D088D`
- Size: 15,304,704 bytes.
- Fresh link inputs: 117/117.
- Standalone no-BIOS product gate: pass.
- Native Vulkan GPU-product gate: pass; the gate rejects CPU-only artifacts.
- Reachable SH-4 symbols: 1,079.
- Missing, unwired, and unknown static frontiers: 0/0/0.
- Reachable SH-4 JSR/BSRF sites checked: 670; missing architectural PR writes:
  zero.
- The owner completed a roughly 20-minute standalone Direct-Vulkan session
  without a process crash. This does not replace broader long-session and
  multi-PC testing.

## Public launcher changes

- Fresh installations default to Direct Vulkan and retain a user's saved
  Direct/Safe selection during updates.
- A per-installation mutex and early runtime-process check prevent duplicate
  launchers from performing update or extraction work concurrently.
- A transient integrity failure gets one delayed selective retry. The launcher
  no longer forces a complete hash of all 2,196 extracted assets during an
  ordinary launch.
- The update prompt is visible in the taskbar, closeable, and defaults to No
  after 30 seconds. A failed staged update removes only its bounded updater
  working directory.
- The five-level saved game difficulty is validated and forwarded to the
  runtime.

## Package audit

- ZIP SHA-256:
  `6AF00678196E9E54AC1C625866D08D2CD9855D0F43866B5DE1EDCBD7DCE0EF93`
- ZIP size: 18,424,321 bytes.
- File count: 17, under one canonical
  `Initial D Arcade Stage 3 Recompiled` folder.
- Direct Vulkan is the factory default in the packaged settings.
- Exact allowlist and extension/text scans found no game image, PIC, BIOS,
  extracted asset, card, music, log, capture, object, map, private host path,
  or Codex/development launch variable.
- An isolated real-package expansion and merge-install acceptance verified the
  GitHub digest, version marker, canonical folder/runtime names, Direct Vulkan
  default, preservation of settings/game files/cards/music/logs and unrelated
  destination files, and confirmed that no game process was started.

## Current limitations

- Start with 60 FPS. Distinct 90/120 Hz presentation remains experimental and
  may expose route-specific jitter, clipping, or flicker.
- Controller, wheel, audio, card, replay, and visual correctness still need
  broader real-hardware and long-session acceptance.
- Newer v2490 source hardening is not included in this executable and is not
  being presented as runtime-verified.
