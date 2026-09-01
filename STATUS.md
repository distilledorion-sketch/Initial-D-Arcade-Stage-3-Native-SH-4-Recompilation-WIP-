# Status and evidence ledger

Status date: 2026-09-01
Integration checkpoint: v2441 WIP
Public checkpoint: v2415 Windows x64 early-demo prerelease

This file separates demonstrated behavior from hypotheses. Percentages are deliberately avoided: a technically advanced attract-mode path is not the same thing as a playable game.

| Area | Demonstrated | Current limitation |
|---|---|---|
| Static SH-4 translation | Broad instruction decoding/code generation; direct translated calls; FPSCR-aware FPU regression tests | Whole-program coverage and every indirect target are not complete |
| Native boot/runtime | BIOS-free static translated execution cold-boots and advances tested menu, loading, live-race, result, and save paths; 48/48 route/branch targets load | Remaining long campaign permutations and error branches are not complete |
| Deterministic replay | N70 accepted 70/70 with `unimplemented=0`, `FPSCR=0`, no fault/watchdog, and cross-machine identical frame hash | Uses private user-owned replay/input state that cannot be published |
| ELAN 3D path | Submitted links, CH2 DMA, completion interrupts, persistent state, materials, instances, lighting, textures, culling, depth, and retained presentation scenes advance continuously through tested races | Untested cars, courses, weather, and scene combinations still need systematic comparison |
| PVR rendering | Native handling exists for observed opaque/translucent lists, fog, modifier volumes, punch alpha, blend modes, texture layouts, autosort, tile clipping, tested HUD placement, and mirrors | Remaining combinations and presentation edge cases need scene-by-scene validation |
| Course environment maps | Missing logical lookup paths were recovered from two exact user-owned ISO files; rainbow/static maps are corrected | Other courses and weather conditions are not yet audited |
| JVS/input | 837-13551 identity/features, EEPROM, Maple VBlank/reset, keyboard routes, analog driving input, digital shifter routes, XInput-first device selection, hotplug rescan, paused-menu remapping, adjustable FFB, and saved 0–100% steering smoothing pass focused tests | Physical controller, wheel, and FFB acceptance across hardware remains open |
| Audio | Flycast-derived AICA SGC/mailbox tests pass; the ARM7/AICA path produces sustained non-silent samples; 13 custom music mappings follow authentic stream commands | Audible end-user acceptance and exhaustive music/voice/effect coverage remain open |
| Card | No-card operation works; a disposable 207-byte card passes insert/load/save/reload on a tested Legend flow | Real user card data is never used for QA; damaged-card and remaining error branches need coverage |
| Performance | Authentic 60 Hz guest timing with distinct 120 Hz presentation on measured routes; GPU projection, ELAN lighting, depth normalization, mapped geometry/uniform streams, static topology summaries, and Direct Vulkan display are active. The v2440 live course/result run held 119.7–120.3 FPS with distinct intermediate motion; v2441 reduced the controlled warmed static-reuse frame from 0.696 ms to 0.629 ms median | 120 Hz is experimental; exhaustive content coverage and uncapped presentation are not complete |
| Host shutdown | Cooperative Vulkan stop request, raster-worker exit handshake, message pumping, join-before-window-destroy order, bounded acquire/fence/startup-upload waits, cooperative QA watchdog, single-instance enforcement, and Direct-session crash lock pass source/offscreen checks. One sustained v2440 live Direct run completed exit code 0 and removed its session marker | Repeated live close/restart stress across more GPUs/drivers remains open |
| Distribution | A BIOS-free, Python-free Windows x64 public early demo accepts only matching user-owned inputs and performs verification/extraction locally | Clean-machine coverage is limited and the release remains unfinished |

## Strongest accepted evidence

- v2441 native executable SHA-256: `78FFA018D058744A5AA5AAF5F5580C09F313CB8F5725A744AD0BFBCD6E30C6DD` (binary intentionally not published).
- Seven fresh-process samples of the warmed 16,384-vertex reuse case reduced
  median batch-loop time from 0.089 ms to 0.066 ms, topology time from 0.064 ms
  to 0.041 ms, and complete synthetic frame time from 0.696 ms to 0.629 ms.
