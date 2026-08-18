# Flycast ELAN attribution

The native NAOMI 2 ELAN command traversal in `src/runtime/native_elan_bridge.h` is adapted from Flycast v2.6:

- Upstream: <https://github.com/flyinghead/flycast>
- Version: `v2.6`
- Pinned commit: `392a429e8b040b3e5bf6696cb4f984274fc44123`
- Adapted files: `core/hw/pvr/elan.cpp`, `core/hw/pvr/elan_struct.h`, and TA record-sizing rules in `core/hw/pvr/ta.cpp`
- Copyright: 2022 flyinghead and Flycast contributors
- License: GNU General Public License version 2 or later; see [the included license text](../LICENSES/GPL-2.0-or-later.txt)

## What was retained

Only the ELAN-facing command grammar and behavior needed by the static recompiler were retained: command classification and sizing, `Link` and `Model` recursion, raw TA record alignment, `RegisterWait` signaling, texture DMA, and the ELAN completion/error contract.

## What was not imported

The product does not link Flycast's SH-4 interpreter or dynarec, address-space dispatcher, renderer backends, scheduler, networking, rollback, UI, or save-state system. The game's SH-4 instructions remain ahead-of-time translated to native code with no interpreter/JIT fallback.

The pinned upstream checkout used for comparison remains in the private working tree and is intentionally not duplicated in this public repository.

The Flycast-derived ELAN implementation and any combined/modified form of it are distributed under GPL-2.0-or-later. Original project files outside that derived boundary remain under the terms stated in [LEGAL.md](../LEGAL.md).
