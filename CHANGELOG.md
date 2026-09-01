# Changelog

## Unreleased v2449 cinematic-mask widescreen checkpoint — 2026-09-01

- Traced the centered-4:3 black bars in the authentic attract sequence to two
  object-space four-vertex cinematic matte batches rather than HUD sprites or
  terrain.
- Added a fail-closed classifier using the exact matte mesh/material signature
  plus proof of projected native x=0..640 coverage.
- Expanded only accepted mattes about the output centre, reaching the true
  2560x1080 edges without stretching HUD or changing Hor+ world projection;
  native 4:3 placement remains unchanged.
- Passed two BIOS-free attract captures across scenes 3300..3630, including
  camera cuts whose focal length changes during the matte animation.
- Matched six v2448/v2449 640x480 captures byte-for-byte and completed a normal
  visible 2560x1080 Direct run at 119.5–120.5 FPS after warm-up with no
  diagnostic readback or recognized fault.
- Passed the complete controller, real-Xbox, audio, music, card, interpolation,
  ELAN, Vulkan, timing, lifecycle, translation, freshness, and standalone
  no-firmware suite with cooperative exit and no stale process/session marker.
- Renewed the complete 70-row route matrix on the exact final v2449
  executable: all 62 defined Time Attack direction/weather/time rows and all
  eight Bunta courses. Every check matched its requested state, reached real
  movement, logged zero recognized faults, and exited cooperatively; strict
  no-launch reanalysis returned 70 passes, zero failures, and zero missing.
- Began exact-v2449 natural-result renewal for the 32 retained Legend/Bunta
  rival profiles. Sakamoto and Wataru passed first: each target moved before a
  natural game-owned result, received no requested/applied QA outcome after
  its target gate armed, held at least 59.6 FPS in the accepted race samples,
  reported zero recognized errors/runtime faults, and exited cooperatively.

## Unreleased v2448 honest-FPS checkpoint — 2026-09-01

- Corrected the in-game FPS overlay so `OUT` is sampled from actual swapchain
  presentations rather than renderer submissions.
- Made the presentation counters atomic, removing stale cross-thread reads.
- Counted both fresh authentic endpoints and interpolated midpoints as `NEW`,
  while excluding exact cadence and overlay re-presents.
- Updated the 120 Hz analyzer to use race-motion/positive-travel telemetry so
  distinct animated menu frames cannot be misclassified as racing.
- Passed 33 lifecycle/renderer policies, 20 route-parser policies, the new
  12-case analyzer contract, and the complete BIOS-free product suite.
- Completed a normal visible 2560x1080 Direct-Vulkan Akagi run at 120.0 FPS
  minimum/average across 21 moving-race samples, 240 distinct frames per
  two-second sample, zero repeat growth, zero faults, 129.204 m of movement,
  cooperative exit code 0, and clean Direct-session-marker removal.

## Unreleased route-evidence QA — 2026-09-01

- Corrected the interim route-evidence interpretation after broader course
  coverage proved `_etc_f` means dry and `_etc_r` means wet/snow, rather than
  left and right.
- Extended the developer-only selector snapshot to the segmented `_pol_a.tbl`
  course format, while leaving ordinary product launches unchanged.
- Kept direction validation fail-closed when the three independent selector
  fields are missing, partial, or disagree with the manifest.
- Normalized mixed-generation CSV rows before export so newer evidence columns
  cannot be silently discarded by the first legacy object in the collection.
- Added a no-launch evidence reanalysis mode which accepts only logs already
  tied to the exact executable SHA-256.
- Passed 20 route-policy contracts and the complete v2446 BIOS-free product,
  controller, audio, card, renderer, timing, lifecycle, freshness, and
  standalone suites.
- Revalidated Akina Snow left/snow/night and Happogahara left/dry/night with
  exact course/condition assets, independent `0,0,0` direction identities,
  real movement, no recognized faults, and cooperative exit code 0.

## v2445-direct-persistence — 2026-09-01

- Made the F1 Direct/Safe Vulkan presentation choice survive launcher restart.
- Mapped Direct to the optimized Vulkan swapchain and Safe to the bounded
  GPU-readback/Win32 presentation path through one shared launcher helper.
- Added read-only validation output for the exact runtime flag without starting
  a game or Vulkan process.
- Added public static contracts for validated parameters, saved-setting
  precedence, mode mapping, Safe defaults, and private-path exclusion.
- Repackaged the unchanged accepted v2444 native executable in a re-audited
  14-file Windows x64 early demo.

## v2444-audio-card-portability — 2026-09-01

- Persisted managed card selection by filename so the local `card data` slot
  survives moving or renaming an extracted demo.
- Added matching launcher repair for older absolute card paths and verified all
  private/public PowerShell launchers parse without errors.
- Added a product-shared Windows audio format/endpoint component and a silent
  endpoint acceptance probe; the probe opens and closes the real output format
  without writing sound.
- Published product-shared XInput discovery/normalization and its no-window
  hardware probe, including the 35% partial-axis remapping contract.
- Rebuilt all presenter-dependent owners and passed the complete off-screen,
  input, audio, music, card, ELAN, Vulkan, timing, lifecycle, freshness,
  translation, and BIOS-free standalone suite.
