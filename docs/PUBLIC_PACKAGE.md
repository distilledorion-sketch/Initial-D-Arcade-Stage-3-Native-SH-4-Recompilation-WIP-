# Public package boundary

## Included

- General SH-4 decoding and code-generation source.
- Selected original and properly attributed Flycast-derived native runtime snapshots.
- Tests that run without game content.
- Original project documentation, hashes, diagrams, and a limited set of progress captures.
- Required license and attribution text for the Flycast-derived ELAN boundary.

## Intentionally excluded

- Game images, ROMs, BIOS/firmware dumps, PIC data, GD-ROM/CHD files, or disk images.
- HostFS/game assets, extracted textures, music, voice, movies, or reference video.
- RAM/context/asset/ELAN/VRAM snapshots and replay payloads.
- Generated translation units derived from the game binary.
- Executables, object files, compiler caches, private absolute paths, credentials, and proprietary SDK material.

This means the public package documents and tests the engineering work but cannot boot the game by itself. That limitation is intentional.
