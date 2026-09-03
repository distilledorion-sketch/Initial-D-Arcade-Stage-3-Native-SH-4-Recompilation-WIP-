# Public package boundary

## Included

- General SH-4 decoding and code-generation source.
- Selected clean-room native runtime snapshots.
- Tests that run without game content, including Windows-only product-shared
  XInput discovery and silent audio-endpoint probes.
- Original project documentation, hashes, diagrams, and a limited set of progress captures.
- The source-safe Windows launcher and public default settings, including the
  tested Direct/Safe Vulkan presentation mapping.
- The source-safe GitHub update checker and external installer. They select
  only a newer versioned release asset, require GitHub's SHA-256 digest,
  validate the package version and extraction paths, preserve user-owned data,
  and roll back product-file changes after an install failure.

## Intentionally excluded

- Game images, ROMs, BIOS/firmware dumps, PIC data, GD-ROM/CHD files, or disk images.
- HostFS/game assets, extracted textures, music, voice, movies, or reference video.
- RAM/context/asset/ELAN/VRAM snapshots and replay payloads.
- Generated translation units derived from the game binary.
- Executables, object files, compiler caches, private absolute paths, credentials, and proprietary SDK material.

The Git source package documents and tests the engineering work but cannot boot
the game by itself. The separate Windows prerelease includes the native runtime
and local setup tools, but still contains no user-owned game inputs. That
boundary is intentional. The v2489 public ZIP retains `PRODUCT_VERSION.txt`
and the two updater helpers. It no longer needs the byte-identical legacy
`demo.exe` compatibility copy used during the v2457 naming transition.
Update packages
preserve `game files`, `card data`, `custom music`, `logs`, the user settings
INI, the automatic-update preference, and unrelated destination-only files.
