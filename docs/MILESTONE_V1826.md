# v1826 projected-renderer, texture-parity, and input milestone

Date: 2026-08-18

## Outcome

The static-AOT Naomi 2 build now turns the reached game's command-port submissions into coherent projected scenes through the native ELAN path. The earlier duplicated/two-cars-in-one output is fixed, the current frame no longer relies on fit-to-view geometry, and the optional native window displays changing attract/course frames.

This remains a work in progress. A controllable race, complete TA/PVR presentation, and audible AICA output are not yet demonstrated.

## Store-queue correction

The translated game uses the SH-4's two 32-byte store queues to submit geometry. One recovered function contained a stale no-op where its `PREF` instruction should have flushed a queue. Restoring that operation removed the visibly duplicated car geometry.

The audit then restored all 100 suppressed/no-op `PREF` sites across 14 geometry/ELAN functions. The private generated translation now contains 531 active `host_pref` calls. The generated translation unit is derived from game code and is not included in this repository.

## Flycast ELAN parity

The stripped native decoder was checked against Flycast v2.6 rather than visually guessed:

- The exact saved ELAN snapshot decodes identically.
- A long active trace matches the first 64,439 semantic append events exactly.
- Word 7 still triggers immediate execution, ELAN RAM remains masked to 32 MiB, and link/model address masking follows the pinned Flycast implementation.
- No Flycast SH-4 interpreter, dynarec, renderer backend, scheduler, UI, or save-state system is linked into the product.

## Projected frame evidence

A recent optimized native frame reported:

- frame 2,615
- 751 accepted batches
- 63,356 vertices
- 12,393 triangles
- zero projection rejection
- zero near/far-cull rejection
- 10 rejected submissions that cannot form valid triangles
- all 307,200 output pixels attributed to submitted game batches

The accepted output is a coherent single-car scene. Dark forest, sky, car, and course regions are owned by actual batches rather than holes filled with synthetic geometry.

## Texture evidence

The native run observed 183 texture states. Comparison with four Flycast reference traces found 160 exact payload/size/format/nonzero matches. Only two remain at the same VRAM address because allocation addresses move between phases; payload hashes provide the stable comparison.

The diagnostic atlas contains 172 unique decoded bindings. Of those, 149 exact-matched payloads account for 671,984 of 773,632 textured raster operations (86.9%). Only eight unmatched bindings touch visible pixels. The two dominant unmatched payloads decode coherently as a night forest/sky atlas and a dark environment/reflection map, so they remain intact rather than being discarded as corruption.

No extracted texture, VRAM dump, game asset, screenshot from this checkpoint, ROM, BIOS, PIC/CHD, or memory capture is included here.

## Optimization and live presentation

The software validation renderer now:

- resolves bilinear X/Y sample indices once per sample;
- directly copies the common PVR ONE/ZERO blend case;
- uses exact positive bounded rounding in the color hot path;
- skips vertex-alpha interpolation when alpha is disabled;
- skips offset-color work when its material/fog modes do not use it.

A representative comparison reduced total frame time from 180.786 ms to 150.522 ms and raster time from 167.640 ms to 139.445 ms. The triangle counts differed slightly between runs, so the normalized raster improvement (about 14%) is the safer performance comparison.

The optional Windows preview produced 282 raster frames and 2,384 cabinet-input polls in 45 seconds. Because one software frame can take roughly 150 ms, initial Windows key-down messages are now latched and consumed once for coin, start, service, and related controls. Automated UI injection could not focus/deliver a key to the custom window, so a real interactive validation remains an explicit acceptance gate.

## Audio boundary

The native AICA mailbox and Flycast-derived SGC regression tests pass, and the reached state initializes AICA RAM descriptors. It has not yet emitted the authentic stream-start command. A host audio callback would therefore output silence today; the next audio task is reaching and implementing that command path, not adding a cosmetic silent sink.

## Regression gates

The current private optimized executable passes:

1. AICA mailbox test
2. Flycast-derived AICA SGC test
3. system-ROM read-only test
4. native ELAN bridge test
5. JVS test

## Next acceptance gate

Use a real focused native window to prove that coin/start/control transitions reach translated game state and advance into a controllable menu/race flow. In parallel, complete remaining TA/PVR layers and trace the first real AICA stream-start command.
