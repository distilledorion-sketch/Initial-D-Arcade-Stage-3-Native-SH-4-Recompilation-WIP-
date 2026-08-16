# Status and evidence ledger

Status date: 2026-08-15
Public checkpoint: v1335 WIP

This file separates demonstrated behavior from hypotheses. Percentages are deliberately avoided: a technically advanced attract-mode path is not the same thing as a playable game.

| Area | Demonstrated | Current limitation |
|---|---|---|
| Static SH-4 translation | Broad instruction decoding/code generation; direct translated calls; FPSCR-aware FPU regression tests | Whole-program coverage and every indirect target are not complete |
| Deterministic replay | N70 accepted 70/70 with `unimplemented=0`, `FPSCR=0`, no fault/watchdog, and cross-machine identical frame hash | Uses private user-owned replay/input state that cannot be published |
| ELAN 3D path | Persistent state, recursive links, materials, instances, lighting, textures, culling, depth, and native diagnostic rendering | New submitted-stream lifecycle does not yet complete end to end |
| Intro visuals | Roughly 53 seconds of ordered 3D cinematography through slice 3200; AE86, FD3S, red rival, shot cuts and chase | 2D title/logo layers absent; texture/material polish remains |
| TA/PVR 2D | Store-queue traffic and submitted command history have been observed and mapped | Native list execution/compositing is dormant |
| ELAN completion | Five observed list-complete `RegisterWait` masks classified; both CLX status banks and TA-ITP update behavior covered in the private integration test | Embedded waits inside the submitted `Link` stream are not reaching the handler yet |
| JVS/input | Clean-room 837-13551 / 315-6149-facing model exists | Complete cabinet controls and full gameplay validation remain |
| Audio | Narrow opt-in AICA driver-ready bootstrap seam | No complete native AICA/ARM audio path |
| Distribution | Sanitized source/evidence package with reproducible public tests | No executable game package; users must supply legally owned inputs to a future private build |

## Strongest accepted evidence

- N70 frame SHA-256: `34D6B91C0550A5CC2A60D4B1F8930812908CEA6DCD6C354749A0881BE426D9E2`.
- Accepted N70 state: 667 batches, 61,498 vertices, 17,490 triangles, 990,196 textured pixels, and 53,996 lit vertices.
- Full-intro ladder reached N3200 with each recorded run accepted N/N, `unimplemented=0`, `FPSCR=0`, and no recorded memory fault, crash, or watchdog.
- Current public classifier smoke test covers recursive links, SH-4/ERAM texture DMA links, all five known list-complete wait bits, empty/unknown-mask rejection, and false command-header aliases.

## Latest WIP truth

v1335 broadened direct `RegisterWait` handling for masks `0x80`, `0x100`, `0x200`, `0x400`, and `0x200000`. The focused private Windows integration test passed. An exact fast WSL rebuild was stopped cleanly at the user's request, so the full gate-off and stage-112 acceptance runs have not been completed.

The preceding v1334 trace reported `register_waits=0`. That means the change must not be described as fixing the lifecycle blocker. The best current explanation is that the next frame arrives as a linked command stream and its embedded waits are never walked to the existing handler.

## Definition of playable

This project will not call itself playable until a user can cold boot from legally owned inputs, reach menus, start and control a race, receive working audio/video/input, complete meaningful gameplay, and do so through the static native path without an interpreter/JIT fallback.
