# Current checkpoint — v2414 public early demo

Date: 2026-08-31

## Download and integrity

- Release: [Public Early Demo v2414](https://github.com/distilledorion-sketch/Initial-D-Arcade-Stage-3-Native-SH-4-Recompilation-WIP-/releases/tag/v2414-bounded-vulkan-upload)
- Public ZIP SHA-256:
  `826780D48ECAF3411E12F2EBD91EA0989EC66145AF1C692546BC36955C68A90F`
- Native `demo.exe` SHA-256:
  `68F3D8501E1BE58EC07B167E1703703260B3523D47CA545B786ABAAB1FD53D0B`

GitHub reports the exact 17,768,925-byte ZIP and matching SHA-256. The package
contains 14 files totaling 27,989,968 uncompressed bytes. Its audit found no
CHD, PIC, BIOS, extracted HOSTFS, game image, card save, custom song, user log,
or personal filesystem path.

## Host-safety change

Vulkan startup texture uploads previously retired work with
`vkQueueWaitIdle`, which has no timeout. v2414 gives each upload its own fence
and waits no longer than two seconds. If a driver does not signal the fence,
the runtime disables Vulkan and deliberately leaves potentially in-flight
resources untouched instead of resetting, destroying, or reusing them.

The backend now contains no `vkQueueWaitIdle`, `vkDeviceWaitIdle`, infinite
fence wait, or `UINT64_MAX` wait. Frame submission, image acquisition,
presentation-fence retirement, and startup-upload waits are all bounded.

## Verification

- The actual offscreen Vulkan initialization path completed both startup
  texture uploads and initialized the AMD device successfully.
- All 13 controller/music and 12 lifecycle policy tests passed.
- Controller binding/smoothing, custom music, interpolation, ELAN, card, AICA,
  offscreen Vulkan, card-eject, translation, freshness, and standalone-product
  checks passed.
- The product linked 116 objects; all 108 checked source owners were fresh.
- The standalone audit found zero firmware callbacks, firmware AOT objects,
  firmware input contracts, or cached firmware translations.
- No full game window or live Win32 Vulkan surface was opened for this change.

Current race evidence remains 48/48 route/branch loads, 32/32 moving rival
profiles, 16/16 natural Time Attack results, and 32/32 natural rival results.
The separate 70-row condition ledger remains mixed-checkpoint evidence rather
than a full same-build completion matrix.

## Remaining acceptance work

- Broader physical controller, wheel, force-feedback, and audible-audio tests.
- A same-build Time Attack/Bunta matrix and remaining campaign/error branches.
- Scene-by-scene visual coverage across cars, courses, weather, and time.
- Conservative live Vulkan close/restart coverage across GPU/driver setups.
- Stable higher-refresh coverage beyond measured routes and, later, uncapped
  presentation independent from guest timing.
- Broader clean-machine launcher/setup testing.

v2414 is suitable for cautious public playtesting at the default 60 FPS. It is
not a finished release or proof of exhaustive whole-game coverage.
