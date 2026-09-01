# Current checkpoint — v2441 static topology cache

Date: 2026-09-01

This is an integration checkpoint. The latest downloadable public early demo
remains v2415 while v2441 receives broader route and hardware testing.

## What changed after v2440

v2440 already moved projection, ELAN lighting, normalized depth, packed
geometry/index streams, object uniforms, and Direct presentation onto bounded
Vulkan paths. Profiling then isolated a smaller remaining host cost: unchanged
course and car batches repeatedly scanned every retained point to rediscover
the same connector, fan, point-validity, and UV-readiness answers before
submitting already-packed geometry.

v2441 stores those immutable predicates beside the exact-matched static point
cache. A changed vertex/material payload, unsupported topology, animation,
screen-space/HUD draw, diagnostic mode, modifier path, or failed eligibility
check still takes the established full validation and rebuild path.

## Controlled timing result

Seven fresh-process samples of the warmed 16,384-vertex static reuse case:

| Metric | v2440 median | v2441 median | Change |
| --- | ---: | ---: | ---: |
| Batch loop | 0.089 ms | 0.066 ms | -25.8% |
| Topology preparation | 0.064 ms | 0.041 ms | -35.9% |
| Complete synthetic frame | 0.696 ms | 0.629 ms | -9.6% |

These are controlled renderer timings, not a claim that every game screen is
9.6% faster. The preceding v2440 live Direct run had already sustained
119.7–120.3 visible presents per second through a heavy course/result sequence.

## Validation

v2441 passed the complete offline suite covering:

- CPU/Vulkan pixel comparisons and static cache/bulk submission;
- strips, fans, flat shading, translucency, clipping, projection, lighting,
  environment mapping, textures, and depth;
- controller discovery/bindings/smoothing, custom music, cards, and
  high-refresh interpolation;
- Direct-session marker lifecycle, cooperative shutdown policies, guest
  timing, idle waits, link freshness, and translation-source integrity;
- a new BGR24/direct-BGRA32 presenter contract whose decoded channels are
  pixel-identical to ordinary RGB output; and
- a standalone audit with zero firmware callbacks, firmware AOT objects,
  firmware input contracts, or firmware translations in product caches.

Executable SHA-256:
`78FFA018D058744A5AA5AAF5F5580C09F313CB8F5725A744AD0BFBCD6E30C6DD`

The executable is intentionally not published from this integration
checkpoint. No game data, firmware, PIC/CHD input, extracted asset, card save,
custom music, capture, log, personal path, or generated game translation unit
is included in this repository update.

## Current boundary

The public v2415 package remains the cautious playtest download at 60 FPS.
v2441 improves a measured CPU preparation path, but 120 Hz is still
experimental, uncapped presentation is unfinished, and exhaustive route,
controller/wheel, audio, card-error, clean-machine, and cross-driver acceptance
remains open.
