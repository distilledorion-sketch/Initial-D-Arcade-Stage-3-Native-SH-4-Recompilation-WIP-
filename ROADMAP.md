# Roadmap

Work is ordered by the first unresolved native hardware boundary. Each milestone has a concrete acceptance gate.

## Completed native-path milestones

- Bounded ELAN `Link`/`Model` stream walking with malformed/cyclic streams rejected fail-closed.
- Known `RegisterWait` and texture-DMA events delivered at their real command boundaries.
- Accepted native boot with 1,566/1,566 ELAN submissions handled and zero walk failures.
- Private credit-asset allocation defect traced and corrected without redistributing the assets.
- Frontend timed transition completed and the course/attract component observed running continuously.

## P0 — Associate camera/projection state with active scene draws

- Trace projection, model-view, and instance state through resident links and scene selection.
- Determine whether course draws inherit state from a prior submission or use a currently unhandled record shape.
- Keep unmatched or malformed state visible as a diagnostic rejection.

Acceptance: the same v1400 native scene records nonzero projected batches and produces a stable camera-space frame without fit-to-view substitution.

## P0 — Execute and composite TA/PVR layers

- Execute opaque, punch-through, translucent, modifier-volume, and sprite lists.
- Composite TA/PVR output with ELAN 3D at the authentic frame boundary.
- Preserve list ordering and interrupt timing.

Acceptance: title/logo cards, HUD/cabinet overlays, and the projected 3D scene appear from guest-submitted data with no fabricated textures.

## P1 — Restore faithful materials and presentation

- Resolve the 21 currently rejected full-scene batches.
- Validate depth, culling, blend, transparency, palette, mip, and filtering behavior.
- Trace missing uploads only from legally owned private inputs.

Acceptance: repeatable comparisons show the native renderer producing the expected layers rather than a diagnostic substitution.

## P1 — Complete audio and controls

- Replace the AICA ready-only bootstrap seam with the required native audio path.
- Validate JVS steering, accelerator, brake, buttons, coin/service, and outputs.

Acceptance: controllable attract/menu/gameplay flow with synchronized audio and no input-script dependency.

## P2 — Whole-game static-AOT coverage

- Translate every reachable function and indirect table coherently.
- Eliminate unresolved targets and compatibility-only shims.
- Add race, course, results, continue, save/configuration, and long-run determinism suites.

Acceptance: cold boot through a complete race and return flow entirely through the native static path.

## P2 — Productization

- Reproducible builds that accept only user-supplied legally owned inputs.
- Clear extraction boundary, configuration UI, controller mapping, crash diagnostics, and release packaging.
- Performance profiling and native hot-path optimization after correctness gates pass.

Acceptance: clean-machine documentation and packaging that never redistributes third-party game content.