- v2441 passed the complete v2440 offline suite plus an explicit BGR24/BGRA32
  presenter channel-accuracy contract. Its standalone audit remains at zero
  firmware callbacks, AOT objects, input contracts, and cached translations.
- v2441 CPU-only cold-boot probes passed Myogi left/dry/day Time Attack with
  9.548 m of player movement and Shomaru dry/night Bunta with 5.097 m. Both
  identified the expected course/condition/environment assets and exited 0
  through cooperative close without creating a Vulkan WSI surface.
- A complementary Usui right/wet/night Time Attack probe loaded the expected
  course, reverse condition geometry, direction identity `1,1,1`, night
  environment, and rain; it advanced 9.008 m and exited cooperatively with 0.
- Tsuchisaka dry/night Bunta likewise loaded `k_tu2`, its forward condition
  geometry, night/no-rain environment, and the Bunta rival package; it advanced
  6.302 m and exited cooperatively with 0.
- v2440 native executable SHA-256: `7FB61B45A0137F8FCE4A9A6CD36200B212F7DFDC6717D851739AD9CBB4D798BC` (binary intentionally not published).
- The complete offline suite passed CPU/Vulkan pixel comparison, topology,
  projection, ELAN lighting, environment mapping, homogeneous near clipping,
  16,384-static-vertex reuse, controllers, custom music, interpolation, cards,
  Direct-session marker lifecycle, 31 shutdown/presentation policies, 13
  controller/music policies, guest timing, link freshness, and the no-firmware
  standalone audit.
- A live 2560x1080 RX 9070 XT Direct Vulkan run sustained 119.7–120.3 FPS
  across roughly 75,000–127,000 vertices per authentic course frame. During
  moving content the generated-frame count advanced at the required cadence
  and the repeated-frame counter stayed effectively flat.
- The v2440 process crossed the result/continue transition, stabilized near a
  1.34 GiB working set with zero handle growth in the steady sample, exited 0,
  completed renderer/audio/DirectInput/window teardown, and removed its
  Direct-session marker.

- v2415 native executable SHA-256: `3185FF67F00EE5E8EF17E63F5B10ECDF66494B3B604358ED7DA863C7AB56B8D4`.
- Public ZIP SHA-256: `8CA9D017EA5BCFE8EB58245D162D89A22D75267BC6D3BABAD76D922E283B2E8E`; local audited size is 17,877,033 bytes.
- The exact public ZIP was audited as 14 files with no CHD, PIC, BIOS, extracted HOSTFS, card save, custom song, user log, or personal filesystem path.
- v2415 linked 116 objects, passed freshness checks for 108 source owners, and passed 13 controller/music plus 16 lifecycle policy tests, controller-binding/smoothing, exact custom-music, interpolation, ELAN/card/AICA/offscreen Vulkan, card-eject, translation, freshness, and no-firmware checks.
- v2415 CPU-only same-build probes passed Myogi left/dry/day Time Attack and the Shomaru and Tsuchisaka Bunta routes with correct course/time/weather assets, real motion, zero fault markers, one process, exit code 0, and complete shutdown markers.
- Current broader coverage is 48/48 route/branch loads, 32/32 rival movement, 16/16 natural Time Attack results, and 32/32 natural rival results. The separate 70-row Time Attack/Bunta load-and-movement ledger mixes checkpoints and is not a full same-build v2415 completion matrix.

- v2396 Windows x64 checkpoint SHA-256: `966F218E54CC2C3A112D174163C85B141DDA25FE96836729D310E8773C555E91` (binary intentionally not published).
- One BIOS-free 640×480/60 bounded route proved the opponent-menu label, stream override, Media Foundation decode, and AICA replacement activation in a single run while Vulkan held 60.0 Hz and live movement advanced.
- The build verifier now rejects a translated COMDAT owner that predates the inline host headers it owns. The historical stale build fails with the exact owner name; v2396 passes fresh.
- The v2396 checkpoint closed 48/48 route/branch loads, 32/32 rival movement, 16/16 natural Time Trial results, and 32/32 natural rival results.
- The accepted four-course high-refresh matrix measured 119.8–120.0 FPS minimum presentation and no repeated endpoints in accepted moving intervals.

