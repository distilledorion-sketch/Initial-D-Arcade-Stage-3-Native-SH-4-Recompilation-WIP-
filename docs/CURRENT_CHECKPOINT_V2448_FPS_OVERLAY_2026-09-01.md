# v2448 honest-FPS overlay checkpoint — 2026-09-01

This private integration checkpoint corrects the in-game performance counter
and validates the exact fast Direct-Vulkan path. The current downloadable
public package remains v2445.

## Counter repair

The earlier overlay mixed two clocks: renderer submissions updated `OUT`,
while the presentation worker updated `NEW`. Those values also crossed threads
through ordinary non-atomic doubles. The overlay could consequently show
`OUT060 NEW000` while the swapchain telemetry proved 120 Hz output.

v2448 samples both values on the presentation thread and publishes them
atomically:

- `OUT` counts successful displayed swapchain presentations;
- `NEW` counts fresh authentic game endpoints and valid interpolated
  midpoints;
- exact cadence repeats and overlay-only re-presents do not increment `NEW`.

The interpolation/rendering flag remains separate from the distinct-frame
accounting flag, so this repair does not change game timing, physics, scene
generation, or interpolation eligibility.

## Acceptance

- Native executable SHA-256:
  `AA068705DB38295967802DBEDAFC2E643A83119F763755D97428567C0DE1DC64`.
- All 116 linked objects passed freshness checks; 108 direct source owners
  were checked with zero stale objects.
- The standalone audit found zero firmware callbacks, firmware AOT objects,
  firmware input contracts, or cached firmware translations.
- The complete controller, attached-Xbox, audio endpoint, AICA, custom-music,
  interpolation, Direct-session, card, ELAN, offscreen Vulkan, guest-timing,
  lifecycle, translation, freshness, and standalone suite passed.
- Focused checks passed: 33 lifecycle/renderer policies, 20 route-parser
  policies, and 12 analyzer contracts.
- A normal visible Direct-Vulkan run used a 2560x1080 internal render,
  authentic 60 Hz gameplay, 120 Hz presentation, VSync, the main monitor,
  normal process priority, and no diagnostic GPU readback.
- Across 21 steady moving-race samples, display and Vulkan minimum/average
  were all 120.0 FPS. Every two-second sample contained at least 240 distinct
  frames, the repeat-counter delta was zero, and recognized faults were zero.
- The Akagi route advanced 129.204 m and closed cooperatively with exit code 0.
  No game process or unclean Direct-Vulkan session marker remained.

## Analyzer compatibility

Now that authentic menu frames correctly count as distinct output, generated
activity alone cannot identify the start of a race. Current logs are therefore
anchored to explicit player-motion telemetry and positive travel. Historical
logs without that telemetry retain the older generated-frame fallback.

## Current limits

This is strong acceptance for one measured route and host, not whole-game 120
Hz certification. Same-build renewal of the route matrix, remaining visual
artifact isolation, physical wheel/controller acceptance, audio listening
coverage, additional GPU/driver shutdown stress, and clean-machine packaging
remain open.
