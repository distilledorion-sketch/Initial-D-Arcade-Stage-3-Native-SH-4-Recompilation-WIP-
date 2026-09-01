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

## Direct-Vulkan natural-finish renewal

The 16 distinct retained Time Attack layout routes were then renewed on the
same final v2449 executable with normal Direct Vulkan presentation at
2560x1080 and 60 Hz. Each case ran BIOS-free, muted, at BelowNormal process
priority, with one raster thread and only one game process at a time.

Acceptance required positive player-car travel followed by the game's later
`RESULT.bin.nz` transition, exactly one natural result, no developer-forced
outcome marker, zero recognized errors/runtime faults, and a cooperative clean
Direct-session shutdown. The strict no-launch audit returned:

- 16/16 natural-finish passes and zero failures;
- 85.590-286.250 m of authentic travel per route;
- 59.5 FPS or better race-display minimum and 60.0 FPS average;
- 59.8 FPS or better Direct-Vulkan minimum and 60.0 FPS average; and
- zero forced outcomes, zero recognized faults, and no game process left
  running after the campaign.

## What this does not prove

The 70-row matrix proves route selection, state identity, race entry, and
initial real movement. The separate natural-finish renewal proves complete
results for all 16 retained Time Attack layout routes, but does not claim that
all 70 direction/weather/time rows were individually driven to a natural
finish, that every car/opponent/campaign branch was exercised, or that every
visual, audio, controller, card, continuation, and save behavior is correct.
High-refresh acceptance remains a separate measured campaign.

No executable, game files, generated guest translations, logs, replay inputs,
captures, card data, or custom music are published in this source checkpoint.