- v2374 Windows x64 checkpoint SHA-256: `81F18BBEA04D42B692BE295630A3F6A6DE8DF6F479ECF4881E38FCAC100FD0C4` (binary intentionally not published).
- On the identical resolved-input/card `k_ez` route, v2373 measured 100.0 minimum / 116.609 average generated FPS across 22 full race samples. The exact main-RAM object-clear repair raised v2374 to 120.0 minimum / 120.0 average across 23 full samples.
- The moving-race repeat-counter delta was zero in both runs; v2374's 120 FPS result is distinct interpolated motion, not repeated 60 Hz endpoints.
- The exact linked memset regression passes zero, one-byte, unaligned 257-byte, and P1-alias 64 KiB cases with preserved registers, T flag, stack, guard bytes, and `length + 2` memory-access cycles.
- The standalone audit reports zero firmware callbacks, firmware AOT objects, firmware input contracts, or cached firmware translations in v2374.

- v2217 Windows x64 checkpoint SHA-256: `0F225655BF9F32C1FAEE76CF559EABD1D7D5C19A4960BD7A02B512582A4A08E6` (binary intentionally not published).
- At 3840×2160, 43 samples from takeover onward measured 59.8–60.1 FPS while genuine JVS/physics-driven race motion advanced 668.687 m.
- The v2216 29.3/51.2 transition samples were traced to the 60 Hz presenter waiting for new guest scenes during synchronous loads. v2217 retains the last completed image at fixed wall-clock cadence without advancing guest code.
- At 1920×1080, all eight sampled race intervals from takeover through 160 m measured 119.8–120.1 FPS using presentation interpolation over authentic guest timing.
- Vulkan prewarming moved seven observed race-only pipelines to startup. The two heaviest entry frames dropped from 20.97/19.01 ms to 11.36/5.69 ms and created zero pipelines in-frame.
- The final acceptance runs recorded no fatal, exception, access-violation, unimplemented-target, NSEQ, or Vulkan fault markers.
- N70 frame SHA-256: `34D6B91C0550A5CC2A60D4B1F8930812908CEA6DCD6C354749A0881BE426D9E2`.
- Accepted N70 state: 667 batches, 61,498 vertices, 17,490 triangles, 990,196 textured pixels, and 53,996 lit vertices.
- Full-intro ladder reached N3200 with each recorded run accepted N/N, `unimplemented=0`, `FPSCR=0`, and no recorded memory fault, crash, or watchdog.
- Current public classifier smoke test covers recursive links, SH-4/ERAM texture DMA links, all five known list-complete wait bits, empty/unknown-mask rejection, and false command-header aliases.

## Latest WIP truth

The earlier linked-stream/frame-lifecycle and attract-mode-only blockers are no
longer the active frontier. The v2415 native build advances targeted
menu-to-race and result/save paths, presents retained NAOMI 2 scenes
continuously, and is available as a public early-demo prerelease that prepares
data from matching user-owned inputs without a BIOS or Python installation.

The latest visual root cause was external asset preparation: two logical course
lookup names were absent because their ISO9660 physical names are truncated.
The null table pointer made the guest compositor read unrelated low-memory bytes
as selectors. Restoring the two exact files from the user's own dump changed
the invalid selector record to the expected course record and removed the
alternating rainbow/static environment maps. Subsequent integration work also
corrected the tested HUD projection, mirror placement, RX-7 geometry/texture
failures, and renderer lifecycle defects. This does not imply that all graphics
or gameplay paths are complete.

The current priority frontier is reducing remaining CPU command preparation,
broad physical-input acceptance, repeated cross-driver shutdown validation,
whole-game same-build coverage, remaining visual/audio defects, synchronous
transition latency, broader clean-machine packaging, and eventually uncapped
presentation that stays independent from guest gameplay timing. The measured
high-refresh routes do not make 120 Hz a whole-game validated mode.

## Definition of release-ready

The downloadable build is an early playtest, not a finished release. The
project will not call itself release-ready until users can cold boot from
legally owned inputs, reach and control every supported mode with working
audio/video/input, complete meaningful gameplay and persistence branches, and
do so repeatedly through the static native path without an interpreter/JIT
fallback or host-level failure.
