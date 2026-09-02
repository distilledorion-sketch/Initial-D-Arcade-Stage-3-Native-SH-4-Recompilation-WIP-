# v2457 Direct-authoritative public checkpoint — 2026-09-01

v2457 changes how the Windows runtime respects the selected Vulkan presenter.
If Direct is saved, the runtime now uses Direct automatically. A stale
diagnostic session marker is cleared when possible, but neither a stale marker
nor unavailable marker maintenance opens a prompt or silently changes the
renderer to Safe. Safe remains the first-run default and a manual F1 setting.

## Verification

- Native executable SHA-256:
  `CCDF9A0C3E195C8845CBFF0D8224D07854855D47CAC9DE4452F6598BB1581F94`.
- Compilation and the full bounded build script completed successfully.
- Restart/shutdown policy suite: 34 passing checks.
- Route parser: 20 passing checks.
- Controller/music policy: 15 passing checks.
- Synthetic developer path capture/playback round trip, translation-source
  verification, 116-object link freshness, and the standalone no-firmware
  audit passed.
- The no-firmware audit reported zero firmware callbacks, firmware AOT
  objects, firmware input contracts, and cached firmware translations.
- A live isolated updater acceptance queried GitHub Releases, selected v2456
  from an older installation, downloaded and verified the published asset,
  installed it, preserved user-owned data and settings, and confirmed that a
  current installation is not offered the same version. It did not launch the
  game.

No live Direct-renderer run was performed for v2457, so this checkpoint does
not claim same-build GPU/driver acceptance. Earlier bounded Direct evidence is
retained in the status ledger; broader driver and restart stress remains open.

## Public package

- Asset: `Public.Early.Demo.v2457.zip`
- ZIP size: 18,576,649 bytes
- ZIP SHA-256:
  `5E6785CAC48C1BFEA6509E0AAADF28C9B2F1639969806A85093D103081346A2C`
- Entries: 17

The archive contains the launcher, native executable, local setup helpers,
public defaults, version marker, updater, and documentation. It contains no
game data, BIOS, PIC, extracted assets, cards, custom music, logs, captures,
learned driving paths, generated guest translation bodies, or private host
paths. Users must provide their own legally obtained matching game files.

This remains unfinished early-demo software. Use the default 60 FPS mode first;
higher-refresh presentation and whole-game coverage are still experimental.
