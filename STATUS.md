# Status and evidence ledger

Status date: 2026-08-31
Integration checkpoint: v2414 WIP
Public checkpoint: v2414 Windows x64 early-demo prerelease

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
| Performance | Authentic 60 Hz guest timing with distinct 120 Hz presentation on measured routes; the four-course priority matrix held 119.8–120.0 FPS minimum with zero accepted moving-race repeats | 120 Hz is experimental; exhaustive content coverage and uncapped presentation are not complete |
| Host shutdown | Cooperative Vulkan stop request, raster-worker exit handshake, message pumping, join-before-window-destroy order, driver-operation diagnostics, and bounded acquire/fence/startup-upload waits pass source/offscreen checks; no unbounded Vulkan idle/fence call remains | Live Win32 WSI stress across GPUs/drivers remains pending and is not replaced by offscreen evidence |
| Distribution | A BIOS-free, Python-free Windows x64 public early demo accepts only matching user-owned inputs and performs verification/extraction locally | Clean-machine coverage is limited and the release remains unfinished |

## Strongest accepted evidence

- v2414 native executable SHA-256: `68F3D8501E1BE58EC07B167E1703703260B3523D47CA545B786ABAAB1FD53D0B`.
- Public ZIP SHA-256: `826780D48ECAF3411E12F2EBD91EA0989EC66145AF1C692546BC36955C68A90F`; GitHub reports the same uploaded-asset digest and 17,768,925-byte size.
- The exact public ZIP was audited as 14 files with no CHD, PIC, BIOS, extracted HOSTFS, card save, custom song, user log, or personal filesystem path.
- v2414 linked 116 objects, passed freshness checks for 108 source owners, and passed 13 controller/music plus 12 lifecycle policy tests, controller-binding/smoothing, exact custom-music, interpolation, ELAN/card/AICA/offscreen Vulkan, card-eject, translation, freshness, and no-firmware checks.
- Current coverage is 48/48 route/branch loads, 32/32 rival movement, 16/16 natural Time Attack results, and 32/32 natural rival results. The separate 70-row Time Attack/Bunta load-and-movement ledger mixes checkpoints and is not a full same-build v2414 completion matrix.

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
longer the active frontier. The v2414 native build advances targeted
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

The current priority frontier is broad physical-input acceptance, conservative host
shutdown validation, whole-game same-build coverage, remaining visual/audio
defects, reducing synchronous transition latency, broader clean-machine
packaging, and eventually uncapped presentation that stays independent from
guest gameplay timing. The measured high-refresh routes do not make 120 Hz a
whole-game validated mode.

## Definition of release-ready

The downloadable build is an early playtest, not a finished release. The
project will not call itself release-ready until users can cold boot from
legally owned inputs, reach and control every supported mode with working
audio/video/input, complete meaningful gameplay and persistence branches, and
do so repeatedly through the static native path without an interpreter/JIT
fallback or host-level failure.
