# v2446 strict route-evidence checkpoint — 2026-09-01

This checkpoint corrects the superseded v2445 route-QA interpretation. It
changes developer-only diagnostics and private acceptance tooling; the current
public package remains v2445.

## Evidence meaning

The broader 70-row ledger proves that condition tokens track weather:

- `_etc_f` is the dry condition mesh;
- `_etc_r` is the wet or snow condition mesh;
- both tokens occur for both left and right menu positions;
- primary course suffixes similarly track day/night on affected courses.

Therefore neither the condition mesh nor the primary table can prove road
direction. Schema 5 accepts Time Attack direction only when three independently
isolated persistent guest selector fields are all present and equal the
manifest's Left=0 or Right=1 value. Missing, partial, or contradictory fields
fail closed.

## Diagnostic repair

The first selector snapshot recognized only a primary path ending in
`_pol.tbl`. Several courses load segmented `_pol_a.tbl`, `_pol_b.tbl`, and
`_pol_c.tbl` tables instead, so no direction identity was emitted.

v2446 recognizes the first `_pol_a.tbl` segment as the same primary-course
boundary. The code remains gated by `IDAS3_DIAG_COURSE_SELECTION`; normal
product launches do not read or report the private guest selector fields.

## Acceptance

- Native executable SHA-256:
  `BB453D859F3D7DFD505CAC24501E16B7D17DEBC44ED0B2A7D557574923CB58C8`.
- All 116 linked objects passed freshness checks; 108 direct source owners
  were checked with zero stale objects.
- The standalone audit found zero firmware callbacks, firmware AOT objects,
  firmware input contracts, or cached firmware translations.
- The full controller, attached-Xbox, audio endpoint, AICA, custom-music,
  interpolation, Direct-session, card, ELAN, offscreen Vulkan, guest-timing,
  lifecycle, and policy suite passed.
- The route parser/policy suite passed 20 focused contracts.
- Akina Snow left/snow/night loaded `k_df3`, condition `r`, night/snow assets,
  and direction identity `0,0,0`; movement reached 7.964 m and exit was 0.
- Happogahara left/dry/night loaded `s_vh`, condition `f`, night/dry assets,
  and direction identity `0,0,0`; movement reached 8.218 m and exit was 0.

## Current limits

The 70-row route ledger still mixes executable checkpoints. Exact
v2444/schema-5 evidence accepts all eight Bunta rows and two Time Attack rows
whose original diagnostics were complete. Exact v2446/schema-5 evidence now
accepts two segmented Time Attack rows. Remaining rows require renewal on one
build before the matrix can be described as current-build complete.
