# v1400 native boot and course-scene milestone

Date: 2026-08-16

## Outcome

The static-AOT build now follows the validated native boot path through the frontend transition and into an active course/attract component. The current blocker is graphics state association, not a stopped game loop.

## Root cause fixed privately

The credit frontend object is constructed twice. Four credit-screen files were absent from the external HOSTFS overlay, so the game counted zero records and requested a zero-byte allocation. The first construction happened to receive a minimum allocation while the second received null, leaving a renderer argument rooted at zero and causing store-queue traffic to overwrite the main heap.

The four files were extracted from the user's legally owned disc image into the external private HOSTFS. Both constructions then counted 25 records and requested 2,000 bytes. The renderer stopped writing through the heap root, and the course/path allocation succeeded.

No extracted file, game executable, disc image, BIOS, PIC, memory snapshot, or HOSTFS overlay is included in this repository.

## Accepted observations

- Native ELAN summary: 1,566 observed, 1,566 handled, zero rejected, zero stream-walk failures.
- Credit record allocation: 25 records, 2,000 bytes, valid result on both constructions.
- Timed frontend transition: completion callback and tail calls returned normally.
- Course/attract component: 745 completed sampled calls over 20 seconds; active state remained coherent.
- Post-fix SQ capture: 41,515 vertices, 29,389 triangles, 208 textured batches, 44 decoded textures, zero rejected batches.
- Full ELAN scene capture: 620 accepted batches, 21 rejected batches, 48,686 vertices, 33,810 triangles.

## Why the latest image is not a gameplay frame

The v1400 full-scene renderer reported `projection=0` and zero projected batches. It therefore rasterizes substantial real scene geometry without the game's intended camera transform. The resulting faceted/dome-like image is diagnostic evidence, not a claim of correct presentation.

## Next acceptance gate

Trace how projection and model-view state attach to this active scene and require nonzero projected batches from the same native boot. No fit-to-view camera, interpreter/JIT fallback, or synthetic success path is acceptable.
