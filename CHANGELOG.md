# Changelog

## v2411-public-early-demo — 2026-08-31

- Published the first Windows x64 public early-demo prerelease. The package
  contains no game data, BIOS, extracted assets, card saves, custom music,
  logs, or personal paths and accepts only matching user-owned CHD/PIC inputs.
- Added a self-contained Python-free Windows setup/integrity flow with specific
  missing, incorrect-revision, corrupt-input, and incomplete-extraction errors.
- Added an XInput controller provider with persistent button/axis mappings while
  retaining DirectInput wheel support and independent Xbox trigger axes.
- Added automatic last-card selection, visible card-data storage, named card
  creation, and a Yes/No unsaved-data warning before a card-triggered restart.
- Replaced guessed custom-music cross-slot routing with exact opened-stream
  replacement; clearing or disabling a slot restores the original game track.
- Reworked renderer shutdown to stop new Vulkan presentation, wait for the
  raster worker's explicit exit handshake, join it, and destroy the window only
  afterward; removed the forced `TerminateProcess` fallback.
- Kept 60 FPS as the safe default and labeled higher-refresh presentation as
  experimental.
- Documented a post-release controller defect: at least one Xbox controller is
  not detected by v2411 despite the provider-neutral binding tests passing.

## v2396-wip — 2026-08-30

- Removed the optional held-View host OST picker and kept song selection on a quick Change View press in the authentic opponent selector.
- Added filename-based custom BGM titles only for slots with valid user replacements; clearing a slot leaves the original game title and music.
- Unified physical, controller, and deterministic replay View input at the final logical JVS boundary.
- Narrowed song-selection context from early card/mode screens to the real course-specific rival-face selector.
- Synchronized the displayed 0/1–13 choice with the actual race stream, custom decoder, and AICA mixer; one bounded live route proved all layers agree.
- Rebuilt the translated `DIAG_0C0E6280.obj` owner that supplies the active inline asset-loading shim and added an automatic freshness gate so stale COMDAT behavior cannot silently return.
- Reconfirmed the product is BIOS-free and contains zero firmware callbacks, firmware AOT objects, firmware input contracts, or cached firmware translations.
- Kept the demo executable, game data, translated guest bodies, cards, private logs, and user music out of the public repository.

## v2374-wip — 2026-08-30

- Profiled a reproducible cold `k_ez` race-start slowdown and ruled out Vulkan, ELAN average cost, AICA, and repeated presentation endpoints.
- Traced the dominant guest-side startup cost to the game's canonical byte-wise object clear.
- Added a private exact bulk-main-RAM path that retains one guest memory cycle per byte and final SH-4 registers/flags, while leaving device, VRAM, ELAN, host-aperture, boundary-crossing, and zero-length cases on the original translated path.
- Improved the identical resolved-input/card route from 100.0 minimum / 116.609 average generated FPS on v2373 to 120.0 minimum / 120.0 average on v2374 across 23 complete two-second race intervals.
- Confirmed a zero repeat-counter delta during moving gameplay, zero faults, and unchanged course/transition timing.
- Added helper-level and exact-linked-function regression coverage, revalidated the card-eject BRAF classifier, and passed the native audio/music, presenter, ELAN/JVS/card, AICA, and Vulkan suites.
- Reconfirmed the private build is BIOS-free through a standalone audit reporting zero firmware callbacks, objects, input contracts, or cached firmware translations.
- Kept broader course validation, exhaustive whole-game coverage, physical hardware acceptance, and unlimited presentation explicit as open work.

## v2217-wip — 2026-08-27

- Extended the independent fixed presentation cadence to the native 60 Hz target.
- Traced the earlier 29.3/51.2 FPS race-entry samples to the 60 Hz presenter waiting for a new guest scene during synchronous course/music loads, rather than slow raster or Vulkan work.
- Retained the last completed image during those guest pauses without advancing game functions, timers, input, audio sequencing, or physics.
- Validated 43 takeover/race samples at 3840×2160 between 59.8 and 60.1 FPS while normal JVS/physics input drove 668.687 m.
- Revalidated 1920×1080 at 120 FPS between 119.8 and 120.1 during operation; recorded a diagnostic-only shutdown-dump outlier separately.
- Kept whole-game coverage, underlying load latency, hardware acceptance, packaging, and uncapped presentation as explicit open work.

## v2216-wip — 2026-08-26

- Advanced the private NAOMI 2 static-AOT integration through targeted menu, loading, live-race, result, and disposable-card save/reload paths without an interpreter or JIT fallback.
- Corrected tested HUD projection, mirror placement, car visibility/texture mapping, modifier-volume behavior, audio ring lifecycle, JVS input, and digital-shifter behavior.
- Added a persistent renderer worker pool and batched Vulkan texture-upload barriers.
- Identified seven race-entry Vulkan pipeline variants through timing probes and prewarmed them at initialization, reducing the two heaviest measured entry frames from 20.97/19.01 ms to 11.36/5.69 ms with zero in-frame pipeline creation.
- Validated sustained 3840×2160 presentation at 59.9–60.2 FPS after live motion begins and 1920×1080 presentation interpolation at 119.8–120.1 FPS while preserving authentic guest timing.
- Verified sustained AICA output/race-stream selection, no-card operation, and disposable-card persistence while keeping automated QA muted and isolated from real card data.
- Kept remaining limitations explicit: whole-game coverage, two synchronous transition-load gaps, physical wheel/audio acceptance, clean-machine packaging, and uncapped presentation remain open.
- Removed Dreamcast from the roadmap; this remains a NAOMI 2 recompilation project.

## v2018-wip — 2026-08-18

- Advanced the private static-AOT runtime beyond the earlier submitted-stream lifecycle blocker into sustained native Naomi 2 scene presentation.
- Added focused native validation for AICA SGC/mailbox behavior, Maple VBlank/reset, system-ROM protection, JVS behavior, and PVR texture layout.
- Traced alternating rainbow/static environment maps to two missing ISO9660-truncated course lookup files rather than the 30/60 FPS presentation cap.
- Added a source-safe, hash-checked preparation tool that restores those logical files from user-owned inputs without redistributing game data.
- Validated a coherent native Trueno camera sequence with a 60 FPS presentation target.
- Kept the current limitation explicit: the diagnostic renderer does not yet sustain 60 unique frames per second, and a complete controllable race is not yet proven.

## v1335-wip — 2026-08-15

- Packaged a source-safe public progress repository.
- Published general SH-4 decoder/code-generator tests and a native ELAN classifier smoke test.
- Documented the accepted 3D intro milestone and current frame-lifecycle blocker.
- Expanded `RegisterWait` classification to the five observed list-complete masks in the integration snapshot.
- Kept the stage-112 fix explicitly unclaimed until linked-stream execution passes the exact replay gates.
