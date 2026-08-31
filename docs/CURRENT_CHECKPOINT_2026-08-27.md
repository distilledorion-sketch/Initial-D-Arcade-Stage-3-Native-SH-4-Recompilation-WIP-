# Current source-safe checkpoint — 2026-08-27

This document reports the private v2217 integration checkpoint without
publishing a game binary, generated game translation, ROM/BIOS/PIC/CHD data,
assets, captures, logs, input recordings, save/card data, memory snapshots, or
private host paths.

## Project boundary

- Target hardware: Sega NAOMI 2.
- Execution model: native static ahead-of-time SH-4 recompilation.
- Graphics API: Vulkan.
- No interpreter, JIT, emulator runtime, fabricated guest state, or replacement game assets.
- Flycast-derived device knowledge/code may be integrated where licensing permits; Flycast is not the runtime.
- Dreamcast is outside the project scope.

## v2217 presentation fix

The v2216 renderer was already under the 16.67 ms 60 FPS budget, but its clean
4K acceptance run contained two 29.3/51.2 FPS samples before car motion. Those
samples occurred while the guest synchronously loaded course and music data.

The presenter already maintained an independent wall-clock cadence above 60
Hz by retaining the last completed scene when the guest had not submitted a
new one. At exactly 60 Hz it waited for a new guest scene instead. v2217 applies
the same fixed cadence at 60 Hz. Re-presenting a completed image does not call
or advance guest functions, timers, input, audio sequencing, or physics.

## Build identity and checks

Checkpoint executable SHA-256 (binary not distributed):

`0F225655BF9F32C1FAEE76CF559EABD1D7D5C19A4960BD7A02B512582A4A08E6`

The link-freshness verifier accepted all 116 objects. The native ELAN/Vulkan,
render completion/DMA, card, AICA, and 837-13551 JVS/shifter tests passed.

## Performance evidence

At 3840×2160, 43 samples from race takeover onward measured:

- minimum: 59.8 FPS
- average: 60.000 FPS
- maximum: 60.1 FPS

The same synchronous sound-pack loads occurred without the earlier cadence
collapse. Normal JVS/physics input drove the player 668.687 metres. The real
DIMM path selected the tested course stream, and ARM7/AICA produced 7,942,187
samples, including 6,719,814 non-silent samples. Automated QA stayed muted.

At 1920×1080, ten operational takeover/race samples measured 119.8–120.1 FPS
through 190.065 metres of motion. One 117.9 sample occurred later during the
intentional watchdog shutdown dump while hundreds of diagnostic lines were
being emitted; it is recorded as diagnostic overhead rather than hidden or
misreported as gameplay performance.

Both acceptance logs contain zero fatal, exception, access-violation,
unimplemented-target, NSEQ, or Vulkan fault markers.

## Remaining work

1. Validate every mode, course, car, opponent, condition, outcome, continue/result path, and card-error branch.
2. Reduce the underlying synchronous asset/music transition latency while preserving guest-visible ordering.
3. Complete hardware acceptance for wheels, force feedback, audio devices, and controller remapping.
4. Finish clean-machine preparation, crash reporting, settings polish, and legally safe release packaging.
5. Protect 120 FPS across broader content, then pursue unlimited presentation without changing gameplay, timers, physics, or audio.

This is substantial progress, not a completion claim. The public repository
remains a source-safe engineering showcase rather than a playable game release.
