# Initial D Arcade Stage 3 — Native SH-4 Recompilation (WIP)

<p align="center">
  <img src="docs/media/project-logo.png" alt="Initial D Arcade Stage Ver. 3 — Recompiled" width="900">
</p>

An experimental static ahead-of-time recompilation of the **NAOMI 2** release
of *Initial D Arcade Stage 3* (GDS-0033) for modern Windows PCs.

> **Public early demo available:** this is unfinished playtest software, not a
> finished port. Start with the default 60 FPS mode. The public download does
> not include game data, a BIOS, card saves, custom music, logs, or extracted
> assets. You must provide your own legally obtained matching game files.

[**Download Public Early Demo v2456 for Windows x64**](https://github.com/distilledorion-sketch/Initial-D-Arcade-Stage-3-Native-SH-4-Recompilation-WIP-/releases/tag/v2456-direct-vulkan-retry)

![Native intro progress](docs/media/native-intro-full.gif)

## What this project is

- A native static-AOT recompilation of the original SH-4 program.
- A native NAOMI 2 runtime with Flycast-derived hardware knowledge where noted.
- A Vulkan renderer for the game's submitted ELAN/PVR work.
- A BIOS-free executable path with no SH-4 interpreter or JIT fallback.

It is **not** a Dreamcast port and the released product does **not** run the
game through a NAOMI 2 BIOS. The setup utility uses a matching CHD and security
PIC only to verify and locally extract the data required by the recompilation.

## Quick start

1. Download and extract `Public.Early.Demo.v2456.zip` from the
   [v2456 prerelease](https://github.com/distilledorion-sketch/Initial-D-Arcade-Stage-3-Native-SH-4-Recompilation-WIP-/releases/tag/v2456-direct-vulkan-retry).
2. Place your own matching files in the included `game files` folder:
   - `gds-0033.chd`
   - `317-0384-com.pic`
3. Run `Initial D Arcade Stage 3 Recompiled.exe`.

The Windows launcher checks each input, explains which file is missing or
incorrect, extracts the required files, and repairs an incomplete setup.
Python is not required. Allow roughly 1.3 GB of temporary free space during
first-run extraction. A NAOMI 2 BIOS is neither required nor accepted.

Public ZIP SHA-256:
`F5A101272EC7B8EFB04C89EAD4ADBCC2F84CD5CB74C12DE2B94C7C9109398B88`

## Current integration progress (v2456 WIP and public early demo)

- BIOS-free translated SH-4 boot, menu, race, result, and tested save flows.
- Vulkan rendering with an authentic 60 Hz mode plus experimental 90/120 Hz
  presentation that keeps gameplay, physics, timers, input, and audio on the
  original 60 Hz guest cadence.
- Output resolution, widescreen, antialiasing, texture filtering, fullscreen,
  V-sync, FPS counter, and live HUD-position settings.
- An F1 menu with Video, Controls, HUD, Card, and Music pages.
- Keyboard, Xbox/XInput, and DirectInput wheel paths with persistent remapping;
  Xbox triggers are independent accelerator/brake axes.
- The F1 Controls page now accepts D-pad/left-stick navigation and A/B actions,
  and deliberate 35% axis travel is sufficient for remapping. The same shared
  XInput loader used by the product passed a no-window probe against a real
  attached Xbox controller on Windows.
- Adjustable force-feedback strength for supported DirectInput wheels.
- Live 0–100% Steering Smoothing for controllers and wheels; 0% preserves the
  original unfiltered response and the chosen value is saved.
- Native ARM7/AICA audio and exact-slot optional MP3/WAV/FLAC race-music
  replacement. Clearing a replacement restores the original game track and no
  original asset is overwritten.
- Card creation/selection in the visible `card data` folder. The last selected
  card is loaded on the next start, and changing cards displays an unsaved-data
  restart warning. Managed selections now remain portable when the extracted
  demo folder is moved or renamed.
- Post-race ejection now reports the required empty-reader/removal state before
  queuing the selected saved card for the next prompt. A bounded Bunta result
  run wrote exactly 207 bytes and crossed this reinsert transition with zero
  recognized runtime faults.
- The Windows launcher checks GitHub releases before starting the game. Newer
  versions are shown with their release date and Yes/No choices; an optional
  checkbox enables verified automatic installs on future launches. Updates
  preserve game files, cards, music, logs, and user settings and roll back if
  installation fails.
- A self-contained, Python-free setup and integrity checker for user-owned
  inputs.
- Cooperative renderer shutdown: the program stops new Vulkan presentation,
  joins the raster worker, and only then destroys its window.
- Single-instance enforcement prevents two recomp graphics contexts from
  running concurrently. Windows QA/watchdog exits use the same cooperative
  renderer drain as a normal close.
- Bounded Vulkan acquire, graphics-fence, presentation-fence, and startup
  upload waits; no queue-idle, device-idle, or infinite fence wait remains.
- Direct Vulkan presentation now bypasses the CPU/GDI display copy when the
  user explicitly selects it. A clean-session marker automatically falls back
  to Safe presentation after an interrupted Direct session. v2456 no longer
  mistakes the current healthy process's own marker for an earlier crash, and
  a genuinely stale marker offers an explicit Direct retry after restart.
- GPU projection, ELAN lighting, depth normalization, static-geometry reuse,
  packed topology, and persistent mapped geometry/uniform streams remove the
  previous per-frame CPU staging copies.
- Exact-matched static course/car batches retain their immutable topology
  eligibility summaries, avoiding repeated full-array scans on each 120 Hz
  presentation phase.
- The FPS overlay now reports actual swapchain output as `OUT` and genuinely
  distinct authentic/interpolated frames as `NEW`; exact repeats are excluded.
- Authored attract-mode cinematic mattes now expand to the real widescreen
  edges. Recognition requires an exact four-vertex mesh/material signature and
  proof that the projected quad spans the native 0..640 viewport, so HUD and
  world geometry retain their existing aspect behavior.

## Verified progress

- All 48 defined route/branch targets have loaded and all 32 rival profiles
  have produced real movement through normal guest physics/input.
- All 16 unique Time Attack layouts and all 32 retained rival profiles (30
  Legend plus two Bunta profiles) have separate natural, game-owned result
  evidence across the historical accepted ledger.
- The complete 70-row Time Attack/Bunta condition matrix now passes on the
  exact final v2449 executable: 62 Time Attack direction/weather/time rows and
  all eight Bunta courses. Every row matched its requested course, condition,
  state, and movement requirements; Time Attack direction was independently
  proven by three agreeing selector fields. All runs reported zero recognized
  faults and exited cooperatively.
- All 16 retained Time Attack layout routes were also renewed to a natural,
  game-owned result on that exact v2449 executable using normal 2560x1080
  Direct Vulkan at 60 Hz. Every route showed authentic movement before the
  result, no forced outcome, zero recognized errors/runtime faults, a 59.8 FPS
  or better Vulkan minimum, and cooperative clean-session shutdown.
- Exact-v2453 rival-result renewal is now tracked separately from the
  historical mixed-checkpoint ledger. Evo 5, Evo 6, the Keisuke rematch, Kyoko,
  the Ryosuke rematch, Sakamoto, Smiley, the Sudo rematch, and Wataru each
  reached positive player travel before a natural, game-owned result. After
  each target armed, the log recorded zero requested or applied helper
  outcomes. All nine strict runs averaged 60.0 FPS for display and Direct
  Vulkan presentation, reported zero fault signals, and shut down
  cooperatively. This is 9/32 profiles
  renewed on the exact v2453 SHA-256, not a whole-ledger same-build claim.
- The fixed `k_ez` regression route produced 120 distinct motion samples in
  each of 23 complete two-second race intervals, with no repeated moving-race
  endpoints. A four-course priority matrix measured 119.8–120.0 FPS minimum
  presentation in its accepted moving windows.
- v2415 offline checks cover controller discovery/remapping policy, axis/button
  normalization and capture while the F1 menu is paused, exact custom
  music routing, presentation interpolation, ELAN/card/AICA behavior, offscreen
  Vulkan, card-eject classification, translation integrity, link freshness,
  the no-firmware product boundary, cooperative QA shutdown, and single-instance
  enforcement.
- The v2441 integration build passed the same complete offline suite, including
  a new BGR24/BGRA32 presenter channel-accuracy test. Four fresh v2441
  Direct-Vulkan route runs then completed cooperative shutdown with exit code
  0 and zero moving-race repeats. The exact 2560x1080 Akagi target sustained
  120.0 FPS minimum/average for both display and Vulkan across 35 steady
  two-second samples, with 120 distinct generated motion samples minimum per
  bucket. Myogi and Akina/Akagi 640x480 controls measured 119.0–120.0 FPS
  minimum and also recorded zero steady-race repeats.
- The v2442 controller checkpoint preserved that renderer/runtime base and
  passed the complete offline suite. Its product-shared XInput probe loaded
  `xinput1_4.dll`, found the attached controller in slot 0, converted its
  neutral state to centered steering and released pedals, and left no game or
  Vulkan process running. Axis/button capture, lower-travel axis remapping,
  controller menu navigation, music, card, interpolation, lifecycle, link
  freshness, and the no-firmware boundary all passed their focused checks.
- The v2443 audio checkpoint opened the exact product 44.1 kHz stereo PCM
  format on the preferred Windows endpoint without emitting sound, and its
  null-sink cold run produced 7,080,718 post-mix signal frames with zero AICA
  drops before cooperative exit.
- The v2444 card-portability checkpoint rebuilt every presenter-dependent
  owner and passed the complete controller, physical XInput, AICA mailbox/SGC,
  Windows endpoint, exact custom-music, interpolation, Direct-session, ELAN,
  card, offscreen Vulkan, timing, lifecycle, translation, freshness, and
  standalone no-firmware suite. The clean public ZIP contains 14 audited files
  and no game data, BIOS, cards, music, logs, captures, or private paths.
- The v2445 launcher checkpoint maps the saved F1 Direct choice to the native
  swapchain path and Safe to the bounded GPU-readback path through one tested
  helper. Direct and Safe no-launch validation both passed, all maintained
  launchers parsed cleanly, and the native v2444 executable was unchanged.
- The v2454 card/updater checkpoint passed 27 exact card-device transactions,
  the complete offline product suite, updater selection/digest/extraction/
  preservation/rollback tests, and the moved-launcher test. Its 17-file public
  ZIP was then extracted and re-audited with no game data, BIOS, card, music,
  log, capture, or private path. Bunta win and loss post-result runs plus a
  three-minute no-input attract run all exited cooperatively without a
  recognized crash or untranslated-runtime fault.
- The v2456 Direct-Vulkan retry checkpoint passed compilation, a 33-case
  restart/shutdown suite, 20 route-parser checks, 15 controller/music checks,
  an exact synthetic learned-path round trip, link-freshness checks, and the
  standalone no-firmware audit. The audited 17-file ZIP contains no game data,
  BIOS, PIC, cards, music, logs, captures, private racing paths, generated guest
  translations, or private host paths. Live acceptance of the corrected F1
  Direct-retry flow remains pending and is not claimed by this checkpoint.
- The earlier v2446/schema-5 checkpoint established independent three-field
  direction identity and stopped treating dry/wet condition meshes as road
  direction. The final-v2449 70/70 matrix below supersedes its mixed-build
  route count while retaining that fail-closed evidence policy.
- The v2448 integration build passed the complete BIOS-free product suite.
  A normal visible 2560x1080 Direct-Vulkan Akagi run then held 120.0 FPS
  minimum/average for both display and Vulkan across 21 moving-race samples,
  produced at least 240 distinct frames per two-second bucket, added zero
  repeats, advanced 129.204 m, and exited cooperatively with no unclean marker.
- The v2449 integration build corrected the centered-4:3 cinematic matte on a
  2560x1080 BIOS-free attract run. Twelve captured scene points covered both
  the initial narrow-road matte and later changing-zoom car close-ups; the
  masks reached both output edges while Hor+ world rendering and unstretched
  UI remained intact. Six matched 640x480 scenes are byte-identical to v2448,
  proving native presentation is unchanged. A normal visible Direct run without
  capture readback held 119.5–120.5 FPS after warm-up and exited without a
  fault, process, or Direct-session marker. The final build also passed the
  complete product suite.
- Conservative BIOS-free v2449 route renewal now covers the entire defined
  70-row matrix: 62 Time Attack combinations and all eight Bunta courses. A
  no-launch strict reanalysis also returned 70 passes, zero failures, and zero
  missing rows for evidence tied to the final executable SHA-256.
- In the preceding v2440 live RX 9070 XT Direct Vulkan run, a
  75,000–127,000-vertex course/race sequence sustained 119.7–120.3 visible
  presents per second, crossed into the result/continue transition, and then
  completed every cooperative shutdown phase with exit code 0.
- During that live run the Direct path reduced sampled renderer/process CPU
  demand from roughly 1.56 host cores in Safe/GDI presentation to 0.2–1.2
  cores depending on scene load. Memory and handle counts stabilized, and the
  Direct-session marker was removed on normal exit.
- A cross-machine renderer replay produced the same accepted frame SHA-256:
  `34D6B91C0550A5CC2A60D4B1F8930812908CEA6DCD6C354749A0881BE426D9E2`.

These are targeted engineering results, not proof that every game combination
is complete.

![Intro shot progression](docs/media/native-intro-strip.png)

## Known early-build limitations

- This is not release-quality or whole-game complete. Visual, audio, timing,
  transition, controller, card, and course-specific defects may remain.
- **Use 60 FPS first.** Higher-refresh presentation is experimental and may
  expose jitter, clipping, flicker, or unstable performance on unverified
  content and hardware.
- Unlimited presentation rate is not finished.
- The defined Time Attack/Bunta route-state matrix is current, but every car,
  opponent, long campaign/result/persistence permutation, and error branch has
  not been exhaustively completed on one integration executable.
- Physical Xbox discovery now has a product-shared hardware acceptance result;
  live axis/button movement, multiple controller models, wheels/force-feedback,
  and audible end-user audio behavior still need broader acceptance.
- The cooperative shutdown path passes source/offscreen checks and bounded
  live Direct-Vulkan runs. Repeated stress across more GPUs and drivers is
  still required after earlier host black-screen incidents. Please close the
  game normally and report the newest logs if a failure occurs.
- Clean-machine packaging needs broader community testing.

When reporting a problem, include the mode, course/opponent, direction,
weather/time choice, display resolution, FPS mode, controller, exact point of
failure, and the newest logs. Do not upload game files, extracted assets, card
data, or copyrighted music.

See [STATUS.md](STATUS.md) for the evidence ledger,
[the v2456 public checkpoint](docs/CURRENT_CHECKPOINT_V2456_DIRECT_VULKAN_RETRY_2026-09-01.md)
for the latest Direct-Vulkan/release facts,
[the v2454 public checkpoint](docs/CURRENT_CHECKPOINT_V2454_CARD_REINSERT_UPDATER_2026-09-01.md)
for the preceding card/updater facts,
[the v2445 public checkpoint](docs/CURRENT_CHECKPOINT_V2445_2026-09-01.md)
for the preceding launcher facts,
[the v2449 integration checkpoint](docs/CURRENT_CHECKPOINT_V2449_CINEMATIC_MASK_2026-09-01.md)
for the current native-runtime acceptance,
[the v2449 route-matrix checkpoint](docs/CURRENT_CHECKPOINT_V2449_ROUTE_MATRIX_2026-09-01.md)
for the complete same-build condition ledger,
[the v2449 rival-result renewal checkpoint](docs/CURRENT_CHECKPOINT_V2449_RIVAL_RENEWAL_2026-09-01.md)
for the earlier target-gated ledger,
[the v2453 exact rival-result checkpoint](docs/CURRENT_CHECKPOINT_V2453_EXACT_RIVAL_RENEWAL_2026-09-01.md)
for the current same-SHA strict ledger, and [ROADMAP.md](ROADMAP.md)
for the ordered acceptance criteria.

## Repository contents

- `translator/` — general SH-4 instruction decoding and C++ generation.
- `src/runtime/` — selected source-safe runtime snapshots for ELAN and JVS.
- `tests/` — public tests that require no game data.
- `tools/` — source-safe utilities and the versioned Windows launcher; setup
  tools operate only on user-provided inputs.
- `docs/` — architecture, media, and validation notes.

The Git source tree intentionally omits game images, BIOS/PIC/CHD data,
extracted assets, cards, logs, memory captures, and generated game translation
units. The GitHub prerelease supplies the Windows launcher/runtime but still
contains none of those user-owned inputs.

## Run the public source checks

Python translator tests:

```bash
python -m unittest discover -s tests -v
```

Native ELAN classifier, controller/smoothing tests, and Windows-only shared
XInput/audio endpoint probes:

```bash
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

No proprietary input is needed for either check. See [BUILDING.md](BUILDING.md)
for the boundary between the public source checks and the downloadable demo.

## Project boundary

This independent preservation/interoperability research project is not
affiliated with or endorsed by Sega, Kodansha, Shuichi Shigeno, or the original
developers and publishers. All referenced names, artwork, music, game data,
and trademarks belong to their respective owners. Only test with material you
are legally permitted to use; never submit those inputs to this repository.
