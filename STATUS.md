# Status and evidence ledger

Status date: 2026-08-18
Public checkpoint: v2018 WIP

This file separates demonstrated behavior from hypotheses. Percentages are deliberately avoided: a technically advanced attract-mode path is not the same thing as a playable game.

| Area | Demonstrated | Current limitation |
|---|---|---|
| Static SH-4 translation | Broad instruction decoding/code generation; direct translated calls; FPSCR-aware FPU regression tests | Whole-program coverage and every indirect target are not complete |
| Native boot/runtime | Static translated execution cold-boots and advances sustained native attract-mode scenes | A complete menu-to-race-to-results flow is not yet validated |
| Deterministic replay | N70 accepted 70/70 with `unimplemented=0`, `FPSCR=0`, no fault/watchdog, and cross-machine identical frame hash | Uses private user-owned replay/input state that cannot be published |
| ELAN 3D path | Submitted links, CH2 DMA, completion interrupts, persistent state, materials, instances, lighting, textures, culling, depth, and retained presentation scenes advance continuously | Whole-intro and gameplay correctness still need systematic comparison |
| PVR rendering | Native handling exists for observed opaque/translucent lists, fog, modifier volumes, punch alpha, blend modes, texture layouts, autosort, and tile clipping | Remaining overlays and rejected/projected geometry need scene-by-scene validation |
| Course environment maps | Missing logical lookup paths were recovered from two exact user-owned ISO files; rainbow/static maps are corrected | Other courses and weather conditions are not yet audited |
| JVS/input | 837-13551 identity/features, EEPROM, Maple VBlank/reset, and keyboard-facing seams pass focused tests | Steering, pedals, gears, buttons, coin/service, and full race control remain to be proven together |
| Audio | Flycast-derived AICA SGC and mailbox protocol tests pass; the exact driver can be loaded from the private HOSTFS | Complete music, engine, voice, effects, looping, and synchronization remain incomplete |
| Performance | A coherent sequence survives a 60 FPS presentation target | The diagnostic renderer does not yet sustain 60 unique frames per second |
| Distribution | Sanitized source/evidence package and user-owned-input preparation tool | No executable game package; private inputs and generated game code remain excluded |

## Strongest accepted evidence

- N70 frame SHA-256: `34D6B91C0550A5CC2A60D4B1F8930812908CEA6DCD6C354749A0881BE426D9E2`.
- Accepted N70 state: 667 batches, 61,498 vertices, 17,490 triangles, 990,196 textured pixels, and 53,996 lit vertices.
- Full-intro ladder reached N3200 with each recorded run accepted N/N, `unimplemented=0`, `FPSCR=0`, and no recorded memory fault, crash, or watchdog.
- Current public classifier smoke test covers recursive links, SH-4/ERAM texture DMA links, all five known list-complete wait bits, empty/unknown-mask rejection, and false command-header aliases.

## Latest WIP truth

The earlier linked-stream/frame-lifecycle blocker is no longer the active
frontier. The current native run advances thousands of guest scene frames and
presents retained Naomi 2 scenes continuously.

The latest visual root cause was external asset preparation: two logical course
lookup names were absent because their ISO9660 physical names are truncated.
The null table pointer made the guest compositor read unrelated low-memory bytes
as selectors. Restoring the two exact files from the user's own dump changed
the invalid selector record to the expected course record and removed the
alternating rainbow/static environment maps. This does not imply that all
graphics or gameplay are complete.

## Definition of playable

This project will not call itself playable until a user can cold boot from legally owned inputs, reach menus, start and control a race, receive working audio/video/input, complete meaningful gameplay, and do so through the static native path without an interpreter/JIT fallback.
