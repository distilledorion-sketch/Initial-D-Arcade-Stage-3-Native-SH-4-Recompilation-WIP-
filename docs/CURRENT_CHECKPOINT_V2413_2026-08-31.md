# Current checkpoint — v2413 public early demo

Date: 2026-08-31

## Download and integrity

- Release: [Public Early Demo v2413](https://github.com/distilledorion-sketch/Initial-D-Arcade-Stage-3-Native-SH-4-Recompilation-WIP-/releases/tag/v2413-steering-smoothing)
- Public ZIP SHA-256:
  `08EA0678658AB154B0EFEFD56D326740B89F7467D6692BDE47D94DD7A4CDB830`
- Native `demo.exe` SHA-256:
  `7AA661E40014326FC29264DF95774444C47DD73B7353FF30D063BCC944700FD5`

GitHub reports the exact 17,768,600-byte ZIP and matching SHA-256. The package
contains 14 files totaling 27,989,406 uncompressed bytes. Its audit found no
CHD, PIC, BIOS, extracted HOSTFS, game image, card save, custom song, user log,
or personal filesystem path.

## Current product behavior

- BIOS-free statically translated SH-4 execution with native NAOMI 2 services;
  there is no SH-4 interpreter, JIT fallback, or BIOS dependency.
- Python-free first-run verification and local extraction from matching
  user-owned `gds-0033.chd` and `317-0384-com.pic` inputs.
- Vulkan presentation with authentic 60 Hz timing and experimental higher-rate
  presentation kept independent from gameplay, timers, physics, input, and
  audio cadence.
- F1 Video, Controls, HUD, Card, and Music pages.
- Keyboard, XInput controller, and DirectInput wheel paths with persistent
  remapping, independent trigger/pedal axes, force-feedback strength, and a
  saved 0–100% Steering Smoothing slider. Zero is an exact bypass.
- Native AICA audio, optional exact-slot custom music, and named card storage.

## Verification

The v2413 product linked 116 objects and passed freshness checks for 108 source
owners. The complete offline suite covers controller discovery, remapping,
normalization, smoothing, music routing, presentation interpolation, ELAN,
cards, AICA, offscreen Vulkan, lifecycle policy, translation integrity, and the
standalone no-firmware boundary.

Current race evidence remains 48/48 route/branch loads, 32/32 moving rival
profiles, 16/16 natural Time Attack results, and 32/32 natural rival results.
The separate 70-row Time Attack/Bunta condition ledger mixes checkpoints and
is not yet a full same-build completion matrix.

## Remaining acceptance work

- Broader physical controller, wheel, force-feedback, and audible-audio tests.
- A same-build Time Attack/Bunta matrix and remaining campaign/error branches.
- Scene-by-scene visual coverage across cars, courses, weather, and time.
- Conservative live Vulkan close/restart coverage across GPU/driver setups.
- Stable higher-refresh coverage beyond measured routes and, later, uncapped
  presentation independent from guest timing.
- Broader clean-machine launcher/setup testing.

v2413 is suitable for cautious public playtesting at the default 60 FPS. It is
not a finished release or proof of exhaustive whole-game coverage.
