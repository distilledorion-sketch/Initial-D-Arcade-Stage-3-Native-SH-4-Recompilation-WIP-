# Current checkpoint — v2449 rival-result renewal

## Scope

This checkpoint renews the retained Legend/Bunta rival ledger on the exact
final v2449 BIOS-free native executable. It does not upload the executable,
game data, generated guest translations, disposable cards, captures, or raw
private logs.

- Executable SHA-256:
  `1B1E1990A588DE804CCB6FF19D0D920F60E3EAC038655EA79D39780E5270F479`
- Native guest cadence: 60 Hz
- Presentation: Direct Vulkan, 640x480, 60 Hz, VSync enabled
- Safety profile: muted, offscreen/background, BelowNormal process priority,
  one PVR raster thread, cooperative exact-process shutdown
- Product boundary: cold BIOS-free start; no firmware handoff or interpreter
  fallback is used

## Accepted target results

| Target | Completed races | Forced prerequisites | Natural target results | Player travel | Display min/avg | Vulkan min/avg | Errors | Runtime faults |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| Sakamoto | 6 | 5 | 1 | 314.508 m | 59.7 / 60.0 FPS | 59.6 / 60.0 FPS | 0 | 0 |
| Wataru | 7 | 6 | 1 | 243.602 m | 59.6 / 60.0 FPS | 59.7 / 60.0 FPS | 0 | 0 |

The outcome helper advanced only prerequisite races on a disposable QA card.
For both accepted targets, the log first recorded the natural-target arm gate
and then recorded zero requested and zero applied helper outcomes after that
line. The target race therefore had to produce its own game-owned result after
positive player travel. Each run stopped only after that result, requested the
native presenter's cooperative close path, exited code 0, and removed its
Direct-session marker.

## Honest coverage statement

The historical accepted ledger contains natural-result evidence for all 32
retained rival profiles: 30 Legend profiles and two Bunta profiles. Those runs
span older accepted checkpoints. The same-build final-v2449 renewal is **2/32**
at this checkpoint, so this document does not claim that the full rival ledger
has already been renewed on v2449.

The exact-v2449 Time Attack work is further ahead: all 16 retained layouts have
already completed naturally on this executable, and the separate route-state
matrix has passed all 70 defined Time Attack/Bunta condition rows.

## Remaining gate

Renew the other 30 target profiles on this exact executable, strictly recheck
target-arm/result ordering and metrics, fix any same-build regression found,
and publish only the aggregate source-safe evidence. Whole-game readiness still
also requires broader visual/audio/input/persistence acceptance and repeated
cross-driver shutdown stress.
