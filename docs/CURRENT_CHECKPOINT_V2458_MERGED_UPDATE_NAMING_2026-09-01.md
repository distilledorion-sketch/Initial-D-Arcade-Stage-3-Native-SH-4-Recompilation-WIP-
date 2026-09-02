# v2458 merged-update and product-naming checkpoint — 2026-09-01

v2458 standardizes the downloadable layout and verifies the update transition
from the already-published v2457 build.

## Names and merge behavior

- Archive: `Initial.D.Arcade.Stage.3.Recompiled.v2458.zip`
- Top-level folder: `Initial D Arcade Stage 3 Recompiled`
- Main entry point: `Initial D Arcade Stage 3 Recompiled.exe`
- Native helper: `Initial D Arcade Stage 3 Recompiled Runtime.exe`

The installer merges the verified package into the existing product folder.
It adds new product files and replaces versioned product files while preserving
`game files`, `card data`, `custom music`, `logs`, user settings, the automatic
update preference, and unrelated destination-only files. A failed install
restores overwritten files and removes product files created by the failed
attempt.

The v2458 archive includes one byte-identical `demo.exe` compatibility copy.
This is required because the updater already installed by v2457 validates that
legacy name before it can install the new scripts. After the transition, the
new installer/launcher removes the legacy file only if its SHA-256 equals the
canonical runtime; any modified or unrelated file is kept. Future packages can
drop the compatibility copy after the v2457 transition window.

## Accepted evidence

- Native executable SHA-256:
  `5636197D7CA5D3DEDEAA2FA7F016A2C6899516D5C8D30A74AD150636E53C49A2`
- ZIP SHA-256:
  `D2DA3CFA1113E9DDA7DE88C224C4E92DC4E73540C4F44868FBB10E4A6B89CD0F`
- ZIP size: 23,812,401 bytes
- Audited files: 18
- Public source checks: 18 passing tests
- Private build checks: ordered-path round trip/regression, 34
  restart/shutdown, 20 route-parser, 15 controller/music, translation-source,
  116-object link freshness, and standalone no-firmware checks pass.
- The actual archive passed the current installer acceptance with protected
  and destination-only files preserved, the canonical runtime hash verified,
  the legacy duplicate retired, and no game process launched.
- A separate acceptance used the exact updater and installer shipped inside
  v2457. It located the nested product folder, merged v2458, preserved user
  data, selected the canonical runtime, completed first-launch name migration,
  and launched no game process.

The archive contains no game data, BIOS, PIC, extracted assets, cards, custom
music, logs, captures, learned paths, generated guest translation bodies, or
private host paths. Users provide their own legally obtained matching inputs.

The learned-path correction prevents a spatially nearby future switchback from
becoming the steering authority ahead of the ordered cursor. The exact
parallel-switchback regression passes, but a live v2458 race past the previous
663 m stall is not claimed by this packaging checkpoint.

This remains unfinished early-demo software. Start at 60 FPS; higher-refresh
presentation and whole-game coverage remain experimental.
