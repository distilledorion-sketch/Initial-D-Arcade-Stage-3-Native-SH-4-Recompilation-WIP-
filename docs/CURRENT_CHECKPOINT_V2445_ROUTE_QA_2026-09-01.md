# v2445 route-evidence QA checkpoint — 2026-09-01

> Superseded: broader course evidence disproved this checkpoint's interim
> interpretation of `_etc_f`/`_etc_r`. See
> [the v2446 correction](CURRENT_CHECKPOINT_V2446_ROUTE_QA_2026-09-01.md).

This checkpoint changes QA policy and evidence handling only. The distributed
native executable remains the accepted BIOS-free v2444 binary packaged by the
v2445 launcher release.

## What was corrected

Two fresh Time Attack probes loaded the intended course, exact condition mesh,
time/weather assets, and real player motion, then exited cooperatively with no
fault marker. The matrix nevertheless marked them failed because their guest
paths did not emit an optional three-field course-selector diagnostic.

This checkpoint temporarily accepted an exact condition mesh as direction
proof:

- left requires `_etc_f`;
- right requires `_etc_r`;
- a missing or wrong condition mesh fails;
- a partially emitted selector identity fails;
- a complete selector identity must agree with the manifest and mesh;
- absence of the optional identity alone does not fail.

The implementation also normalizes every row to one ordered schema before CSV
export. This prevents PowerShell from selecting a legacy first-row property
set and silently dropping newer evidence columns.

## Safety and acceptance

The focused parser/policy suite passes 18 contracts, including both missing-
identity cases and all fail-closed contradiction cases. An offline reanalysis
mode can upgrade an existing log only when its row already records the exact
requested executable SHA-256; it cannot relabel evidence from another build.

Akagi left/dry/day and Akina right/wet/night were rerun fresh on v2444. They
advanced 8.612 m and 8.083 m respectively, produced no recognized fault, and
closed normally with exit code 0. Six other exact-build rows were then parsed
again offline, with the game-process count remaining zero before and after.

The accepted count in this superseded checkpoint must not be used as current
direction coverage. The underlying course, condition, movement, fault, and
cooperative-exit observations remain useful; direction was reclassified under
the stricter schema-5 policy.

## Remaining work

The retained matrix has 70 rows and still needs one-build renewal.
These are load/condition/movement checks, not claims that every race has been
driven to a natural finish or that every visual/audio frame is final. Live
Direct-Vulkan acceptance remains separate from conservative CPU-preview route
coverage.