- Re-audited and packaged the 14-file Windows x64 public early demo with no
  game data, BIOS, card saves, custom music, logs, captures, generated guest
  code, or private paths.

## v2441-static-topology-cache — 2026-09-01

- Cached immutable connector, fan, point-validity, and UV-readiness summaries
  beside exact-matched static Vulkan points and packed topology.
- Kept the established full validation/rebuild path for changed, animated,
  screen-space, diagnostic, modifier, and unsupported geometry.
- Added an explicit presenter contract test proving BGR24 and direct BGRA32
  output decode pixel-identically to the ordinary RGB diagnostic result.
- Reduced median topology preparation by 35.9% and the complete controlled
  16,384-vertex static-reuse frame by 9.6% across seven fresh-process samples.
- Passed the complete offline correctness, input, audio, card, interpolation,
  lifecycle, link-freshness, timing, and BIOS-free standalone suite.
- Passed conservative CPU-only cold-boot/movement probes for Myogi Time Attack
  (left/dry/day), Usui Time Attack (right/wet/night), and Shomaru Bunta, each
  plus Tsuchisaka Bunta, each with correct assets/conditions and cooperative
  exit.
- Corrected the private route-evidence parser to recognize the segmented
  `*_pol_a/b/c.tbl` geometry used by Shomaru while still excluding condition
  meshes from primary-course identification.
- Made the private runner compatible with retained manifests that predate the
  numeric direction column by deriving the generator's exact Left=0/Right=1
  mapping from the explicit label.
- Kept v2415 as the current downloadable public early demo; v2441 is an
  integration checkpoint pending broader live route and hardware acceptance.

## v2440-gpu-presentation — 2026-09-01

- Moved PVR depth normalization into the Vulkan vertex shader.
- Replaced CPU-staged vertex/index arrays with two bounded regions inside the
  persistent mapped Vulkan geometry buffers.
- Replaced CPU-staged ELAN projection/lighting uniforms with two aligned mapped
  regions inside the existing uniform allocation.
- Preserved exact adjacent-state reuse and fail-closed overflow handling for
  geometry, topology, and uniform streams.
- Added a native BGRA fallback contract so compatible Vulkan readback can be
  copied directly into a Win32 32-bit DIB without a per-pixel 4-to-3-byte
  conversion loop.
- Added Direct Vulkan unclean-session protection: an interrupted Direct run
  automatically falls back to the host-safe presentation path on next start.
- Passed the complete controller, music, interpolation, card, renderer,
  lifecycle, translation-freshness, and BIOS-free product suite.
- Completed a live 2560x1080 Direct Vulkan course/result run at sustained
  119.7–120.3 presentation FPS, bounded memory/handles, exit code 0, complete
  renderer/audio/input teardown, and clean crash-marker removal.
- Kept the public v2415 demo as the current downloadable prerelease while the
  v2440 integration checkpoint receives broader hardware and route coverage.

## v2415-host-safety — 2026-08-31

- Added a process-wide single-instance guard so a second recomp exits before
  creating another graphics context.
- Changed the Windows QA watchdog to request ordinary cooperative presenter
  shutdown instead of bypassing renderer teardown.
- Removed abrupt process termination from the route harness; a failed normal
  close now aborts further coverage without touching live driver work.
- Made route-only coverage default to CPU rendering at a minimal preview rate,
  with live Vulkan WSI requiring explicit opt-in.
- Passed current-build CPU-only probes for Myogi Time Attack and the Shomaru
  and Tsuchisaka Bunta routes, each with real movement and clean teardown.
- Published a re-audited 14-file public package.

## v2414-bounded-vulkan-upload — 2026-08-31

- Replaced the Vulkan startup texture-upload `vkQueueWaitIdle` call with a
  per-upload fence capped at two seconds.
- On timeout, the runtime disables Vulkan while deliberately retaining any
  potentially in-flight command, fence, staging, and image resources.
- Added a lifecycle policy that rejects queue-idle, device-idle, and infinite
  fence waits from the product backend.
- Passed the complete offscreen Vulkan and product regression suite and
  published a re-audited 14-file public package.

## v2413-steering-smoothing — 2026-08-31

- Added a saved 0–100% Steering Smoothing slider to F1 > Controls for XInput
  controllers and DirectInput wheels; 0% is an exact bypass.
- Kept steering filtering on the authentic 60 Hz cabinet input cadence so the
  setting remains independent from presentation FPS.
- Passed the complete offline controller, music, interpolation, ELAN, card,
  AICA, Vulkan, lifecycle, translation, freshness, and no-firmware suite.
- Published a re-audited 14-file public ZIP containing no game data, BIOS,
  PIC, CHD, extracted assets, cards, custom music, logs, or personal paths.

## v2412-controller-fix — 2026-08-31

- Improved controller selection, wireless reconnect discovery, and remapping
  while the F1 menu is open.
- Kept XInput and DirectInput providers explicitly labeled and independently
  mapped.
- Passed the full offline suite, linked-owner freshness, standalone product
  audit, and public-package content audit.
- Published a re-audited 14-file public ZIP containing no game data, BIOS,
  PIC, CHD, extracted assets, cards, custom music, logs, or personal paths.

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
