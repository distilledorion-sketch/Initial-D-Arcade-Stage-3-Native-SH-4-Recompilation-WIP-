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

Conservative CPU-only cold-boot probes also passed two real route families:

- Myogi left/dry/day Time Attack loaded `k_ez`, the forward condition mesh,
  day environment, and no rain; normal guest input/physics advanced 9.548 m.
- Shomaru dry/night Bunta loaded segmented `n_sy2` course geometry, the forward
  condition mesh, night environment, no rain, and the Bunta rival package;
  normal guest input/physics advanced 5.097 m.
- Usui right/wet/night Time Attack loaded `s_nm2`, the reverse condition mesh,
  all three right-direction guest flags, night environment, and rain; normal
  guest input/physics advanced 9.008 m.
- Tsuchisaka dry/night Bunta loaded `k_tu2`, its forward condition mesh,
  night/no-rain environment, and the Bunta rival package; normal guest
  input/physics advanced 6.302 m.

Each probe ran Below Normal with Vulkan disabled and one offscreen preview
frame per second, then used cooperative close after movement and exited 0. The
route-evidence parser was corrected to recognize Shomaru's authentic segmented
`*_pol_a/b/c.tbl` geometry without treating its separate condition mesh as the
primary course. The runner also now derives Left=0/Right=1 from the explicit
label when a retained manifest predates the generator's numeric direction
column.

Executable SHA-256:
`78FFA018D058744A5AA5AAF5F5580C09F313CB8F5725A744AD0BFBCD6E30C6DD`

The executable is intentionally not published from this integration
checkpoint. No game data, firmware, PIC/CHD input, extracted asset, card save,
custom music, capture, log, personal path, or generated game translation unit
is included in this repository update.

## Direct 120 Hz acceptance update

Four fresh current-build Direct-Vulkan processes covered Myogi, Akina, and
Akagi at 640x480 plus Akagi at the requested 2560x1080 ultrawide target. Each
used authentic 60 Hz guest/gameplay timing with distinct generated
presentation phases, one raster thread, Below Normal process priority, and a
cooperative window close.

| Route | Target | Complete steady samples | Display min / avg | Vulkan min / avg | Generated min | Moving repeats | Faults |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Myogi left/dry/day | 640x480 | 23 | 120.0 / 120.017 | 120.0 / 120.0 | 120 | 0 | 0 |
| Akina route | 640x480 | 34 | 119.0 / 119.971 | 119.0 / 119.967 | 119 | 0 | 0 |
| Akagi left/dry/day | 640x480 | 35 | 119.9 / 120.011 | 119.9 / 120.0 | 120 | 0 | 0 |
| Akagi left/dry/day | 2560x1080 | 35 | 120.0 / 120.0 | 120.0 / 120.0 | 120 | 0 | 0 |

The 2560x1080 run used anisotropic 16x filtering. Its midpoint-plus-authentic
worker pair averaged about 16.665 ms. Every process exited with code 0, removed
its direct-session marker, and left no game process behind. The analyzer used
for this evidence excludes deliberate F1 pauses, the race-loading ramp, TIME
UP, and result/menu transitions from moving-race statistics; a deterministic
10-assertion contract protects those boundaries.

## Current boundary

The public v2415 package remains the cautious playtest download at 60 FPS.
v2441 now has fresh targeted 120 Hz Direct-Vulkan acceptance at native and
ultrawide targets in addition to its complete offline suite. Broader
whole-game, physical-hardware, clean-machine, and cross-driver acceptance is
still required before replacing the public checkpoint.
