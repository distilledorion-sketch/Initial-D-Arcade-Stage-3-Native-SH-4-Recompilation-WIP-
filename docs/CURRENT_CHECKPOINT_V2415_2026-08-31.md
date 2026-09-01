# Current checkpoint — v2415 public early demo

Date: 2026-08-31

## Download and integrity

- Release: [Public Early Demo v2415](https://github.com/distilledorion-sketch/Initial-D-Arcade-Stage-3-Native-SH-4-Recompilation-WIP-/releases/tag/v2415-host-safety)
- Public ZIP SHA-256:
  `8CA9D017EA5BCFE8EB58245D162D89A22D75267BC6D3BABAD76D922E283B2E8E`
- Native `demo.exe` SHA-256:
  `3185FF67F00EE5E8EF17E63F5B10ECDF66494B3B604358ED7DA863C7AB56B8D4`

The local audited ZIP is 17,877,033 bytes and contains 14 files totaling
27,934,284 uncompressed bytes. Its audit found no CHD, PIC, BIOS, extracted
HOSTFS, game image, card save, custom song, user log, or personal filesystem
path.

## Host lifecycle changes

v2415 adds a process-wide Windows mutex. A second product copy exits before it
can create another window or graphics context. The separate QA-profile mutex
also rejects concurrent automated runs.

The Windows QA watchdog now asks the presenter to stop cooperatively. The guest
unwinds at its normal presentation boundary, new WSI work stops, the raster
worker drains, and only then is the window destroyed. The PowerShell harness no
longer has an abrupt termination fallback.

Route-only matrix runs now default to the CPU renderer at a one-frame-per-second
preview rate. This preserves authentic translated menu/input/physics behavior
and the normal window-close path without creating a Vulkan swapchain. Live WSI
profiling requires an explicit opt-in.

## Verification

- All 13 controller/music and 16 lifecycle policy tests passed.
- Controller binding/smoothing, custom music, interpolation, ELAN, card, AICA,
  offscreen Vulkan, card-eject, translation, freshness, and standalone-product
  checks passed.
- The product linked 116 objects; all 108 checked source owners were fresh.
- The standalone audit found zero firmware callbacks, firmware AOT objects,
  firmware input contracts, or cached firmware translations.
- Three v2415 CPU-only route probes passed with zero runtime fault markers and
  complete shutdown markers:
  - Myogi left/dry/day Time Attack: `k_ez`, forward, daytime, 9.548 m.
  - Shomaru Bunta: `n_sy2`, forward, night, 5.097 m and 480 AI samples.
  - Tsuchisaka Bunta: `k_tu2`, forward, night, 6.302 m and 480 AI samples.
- No live Win32 Vulkan surface was opened for these route probes.

Broader accepted evidence remains 48/48 route/branch loads, 32/32 moving rival
profiles, 16/16 natural Time Attack results, and 32/32 natural rival results.
The separate 70-row condition ledger remains mixed-checkpoint evidence rather
than a full same-build completion matrix.

## Remaining acceptance work

- Complete the same-build Time Attack/Bunta matrix and remaining campaign/error
  branches with the safe route-only profile.
- Broaden physical controller, wheel, force-feedback, and audible-audio tests.
- Complete scene-by-scene visual coverage across cars, courses, weather, and
  time of day.
- Perform conservative live Vulkan close/restart coverage across GPU/driver
  setups only after the no-WSI lifecycle gates remain green.
- Expand clean-machine launcher/setup testing and higher-refresh coverage.

v2415 is suitable for cautious public playtesting at the default 60 FPS. It is
not a finished release or proof of exhaustive whole-game coverage.
