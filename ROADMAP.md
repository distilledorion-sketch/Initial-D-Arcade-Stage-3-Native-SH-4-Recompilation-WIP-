# Roadmap

Work is ordered by the first unresolved hardware boundary. Each milestone has a concrete acceptance gate.

## P0 — Execute the submitted ELAN link stream

- Add a bounded walk at the existing native ELAN command-port seam.
- Follow only validated link shapes and ranges.
- Route embedded `RegisterWait` records through the existing classifier.
- Reject unknown/empty wait masks and malformed/cyclic links fail-closed.

Acceptance: the exact v1334 configuration records nonzero waits and no new unimplemented commands, memory faults, crashes, or watchdogs.

## P0 — Close the frame-lifecycle handshake

- Prove native CH2-to-ELAN DMA kicks.
- Observe request slot 0 transition from `2` to `0`.
- Advance the producer beyond frame 1.
- Preserve both CLX normal-status banks and advance `TA_ITP_CURRENT` per active wait.

Acceptance: repeated exact runs pass N/N with `unimplemented=0`, `FPSCR=0`, stable provenance, and the accepted N70 frame hash when compatibility gates are off.

## P1 — Activate TA/PVR list execution and compositing

- Execute captured opaque, punch-through, translucent, modifier-volume, and sprite lists.
- Composite TA/PVR 2D output over ELAN 3D at the real frame boundary.
- Preserve list ordering and interrupt timing.

Acceptance: title/logo cards and cabinet overlays appear from guest-submitted data, with no fabricated textures or manual replacements.

## P1 — Restore faithful textures and materials

- Trace missing game-driven uploads from legally owned private inputs.
- Validate sky, road, alpha, palette, mip, and blend cases.
- Continue reference comparisons without embedding reference video or extracted assets.

Acceptance: repeatable frame diffs show the renderer—not a diagnostic substitution—producing the expected layers.

## P1 — Complete audio and controls

- Replace the AICA ready-only bootstrap seam with the required native audio path.
- Validate JVS steering, accelerator, brake, buttons, coin/service, and outputs.

Acceptance: controllable attract/menu/gameplay flow with synchronized audio and no input-script dependency.

## P2 — Whole-game static-AOT coverage

- Translate every reachable function/table coherently.
- Eliminate unresolved indirect targets and compatibility shims.
- Add race, course, results, continue, save/configuration, and long-run determinism suites.

Acceptance: cold boot through a complete race and return flow, entirely through the native static path.

## P2 — Productization

- Reproducible build orchestration that accepts only user-supplied legally owned inputs.
- Clear asset extraction boundary, configuration UI, controller mapping, crash diagnostics, and release packaging.
- Performance profiling and native hot-path optimization after correctness gates pass.

Acceptance: clean-machine documentation and packaging that never redistributes third-party game content.
