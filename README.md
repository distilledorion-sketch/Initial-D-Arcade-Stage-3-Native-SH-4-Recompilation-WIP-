# Initial D Arcade Stage 3 — Native SH-4 Recompilation (WIP)

<p align="center">
  <img src="docs/media/project-logo.png" alt="Initial D Arcade Stage Ver. 3 — Recompiled" width="900">
</p>

An experimental static ahead-of-time recompilation project for the NAOMI 2 release of *Initial D Arcade Stage 3* (GDS-0033).

> **Research status:** this is not an emulator, not a finished port, and not a playable game distribution. The public repository contains original clean-room infrastructure and progress evidence only. It contains no game image, BIOS, PIC/CHD data, extracted assets, memory snapshots, or generated game-code translation.

![Native intro progress](docs/media/native-intro-full.gif)

## What works today (2026-08-30)

- A Python SH-4 decoder/code generator covers the instruction families exercised by the current static-AOT path, including FPSCR-aware floating-point register width and bank handling.
- Native C++ execution cold-boots, advances through targeted menu and loading paths, enters live races, drives through normal JVS/physics input, and reaches tested result/save flows with direct translated-function dispatch and no interpreter or JIT fallback.
- The submitted ELAN stream, CH2 DMA, render-complete interrupts, retained scenes, and native presentation path now advance continuously instead of stalling at the earlier frame-lifecycle frontier.
- The Naomi 2 renderer handles the observed opaque, translucent, modifier-volume, punch-through, fog, blend, texture, and user-tile-clip cases used by the current scenes.
- A missing course environment/path lookup was traced to two ISO9660-truncated files. A source-safe tool now restores their logical names from a user's own dump and verifies exact hashes.
- The renderer now preserves the tested HUD, mirror placement, projection, car geometry, texture mapping, and translucent/modifier-volume behavior through targeted menu and race scenes.
- A persistent renderer worker pool and Vulkan pipeline prewarming remove the measured CPU thread-creation jitter and cold race-entry pipeline compilation stalls.
- The private integration build keeps game logic, physics, timers, input, and audio on the authentic 60 Hz guest cadence while presenting distinct interpolated motion at 120 Hz. On the fixed `k_ez` regression route, v2374 produced exactly 120 new motion samples in all 23 complete two-second race intervals, with no repeated moving-race endpoints and no faults.
- A guest-side cold-start hotspot was traced to the game's canonical byte-wise object clear. The private integration now bulk-clears only proven ordinary-main-RAM ranges while preserving one guest memory cycle per byte, final SH-4 registers/flags, and the original fallback for every other mapping.
- AICA/ARM7 processing produces sustained non-silent output and selects the tested race music stream; automated QA runs use a deliberate null audio sink.
- Keyboard/JVS controls, digital shifter routes, no-card operation, and a disposable-card insert/load/save/reload path pass focused tests.
- The private launcher performs BIOS-free setup from user-owned inputs, reports missing/corrupt inputs with normal Windows errors, and requires no separate Python installation. Card files and custom race-music mappings are managed in user-visible folders.
- Cross-machine N70 validation reproduced an identical rendered-frame SHA-256: `34D6B91C0550A5CC2A60D4B1F8930812908CEA6DCD6C354749A0881BE426D9E2`.
- Private integration tests cover ELAN command/VRAM classification, JVS identity and EEPROM behavior, Maple VBlank/reset behavior, Naomi system-ROM write protection, Flycast-derived AICA SGC/mailbox behavior, and PVR twiddled texture layout.

![Intro shot progression](docs/media/native-intro-strip.png)

## What does not work yet

- This is not a finished or publicly playable game package, and it must not be treated as whole-game completion.
- Every course, car, weather condition, opponent, Legend route, Time Trial, Bunta Challenge, result branch, and card-error branch has not yet been exhaustively validated.
- Some synchronous guest asset/music loading still pauses creation of new game scenes before car motion begins. The fixed presenter maintains cadence during static transitions, and the proven `k_ez` cold-race hotspot is removed, but other courses still require the same validation.
- Physical wheel force feedback and audible end-user audio acceptance still need hardware validation even though the software paths and automated checks advance.
- Unlimited presentation rate independent of gameplay is not complete. The validated high-rate target is currently 120 FPS with original guest timing.
- Long-run determinism, clean-machine user packaging, crash diagnostics, and remaining presentation polish are still open.
- The public repository deliberately omits private generated game code, captures, and all legally restricted inputs, so it is not a standalone game build.

See [STATUS.md](STATUS.md) for the evidence ledger,
[the current source-safe checkpoint](docs/CURRENT_CHECKPOINT_2026-08-30.md)
for the latest measured results, and [ROADMAP.md](ROADMAP.md) for ordered
acceptance criteria.

## Repository contents

- `translator/` — general SH-4 instruction decoding and C++ statement generation.
- `src/runtime/` — selected clean-room runtime snapshots for ELAN and JVS work.
- `tests/` — public tests that require no game data.
- `docs/` — architecture, media, and validation notes.

The renderer snapshot is included to show the real integration work, while the
complete private runtime and generated translation units remain intentionally
absent. The preparation utility also contains no game data: it operates only
on user-supplied legally owned inputs.

## Run the public checks

Python translator tests:

```bash
python -m unittest discover -s tests -v
```

Native ELAN classifier smoke test:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

No proprietary input is needed for either check.

## Project boundary

The private integration build is a native static-AOT recompilation of the
NAOMI 2 game. A Dreamcast port is explicitly outside the project scope.

This independent research project is not affiliated with or endorsed by Sega, Kodansha, Shuichi Shigeno, or the original developers and publishers. All referenced names and trademarks belong to their respective owners.
