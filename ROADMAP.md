# Roadmap

Work is ordered by the first unresolved hardware boundary. Each milestone has a concrete acceptance gate.

## Current integration checkpoint — v2458 ordered path and merged updates

- Direct Vulkan presentation bypasses the Safe path's CPU/GDI display boundary
  when explicitly selected.
- Projection, ELAN lighting, normalized depth, static geometry/topology, and
  per-object uniform data now use GPU-side work or bounded persistent mapped
  streams instead of per-frame CPU staging copies.
- A compatible Safe-mode BGRA readback can be copied directly into a Win32
  32-bit DIB; diagnostics retain exact RGB24 output.
- A saved Direct selection is authoritative. v2457 automatically clears a
  stale diagnostic marker when possible and proceeds with Direct even when
  marker maintenance is unavailable; it neither prompts nor forces Safe.
  Safe remains the first-run default and a manual selection.
- One live 2560x1080 RX 9070 XT run sustained 119.7–120.3 FPS through a heavy
  course and result transition, then exited normally and removed the marker.
- v2441 retains immutable topology eligibility summaries for exact-matched
  static geometry. In the controlled 16,384-vertex reuse case this reduced
  median topology preparation by 35.9% and complete frame time by 9.6%.
- v2448 corrected the in-game `OUT`/`NEW` presentation counters and held 120.0
  FPS with 240 distinct frames per two-second moving-race sample on the exact
  accepted Akagi run.
- v2449 expands only exact authored cinematic mattes that prove native
  x=0..640 coverage. BIOS-free 2560x1080 attract captures pass fixed and
  changing camera zoom without stretching HUD or world geometry.
- The complete offline suite, including physical Xbox discovery, audio,
  music, card, interpolation, Vulkan, lifecycle, and standalone no-firmware
  checks, remains green.
- Conservative final-v2449 route renewal passes the complete 70-row matrix:
  all 62 defined Time Attack direction/weather/time rows and all eight Bunta
  courses. Every row reached real movement, reported zero recognized faults,
  and exited cooperatively; strict no-launch reanalysis returned 70/70.
- Exact-v2453 target-gated rival-result renewal has nine strict passes: Evo 5,
  Evo 6, Keisuke rematch, Kyoko, Ryosuke rematch, Sakamoto, Smiley, Sudo
  rematch, and Wataru. This is 9/32 current-build profiles; the historical
  32/32 accepted ledger is intentionally not promoted to a same-build claim.
- Developer-only persisted Time Attack path capture/playback now has a
  validated binary format and live-control-clock recovery policy. No learned
  input data is published. v2458 prevents a later spatially adjacent
  switchback from overriding the ordered controller; live validation past the
  existing 663 m route stall remains an open QA gate.

Checkpoint result: the newest Direct performance and attract runs are
host-clean, the cinematic widescreen defect is regression-protected, the
entire route-state matrix has exact-v2449 movement proof, and nine natural
rival results now have strict exact-v2453 evidence. v2458 retains the saved
Direct choice in offline policy tests; a live Direct-renderer acceptance run
for this exact build is not claimed. Full-race,
cross-driver, car/opponent, campaign, persistence, and visual stress remain.

## Published checkpoint — v2458 public early demo

- A Windows x64 package now verifies matching user-owned CHD/PIC inputs,
  extracts required data locally, and launches the BIOS-free static-AOT path
  without requiring Python.
- The release contains no game input, BIOS, extracted asset, card, custom
  music, log, or personal path.
- Video, HUD, card, music, keyboard, XInput, DirectInput, and force-feedback
  controls are exposed through the F1 settings UI.
- The Controls page includes XInput-first selection, wireless reconnect scan,
  keyboard/controller/wheel remapping, force-feedback strength, and saved
  0–100% steering smoothing.
- The complete controller and offscreen regression suite passes; broad physical
  controller/wheel coverage remains an acceptance target.
- Startup uploads and frame/presentation retirement use bounded fences; the
  product has no unbounded Vulkan queue-idle, device-idle, or fence wait.
- The product rejects concurrent instances, Windows QA shutdown is
  cooperative-only, and route-only coverage defaults to no Vulkan WSI.
- The saved Direct/Safe Vulkan presentation choice survives launcher restart;
  Direct is not silently replaced by Safe because of stale or unavailable
  marker state. The public ZIP was re-audited with no game data,
  firmware, cards, music, captures, learned paths, generated guest code, or
  private paths.
- The live updater acceptance verified release selection, GitHub's SHA-256,
  installation, user-data preservation, and current-version suppression in an
  isolated folder without starting the game.
- The requested folder/main/runtime names and merge semantics pass against the
  real v2458 archive. An additional acceptance used the exact v2457-shipped
  updater and installer, proving existing users can cross the naming change
  while keeping user and destination-only files.

Checkpoint result: suitable for cautious public testing at the default 60 FPS,
not release-ready.

## P0 — Complete physical-input and host-safe lifecycle acceptance

- Validate controller and wheel selection, remapping, menu navigation, steering,
  smoothing, independent triggers/pedals, shifts, View, Start, Coin, and FFB.
- Preserve the passing discovery, ordering, reconnect, paused-menu capture,
  smoothing, and launcher-setting regressions.
- Complete a conservative live Vulkan close/restart stress pass only after the
  offline shutdown gates remain green.
- Preserve the passing Direct-session marker lifecycle and v2440 clean live
  shutdown while extending the matrix across more hardware and restart cycles.

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
- Preserve the completed 70/70 v2449 Time Attack/Bunta route-state matrix while
  extending coverage through full races, results, continuation, and save paths.
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
