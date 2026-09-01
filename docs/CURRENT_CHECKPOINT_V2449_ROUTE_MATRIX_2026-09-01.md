# v2449 complete route-matrix checkpoint — 2026-09-01

The complete retained Time Attack/Bunta route-state matrix now passes on one
exact BIOS-free native executable. This closes the earlier mixed-build evidence
gap without publishing private logs, replay inputs, or game-derived data.

## Exact build boundary

- Native executable SHA-256:
  `1B1E1990A588DE804CCB6FF19D0D920F60E3EAC038655EA79D39780E5270F479`.
- Evidence schema: 5 for all rows.
- Total: 70/70 pass, 0 fail, 0 missing.
- Modes: 62 Time Attack rows and eight Bunta rows.
- Course families: Myogi, Usui, Akagi, Akina, Irohazaka, Akina Snow,
  Happogahara, Shomaru, and Tsuchisaka.

## What each pass proves

Every Time Attack row independently requires:

- the expected primary course asset, including segmented Shomaru and
  Tsuchisaka geometry;
- the correct dry or wet/snow condition mesh;
- the requested day or night state and matching rain/snow activity;
- three separate guest direction-selector fields that all agree with the
  requested left or right choice;
- authentic player-car movement beyond the acceptance threshold;
- zero recognized runtime fault markers and cooperative exit code 0.

Every Bunta row requires the expected game-fixed course/time/weather state,
the Bunta driving route to be applied, authentic player-car movement, zero
recognized runtime faults, and cooperative exit code 0.

The original isolated runs used muted, BelowNormal-priority CPU preview with
one process at a time, a cooldown between cases, and cooperative-only watchdog
closure. No live Vulkan WSI surface was opened for this route-state campaign.
Afterward, a no-launch reanalysis reparsed all retained logs, accepted only
evidence tied to the exact executable hash above, and again returned 70/70.

## What this does not prove

This checkpoint proves route selection, state identity, race entry, and initial
real movement. It does not claim that all 70 rows were driven to a natural
finish, that every car/opponent/campaign branch was exercised, or that every
visual, audio, controller, card, result, continuation, and save behavior is
correct. Live Vulkan performance and high-refresh acceptance remain separate
measured campaigns.

No executable, game files, generated guest translations, logs, replay inputs,
captures, card data, or custom music are published in this source checkpoint.
