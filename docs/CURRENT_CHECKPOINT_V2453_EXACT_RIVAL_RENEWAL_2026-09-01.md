# Current checkpoint — v2453 exact rival-result renewal

## Scope

This checkpoint moves the conservative rival-result renewal to the exact v2453
BIOS-free native executable. It publishes source-safe verification tooling and
aggregate evidence only. The executable, game data, generated guest
translations, disposable cards, captures, and raw private logs remain excluded.

- Executable SHA-256:
  `4297DD4906E5A12CD474BC4496E7CF76F907E17CBBB19F92CD6178A09FB3FBD8`
- Native guest cadence: 60 Hz
- Presentation: Direct Vulkan, 640x480, 60 Hz, VSync enabled
- Safety profile: muted/background, BelowNormal process priority, one PVR
  raster thread, one exact process at a time, cooperative presenter shutdown
- Product boundary: cold BIOS-free static-AOT start; no firmware handoff,
  SH-4 interpreter, or JIT fallback

The v2450-v2453 development sequence corrected natural-detour attempt
bookkeeping and added bounded, developer-only selector-window/pause controls.
Those controls default off. While a QA outcome pause is active, v2453 also
suppresses already-generated legacy overlay Start/Accelerator pulses so the
pause is real rather than merely cosmetic. Ordinary product input behavior is
unchanged when the controls are not explicitly enabled.

## Strict exact-candidate results

Each accepted row meets all of these gates:

- the run summary and live executable match the SHA-256 above;
- the exact target arms naturally exactly once;
- the target travels at least 10 m through normal input/physics;
- `RESULT.bin.nz` loads after the natural arm;
- no developer outcome request or apply occurs after the arm;
- Direct Vulkan device initialization and post-arm display/present samples are
  present;
- no fatal, exception, crash, access-violation, unimplemented, or parser
  fail-soft signal occurs;
- cooperative close leaves zero candidate processes and no Direct-session
  marker.

| Target | Natural travel | Display min/avg | Vulkan min/avg | Natural RESULT loads | Post-arm developer writes | Fault signals |
|---|---:|---:|---:|---:|---:|---:|
| Evo 5 | 289.136 m | 59.8 / 60.0 FPS | 59.8 / 60.0 FPS | 1 | 0 | 0 |
| Evo 6 | 301.538 m | 59.8 / 60.0 FPS | 59.9 / 60.0 FPS | 1 | 0 | 0 |
| Keisuke rematch | 787.215 m | 59.8 / 60.0 FPS | 59.9 / 60.0 FPS | 1 | 0 | 0 |
| Kyoko | 256.297 m | 59.8 / 60.0 FPS | 59.9 / 60.0 FPS | 1 | 0 | 0 |
| Ryosuke rematch | 360.507 m | 59.8 / 60.0 FPS | 60.0 / 60.0 FPS | 1 | 0 | 0 |
| Sakamoto | 314.508 m | 59.9 / 60.0 FPS | 59.9 / 60.0 FPS | 1 | 0 | 0 |
| Smiley | 295.724 m | 59.9 / 60.0 FPS | 59.8 / 60.0 FPS | 1 | 0 | 0 |
| Sudo rematch | 888.622 m | 59.8 / 60.0 FPS | 59.9 / 60.0 FPS | 1 | 0 | 0 |
| Wataru | 243.602 m | 59.9 / 60.0 FPS | 59.9 / 60.0 FPS | 1 | 0 | 0 |

The Keisuke route also exercised natural loss/retry detours for Ryosuke and
Evo 5 before the finale. The Sudo route proved a selector detent change that
began only after its fifth prerequisite result. Both then completed naturally
without a post-arm developer result write.

## Honest coverage statement

The historical accepted ledger still contains natural-result evidence for all
32 retained rival profiles: 30 Legend profiles and two Bunta profiles. Those
runs span multiple accepted binaries. The exact-v2453 ledger is **9/32**, not
32/32. This checkpoint deliberately keeps those two claims separate.

The broad historical coverage ledger also reports 48/48 defined route/branch
targets, 32/32 rival movement, and natural results for all 32 rival profiles.
It remains useful regression evidence, but it is not proof that every target
has been renewed on v2453.

## Published QA tools

- `tools/qa/verify_strict_natural_rival_result.ps1` audits one natural target
  and fails closed on an executable mismatch, weak result evidence, post-arm
  helper write, runtime fault, live process, or stale Direct-session marker.
- `tools/qa/update_exact_candidate_natural_coverage.ps1` evaluates summaries
  tied to one executable SHA-256 and retains the newest strict pass per target.
  Failed attempts cannot silently replace a valid pass or inflate the count.

## Remaining gate

Renew the other 23 rival targets on this exact executable, strictly recheck
target-arm/result ordering and metrics, and fix any same-build regression found.
Whole-game readiness still requires broader visual, audio, controller/wheel,
card/error-branch, cross-driver, and repeated close/restart acceptance.
