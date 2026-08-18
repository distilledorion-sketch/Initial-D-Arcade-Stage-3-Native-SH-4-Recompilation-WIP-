# Initial D Arcade Stage 3 — Native SH-4 Recompilation (WIP)

<p align="center">
  <img src="docs/media/project-logo.png" alt="Initial D Arcade Stage Ver. 3 — Recompiled" width="900">
</p>

An experimental static ahead-of-time recompilation project for the NAOMI 2 release of *Initial D Arcade Stage 3* (GDS-0033).

> **Research status:** this is not an emulator, not a finished port, and not a playable game distribution. The public repository contains original clean-room infrastructure and progress evidence only. It contains no game image, BIOS, PIC/CHD data, extracted assets, memory snapshots, or generated game-code translation.

![Native intro progress](docs/media/native-intro-full.gif)

## What works today (2026-08-18)

- A Python SH-4 decoder/code generator covers the instruction families exercised by the current static-AOT path, including FPSCR-aware floating-point register width and bank handling.
- Native C++ execution cold-boots into sustained attract-mode rendering with direct translated-function dispatch and no interpreter or JIT fallback.
- The submitted ELAN stream, CH2 DMA, render-complete interrupts, retained scenes, and native presentation path now advance continuously instead of stalling at the earlier frame-lifecycle frontier.
- The Naomi 2 renderer handles the observed opaque, translucent, modifier-volume, punch-through, fog, blend, texture, and user-tile-clip cases used by the current scenes.
- A missing course environment/path lookup was traced to two ISO9660-truncated files. A source-safe tool now restores their logical names from a user's own dump and verifies exact hashes.
- A corrected native sequence renders a single coherent textured Trueno with stable camera motion at a 60 FPS presentation target; the earlier alternating rainbow/static environment maps are gone.
- Cross-machine N70 validation reproduced an identical rendered-frame SHA-256: `34D6B91C0550A5CC2A60D4B1F8930812908CEA6DCD6C354749A0881BE426D9E2`.
- Private integration tests cover ELAN command/VRAM classification, JVS identity and EEPROM behavior, Maple VBlank/reset behavior, Naomi system-ROM write protection, Flycast-derived AICA SGC/mailbox behavior, and PVR twiddled texture layout.

![Intro shot progression](docs/media/native-intro-strip.png)

## What does not work yet

- The result is not a start-to-finish playable build.
- A complete controllable race has not yet been reached and validated end to end.
- The entire intro/menu/course-loading sequence still needs frame-by-frame correctness checks; later texture, depth, transparency, shadow, fog, overlay, and camera issues may remain.
- The current diagnostic renderer does not consistently sustain 60 unique rendered frames per second and still needs profiling and optimization.
- Complete cabinet controls, synchronized game audio, whole-game static-AOT coverage, long-run determinism, and presentation polish remain incomplete.
- The public repository deliberately omits private generated game code, captures, and all legally restricted inputs, so it is not a standalone game build.

See [STATUS.md](STATUS.md) for the evidence ledger and [ROADMAP.md](ROADMAP.md) for ordered acceptance criteria.

## Repository contents

- `translator/` — general SH-4 instruction decoding and C++ statement generation.
- `src/runtime/` — selected clean-room runtime snapshots for ELAN and JVS work.
- `tests/` — public tests that require no game data.
- `docs/` — architecture, media, and validation notes.

The renderer snapshot is included to show the real integration work, while the
complete private runtime and generated translation units remain intentionally
absent. The new preparation utility also contains no game data: it operates
only on user-supplied legally owned inputs.

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

this is a native static-AOT port

This independent research project is not affiliated with or endorsed by Sega, Kodansha, Shuichi Shigeno, or the original developers and publishers. All referenced names and trademarks belong to their respective owners.
