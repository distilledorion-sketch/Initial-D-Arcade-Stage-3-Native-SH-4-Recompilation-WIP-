# Status and evidence ledger

Status date: 2026-08-18

Public checkpoint: v1826 WIP

This file separates demonstrated behavior from hypotheses. A single overall percentage is deliberately avoided: a technically advanced native boot/attract and projected graphics path is not the same thing as a playable product.

| Area | Demonstrated | Current limitation |
|---|---|---|
| Static SH-4 translation | Direct ahead-of-time translated calls on the validated path; FPSCR-aware FPU regression tests | Whole-program coverage and every indirect target are not complete |
| Native boot | Accepted trace reaches the frontend and course/attract sequence with no interpreter/JIT fallback | Menus, controllable races, results, and long-run coverage are incomplete |
| Store queues | All 100 previously suppressed `PREF` operations in 14 geometry/ELAN functions now perform real 32-byte store-queue flushes; the private generated unit contains 531 active `PREF` calls | Unvisited translated paths can still expose semantic gaps |
| ELAN control path | Native append semantics match the Flycast reference for the first 64,439 events; bounded `Link`/`Model` traversal remains fail-closed | Broader paths and unsupported records still need coverage |
| Projected framebuffer | Recent frame: 751 accepted batches, 63,356 vertices, 12,393 triangles; zero projection or near/far-cull rejection; all 307,200 output pixels have a real game-batch owner | This is a software validation renderer, not complete final graphics presentation |
| Texture path | 160/183 states exactly match a Flycast payload, dimensions, format, and nonzero count; 149/172 unique decoded bindings cover 86.9% of textured raster work | The remaining unmatched captures require reference coverage and material validation |
| Geometry integrity | The store-queue fix removed the duplicated/two-cars-in-one render; the 10 remaining rejected submissions have too few/invalid vertices to form triangles | Long-run scene and course coverage remains incomplete |
| Native preview | 45-second run produced 282 changing raster frames and 2,384 input polls | Software rasterization is still expensive and presentation layers are incomplete |
| JVS/input | Clean-room cabinet boundary plus Windows key-edge latching for short coin/start/service presses | Real interactive start/control validation is still pending |
| Audio | Native Flycast-derived SGC tests pass and AICA RAM descriptors initialize | Reached state has not emitted the real stream-start command; there is no useful host sink yet |
| Distribution | Sanitized source/evidence repository with public tests | No game executable or copyrighted input package |

## Latest accepted sequence

1. The Flycast-derived ELAN grammar and direct source adaptation remain limited to the Naomi 2 hardware seam; no Flycast CPU, dynarec, interpreter, UI, or save-state engine is used.
2. Restoring the translated SH-4 store-queue flushes fixed corrupted/duplicated geometry at its source.
3. The native ELAN decoder matched a Flycast snapshot and then matched the first 64,439 semantic append events of a long active trace.
4. Projection/model/view state now attaches to the submitted command-port batches, producing a coherent single-car scene without a synthetic fit-to-view camera.
5. VRAM-backed texture comparison found 160/183 exact native states. The dominant unmatched textures decode coherently as game environment maps rather than random/corrupt memory.
6. Bilinear sampling, common blend, color rounding, alpha interpolation, and offset-color hot paths were optimized without changing intended output semantics.
7. The optional Windows preview showed changing course/attract scenes. A message-edge latch was added because a roughly 150 ms software frame can otherwise miss short cabinet-key taps.

## Strongest current evidence

- Long ELAN parity: first 64,439 semantic append events match Flycast exactly.
- Store queues: 531 active translated `PREF` calls, with all 100 stale geometry/ELAN no-ops restored.
- Projected frame: 751 accepted batches, 63,356 vertices, 12,393 triangles, zero projection rejection.
- Texture parity: 160/183 exact states; 86.9% of textured raster work covered by exact-matched unique payloads.
- Frame ownership: 307,200/307,200 pixels attributed to submitted game batches.
- Representative optimization: total 180.786 ms to 150.522 ms; raster 167.640 ms to 139.445 ms, with slightly different triangle workloads.
- Private regression gates: AICA mailbox, Flycast-derived AICA SGC, system-ROM read-only, native ELAN bridge, and JVS tests all pass.

## Immediate blocker

The next user-visible gate is a real interactive run that turns coin/start/control input into menu or race progression while preserving the projected renderer. In parallel, the first authentic AICA stream-start command must be reached before a host audio sink can produce useful sound.

## Definition of playable

The project will not call itself playable until a user can cold boot from legally owned inputs, reach menus, start and control a race, receive working audio/video/input, complete meaningful gameplay, and do so through the static native path without an interpreter/JIT fallback.
