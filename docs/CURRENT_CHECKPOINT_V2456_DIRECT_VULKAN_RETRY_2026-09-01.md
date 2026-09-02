# v2456 Direct-Vulkan retry checkpoint — 2026-09-01

## Scope

This public early-demo checkpoint corrects Direct-Vulkan clean-session marker
handling and carries forward the v2454 card-reinsert and verified-updater work.
It remains unfinished playtest software. Safe Vulkan at 60 FPS remains the
default.

The runtime is the BIOS-free static-AOT NAOMI 2 path. It contains no SH-4
interpreter or JIT fallback and does not boot through a NAOMI 2 BIOS.

## Direct-Vulkan correction

The runtime arms a marker while Direct Vulkan is active so an interrupted
session can return the next launch to Safe presentation. The earlier F1 Apply
path could see the marker created by its own current process and incorrectly
treat that healthy session as a previous crash.

v2456 distinguishes the current process's armed marker from a marker inherited
from an earlier interrupted session. A genuinely stale marker now presents an
explicit choice:

- **Yes** removes the stale marker and restarts into Direct Vulkan.
- **No** keeps Safe Vulkan.
- A failed marker removal reports that the demo folder must be writable.

The source/offline restart policy passes. A live end-to-end acceptance of the
corrected F1 prompt on the packaged v2456 executable is still pending and is
not claimed here.

## Developer-only route QA

The private integration adds persisted Time Attack player-path capture and
playback to reproduce long route-specific behavior through ordinary 60 Hz
player control. Its binary format and synthetic round trip pass, and playback
recovery now advances from the current live control tick rather than the
capture's final tick.

This is QA support, not shipped game content. No captured path is included in
Git or the public package. A live retest beyond the previously observed 663 m
stall remains open.

## Verification

- Native executable SHA-256:
  `767AA71205F9C45C722FB4857516A1B0AEC822CC64F8FCE8F865E4243E82B8EC`
- Compilation: pass.
- Exact synthetic path-cache capture/playback test: pass.
- Route-matrix parser: 20 checks pass.
- Controller/music policy: 15 checks pass.
- Restart/shutdown policy: 33 checks pass.
- Presenter-owner link freshness: pass.
- Standalone no-firmware audit: zero firmware callbacks, firmware AOT objects,
  firmware input contracts, and cached firmware translations.

These are focused engineering results, not whole-game completion evidence.

## Public package audit

- Asset: `Public.Early.Demo.v2456.zip`
- SHA-256:
  `F5A101272EC7B8EFB04C89EAD4ADBCC2F84CD5CB74C12DE2B94C7C9109398B88`
- Size: 18,577,242 bytes.
- Entries: 17 files.
- Embedded executable hash matches the native executable hash above.
- `PRODUCT_VERSION.txt` contains `2456`.

The package contains no CHD, PIC, BIOS, game data, extracted assets, card
saves, custom music, logs, captures, learned paths, generated guest translation
units, credentials, or private host paths. Users must provide their own legally
obtained matching game inputs. Python is not required.

## Open acceptance work

- Live acceptance of the corrected F1 Direct-Vulkan retry flow.
- Live learned-path playback beyond the existing 663 m Time Attack stall.
- Broader controller, wheel, audio, Vulkan-driver, restart, route, campaign,
  replay, card-error, visual, and higher-refresh coverage.
- Completion of remaining untranslated/indirect paths and removal of
  compatibility-only diagnostics.
