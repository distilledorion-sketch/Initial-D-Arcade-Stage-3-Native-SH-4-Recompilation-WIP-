# Public package boundary

## Included

- General SH-4 decoding and code-generation source.
- Selected clean-room native runtime snapshots.
- Tests that run without game content, including Windows-only product-shared
  XInput discovery and silent audio-endpoint probes.
- Original project documentation, hashes, diagrams, and a limited set of progress captures.
- The source-safe Windows launcher and public default settings, including the
  tested Direct/Safe Vulkan presentation mapping.

## Intentionally excluded

- Game images, ROMs, BIOS/firmware dumps, PIC data, GD-ROM/CHD files, or disk images.
- HostFS/game assets, extracted textures, music, voice, movies, or reference video.
- RAM/context/asset/ELAN/VRAM snapshots and replay payloads.
- Generated translation units derived from the game binary.
- Executables, object files, compiler caches, private absolute paths, credentials, and proprietary SDK material.

The Git source package documents and tests the engineering work but cannot boot
the game by itself. The separate Windows prerelease includes the native runtime
and local setup tools, but still contains no user-owned game inputs. That
boundary is intentional.
