# v2461 RTS delay-slot correctness checkpoint — 2026-09-01

This prerelease keeps the stable BIOS-free product, canonical names, and merge
updater while correcting analyzer-proven SH-4 return delay-slot omissions in
legacy translated functions.

## Runtime acceptance

- Native executable SHA-256:
  `104B7A357D18450A890EBBE7549FF2B180ECDA84C6AD131E63B8849A07B4D12A`
- The audit found 67 decoded, non-NOP instructions placed immediately after an
  early C++ `return` for an SH-4 RTS. They covered 60 functions and are now
  executed before returning, preserving the processor's delay-slot semantics.
- Repeating the audit to a fixed point reports zero proven non-NOP omissions.
  Eighty unresolved legacy source/guest boundary mismatches were reviewed and
  left untouched because their following statements belong to adjacent guest
  functions rather than provable RTS delay slots.
- The private build passed the learned-path regression, 20 route-parser checks,
  15 controller/music checks, 34 restart/shutdown checks, translated-source
  verification, 116-object link freshness, and the standalone no-firmware
  audit.
- A bounded Akina Time Attack gate used no BIOS, strict 60 Hz guest and
  presentation timing, one raster worker, muted audio, offscreen Vulkan, and
  Below Normal process priority. It followed an explicitly validated private
  course path for 1,488.022 m, applied 5,280 AI polls, reached one natural
  result with no forced result, averaged 60.0 FPS game output, reported zero
  untranslated calls or runtime faults, exited cooperatively, and left no game
  process.
- The private QA harness now validates, hashes, forwards, and records its
  course-path input explicitly. The path data is not part of this repository
  or release.

These are targeted correctness and route acceptances, not proof that every
campaign, card, visual, controller, audio, or error branch is complete.

## Package and updater acceptance

- Archive: `Initial.D.Arcade.Stage.3.Recompiled.v2461.zip`
- ZIP SHA-256:
  `1B0ABCFDEFAF3F2A3E4CAC46D738259FF09AA4747AB825266A681097104AFAC5`
- Audited size: 24,067,516 bytes; 18 files.
- The archive contains one top-level folder named
  `Initial D Arcade Stage 3 Recompiled`. Its root entry point is
  `Initial D Arcade Stage 3 Recompiled.exe`; its native helper is
  `Initial D Arcade Stage 3 Recompiled Runtime.exe`.
- The real ZIP passed the current merge installer and the exact updater plus
  installer shipped in v2457. Both accepted version 2461, preserved protected
  user data and unrelated destination-only files, selected the canonical
  folder/runtime, and did not launch the game.
- A byte-identical `demo.exe` compatibility copy remains so older updaters can
  accept a direct jump to the newest release. The installed launcher removes
  only that exact duplicate and preserves modified or unrelated files.

The archive contains no game data, BIOS, PIC, CHD, cards, custom music, logs,
captures, learned paths, generated guest-code source, credentials, or private
host paths. Users must provide legally obtained matching game inputs locally.
