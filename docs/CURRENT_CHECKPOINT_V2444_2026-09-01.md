# Current integration and public checkpoint — v2444

Date: 2026-09-01

Scope: product-shared audio/XInput acceptance and portable managed-card paths

Native executable SHA-256: `49BEFEBCD2D23D97CF017EDB7F9F4E5E3685E23AA3994CA774EFE333EBCC4C5D`

Public ZIP SHA-256: `BEFE5D96AD8467C63E2A767B920AC523447ED42C3C849A3B817E051456E24515`

## What changed

- Managed cards selected or created in F1 are persisted by filename under the
  product-local `card data` folder. Moving or renaming an extracted demo no
  longer strands the selection behind an absolute path to its old location.
- All three maintained Windows launchers recognize older absolute settings and
  repair them to the same named local card when appropriate.
- XInput runtime loading, slot discovery, state normalization, and cabinet
  conversion are shared by the product and a public no-window probe. Axis
  remapping accepts deliberate 35% travel and the F1 menu accepts controller
  navigation.
- The product's 44.1 kHz, stereo, signed-PCM16 wave format and endpoint query
  are shared with a public probe. The probe opens and closes the endpoint
  without submitting audio.

## Accepted evidence

The complete v2444 suite passed:

- controller normalization, button/axis capture, and steering filtering;
- a product-shared probe against an attached Xbox controller through
  `xinput1_4.dll`;
- AICA mailbox and Flycast-derived SGC behavior;
- the product-shared Windows audio endpoint and exact custom-track routing;
- distinct 60-to-120 Hz presenter interpolation and Direct-session marker
  lifecycle;
- ELAN/card behavior and off-screen Vulkan on the attached RX 9070 XT;
- guest timing, card-eject classification, policy checks, source translation
  integrity, linked-owner freshness, and the standalone no-firmware boundary.

The standalone audit found zero firmware callbacks, firmware AOT objects,
firmware input contracts, or cached firmware translations. No game window was
created for this v2444 acceptance run.

A preceding 180-second cold run used the real AICA mix path with a null host
sink. It produced 7,080,718 non-silent post-mix frames, peak magnitude 32768,
mean absolute magnitude 5125.22, zero AICA dropped samples, cooperative exit
code 0, and no remaining process.

## Public package audit

`Public Early Demo v2444.zip` is 18,565,999 bytes and contains 14 files. The
archive contains no game image, BIOS, security PIC, CHD, extracted asset, card
save, custom music, log, frame capture, generated guest translation,
credential, or private filesystem path. It asks the tester to provide matching
legally obtained game inputs and prepares them locally without Python or a
NAOMI 2 BIOS.

## Scope note

This remains an early playtest checkpoint. The accepted results are targeted
engineering evidence; they are not a claim of exhaustive completion across
every course, car, weather, time-of-day, controller, audio endpoint, GPU, or
high-refresh path.
