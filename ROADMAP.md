# Roadmap

Work is ordered by the first unresolved hardware boundary. Each milestone has a concrete acceptance gate.

## Published checkpoint — v2411 public early demo

- A Windows x64 package now verifies matching user-owned CHD/PIC inputs,
  extracts required data locally, and launches the BIOS-free static-AOT path
  without requiring Python.
- The release contains no game input, BIOS, extracted asset, card, custom
  music, log, or personal path.
- Video, HUD, card, music, keyboard, XInput, DirectInput, and force-feedback
  controls are exposed through the F1 settings UI.
- A public report confirms that at least one Xbox controller is not detected;
  controller discovery is therefore not an accepted v2411 feature yet.

Checkpoint result: suitable for cautious public testing at the default 60 FPS,
not release-ready.

## P0 — Repair controller discovery and prove host-safe lifecycle

- Reproduce the v2411 Xbox-controller discovery failure outside the full game.
- Validate XInput DLL discovery, slot enumeration, Auto-device selection,
  reconnect behavior, axis/button capture, and launcher setting propagation.
- Complete a conservative live Vulkan close/restart stress pass only after the
  offline shutdown gates remain green.

Acceptance: supported controllers are detected and remappable on a clean
machine, and repeated normal close/restart cycles leave no live renderer/audio
worker, display-driver failure, black screen, or host freeze.

## Validated checkpoint — Targeted complete race path

- Targeted intro, menu, course-loading, live-race, result, and disposable-card save transitions now advance through the static native path.
- Normal JVS/physics input drives the player car; keyboard, analog, and digital-shifter routes pass focused tests.
- Remaining work is to repeat these gates across every mode, course, opponent, outcome, and error branch.

Checkpoint result: passed on targeted routes, not yet claimed across the whole game.

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

## Validated checkpoint — Sustain the 60 FPS presentation target

- Persistent renderer workers remove per-frame thread churn.
- Vulkan pipeline prewarming removes measured cold race-entry compilation from active frames.
- Clean acceptance runs sustain 4K/60 during race motion and 1080p/120 presentation interpolation while preserving guest timing.
- Fixed 60 Hz presentation retains the last completed image during synchronous guest loading, so output cadence no longer collapses while gameplay is correctly paused.

Checkpoint result: 59.8–60.1 FPS across 43 samples from tested 4K race takeover
onward. Reducing the underlying guest loading latency remains a transition-level
target even though presentation cadence is now stable.

## Validated checkpoint — Distinct 120 Hz on the measured cold route

- Guest logic, physics, timers, input, and audio remain at the authentic 60 Hz cadence.
- Presentation interpolation generates intermediate motion samples without advancing the game twice.
- The measured cold `k_ez` startup hotspot was removed with an exact ordinary-main-RAM object-clear path.
- All 23 complete two-second race intervals produced exactly 120 new presentation frames; the moving-race repeat counter stayed flat and the log contained zero faults.

Checkpoint result: passed for the fixed `k_ez` regression route and the
four-course priority matrix (`s_uh`, `s_vh`, `s_nm`, and `k_df3`). Exhaustive
content protection is still required before 120 FPS is called whole-game
validated.

## P2 — Whole-game coverage and regression testing

- Cover every car, course, mode, results/continue flow, save/configuration path, and long run.
- Preserve the completed natural-result ledger—48/48 target loads, 32/32 rival
  movement, 16/16 Time Attack layouts, and 32/32 rival profiles—while rerunning
  the broader mode/condition matrix on one current executable.
- Eliminate unresolved indirect targets and compatibility-only diagnostics.
- Add deterministic public tests for every source-safe subsystem.

Acceptance: every supported mode, course, car, opponent, outcome, and persistence
branch completes its defined regression path entirely through the native static
runtime, with no unresolved translated target.

## P2 — Productization and release packaging

- Reproducible build orchestration that accepts only user-supplied legally owned inputs.
- Broaden clean-machine acceptance for the current asset extraction boundary,
  configuration UI, controller mapping, crash diagnostics, and release package.
- Performance profiling and native hot-path optimization after correctness gates pass.

Acceptance: clean-machine documentation and packaging that never redistributes third-party game content.

## P3 — Enhanced presentation after correctness and coverage

- Finish configurable higher-resolution, widescreen, filtering, anti-aliasing, HUD, and display options through the F1 menu.
- Validate 120 FPS across content while keeping game logic, physics, timers, and audio on authentic guest timing.
- Pursue unlimited presentation only after the 120 FPS compatibility gate is protected by regressions.

Acceptance: enhanced modes never change gameplay results or replace guest assets,
and every option has a safe native default.

Dreamcast is not a roadmap target. This project remains a NAOMI 2 static-AOT recompilation.
