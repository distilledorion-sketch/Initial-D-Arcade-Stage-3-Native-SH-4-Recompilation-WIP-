# Roadmap

Work is ordered by the first unresolved native hardware and user-flow boundaries. Each milestone has a concrete acceptance gate.

## Completed native-path milestones

- Bounded ELAN `Link`/`Model` stream walking with malformed/cyclic streams rejected fail-closed.
- Known `RegisterWait` and texture-DMA events delivered at their real command boundaries.
- Accepted native boot through the frontend transition into a continuously running course/attract component.
- All 100 suppressed geometry/ELAN `PREF` sites restored as real SH-4 store-queue flushes.
- Long native ELAN trace matched to Flycast for the first 64,439 semantic append events.
- Active model/view/projection state associated with command-port draws; coherent projected single-car frames now render without fit-to-view substitution.
- VRAM-backed texture path validated against Flycast: 160/183 exact states and 86.9% of textured raster workload covered by exact-matched unique payloads.
- Native Windows preview proven across changing frames; short keyboard inputs now use a message-edge latch.
- First CPU software-raster optimization pass completed while all five private hardware regression gates remain green.

## P0 — Prove interactive cabinet flow

- Validate coin, start, service, steering, accelerator, brake, and view-change input on a real focused native window.
- Trace JVS state through the translated game instead of treating a host key transition as sufficient evidence.
- Reach a menu/race transition from live user input with no replay or scripted-input dependency.

Acceptance: a user can insert credit, start, select, and control a live native sequence while the projected renderer continues updating.

## P0 — Execute and composite every TA/PVR layer

- Execute opaque, punch-through, translucent, modifier-volume, and sprite lists at their authentic frame boundaries.
- Complete 2D title, HUD, cabinet, and Japanese overlay compositing over ELAN 3D.
- Preserve guest ordering, blend/depth behavior, and interrupt timing.

Acceptance: all expected layers come from guest-submitted data with no fabricated geometry, textures, or overlays.

## P0 — Reach authentic audio start

- Trace the translated path that should emit the AICA SGC stream-start command.
- Expand native SGC actions only from observed/proven command semantics.
- Add a host audio sink after nonzero native samples exist.

Acceptance: synchronized audible output is produced from the game's own AICA RAM/commands; a silent device callback is not accepted as progress.

## P1 — Finish materials and renderer optimization

- Expand reference coverage for the remaining unmatched texture payloads and validate depth, culling, blend, transparency, palette, mip, fog, modifier-volume, and filtering behavior.
- Investigate only the 10 invalid/non-triangle submissions if later evidence shows they should contain drawable data.
- Profile and optimize hot paths without replacing game state or weakening correctness checks.

Acceptance: repeatable native/Flycast comparisons cover the reached scenes, and optimized frames preserve pixel/semantic regression gates.

## P1 — Whole-game static-AOT coverage

- Translate every reachable function and indirect table coherently.
- Eliminate unresolved targets and compatibility-only shims.
- Add race, course, results, continue, save/configuration, and long-run determinism suites.

Acceptance: cold boot through a complete race and return flow entirely through the native static path.

## P2 — Productization

- Reproducible builds that accept only user-supplied legally owned inputs.
- Clear extraction boundary, configuration UI, controller mapping, crash diagnostics, and release packaging.
- Clean-machine validation without private development paths or snapshots.

Acceptance: a user can build and run from legally owned inputs without any redistributed Sega/game content.
