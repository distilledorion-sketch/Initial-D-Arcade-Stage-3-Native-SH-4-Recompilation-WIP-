# v2460 historical target closure checkpoint — 2026-09-01

This prerelease keeps the stable product identity and merge updater while
closing four genuine SH-4 targets found by auditing retained runtime logs
against the current compiled dispatcher.

## Runtime acceptance

- Native executable SHA-256:
  `FCFBD471638FA7E7FB1643A9EFE5EF1FB95FC3847897434D8CF32EADA8037EE0`
- Accepted targets: `0x0C1037E0`, `0x0C104E40`, `0x0C09EFA0`, and
  `0x0C0288A0`. All have bounded control flow and zero unknown SH-4
  instructions. Null, odd, out-of-image, and corruption-derived candidates
  were rejected.
- The unusually large `0x0C1037E0` target received additional manual boundary
  review: 5,696 bytes, 1,044 reachable instructions, one normal return, no
  BRAF/BSRF edge, no overlapping translated entry, and a 32-byte gap before
  the adjacent `0x0C104E40` function.
- The private build passed the path-cache regression, 20 route-parser checks,
  15 controller/music checks, 34 restart/shutdown checks, translation-source
  verification, 116-object link freshness, and the standalone no-firmware
  audit.
- Separate bounded Akina and Usui runs used no BIOS, authentic 60 Hz guest and
  presentation timing, one raster worker, muted audio, offscreen Vulkan, and
  Below Normal process priority. They reached 49.037 m and 53.181 m of real
  movement with zero untranslated calls, runtime faults, or repeated frames.
  Both exited cooperatively and left no game process.

These are targeted route acceptances, not proof that every campaign, card,
visual, or error branch is complete.

## Package and updater acceptance

- Archive: `Initial.D.Arcade.Stage.3.Recompiled.v2460.zip`
- ZIP SHA-256:
  `7FAE695DA3449FB261B95E1256A60E75F3A12FA9BFAE44FF3B09BFF221D71F03`
- Audited size: 23,826,056 bytes; 18 files.
- The archive contains one top-level folder named
  `Initial D Arcade Stage 3 Recompiled`. Its root entry point is
  `Initial D Arcade Stage 3 Recompiled.exe`; its native helper is
  `Initial D Arcade Stage 3 Recompiled Runtime.exe`.
- The real ZIP passed the current merge installer and the exact updater plus
  installer shipped in v2457. Both accepted version 2460, preserved protected
  user data and unrelated destination-only files, selected the canonical
  folder/runtime, and did not launch the game.
- A byte-identical `demo.exe` compatibility copy remains so pre-v2458 updaters
  can accept a direct jump to the newest release. The installed launcher
  removes only that exact duplicate and preserves modified or unrelated files.

The archive contains no game data, BIOS, PIC, CHD, cards, custom music, logs,
captures, learned paths, generated guest-code source, credentials, or private
host paths. Users must provide legally obtained matching game inputs locally.
