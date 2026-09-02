# v2459 post-result translation checkpoint — 2026-09-01

This prerelease keeps the stable product identity introduced in v2458 and
advances the tested static-AOT route through the Akina Time Attack result/card
transition.

## Runtime acceptance

- Native executable SHA-256:
  `C31ACF51EFB3DE210381E9754B4D3C702C770EC1CD7258DA2CD663E607CF0C5E`
- The mechanically verified `0x0C18FE40` guest helper spans 56 bytes, has 28
  reachable SH-4 instructions, one normal return, no unknown instructions, and
  no unsafe control-flow edge.
- A bounded offscreen 640x480 Vulkan run used authentic 60 Hz guest timing,
  60 FPS presentation, one raster worker, muted audio, and Below Normal process
  priority. It passed the previous 663.071 m learned-path stall, reached
  1,488.022 m, and entered a natural game-owned result without outcome
  injection.
- Race display minimum/average were 59.8/60.0 FPS. The complete run reported
  zero repeated presentation frames, zero recognized errors/runtime faults,
  cooperative exit code 0, and no remaining game process.

This is one targeted route acceptance, not proof that every race, card, or
error branch is complete.

## Package and updater acceptance

- Archive: `Initial.D.Arcade.Stage.3.Recompiled.v2459.zip`
- ZIP SHA-256:
  `B42F76F6D15D6CDE1014B94390755F336B263997ADF7AC73ED5F69C9DB39D5B2`
- Audited size: 23,813,057 bytes; 18 files.
- The archive contains one top-level folder named
  `Initial D Arcade Stage 3 Recompiled`. Its root entry point is
  `Initial D Arcade Stage 3 Recompiled.exe`; its native helper is
  `Initial D Arcade Stage 3 Recompiled Runtime.exe`.
- The real ZIP passed the current merge installer and the exact updater plus
  installer shipped in v2457. Both accepted version 2459, preserved protected
  user data and unrelated destination-only files, selected the canonical
  folder/runtime, and did not launch the game.
- A byte-identical `demo.exe` compatibility copy remains so pre-v2458 updaters
  can accept a direct jump to the newest release. The installed launcher
  removes only that exact duplicate and preserves modified or unrelated files.

The archive contains no game data, BIOS, PIC, CHD, cards, custom music, logs,
captures, learned paths, generated guest-code source, credentials, or private
host paths. Users must provide legally obtained matching game inputs locally.
