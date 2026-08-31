# Current source-safe checkpoint — v2396

Date: 2026-08-30

This note reports private integration evidence without publishing game code,
game data, firmware, extracted assets, cards, captures, or the executable.

## Native architecture boundary

The product path is a static SH-4 AOT runtime for NAOMI 2. Its standalone
audit reports zero firmware callbacks, firmware AOT objects, firmware input
contracts, and cached firmware translations. The launcher prepares required
files only from matching user-owned game inputs and requires no installed
Python runtime.

## Custom BGM synchronization

The separate held-View host picker has been removed. During the authentic
opponent selector, a quick Change View press cycles cabinet entry 0/no music
and original streams 1–13. When a selected slot has a user replacement, the
small in-game label uses a sanitized version of that file's name. A cleared,
missing, or invalid replacement draws no host title and keeps the original.

A bounded BIOS-free route supplied two logical View edges during the real
course-specific rival-face selector. The same run proved:

- first press selected entry 0 with no custom title;
- second press displayed `FLY TO ME TO THE MOON AND BACK` for slot 1;
- the game's default request for stream 5 resolved to original stream 1;
- the replacement decoded successfully;
- the authentic AICA stream-start command activated the replacement mixer;
- Vulkan presentation held 60.0 Hz through live movement; and
- no fatal, access-violation, unimplemented-target, ELAN, or Vulkan fault was
  recorded before the intentional watchdog stop.

The example filename describes private test evidence only; no audio file is in
this repository.

## Link-freshness repair

An inline asset-loading shim was selected by the linker from one translated
COMDAT owner. Rebuilding only the ordinary product-facing translation units
left that owner stale, making a source change visible in tests but absent from
the executable. v2396 explicitly rebuilds the owning object.

The source-safe build verifier now rejects an owner object older than the
inline headers it owns. It identifies the historical stale build by exact
object name and accepts v2396 as fresh.

## Current coverage ledger

- route/branch targets loaded: 48/48;
- rival profiles with real movement: 32/32;
- Time Trial layouts with natural results: 16/16;
- rival profiles with natural results: 25/32; and
- four-course priority high-refresh matrix: passed with 119.8–120.0 FPS
  minimum presentation and no repeated endpoints in accepted moving windows.

The seven remaining natural rival-result proofs are `r_evo5`, `r_keisuke2`,
`r_kyoko`, `r_ryosuke2`, `r_sakamoto`, `r_sudo2`, and `r_wataru`.

## Verified private candidate

SHA-256:

`966F218E54CC2C3A112D174163C85B141DDA25FE96836729D310E8773C555E91`

The binary is intentionally not published.

## Honest remaining work

- Finish the seven remaining natural rival-result proofs and broader long-run
  campaign/error-path coverage.
- Complete physical wheel/force-feedback and audible end-user acceptance.
- Continue visual comparison across remaining car/course/weather combinations.
- Reduce remaining synchronous transition latency.
- Treat unlimited presentation as future work after the 120 FPS compatibility
  matrix is exhaustive.
