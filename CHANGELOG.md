# Changelog

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
