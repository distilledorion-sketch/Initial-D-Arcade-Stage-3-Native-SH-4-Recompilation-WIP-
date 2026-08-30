# Current source-safe checkpoint — 2026-08-30

This document reports the private v2374 integration checkpoint without
publishing a game binary, generated game translation, ROM/BIOS/PIC/CHD data,
assets, captures, logs, input recordings, save/card data, memory snapshots, or
private host paths.

## Project boundary

- Target hardware: Sega NAOMI 2.
- Execution model: native static ahead-of-time SH-4 recompilation.
- Graphics API: Vulkan.
- Authentic game logic, physics, timers, input, and audio cadence: 60 Hz.
- Enhanced presentation target: distinct 120 Hz motion samples.
- No interpreter, JIT, emulator runtime, fabricated game state, or replacement game assets.
- The shipped private runtime is BIOS-free; firmware remains a development oracle, not a runtime dependency.

## Reproducible cold-race slowdown

The same resolved-input and disposable-card route was run on the private v2373
product. It entered course `k_ez` at 102.2 seconds, produced no repeated moving
endpoints and reported no faults, but the first full race intervals fell as low
as 100 generated/display FPS. The 22 complete intervals averaged 116.609 FPS.

Low-overhead timing and A/B runs ruled out Vulkan rendering, average ELAN
capture cost, AICA, and presentation repetition. A guest-thread sampling run
identified the game's canonical byte-wise memset while cold race objects were
being allocated and initialized.

## Exact v2374 repair

The private translated integration now performs one host bulk fill only when
the complete canonical destination range is proven to be ordinary main RAM.
It preserves the translated routine's final registers and T flag and charges
exactly one guest memory-access cycle per byte. Zero-length, device, VRAM,
ELAN, host-aperture, and boundary-crossing cases retain the original path.

No gameplay, physics, timer, audio, card, renderer, or presentation rule was
changed.

## Performance evidence

The matching v2374 route entered the same `k_ez` course at 102.3 seconds.
Across all 23 complete two-second race intervals:

- minimum generated/display FPS: 120.0
- average generated/display FPS: 120.0
- minimum generated motion samples per interval: 120
- moving-race repeat-counter delta: 0
- fault markers: 0

The result is distinct presentation interpolation over authentic 60 Hz guest
state, not duplicated 60 Hz endpoints.

## Build identity and checks

Private checkpoint executable SHA-256 (binary not distributed):

`81F18BBEA04D42B692BE295630A3F6A6DE8DF6F479ECF4881E38FCAC100FD0C4`

Validation passed for:

- exact bulk range/content/cycle behavior;
- the exact linked translated memset across zero, one-byte, unaligned, and P1-alias cases;
- custom music decoding/mixing and native audio integration;
- presenter interpolation;
- card/JVS, native ELAN, and VRAM aliases;
- the exact card-eject jump-table classifier;
- 360-frame Vulkan resize/presentation smoke coverage;
- link freshness across 116 objects;
- standalone BIOS-free audit with zero firmware dependencies.

## Remaining work

1. Repeat the cold distinct-120 gate across every course and heavy scene.
2. Validate every mode, car, opponent, condition, outcome, continue/result path, and card-error branch.
3. Complete physical wheel, force-feedback, controller, and audible-device acceptance.
4. Finish clean-machine packaging, crash reporting, settings polish, and long-session stability.
5. Pursue unlimited presentation only after the 120 Hz whole-game gate is protected.

This is an accepted measured-route improvement, not a whole-game completion claim.
