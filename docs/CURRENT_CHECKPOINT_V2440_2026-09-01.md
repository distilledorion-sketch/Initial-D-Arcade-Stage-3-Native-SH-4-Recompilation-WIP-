# Current checkpoint — v2440 GPU presentation

Date: 2026-09-01

This is an integration checkpoint. The latest downloadable public early demo
remains v2415 while v2440 receives broader route and hardware testing.

## What changed

The remaining high-volume renderer staging work was reduced without changing
the original game's simulation cadence:

- PVR screen-depth normalization moved to the Vulkan vertex shader.
- Vertex and index output now writes directly into alternating bounded regions
  of persistent mapped Vulkan buffers.
- ELAN projection and lighting records now write directly into alternating
  aligned regions of the existing mapped uniform buffer.
- Exact adjacent object-state reuse remains enabled, and every stream rejects
  overflow rather than aliasing an in-flight region.
- Compatible BGRA8 fallback readback uses one bulk copy into a Win32 32-bit
  DIB instead of repacking every pixel into BGR24.
- Direct Vulkan presentation bypasses the GDI display copy. An unclean-session
  marker forces a later launch back to Safe presentation if Direct presentation
  does not finish normal shutdown.

Gameplay, physics, racing AI, timers, input, and audio remain CPU-side at the
authentic guest cadence. The work above changes graphics preparation and
display delivery, not game behavior.

## Product identity

- Private integration executable SHA-256:
  `7FB61B45A0137F8FCE4A9A6CD36200B212F7DFDC6717D851739AD9CBB4D798BC`
- The binary is intentionally not committed to Git.
- Standalone audit: zero firmware callbacks, firmware objects, firmware input
  contracts, or cached firmware translations.

## Offline verification

The v2440 build passed:

- CPU/Vulkan pixel comparisons;
- primitive restart, strips, fans, flat fallback, and ordered translucency;
- object projection, ELAN lighting, environment mapping, and homogeneous near
  clipping;
- the 64-batch / 16,384-static-vertex reuse and bulk path;
- controller binding/capture and steering-smoothing checks;
- exact custom-music decode and stream selection;
- 120 Hz interpolation checks;
- card persistence and eject classification;
- the real Direct-session marker lifecycle test;
- 31 restart/shutdown/presentation policies;
- 13 controller/music policies;
- exact guest delay/assertion timing and idle-wait policy;
- link freshness for 116 product objects with zero stale direct sources; and
- the BIOS-free standalone audit.

## Live Direct Vulkan evidence

A visible 2560x1080 run on an RX 9070 XT used a three-image FIFO Direct Vulkan
swapchain and crossed a heavy course into the result/continue sequence.

- Presentation stayed between 119.7 and 120.3 FPS during the accepted course
  samples.
- Authentic course frames contained roughly 75,000–127,000 vertices.
- During moving content, intermediate-frame generation advanced at the required
  cadence and the repeated-frame counter stayed effectively flat.
- Sampled CPU demand ranged from about 0.2 host cores on light screens to about
  1.2 cores in the heavy course scene.
- After asset loading, working memory stabilized near 1.34 GiB. A 25-second
  steady sample changed by +0.5 MiB working set, -1 MiB private memory, and zero
  handles.
- Opening F1 intentionally paused guest execution; normal 120 Hz course output
  resumed immediately when the menu closed.
- No Vulkan fault, device loss, fatal error, process hang, or host failure was
  recorded.
- The process exited with code 0. Raster helpers, audio, DirectInput, the raster
  worker, and the window all completed normal shutdown.
- The Direct-session marker was removed on exit.

## Current boundary

This successful run materially strengthens the Direct Vulkan and shutdown
evidence, but it is not whole-game or cross-driver proof. Remaining work is to
profile the residual CPU command-preparation path, extend visual/interpolation
checks across content, repeat live close/restart testing on more hardware, and
complete audio, input, card, route, and clean-machine acceptance.

No game image, BIOS, PIC, CHD, extracted asset, card, music, capture, log,
translated guest body, executable, object file, or personal filesystem path is
included in this checkpoint.
