# Status and evidence ledger

Status date: 2026-08-16

Public checkpoint: v1400 WIP

This file separates demonstrated behavior from hypotheses. Percentages are deliberately avoided: a technically advanced native boot/attract path is not the same thing as a playable game.

| Area | Demonstrated | Current limitation |
|---|---|---|
| Static SH-4 translation | Direct ahead-of-time translated calls on the validated path; FPSCR-aware FPU regression tests | Whole-program coverage and every indirect target are not complete |
| Native boot | Accepted trace reached the frontend and course/attract sequence with no interpreter/JIT fallback | Menus, races, results, and long-run coverage are incomplete |
| ELAN control path | 1,566 observed/handled submissions, zero rejected, zero walk failures; bounded Link/Model traversal and known wait/DMA events | Unsupported records remain fail-closed and require implementation, not bypasses |
| Credit/frontend assets | Private owner-supplied files produce 25 records / 2,000-byte allocation on both constructions; heap overwrite disappears | Assets are legally sensitive and intentionally absent from the public repository |
| Course/attract component | Timed transition completes; active component returned cleanly through 745 sampled calls in 20 seconds | A running scene is not yet controllable gameplay |
| Geometry/textures | Post-fix SQ capture: 41,515 vertices, 29,389 triangles, 208 textured batches, 44 decoded textures | Camera/projection association is missing for the active course scene |
| Full-scene renderer | v1400: 620 accepted batches, 48,686 vertices, 33,810 triangles | `projection=0`, zero projected batches, and 21 rejected batches; output is raw diagnostic geometry |
| Deterministic intro replay | Cross-machine frame SHA-256 matched; earlier N70 accepted 70/70 with no untranslated calls or FPSCR failures | Relies on private user-owned replay/input state that cannot be published |
| JVS/input | Clean-room 837-13551 / 315-6149-facing model exists | Complete cabinet controls and full gameplay validation remain |
| Audio | Narrow opt-in AICA bootstrap seam exists | No complete native AICA/ARM audio path |
| Distribution | Sanitized source/evidence repository with public tests | No game executable or copyrighted input package |

## Latest accepted sequence

1. The Flycast-derived, stripped ELAN command grammar was integrated without importing Flycast's CPU, dynarec, renderer, scheduler, UI, or save-state systems.
2. Native boot reported `enabled=1 observed=1566 handled=1566 rejected=0` and `walk_failures=0`.
3. The second frontend construction was found to request zero credit records because four files were absent from the private HOSTFS overlay.
4. Extracting those files from the user's own disc image into the external private overlay produced 25 records and a 2,000-byte allocation on both passes. No asset is stored in this repository.
5. The course/path allocator then succeeded, the timed state-4 transition completed, and the active course component continued returning normally.
6. The native framebuffer captured substantial course-scene geometry, but its scene-selection path still reported no projection association.

## Strongest current evidence

- Native ELAN: 1,566 handled of 1,566 observed; zero rejected; zero walk failures.
- Post-credit-fix capture: 41,515 vertices, 29,389 triangles, 208 textured batches, 44 decoded textures.
- Full-scene v1400 capture: 620 accepted batches, 48,686 vertices, 33,810 triangles, 21 rejected batches, `projection=0`.
- Active course object: 745 completed sampled calls over 20 seconds with no primary-heap store-queue regression.
- Earlier N70 frame SHA-256: `34D6B91C0550A5CC2A60D4B1F8930812908CEA6DCD6C354749A0881BE426D9E2`.

## Immediate blocker

The next task is associating the active scene's ELAN model/view/projection state with its draw batches. Until that is correct, the renderer can prove that real scene geometry exists but cannot present it from the game's intended camera.

## Definition of playable

The project will not call itself playable until a user can cold boot from legally owned inputs, reach menus, start and control a race, receive working audio/video/input, complete meaningful gameplay, and do so through the static native path without an interpreter/JIT fallback.
