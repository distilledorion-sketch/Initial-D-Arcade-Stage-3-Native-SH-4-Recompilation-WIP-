# Roadmap

Work is ordered by the first unresolved hardware boundary. Each milestone has a concrete acceptance gate.

## P0 — Reach and control the first complete race

- Complete the intro, menu, course-loading, race, results, and return transitions.
- Remove any remaining static-AOT target gaps, stalls, or device-handshake blockers on that path.
- Prove steering, accelerator, brake, gears, buttons, coin/service, and race timers together.

Acceptance: a user can cold boot from legally owned inputs, enter a race, drive,
finish or exit, and return through the native static path without an
interpreter/JIT fallback.

## P0 — Validate the complete rendered sequence

- Capture bounded intro/menu/race sequences from the recomp and reference implementation.
- Locate the first divergent frame rather than judging isolated screenshots.
- Fix texture mapping, projection/clipping, depth/order, transparency, shadows, fog, overlays, and camera state from guest-submitted data.
- Audit other cars, courses, weather, and day/night conditions.

Acceptance: repeatable sequence comparisons show coherent geometry and materials
without duplicated objects, fabricated textures, or manual replacement frames.

## P1 — Complete timing, audio, and controls

- Keep guest timing independent from the host presentation cap.
- Complete synchronized music, engine, voice, effect, and looping behavior.
- Validate JVS analog ranges, dead zones, gears, buttons, coin/service, and outputs.

Acceptance: gameplay behaves consistently at different host presentation targets
with synchronized sound and complete cabinet controls.

## P1 — Sustain the 60 FPS presentation target

- Profile rasterization, texture decode/cache use, scene retention, and memory copies.
- Optimize only after correctness gates protect the output.
- Preserve deterministic results across worker counts and machines.

Acceptance: release builds sustain 60 unique presented frames per second at the
baseline resolution on the target PC class without changing guest timing.

## P2 — Whole-game coverage and regression testing

- Cover every car, course, mode, results/continue flow, save/configuration path, and long run.
- Eliminate unresolved indirect targets and compatibility-only diagnostics.
- Add deterministic public tests for every source-safe subsystem.

Acceptance: cold boot through a complete race and return flow, entirely through the native static path.

## P2 — Productization and release packaging

- Reproducible build orchestration that accepts only user-supplied legally owned inputs.
- Clear asset extraction boundary, configuration UI, controller mapping, crash diagnostics, and release packaging.
- Performance profiling and native hot-path optimization after correctness gates pass.

Acceptance: clean-machine documentation and packaging that never redistributes third-party game content.
