# Current checkpoint — v2411 public early demo

Date: 2026-08-31

This checkpoint publishes an unfinished Windows x64 playtest package while
keeping all user-owned and legally restricted inputs out of the repository and
release asset.

## Download and integrity

- Release: [Public Early Demo v2411](https://github.com/distilledorion-sketch/Initial-D-Arcade-Stage-3-Native-SH-4-Recompilation-WIP-/releases/tag/v2411-public-early-demo)
- Public ZIP SHA-256:
  `FE0DC457D40FD1D5E7B9AE472848533BEC933F70343159F8955ADF8FE6C2930A`
- Native `demo.exe` SHA-256:
  `1EE34D3B9BCCEA7CDE2EC2CD192D1A40005BDF1041065142080BE8CCEA9A4155`

The exact uploaded ZIP contains 14 launcher/runtime/setup files totaling
27,989,307 uncompressed bytes. A pre-publication audit found no CHD, PIC, BIOS,
extracted HOSTFS, game image, card save, custom song, user log, or personal
filesystem path. GitHub reports the same SHA-256 digest for the uploaded asset.

## Native architecture boundary

The product executes statically translated SH-4 code and native NAOMI 2
services. It does not use an SH-4 interpreter, JIT fallback, or a NAOMI 2 BIOS.
The first-run utility verifies a matching user-owned `gds-0033.chd` and
`317-0384-com.pic`, then locally extracts only the required program/assets.
Python is not required by the public launcher.

The standalone product audit reports no firmware callbacks, firmware AOT
objects, firmware input contract, or cached firmware translation dependency.

## v2411 changes

### Controller input

- Added a real XInput path for Xbox-compatible controllers while retaining
  DirectInput for wheels and other legacy devices.
- Auto input prefers XInput when an Xbox-compatible pad is present.
- Persisted independent XInput and DirectInput mappings for steering,
  accelerator, brake, shifts, Start, View, and Coin.
- Xbox left stick and independent trigger axes use normalized capture and
  remapping semantics covered by an offline regression.
- A public-playtest report now confirms that at least one Xbox controller is
  not detected by v2411. The provider-neutral mapping regression passed, but it
  did not exercise live device discovery. Controller detection is therefore a
  known release defect, not a validated feature; keyboard is the fallback
  until the XInput discovery/launcher path is repaired.

### Custom music

- Removed guessed cross-slot redirection. A replacement now suppresses only
  the exact game stream slot that actually opened.
- A cleared, disabled, or missing replacement always restores the original
  game stream.
- The displayed replacement title comes from the active stream rather than a
  guessed input/menu selection.
- Original game assets are never edited or overwritten.

### Card selection

- Cards live in the visible `card data` folder and use the
  `InitialD_name_card.card` naming convention.
- The last selected card becomes the next launch default.
- Selecting or creating a different card displays a Yes/No warning that the
  game will restart and unsaved data will be lost; No is the safe default.
- The guest remains responsible for creating/writing card contents.

### Host shutdown safety

- Closing first requests an end to new Vulkan presentation work.
- The main thread keeps the HWND/surface alive while the raster worker exits,
  pumps pending window messages, joins the worker, and only then destroys the
  window.
- The previous forced `TerminateProcess` fallback was removed because ending a
  process inside a display-driver present call was associated with host black
  screens.
- Driver-operation diagnostics identify acquire, fence, submit, and present
  waits if a cooperative shutdown is slow.

This source/offscreen evidence is meaningful but not a claim that every GPU or
driver survives every live WSI stress case. A full live stress pass remains
pending; 60 FPS is the recommended default.

## Offline verification

The v2411 Windows x64 product linked 116 objects, passed freshness checks for
108 source owners, and passed the source-safe/offscreen suite covering:

- controller binding normalization and capture;
- exact-slot custom music routing;
- high-refresh presentation interpolation guards;
- ELAN, card, AICA, and offscreen Vulkan behavior;
- card-eject branch classification;
- controller/music and restart/shutdown source policy;
- translated-source integrity and linked-owner freshness; and
- standalone no-firmware dependency verification.

No full game window or Win32 Vulkan surface was opened for the final offline
packaging pass.

## Coverage evidence

- 48/48 defined route/branch targets loaded.
- 32/32 rival profiles produced real movement.
- 16/16 unique Time Attack layouts reached natural game-owned results.
- 32/32 Legend rival profiles reached natural game-owned results.
- A 70-row Time Attack/Bunta condition ledger contains passing route-load and
  minimum-movement evidence.

The 70-row ledger mixes v2400 and v2401 evidence. In particular, it is not a
complete eight-row Bunta rerun on one v2411 executable, and its PASS condition
does not prove a full natural race completion. Those are still required before
claiming exhaustive whole-game coverage.

## Honest remaining work

- Broader clean-machine testing of the public launcher and extraction flow.
- A same-build regression across all Time Attack and Bunta combinations.
- Visual comparison across remaining cars, courses, weather, day/night, HUD,
  mirror, transparency, texture, and clipping combinations.
- Repair and validate Xbox-controller discovery, then complete physical
  wheel/force-feedback acceptance.
- Audible end-user audio acceptance.
- Long campaign, card error/recovery, continue, loss, and persistence branches.
- Stable 120 Hz coverage beyond the measured priority routes.
- Unlimited presentation independent of guest gameplay timing.
- Live Vulkan/WSI shutdown stress without any display-driver or host failure.

The v2411 package is suitable for cautious public playtesting. It is not a
finished release and should not be described as whole-game complete.
