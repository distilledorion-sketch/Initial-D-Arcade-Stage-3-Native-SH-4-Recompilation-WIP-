# v2449 cinematic-mask widescreen checkpoint — 2026-09-01

This private integration checkpoint fixes a reproducible widescreen defect in
the game's authentic attract sequence. The downloadable public package remains
v2445 while v2449 receives broader route and packaging acceptance.

## Root cause

At 2560x1080, the 3D world correctly used Hor+ projection, but the game's black
cinematic mattes covered only the centered 1440-pixel-wide 4:3 image. World
geometry remained visible in the new left and right regions during camera cuts.

Draw inventories proved that the mattes were not direct-TA HUD sprites. They
were pairs of ordinary ELAN object-space quads with four vertices, a solid 8x8
texture, one exact material/raster state, and projected native coverage from
x=0 through x=640. A later close-up used the same mesh and material while
changing its camera focal length, so focal length was not a stable identity.

## Repair boundary

v2449 keeps the correction fail-closed:

- the draw must match the exact captured four-vertex mesh, zero-UV layout, and
  material/raster state;
- its CPU-projected bounds must independently prove native x=0..640 coverage
  and a large cinematic-band height;
- only then are its x coordinates expanded about the output centre by the
  active aspect compensation;
- HUD, cars, course geometry, and smaller presentation panels retain their
  established paths;
- at 4:3 the compensation is 1.0, so placement is unchanged.

The two four-point batches intentionally leave GPU object projection for this
small correction. All high-volume course and car projection remains on the GPU.

## Acceptance

- Final native executable SHA-256:
  `1B1E1990A588DE804CCB6FF19D0D920F60E3EAC038655EA79D39780E5270F479`.
- A BIOS-free, muted, BelowNormal-priority 2560x1080 attract run captured twelve
  scene points from 3300 through 3630. Both the initial narrow-road matte and
  later changing-zoom car close-ups reached the real left and right edges.
- Hor+ world rendering remained visible in the intended centre band and the UI
  remained unstretched.
- A separate native-resolution A/B captured scenes 3300, 3360, 3420, 3480,
  3540, and 3600 from the unchanged v2448 executable and final v2449. All six
  complete 640x480 BMP files had matching SHA-256 values, proving byte-exact
  native presentation across fixed and changing camera zoom.
- A normal visible Direct-Vulkan run on the main monitor used a 2560x1080
  internal render with no diagnostic GPU readback. After warm-up, presentation
  remained between 119.5 and 120.5 FPS with a 120.0 Vulkan average. Moving
  attract buckets produced approximately 239–241 distinct frames per two
  seconds; static boot/menu periods correctly contained cadence repeats.
- All bounded runs closed cooperatively with exit code 0. No game process or
  unclean Direct-Vulkan marker remained.
- The complete controller, attached-Xbox, Windows audio endpoint, AICA,
  custom-music, interpolation, Direct-session, card, ELAN, offscreen Vulkan,
  guest-timing, lifecycle, link-freshness, translation-integrity, and
  standalone suite passed.
- The standalone audit found zero firmware callbacks, firmware AOT objects,
  firmware input contracts, or cached firmware translations.

## Current limits

This accepts one exact attract sequence and its observed zoom variants; it is
not whole-game visual certification. Remaining cars, courses, modes,
weather/time combinations, transition overlays, high-refresh visual behavior,
physical wheels/controllers, audio listening, and cross-driver shutdown stress
still require systematic same-build coverage. No v2449 binary or game-derived
capture is published in this source checkpoint.
