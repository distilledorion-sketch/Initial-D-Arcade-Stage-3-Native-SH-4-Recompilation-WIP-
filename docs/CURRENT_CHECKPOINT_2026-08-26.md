# Current source-safe checkpoint — 2026-08-26

This document reports the private v2216 integration checkpoint without
publishing a game binary, generated game translation, ROM/BIOS/PIC/CHD data,
assets, captures, logs, save/card data, memory snapshots, or private host paths.

## Project boundary

- Target hardware: Sega NAOMI 2.
- Execution model: native static ahead-of-time SH-4 recompilation.
- Graphics API: Vulkan.
- No interpreter, JIT, emulator runtime, fabricated guest state, or replacement game assets.
- Flycast-derived device knowledge/code may be integrated where licensing permits; Flycast is not the runtime.
- Dreamcast is outside the project scope.

## Demonstrated in the private integration build

- Cold boot and targeted menu, loading, live-race, result, and save transitions.
- Player-car movement through normal JVS inputs and game physics rather than teleporting or substituting rendered frames.
- Corrected tested HUD projection, top mirror placement, RX-7 visibility/texture mapping, retained scenes, modifier volumes, fog, blending, and texture paths.
- Sustained AICA/ARM7 audio production and tested race-stream selection. Automated QA is deliberately muted through a null sink.
- Keyboard/analog input, digital-shifter routes, no-card operation, and isolated disposable-card insert/load/save/reload.
- F1 configuration infrastructure for presentation and input work, with physical wheel/force-feedback acceptance still open.

## Performance checkpoint

Checkpoint executable SHA-256 (binary not distributed):

`BE61B4291784A4A98129677FF7B7991BB982346CE1F5550C72547BF6C2FFCC16`

At 3840×2160, the player car advanced 160 metres through normal JVS/physics
input. Once live motion began, sampled presentation measured 59.9–60.2 FPS.
The first two monitoring intervals included synchronous guest course/music
loading before the car moved and are tracked as transition gaps rather than
steady renderer overload.

At 1920×1080, eight sampled intervals from takeover through 160 metres measured
119.8–120.1 FPS. This uses presentation interpolation over authentic guest
timing; it does not run game logic or physics at double speed.

The renderer now keeps persistent preparation workers instead of creating and
joining worker threads every frame. Timing probes also identified seven
race-only Vulkan pipeline variants. Moving those variants to startup reduced
the two heaviest measured entry frames from 20.97/19.01 ms to 11.36/5.69 ms,
with zero pipeline creation during those active frames.

The final acceptance runs recorded no fatal, exception, access-violation,
unimplemented-target, NSEQ, or Vulkan fault markers.

## Remaining work

1. Validate every mode, course, car, opponent, condition, outcome, continue/result path, and card-error branch.
2. Remove or mask the two synchronous pre-race asset/music transition gaps without changing guest-visible ordering.
3. Complete hardware acceptance for wheels, force feedback, audio devices, and controller remapping.
4. Finish clean-machine preparation, crash reporting, settings polish, and legally safe release packaging.
5. Protect 120 FPS across broader content, then pursue unlimited presentation without changing gameplay, timers, physics, or audio.

This is substantial progress, not a completion claim. The public repository
remains a source-safe engineering showcase rather than a playable game release.
