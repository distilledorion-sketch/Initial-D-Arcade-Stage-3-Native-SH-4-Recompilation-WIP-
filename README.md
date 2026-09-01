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

[**Download Public Early Demo v2444 for Windows x64**](https://github.com/distilledorion-sketch/Initial-D-Arcade-Stage-3-Native-SH-4-Recompilation-WIP-/releases/tag/v2444-audio-card-portability)

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

1. Download and extract `Public Early Demo v2444.zip` from the
   [v2444 prerelease](https://github.com/distilledorion-sketch/Initial-D-Arcade-Stage-3-Native-SH-4-Recompilation-WIP-/releases/tag/v2444-audio-card-portability).
2. Place your own matching files in the included `game files` folder:
   - `gds-0033.chd`
   - `317-0384-com.pic`
3. Run `Initial D Arcade Stage 3 Recompiled.exe`.

The Windows launcher checks each input, explains which file is missing or
incorrect, extracts the required files, and repairs an incomplete setup.
Python is not required. Allow roughly 1.3 GB of temporary free space during
first-run extraction. A NAOMI 2 BIOS is neither required nor accepted.

Public ZIP SHA-256:
`BEFE5D96AD8467C63E2A767B920AC523447ED42C3C849A3B817E051456E24515`

## Current integration progress (v2444 WIP)

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
  to Safe presentation after an interrupted Direct session.
- GPU projection, ELAN lighting, depth normalization, static-geometry reuse,
  packed topology, and persistent mapped geometry/uniform streams remove the
  previous per-frame CPU staging copies.
- Exact-matched static course/car batches retain their immutable topology
  eligibility summaries, avoiding repeated full-array scans on each 120 Hz
  presentation phase.

## Verified progress

- All 48 defined route/branch targets have loaded and all 32 rival profiles
  have produced real movement through normal guest physics/input.
- All 16 unique Time Attack layouts and all 32 Legend rival profiles have
  separate natural, game-owned result evidence.
- A 70-row Time Attack/Bunta condition matrix has passing route-load/movement
  evidence. This is mixed-checkpoint evidence, not yet a complete matrix rerun
  on one v2415 executable. v2415 separately passed CPU-only same-build probes
  for Myogi Time Attack plus the Shomaru and Tsuchisaka Bunta routes.
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
- Every car/course/weather/day/night combination, long campaign permutation,
  Bunta condition, and error branch has not been exhaustively rerun on v2444.
- Physical Xbox discovery now has a product-shared hardware acceptance result;
  live axis/button movement, multiple controller models, wheels/force-feedback,
  and audible end-user audio behavior still need broader acceptance.
- The cooperative shutdown path passes source and offscreen checks, but a full
  live Vulkan/WSI stress pass remains deliberately pending after earlier host
  black-screen incidents. Please close the game normally and report the newest
  logs if a failure occurs.
- Clean-machine packaging needs broader community testing.

When reporting a problem, include the mode, course/opponent, direction,
weather/time choice, display resolution, FPS mode, controller, exact point of
failure, and the newest logs. Do not upload game files, extracted assets, card
data, or copyrighted music.

See [STATUS.md](STATUS.md) for the evidence ledger,
[the v2444 integration/public checkpoint](docs/CURRENT_CHECKPOINT_V2444_2026-09-01.md)
for the latest source/runtime and release facts, and [ROADMAP.md](ROADMAP.md) for the
ordered acceptance criteria.

## Repository contents

- `translator/` — general SH-4 instruction decoding and C++ generation.
- `src/runtime/` — selected source-safe runtime snapshots for ELAN and JVS.
- `tests/` — public tests that require no game data.
- `tools/` — source-safe utilities that operate only on user-provided inputs.
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
