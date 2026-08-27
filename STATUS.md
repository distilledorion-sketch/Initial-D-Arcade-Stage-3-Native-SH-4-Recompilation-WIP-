# Status and evidence ledger

Status date: 2026-08-26
Private integration checkpoint: v2216 WIP
Public checkpoint: source-safe progress report (non-playable)

This file separates demonstrated behavior from hypotheses. Percentages are deliberately avoided: a technically advanced attract-mode path is not the same thing as a playable game.

| Area | Demonstrated | Current limitation |
|---|---|---|
| Static SH-4 translation | Broad instruction decoding/code generation; direct translated calls; FPSCR-aware FPU regression tests | Whole-program coverage and every indirect target are not complete |
| Native boot/runtime | Static translated execution cold-boots and advances tested menu, loading, live-race, result, and save paths | Whole-game route and error-branch coverage is not complete |
| Deterministic replay | N70 accepted 70/70 with `unimplemented=0`, `FPSCR=0`, no fault/watchdog, and cross-machine identical frame hash | Uses private user-owned replay/input state that cannot be published |
| ELAN 3D path | Submitted links, CH2 DMA, completion interrupts, persistent state, materials, instances, lighting, textures, culling, depth, and retained presentation scenes advance continuously through tested races | Untested cars, courses, weather, and scene combinations still need systematic comparison |
| PVR rendering | Native handling exists for observed opaque/translucent lists, fog, modifier volumes, punch alpha, blend modes, texture layouts, autosort, tile clipping, tested HUD placement, and mirrors | Remaining combinations and presentation edge cases need scene-by-scene validation |
| Course environment maps | Missing logical lookup paths were recovered from two exact user-owned ISO files; rainbow/static maps are corrected | Other courses and weather conditions are not yet audited |
| JVS/input | 837-13551 identity/features, EEPROM, Maple VBlank/reset, keyboard routes, analog driving input, and digital shifter routes pass focused tests | Physical wheel, force-feedback strength, and full cabinet hardware validation remain open |
| Audio | Flycast-derived AICA SGC/mailbox tests pass; the ARM7/AICA path produces sustained non-silent samples and selects tested race music | Audible end-user acceptance and exhaustive music/voice/effect coverage remain open |
| Card | No-card operation works; a disposable 207-byte card passes insert/load/save/reload on a tested Legend flow | Real user card data is never used for QA; damaged-card and remaining error branches need coverage |
| Performance | Sustained race motion measures 59.9–60.2 FPS at 3840×2160 and 119.8–120.1 FPS at 1920×1080 without changing guest timing | Two pre-motion transition intervals still include synchronous guest asset/music loads; uncapped presentation is not complete |
| Distribution | Sanitized source/evidence package and user-owned-input preparation tool | No executable game package; private inputs and generated game code remain excluded |

## Strongest accepted evidence

- v2216 Windows x64 checkpoint SHA-256: `BE61B4291784A4A98129677FF7B7991BB982346CE1F5550C72547BF6C2FFCC16` (binary intentionally not published).
- At 3840×2160, genuine JVS/physics-driven race motion advanced 160 m while sampled presentation held 59.9–60.2 FPS.
- At 1920×1080, all eight sampled race intervals from takeover through 160 m measured 119.8–120.1 FPS using presentation interpolation over authentic guest timing.
- Vulkan prewarming moved seven observed race-only pipelines to startup. The two heaviest entry frames dropped from 20.97/19.01 ms to 11.36/5.69 ms and created zero pipelines in-frame.
- The final acceptance runs recorded no fatal, exception, access-violation, unimplemented-target, NSEQ, or Vulkan fault markers.
- N70 frame SHA-256: `34D6B91C0550A5CC2A60D4B1F8930812908CEA6DCD6C354749A0881BE426D9E2`.
- Accepted N70 state: 667 batches, 61,498 vertices, 17,490 triangles, 990,196 textured pixels, and 53,996 lit vertices.
- Full-intro ladder reached N3200 with each recorded run accepted N/N, `unimplemented=0`, `FPSCR=0`, and no recorded memory fault, crash, or watchdog.
- Current public classifier smoke test covers recursive links, SH-4/ERAM texture DMA links, all five known list-complete wait bits, empty/unknown-mask rejection, and false command-header aliases.

## Latest WIP truth

The earlier linked-stream/frame-lifecycle and attract-mode-only blockers are no
longer the active frontier. The current private native build advances targeted
menu-to-race and result/save paths, presents retained NAOMI 2 scenes
continuously, and can drive the player car through normal game inputs.

The latest visual root cause was external asset preparation: two logical course
lookup names were absent because their ISO9660 physical names are truncated.
The null table pointer made the guest compositor read unrelated low-memory bytes
as selectors. Restoring the two exact files from the user's own dump changed
the invalid selector record to the expected course record and removed the
alternating rainbow/static environment maps. Subsequent integration work also
corrected the tested HUD projection, mirror placement, RX-7 geometry/texture
failures, and renderer lifecycle defects. This does not imply that all graphics
or gameplay paths are complete.

The current performance frontier is no longer steady-state 60 FPS. It is broad
content coverage, two synchronous transition-load gaps, hardware input/audio
acceptance, release packaging, and eventually uncapped presentation that stays
independent from guest gameplay timing.

## Definition of playable

This project will not call itself playable until a user can cold boot from legally owned inputs, reach menus, start and control a race, receive working audio/video/input, complete meaningful gameplay, and do so through the static native path without an interpreter/JIT fallback.
